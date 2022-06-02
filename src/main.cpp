/*
 Name:		M17 Analog Hotspot Gateway
 Created:	1-Nov-2021 14:27:23
 Author:	HS5TQA/Atten
 Reflector: https://m17.dprns.com
*/

#include <Arduino.h>
#include <driver/dac.h>

#include <WiFi.h>

#include <Preferences.h>
#include <ctype.h>			   // for isNumber check
#include <ButterworthFilter.h> //In the codec2 folder in the library folder

#include "main.h"
#include "m17.h"

#define EEPROM_SIZE 512

#define SPEAKER_PIN 26
#define MIC_PIN 36
#define PTT_PIN 32

#define ADC_BUFFER_SIZE 320 // 40ms of voice in 8KHz sampling frequency

M17Flags connect_status;
extern uint16_t txstreamid;
extern uint16_t tx_cnt;
extern unsigned short streamid;
extern unsigned short frameid;
extern unsigned long RxTimeout;
extern unsigned long m17ConectTimeout;

int16_t adc_buffer[ADC_BUFFER_SIZE];

// Highpass filter >400Hz  HPF Butterworth Filter. 0-600Hz
ButterworthFilter hp_filter(400, 8000, ButterworthFilter::ButterworthFilter::Highpass, 2);
// Lowpass filter <3KHz  LPF Butterworth Filter.
ButterworthFilter lp_filter(3000, 8000, ButterworthFilter::ButterworthFilter::Lowpass, 2);

CODEC2 *codec2_3200;
CODEC2 *codec2_1600;

// Current Codec2 Mode
int mode = CODEC2_MODE_3200;

// Queue<char> audioq(300);
cppQueue audioq(sizeof(uint8_t), 800, IMPLEMENTATION); // Instantiate queue
cppQueue pcmq(sizeof(int16_t), 8000, IMPLEMENTATION);  // Instantiate queue
cppQueue adcq(sizeof(int16_t), 8000, IMPLEMENTATION);  // Instantiate queue

unsigned char c2[1000];
int c2_offset = 0;
int nsam, nbit, nbyte, i;

String srcCall, mycall, urCall, rptr1;

uint8_t sndW;
int16_t adcR;
int ppm_Level = 0;
int ppm_temp = 0;

bool rxRef = false;
bool tx = false;
bool firstTX = false;
bool firstRX = false;

Configuration config;

unsigned long pingTimeout;

// TaskHandle_t taskSensorHandle;
TaskHandle_t taskNetworkHandle;
TaskHandle_t taskDSPHandle;
TaskHandle_t taskUIHandle;

uint8_t checkSum(uint8_t *ptr, uint16_t count)
{
	uint8_t lrc, tmp;
	uint16_t i;
	lrc = 0;
	for (i = 0; i < count; i++)
	{
		tmp = ptr[i];
		lrc = lrc ^ tmp;
	}
	return lrc;
}

void saveEEPROM()
{
	uint8_t chkSum = 0;
	byte *ptr;
	ptr = (byte *)&config;
	EEPROM.writeBytes(1, ptr, sizeof(Configuration));
	chkSum = checkSum(ptr, sizeof(Configuration));
	EEPROM.write(0, chkSum);
	EEPROM.commit();
}

// Setup default save state.
void defaultConfig()
{
	Serial.println(F("Default configure loaded!"));
	sprintf(config.wifi_ssid, "ssid");
	sprintf(config.wifi_pass, "pass");
	sprintf(config.current_reflector.host, "0.0.0.0");
	sprintf(config.current_reflector.name, "M17-M17");
	config.current_reflector.port = 17000;
	config.current_module = 'C';
	sprintf(config.mycall, "N0CALL");
	config.mymodule = 'M';
	config.codec2_mode = CODEC2_MODE_3200;
	saveEEPROM();
}

void taskDSP(void *pvParameters);
void taskNetwork(void *pvParameters);
void taskUI(void *pvParameters);

// Bandpass Filter
void bp_filter(float *h, int n)
{
	int i = 0;
	for (i = 0; i < n; i++)
	{
		h[i] = lp_filter.Update(h[i]);
	}
	for (i = 0; i < n; i++)
	{
		h[i] = hp_filter.Update(h[i]);
	}
}

// Encode and decode audio Codec2
void process_audio()
{
	uint8_t c2Buf[8];
	short audioBuf[320];
	float audiof[320];
	int16_t adc;

	int pcmWidth = 160;

	if (mode == CODEC2_MODE_1600)
	{
		pcmWidth = 320;
	}
	else
	{
		pcmWidth = 160;
	}
	if (tx)
	{
		if (adcq.getCount() >= pcmWidth)
		{
			for (int x = 0; x < pcmWidth; x++)
			{
				if (!adcq.pop(&adc))
				{
					break;
				}
				audiof[x] = (float)adc;
				ppm_temp = (int)abs((int16_t)audiof[x]);
				if (ppm_temp > ppm_Level)
				{
					ppm_Level = ((ppm_Level * 255) + ppm_temp) >> 8;
				}
				else
				{
					ppm_Level = (ppm_Level * 16383) >> 14;
				}
				for (int i = 0; i < pcmWidth; i++)
				{
					audioBuf[i] = (int16_t)(audiof[i] * ((float)0.1));
				}
			}
			bp_filter(audiof, pcmWidth);
			if (mode == CODEC2_MODE_1600)
			{
				codec2_encode(codec2_1600, c2Buf, audioBuf);
			}
			else
			{
				codec2_encode(codec2_3200, c2Buf, audioBuf);
			}
			for (int x = 0; x < 8; x++)
			{
				audioq.push(&c2Buf[x]);
			}
		}
	}
	else
	{
		int packetSize = audioq.getCount();
		if (packetSize >= 8)
		{
			for (int x = 0; x < 8; x++)
			{
				if (!audioq.pop(&c2Buf[x]))
					break;
			}
			if (mode == CODEC2_MODE_1600)
			{
				codec2_decode(codec2_1600, audioBuf, c2Buf);
			}
			else
			{
				codec2_decode(codec2_3200, audioBuf, c2Buf);
			}
			for (int x = 0; x < pcmWidth; x++)
			{
				pcmq.push(&audioBuf[x]);
			}
		}
	}
}

hw_timer_t *timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

// Offset info
int offset = 29760, offset_new = 0, adc_count = 0;

// Read ADC for Mic and Write DAC for speaker
void IRAM_ATTR onTime()
{
	portENTER_CRITICAL_ISR(&timerMux);
	int sample;
	if (tx)
	{
		// Detect ADC overflow
		sample = (int)analogRead(MIC_PIN);
		sample = sample & 4095;					  // Throw away top four bits of 16 bit to make sure only what we care about comes thru. Since ESP32 can sometimes put stuff there randomly
		adcR = (int16_t)((sample << 4) - offset); // 12bit -> 16bit,convert to sign,29760
		adcq.push(&adcR);
	}
	else
	{
		// Receives sound from the M17 server out of the speakers.
		if (pcmq.getCount() > 0)
		{
			pcmq.pop(&adcR);
			sndW = (uint8_t)map(((int)adcR + 32768), 0, 65535, 0, 255); // Convert from 16-bit to the 8-bit sound that this allows.
			dacWrite(SPEAKER_PIN, sndW);
		}
	}
	portEXIT_CRITICAL_ISR(&timerMux);
}

void setup()
{

	int mode;
	byte *ptr;
	pinMode(PTT_PIN, INPUT_PULLUP);

	analogReadResolution(12);					// Sets the sample bits and read resolution, default is 12-bit (0 - 4095), range is 9 - 12 bits
	analogSetWidth(12);							// Sets the sample bits and read resolution, default is 12-bit (0 - 4095), range is 9 - 12 bits
	analogSetPinAttenuation(MIC_PIN, ADC_11db); // Pressure 0-3.3V

	enableLoopWDT();
	enableCore0WDT();
	enableCore1WDT();

	Serial.begin(115200);

	if (!EEPROM.begin(EEPROM_SIZE))
	{
		Serial.println(F("failed to initialise EEPROM"));
		;
	}

	delay(100);

	// Load Comfiguration info from EEPROM of ESP32
	ptr = (byte *)&config;
	EEPROM.readBytes(1, ptr, sizeof(Configuration));
	uint8_t chkSum = checkSum(ptr, sizeof(Configuration));
	if (EEPROM.read(0) != chkSum)
	{
		Serial.println(F("Config EEPROM Error!"));
		defaultConfig();
	}
	else
	{

		ptr = (byte *)&config;
		Serial.println(F("Load configure!"));
		EEPROM.readBytes(1, ptr, sizeof(Configuration));
	}

	connect_status = DISCONNECTED;

	// Start a timer at 8kHz to sample the ADC and play the audio on the DAC.
	timer = timerBegin(3, 500, true);			// 80 MHz / 500 = 160KHz MHz hardware clock
	timerAttachInterrupt(timer, &onTime, true); // Attaches the handler function to the timer
	timerAlarmWrite(timer, 20, true);			// Interrupts when counter == 20, 8000 times a second

	// Init codec2
	mode = CODEC2_MODE_1600;
	codec2_1600 = codec2_create(mode);
	nsam = codec2_samples_per_frame(codec2_1600);
	nbit = codec2_bits_per_frame(codec2_1600);
	nbyte = (nbit + 7) / 8;

	codec2_set_natural_or_gray(codec2_1600, !1);
	codec2_set_lpc_post_filter(codec2_1600, 1, 0, 0.8, 0.2);
	Serial.printf("Create CODEC2_1600 : nsam=%d ,nbit=%d ,nbyte=%d\n", nsam, nbit, nbyte);

	mode = CODEC2_MODE_3200;
	codec2_3200 = codec2_create(mode);
	nsam = codec2_samples_per_frame(codec2_3200);
	nbit = codec2_bits_per_frame(codec2_3200);
	nbyte = (nbit + 7) / 8;

	codec2_set_natural_or_gray(codec2_3200, !1);
	codec2_set_lpc_post_filter(codec2_3200, 1, 0, 0.9, 0.3);
	Serial.printf("Create CODEC2_3200 : nsam=%d ,nbit=%d ,nbyte=%d\n", nsam, nbit, nbyte);

	xTaskCreatePinnedToCore(
		taskNetwork,		/* Function to implement the task */
		"taskNetwork",		/* Name of the task */
		8192,				/* Stack size in words */
		NULL,				/* Task input parameter */
		2,					/* Priority of the task */
		&taskNetworkHandle, /* Task handle. */
		0);					/* Core where the task should run */

	xTaskCreatePinnedToCore(
		taskDSP,		/* Function to implement the task */
		"taskDSP",		/* Name of the task */
		32768,			/* Stack size in words */
		NULL,			/* Task input parameter */
		1,				/* Priority of the task */
		&taskDSPHandle, /* Task handle. */
		1);				/* Core where the task should run */
	xTaskCreatePinnedToCore(
		taskUI,		   /* Function to implement the task */
		"taskUI",	   /* Name of the task */
		4096,		   /* Stack size in words */
		NULL,		   /* Task input parameter */
		1,			   /* Priority of the task */
		&taskUIHandle, /* Task handle. */
		0);			   /* Core where the task should run */
}

void loop()
{
	vTaskDelay(1000 / portTICK_PERIOD_MS);
	if (Serial.available())
	{
		String command = Serial.readString();
		if (command.startsWith(F("WifiSSID->")))
		{
			sprintf(config.wifi_ssid, command.substring(10).c_str());
		}
		if (command.startsWith(F("WifiPass->")))
		{
			sprintf(config.wifi_pass, command.substring(10).c_str());
		}
		if (command.startsWith(F("SetCall->")))
		{
			sprintf(config.mycall, command.substring(9).c_str());
		}
		if (command.startsWith(F("SetRefName->")))
		{
			sprintf(config.current_reflector.name, command.substring(12).c_str());
		}
		if (command.startsWith(F("SetRefIp->")))
		{
			sprintf(config.current_reflector.host, command.substring(10).c_str());
		}
		if (command.startsWith(F("SetRefPort->")))
		{
			config.current_reflector.port = (uint16_t)command.substring(12).toInt();
		}
		if (command.startsWith(F("SetModule->")))
		{
			config.current_module = command.substring(11).c_str()[0];
		}
		if (command.startsWith(F("Save")))
		{
			saveEEPROM();
		}
	}
}

void taskUI(void *pvParameters)
{
	tx = false;
	for (;;)
	{
		vTaskDelay(10 / portTICK_PERIOD_MS);
		// PTT KEY
		if ((digitalRead(PTT_PIN) == LOW) && (!rxRef))
		{
			// Start Transmit
			if (tx == false)
			{
				tx = true;
				firstTX = true;
				tx_cnt = 0;
				if (config.codec2_mode == CODEC2_MODE_3200)
				{
					mode = CODEC2_MODE_3200;
				}
				else
				{
					mode = CODEC2_MODE_1600;
				}
				audioq.clean();
				pcmq.clean();
				adcq.clean();
				txstreamid++;

				dac_output_disable(DAC_CHANNEL_1);
				dac_output_disable(DAC_CHANNEL_2);
				pinMode(SPEAKER_PIN, OUTPUT);
				digitalWrite(SPEAKER_PIN, 0);
				timerAlarmEnable(timer);
			}
			tx = true;
		}
		else
		{
			tx = false;
			tx_cnt = 0x8000;
		}

		if (millis() > (RxTimeout + 800))
		{
			if (rxRef)
			{
				rxRef = false;
				firstRX = false;
				dac_output_disable(DAC_CHANNEL_1);
				dac_output_disable(DAC_CHANNEL_2);
				pinMode(SPEAKER_PIN, OUTPUT);
				digitalWrite(SPEAKER_PIN, 0);
			}
		}

		// Start RX
		if (firstRX)
		{
			firstRX = false;
			timerAlarmEnable(timer);
			if (SPEAKER_PIN == 25)
			{
				dac_output_enable(DAC_CHANNEL_1);
			}
			else
			{
				dac_output_enable(DAC_CHANNEL_2);
			}
		}
	}
}

void taskDSP(void *pvParameters)
{
	Serial.println(F("Task DSP has been start"));
	for (;;)
	{
		vTaskDelay(2 / portTICK_PERIOD_MS);
		process_audio();
	}
}

void WiFiEvent(WiFiEvent_t event)
{
	switch (event)
	{
	case 7:
		Serial.print(F("WiFi connected! IP address: "));
		Serial.println(WiFi.localIP());
		break;
	default:
		break;
	}
}
void connectToWiFi(const char *ssid, const char *pwd)
{
	Serial.println("Connecting to WiFi network: " + String(ssid));
	WiFi.disconnect(true);
	WiFi.onEvent(WiFiEvent);
	WiFi.begin(ssid, pwd);
	Serial.println(F("Waiting for WIFI connection..."));
}

void taskNetwork(void *pvParameters)
{
	Serial.println(F("Task Network has been start"));
	connectToWiFi(config.wifi_ssid, config.wifi_pass);
	for (;;)
	{
		vTaskDelay(10 / portTICK_PERIOD_MS);
		if (WiFi.isConnected())
		{

			if (connect_status == DISCONNECTED)
			{
				beginM17();
				vTaskDelay(1000 / portTICK_PERIOD_MS);
				if (millis() > (m17ConectTimeout + 10000))
				{
					Serial.print(F("M17 Connecting.. "));
					Serial.println(config.current_reflector.name);
					process_connect();
					timerAlarmEnable(timer); // Start sample audio when M17 First Connected.
				}
				else
				{
					timerAlarmDisable(timer); // Stop sample audio when M17 Disconnect
				}
			}
			else
			{
				readyReadM17();
				transmitM17();
			}
		}
	}
}

