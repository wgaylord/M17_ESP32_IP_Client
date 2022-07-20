/*
 Name:		M17 Analog Gateway
 Created:	1-Nov-2021 14:27:23
 Author:	HS5TQA/Atten
 Reflector: https://m17.dprns.com
*/

#include <Arduino.h>
#include "driver/adc.h"
#include <driver/dac.h>
#include <SPI.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include <ESPmDNS.h>
#include <WiFiClient.h>
#include "esp_adc_cal.h"
#include "esp_system.h"
#include "resampling.h"
#include "esp_vad.h"

#include <WiFiClientSecure.h>
#include <ESP32Ping.h>
#include <Preferences.h>
#include <ctype.h>			   // for isNumber check
#include <ButterworthFilter.h> //In the codec2 folder in the library folder


#include <time.h>
#include <TimeLib.h>

#include "main.h"
#include "webservice.h"
#include "m17.h"

#include <LMS.h>

#include "I2S.h"

LMSClass LMS;

#define EEPROM_SIZE 1024


#define SPEAKER_PIN 26
#define MIC_PIN 36
#define PTT_PIN 32

#define LED_TX 4
#define LED_RX 2


#define ADC1_CHANNEL (ADC1_CHANNEL_0) // GPIO 36 = MIC_PIN

#define oled_timeout 0

M17Flags connect_status;
extern uint16_t txstreamid;
extern uint16_t tx_cnt;
extern unsigned short streamid;
extern unsigned short frameid;
extern unsigned long RxTimeout;
extern unsigned long m17ConectTimeout;

extern hw_timer_t *timer;

#ifdef OLED
#include <Wire.h>
#include "Adafruit_SSD1306.h"
#include <Adafruit_GFX.h>
#include <Adafruit_I2CDevice.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
// The pins for I2C are defined by the Wire-library.
// On an arduino UNO:       A4(SDA), A5(SCL)
// On an arduino MEGA 2560: 20(SDA), 21(SCL)
// On an arduino LEONARDO:   2(SDA),  3(SCL), ...
#define OLED_RESET 4		// Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3D ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// For a connection via I2C using the Arduino Wire include:
//#include <Wire.h>		 // Only needed for Arduino 1.6.5 and earlier
//#include "SSD1306Wire.h" // legacy: #include "SSD1306.h"
// OR #include "SH1106Wire.h"   // legacy: #include "SH1106.h"

// Initialize the OLED display using Arduino Wire:
// SSD1306Wire display(0x3c, SDA, SCL, GEOMETRY_128_64); // ADDRESS, SDA, SCL  -  SDA and SCL usually populate automatically based on your board's pins_arduino.h e.g. https://github.com/esp8266/Arduino/blob/master/variants/nodemcu/pins_arduino.h
// SSD1306Wire display(0x3c, D3, D5);  // ADDRESS, SDA, SCL  -  If not, they can be specified manually.
// SSD1306Wire display(0x3c, SDA, SCL, GEOMETRY_128_32);  // ADDRESS, SDA, SCL, OLEDDISPLAY_GEOMETRY  -  Extra param required for 128x32 displays.
// SH1106Wire display(0x3c, SDA, SCL);     // ADDRESS, SDA, SCL

#endif

int offset = 36848;

// int offset = 36848; // 36848[2303];
//  int offset = 28160; // 36848[1760];
int offset_new = 0, adc_count = 0;
RTC_DATA_ATTR int sample;

int idleTimeout = 0;
int oledTimeout = 0;


#include "I2S.h"
void *agc_handle = esp_agc_open(3, 16000); // Mode 3,Sample 16KHz


ns_handle_t ns_inst; // = ns_create(NS_FRAME_LENGTH_MS);
vad_handle_t vad_inst;

byte daysavetime = 1;

const int natural = 1;

//กรองความถี่สูงผ่าน >300Hz  HPF Butterworth Filter. 0-300Hz ช่วงความถี่ต่ำใช้กับโทน CTCSS/DCS ในวิทยุสื่อสารจะถูกรองทิ้ง
ButterworthFilter hp_filter(300, 8000, ButterworthFilter::ButterworthFilter::Highpass, 1);
//กรองความถี่ต่ำผ่าน <3.5KHz  LPF Butterworth Filter. ความถี่เสียงที่มากกว่า 3KHz ไม่ใช่ความถี่เสียงคนพูดจะถูกกรองทิ้ง
ButterworthFilter lp_filter(4000, 8000, ButterworthFilter::ButterworthFilter::Lowpass, 1);

ButterworthFilter hp16K_filter(300, 16000, ButterworthFilter::ButterworthFilter::Highpass, 1);
ButterworthFilter lp16K_filter(4000, 16000, ButterworthFilter::ButterworthFilter::Lowpass, 1);


CODEC2 *codec2_3200;
CODEC2 *codec2_1600;


//กำหนดค่าเริ่มต้นใช้โหมดของ Codec2
int mode = CODEC2_MODE_3200;

// Queue<char> audioq(300);
cppQueue audioq(sizeof(uint8_t), CODEC2_BUFF, IMPLEMENTATION); // Instantiate queue
cppQueue pcmq(sizeof(int16_t), PCM_BUFF, IMPLEMENTATION);	   // Instantiate queue
// cppQueue adcq(sizeof(int16_t), 2000, IMPLEMENTATION);  // Instantiate queue

char current_module = 'D';
bool voicTx = false;
bool linkToFlage = false;
bool notLinkFlage = false;
bool voiceIPFlage = false;

int nstart_bit, nend_bit, bit_rate;
short *buf;
unsigned char *bits;
float *softdec_bits;

int nsam, nbit, nbyte, i, frames, bits_proc, bit_errors, error_mode;

String srcCall, mycall, urCall, rptr1;

// Set your Static IP address for wifi AP
IPAddress local_IP(192, 168, 4, 1);
IPAddress gateway(192, 168, 4, 254);
IPAddress subnet(255, 255, 255, 0);

uint8_t sndW;
int16_t adcR;
int ppm_Level = 0;
int ppm_temp = 0;

bool rxRef = false;
bool tx = false;
bool firstTX = false;
bool firstRX = false;

Configuration config;
unsigned long NTP_Timeout;
unsigned long pingTimeout;

// TaskHandle_t taskSensorHandle;
TaskHandle_t taskNetworkHandle;
TaskHandle_t taskDSPHandle;
TaskHandle_t taskUIHandle;

time_t systemUptime = 0;
time_t wifiUptime = 0;

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

//กำหนดค่าคอนฟิกซ์เริ่มต้น
void defaultConfig()
{
	Serial.println(F("Default configure mode!"));
	sprintf(config.id, "M17AG");
	sprintf(config.wifi_ssid, "APRSTH");
	sprintf(config.wifi_pass, "aprsthnetwork");
	sprintf(config.wifi_ap_ssid, "M17AGate");
	config.wifi_ap_pass[0] = 0;
	sprintf(config.reflector_host, "203.150.19.24");
	sprintf(config.reflector_name, "M17-THA");
	config.reflector_port = 17000;
	config.reflector_module = 'D';
	sprintf(config.mycall, "N0CALL");
	config.mymodule = 'R';
	config.wifi = true;
	config.wifi_mode = WIFI_STA_FIX;
	config.wifi_ch = 1;
	config.volume = 20;
	config.mic = 13;
	config.agc = false;
	config.noise = true;
	config.codec2_mode = CODEC2_MODE_3200;
	config.wifi_protocol = 7;
	saveEEPROM();
}

RTC_DATA_ATTR pkgListType pkgList[PKGLISTSIZE];
unsigned char pkgList_index = 0;

void sort(pkgListType a[], int size);
void sortPkgDesc(pkgListType a[], int size);

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


void bp_filter16K(float *h, int n)
{
	int i = 0;
	for (i = 0; i < n; i++)
	{
		h[i] = hp16K_filter.Update(h[i]);
	}
	for (i = 0; i < n; i++)
	{
		h[i] = lp16K_filter.Update(h[i]);
	}
}


void ppmUpdate(int adc)
{
	ppm_temp = abs((int)adc);
	if (ppm_temp > ppm_Level)
		ppm_Level = ((ppm_Level * 255) + ppm_temp) >> 8;
	else
		ppm_Level = (ppm_Level * 16383) >> 14;
}

//เข้ารหัสและถอดรหัสเสียง Codec2
audio_resample_config_t resample;

uint16_t pcm_out[320];

float dBV;
float dBu;
int mVrms = 0;
long mVsum = 0;
int nochar_count = 0;
float d_mags[8];


void process_audio()
{
	int mV;
	int mVmax = 0;
	uint8_t c2Buf[8];
	int16_t adc;

	int pcmWidth = 160;

	if (mode == CODEC2_MODE_1600)
		pcmWidth = 320; // 40mS
	else
		pcmWidth = 160; // 20mS

	if (tx)
	{
		if ((pcmq.getCount() >= pcmWidth) && (audioq.getCount() < CODEC2_BUFF - 16))
		{
			float *audiof = (float *)malloc(pcmWidth * sizeof(float));
			short *audio_in = (short *)malloc(pcmWidth * sizeof(short));
			if (audiof == NULL)
				return;
			if (audio_in == NULL)
				return;

			// Serial.print("PCM: " );
			// mVmax = 0;
			mVsum = 0;
			memset(audiof, 0, pcmWidth);
			memset(&audio_in[0], 0, pcmWidth);
			for (int x = 0; x < pcmWidth; x++)
			{
				if (!pcmq.pop(&adc))
					break;
				// adc=Amplify(adc,1000.0);
				// audiof[x] = (float)LMS.computeLMS((int)adc,(int)adc);
				audiof[x] = (float)adc;

			}
			//  dBu=20*log(mVrms/774.6);
			//  dBV = 20 * log(mVrms);

			bp_filter(audiof, pcmWidth);

			if (config.noise)
			{
				short *audioIn = (short *)malloc(pcmWidth * sizeof(short));
				if (audioIn != NULL)
				{
					for (int i = 0; i < pcmWidth; i++)
						audioIn[i] = (int16_t)audiof[i];
					memset(&audio_in[0], 0, pcmWidth);
					esp_agc_process(agc_handle, (short *)&audioIn[0], &audio_in[0], 80, 8000);	 // Audomatic gain control
					esp_agc_process(agc_handle, (short *)&audioIn[80], &audio_in[80], 80, 8000); // Audomatic gain control
					if (pcmWidth == 320)
					{
						esp_agc_process(agc_handle, (short *)&audioIn[160], &audio_in[160], 80, 8000); // Audomatic gain control
						esp_agc_process(agc_handle, (short *)&audioIn[240], &audio_in[240], 80, 8000); // Audomatic gain control
					}

					// for (int i = 0; i < pcmWidth; i++)
					// 	audiof[i] = (float)audio_in[i];

					free(audioIn);
				}
				else
				{
					for (int i = 0; i < pcmWidth; i++)
						audio_in[i] = (int16_t)audiof[i];
				}

				short *audio_out = (short *)malloc(pcmWidth * sizeof(short) * 2);
				short *audio_buf = (short *)malloc(pcmWidth * sizeof(short) * 2);
				// for (int i = 0; i < pcmWidth; i++)
				// 	audio_in[i] = (int16_t)audiof[i];
				if (pcmWidth == 160)
				{
					memset(&audio_out[0], 0, pcmWidth * 2);																					// 3200 F/R mode
					audio_resample((short *)&audio_in[0], (short *)&audio_out[0], SAMPLE_RATE_CODEC2, SAMPLE_RATE, 160, 320, 1, &resample); // Change Sample rate 8Khz->16Khz
					memset(&audio_buf[0], 0, pcmWidth * 2);
					// vad_state_t vad_state = vad_process(vad_inst, audio_out);
					if (vad_process(vad_inst, audio_out) == VAD_SILENCE)
						memset(&audio_out[0], 0, 160);
					ns_process(ns_inst, audio_out, audio_buf);
					// vad_state_t vad_state2= vad_process(vad_inst, &audio_out[160]);
					if (vad_process(vad_inst, &audio_out[160]) == VAD_SILENCE)
						memset(&audio_out[160], 0, 160);
					ns_process(ns_inst, &audio_out[160], &audio_buf[160]);
					memset(&audio_in[0], 0, pcmWidth);
					audio_resample((short *)&audio_buf[0], (short *)&audio_in[0], SAMPLE_RATE, SAMPLE_RATE_CODEC2, 320, 160, 1, &resample); // Change Sample rate 16Khz->8Khz
				}
				else if (pcmWidth == 320) // 1600 V/D mode
				{
					memset(&audio_out[0], 0, pcmWidth * 2);
					audio_resample((short *)&audio_in[0], (short *)&audio_out[0], SAMPLE_RATE_CODEC2, SAMPLE_RATE, 320, 640, 1, &resample); // Change Sample rate 8Khz->16Khz
					memset(&audio_buf[0], 0, pcmWidth * 2);
					ns_process(ns_inst, &audio_out[0], &audio_buf[0]);
					ns_process(ns_inst, &audio_out[160], &audio_buf[160]);
					ns_process(ns_inst, &audio_out[320], &audio_buf[320]);
					ns_process(ns_inst, &audio_out[480], &audio_buf[480]);
					memset(&audio_in[0], 0, pcmWidth);
					audio_resample((short *)&audio_buf[0], (short *)&audio_in[0], SAMPLE_RATE, SAMPLE_RATE_CODEC2, 640, 320, 1, &resample); // Change Sample rate 16Khz->8Khz
				}
				for (int i = 0; i < pcmWidth; i++)
				{
					// audio_in[i]=Amplify(audio_in[i],512.0);
					audiof[i] = (float)audio_in[i];

				}

				free(audio_out);
				free(audio_buf);
				bp_filter(audiof, pcmWidth);
			}

			if (config.agc)
			{
				short *audioIn = (short *)malloc(pcmWidth * sizeof(short));
				if (audioIn != NULL)
				{
					for (int i = 0; i < pcmWidth; i++)
						audioIn[i] = (int16_t)audiof[i];
					memset(&audio_in[0], 0, pcmWidth);
					esp_agc_process(agc_handle, (short *)&audioIn[0], &audio_in[0], 80, 8000);	 // Audomatic gain control
					esp_agc_process(agc_handle, (short *)&audioIn[80], &audio_in[80], 80, 8000); // Audomatic gain control
					if (pcmWidth == 320)
					{
						esp_agc_process(agc_handle, (short *)&audioIn[160], &audio_in[160], 80, 8000); // Audomatic gain control
						esp_agc_process(agc_handle, (short *)&audioIn[240], &audio_in[240], 80, 8000); // Audomatic gain control
					}

					for (int i = 0; i < pcmWidth; i++)
						audiof[i] = (float)audio_in[i];

					free(audioIn);
				}
			}

			for (int i = 0; i < pcmWidth; i++)
				audio_in[i] = (int16_t)(audiof[i] * ((float)config.mic / 10));

			free(audiof);

			memset(c2Buf, 0, sizeof(c2Buf));
			if (mode == CODEC2_MODE_1600)
				codec2_encode(codec2_1600, c2Buf, audio_in);
			else
				codec2_encode(codec2_3200, c2Buf, audio_in);

			free(audio_in);
			// Serial.print("Encode: ");
			for (int x = 0; x < 8; x++)
			{
				if (!audioq.push(&c2Buf[x]))
				{
					Serial.println("audioq is FULL");
				}
				// Serial.printf("%02x ", (unsigned char)c2Buf[x]);
			}

		}
	}
	else
	{

		int packetSize = audioq.getCount();
		if (packetSize >= 8 && (pcmq.getCount() < (PCM_BUFF - pcmWidth)))
		{


			// Serial.print("CODEC2: " + frame);
			for (int x = 0; x < 8; x++)
			{
				if (!audioq.pop(&c2Buf[x]))
					break;
				// Serial.printf("%02x ", (unsigned char)c2Buf[x]);
			}
			// Serial.println();
			short *audio_in = (short *)malloc(pcmWidth * sizeof(short));
			if (audio_in != NULL)
			{
				memset(&audio_in[0], 0, pcmWidth);
				if (mode == CODEC2_MODE_1600)
					codec2_decode(codec2_1600, audio_in, c2Buf);
				else
					codec2_decode(codec2_3200, audio_in, c2Buf);

				// if (config.agc)
				//{
				short *audioIn = (short *)malloc(pcmWidth * sizeof(short));
				if (audioIn != NULL)
				{
					for (int i = 0; i < pcmWidth; i++)
					{
						audioIn[i] = audio_in[i];
						audio_in[i] = 0;
					}
					esp_agc_process(agc_handle, (short *)&audioIn[0], &audio_in[0], 80, 8000);	 // Audomatic gain control
					esp_agc_process(agc_handle, (short *)&audioIn[80], &audio_in[80], 80, 8000); // Audomatic gain control
					if (pcmWidth == 320)
					{
						esp_agc_process(agc_handle, (short *)&audioIn[160], &audio_in[160], 80, 8000); // Audomatic gain control
						esp_agc_process(agc_handle, (short *)&audioIn[240], &audio_in[240], 80, 8000); // Audomatic gain control
					}
					free(audioIn);
				}


				for (int x = 0; x < pcmWidth; x++)
				{


					audio_in[x] = Amplify(audio_in[x], 150.0); // 64=x1

					// while (pcmq.isFull())
					// 	delay(50);
					if (!pcmq.push(&audio_in[x]))
					{
						Serial.println("audioBuf is FULL");
						break;
					}
				}
				free(audio_in);
			}
		}
	}
}

hw_timer_t *timer = NULL;

int offset_count = 0;
int adcR_old = 0;
// RTC_DATA_ATTR float adcf;
//แซมปลิ้งเสียง 8,000 ครั้งต่อวินาที ทั้งเข้า ADC และออก DAC


const char *lastTitle = "LAST HERT";

char pkgList_Find(char *call)
{
	char i;
	for (i = 0; i < PKGLISTSIZE; i++)
	{
		if (strstr(pkgList[(int)i].calsign, call) != NULL)
			return i;
	}
	return -1;
}

char pkgListOld()
{
	char i, ret = 0;
	time_t minimum = pkgList[0].time;
	for (i = 1; i < PKGLISTSIZE; i++)
	{
		if (pkgList[(int)i].time < minimum)
		{
			minimum = pkgList[(int)i].time;
			ret = i;
		}
	}
	return ret;
}

void sort(pkgListType a[], int size)
{
	pkgListType t;
	char *ptr1;
	char *ptr2;
	char *ptr3;
	ptr1 = (char *)&t;
	for (int i = 0; i < (size - 1); i++)
	{
		for (int o = 0; o < (size - (i + 1)); o++)
		{
			if (a[o].time < a[o + 1].time)
			{
				ptr2 = (char *)&a[o];
				ptr3 = (char *)&a[o + 1];
				memcpy(ptr1, ptr2, sizeof(pkgListType));
				memcpy(ptr2, ptr3, sizeof(pkgListType));
				memcpy(ptr3, ptr1, sizeof(pkgListType));
			}
		}
	}
}

void sortPkgDesc(pkgListType a[], int size)
{
	pkgListType t;
	char *ptr1;
	char *ptr2;
	char *ptr3;
	ptr1 = (char *)&t;
	for (int i = 0; i < (size - 1); i++)
	{
		for (int o = 0; o < (size - (i + 1)); o++)
		{
			if (a[o].pkg < a[o + 1].pkg)
			{
				ptr2 = (char *)&a[o];
				ptr3 = (char *)&a[o + 1];
				memcpy(ptr1, ptr2, sizeof(pkgListType));
				memcpy(ptr2, ptr3, sizeof(pkgListType));
				memcpy(ptr3, ptr1, sizeof(pkgListType));
			}
		}
	}
}

void pkgListUpdate(char *call, bool type)
{
	char i = pkgList_Find(call);
	time_t now;
	time(&now);
	if (i != 255)
	{ // Found call in old pkg
		pkgList[(uint)i].time = now;
		pkgList[(uint)i].pkg++;
		pkgList[(uint)i].type = type;
		// Serial.print("Update: ");
	}
	else
	{
		i = pkgListOld();
		pkgList[(uint)i].time = now;
		pkgList[(uint)i].pkg = 1;
		pkgList[(uint)i].type = type;
		strcpy(pkgList[(uint)i].calsign, call);
		// strcpy(pkgList[(uint)i].ssid, &ssid[0]);
		pkgList[(uint)i].calsign[10] = 0;
		// Serial.print("NEW: ");
	}
}


// Define meter size
#define M_SIZE 0.5

float ltx = 0;									 // Saved x coord of bottom of needle
uint16_t osx = M_SIZE * 120, osy = M_SIZE * 120; // Saved x & y coords
uint32_t updateTime = 0;						 // time for next update
int old_analog = -999;							 // Value last displayed
int old_digital = -999;							 // Value last displayed
uint16_t vuOffsetX = 2;
uint16_t vuOffsetY = 23;

String getValue(String data, char separator, int index)
{
	int found = 0;
	int strIndex[] = {0, -1};
	int maxIndex = data.length();

	for (int i = 0; i <= maxIndex && found <= index; i++)
	{
		if (data.charAt(i) == separator || i == maxIndex)
		{
			found++;
			strIndex[0] = strIndex[1] + 1;
			strIndex[1] = (i == maxIndex) ? i + 1 : i;
		}
	}
	return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
} // END

boolean isValidNumber(String str)
{
	for (byte i = 0; i < str.length(); i++)
	{
		if (isDigit(str.charAt(i)))
			return true;
	}
	return false;
}

#ifdef OLED
void callLastDisp()
{

	uint8_t wifi = 0, k = 0, l;
	char i;
	char list[4];
	int x, y;
	String str;

	display.fillRect(0, 16, 128, 10, WHITE);
	display.drawLine(0, 16, 0, 63, WHITE);
	display.drawLine(127, 16, 127, 63, WHITE);
	display.drawLine(0, 63, 127, 63, WHITE);
	display.fillRect(1, 25, 126, 38, BLACK);
	display.setTextColor(BLACK);
	display.setCursor(27, 17);
	display.print("LAST STATIONS");
	display.setTextColor(WHITE);

	sort(pkgList, PKGLISTSIZE);
	k = 0;
	for (i = 0; i < PKGLISTSIZE; i++)
	{
		if (pkgList[i].time > 0)
		{
			y = 26 + (k * 9);
			// display.drawBitmap(3, y, &SYMBOL[0][0], 11, 6, WHITE);
			display.setCursor(3, y);
			pkgList[i].calsign[10] = 0;
			display.print(pkgList[i].calsign);
			if ((pkgList[i].ssid[0] != 0) && (pkgList[i].ssid[0] != 0x20))
			{
				display.setCursor(45, y);
				display.print("/");
				display.print(pkgList[i].ssid);
			}

			struct tm tmstruct;
			time_t tm = pkgList[i].time;
			localtime_r(&pkgList[i].time, &tmstruct);
			str = String(tmstruct.tm_hour, DEC) + ":" + String(tmstruct.tm_min, DEC) + ":" + String(tmstruct.tm_sec, DEC);

			// if (hour(pkgList[i].time)<10) str += "0";
			// str += String(hour(pkgList[i].time), DEC) + ":";
			// if (minute(pkgList[i].time)<10) str += "0";
			// str += String(minute(pkgList[i].time), DEC);
			// if(second(pkgList[i].time)<10) str += "0";
			// str += String(second(pkgList[i].time), DEC);
			// str = String(pkgList[pkgLast_array[i]].time, DEC);
			x = str.length() * 6;
			display.setCursor(126 - x, y);
			display.print(str);
			k++;
			if (k >= 4)
				break;
		}
	}
	display.display();
}

void topBar(int ws)
{
	// int ang = analogRead(A0);
	//  float vbat;
	//  uint8_t vbatScal = 0;
	int wifiSignal = ws;
	uint8_t wifi = 0, i;
	int x, y;
	String netMode = "";
	if (config.wifi_mode == WIFI_STA_FIX)
	{
		wifiSignal = WiFi.RSSI();
		netMode = "STA";
	}
	else if (config.wifi_mode == WIFI_AP_STA_FIX)
	{
		netMode = "AP+STA";
	}
	else if (config.wifi_mode == WIFI_AP_FIX)
	{
		netMode = "AP";
	}
	else
	{
		netMode = "OFF";
	}

	display.fillRect(0, 0, 128, 16, BLACK);
	// Draw Attena Signal
	display.drawTriangle(0, 0, 6, 0, 3, 3, WHITE);
	display.drawLine(3, 0, 3, 7, WHITE);
	x = 5;
	y = 3;
	wifi = (wifiSignal + 100) / 10;
	if (wifi > 5)
		wifi = 5;
	if (wifi < 0)
		wifi = 0;
	for (i = 0; i < wifi; i++)
	{
		display.drawLine(x, 7 - y, x, 7, WHITE);
		x += 2;
		y++;
	}
	display.setCursor(0, 8);
	display.print(wifiSignal);
	display.print("dBm");

	// x = 109;
	// display.drawLine(0 + x, 1, 2 + x, 1);
	// display.drawLine(0 + x, 6, 2 + x, 6);
	// display.drawLine(0 + x, 2, 0 + x, 5);
	// display.drawLine(2 + x, 0, 18 + x, 0);
	// display.drawLine(2 + x, 7, 18 + x, 7);
	// display.drawLine(18 + x, 1, 18 + x, 6);
	// Wifi Status
	if (WiFi.status() == WL_CONNECTED)
	{
		display.setCursor(15, 0);
		// display.print("WiFi");
		display.print(" " + netMode);
	}

	if (connect_status != DISCONNECTED)
	{
		display.setCursor(50, 0);
		display.print(String(config.reflector_name));
		display.print("[");
		display.print(String(current_module));
		display.print("]");
		// display.drawString(50, 1, "[" + String(config.reflector_module) + "]");
		//  display.drawLine(50,0,65,8,WHITE);
	}

	struct tm tmstruct;
	char time[20];
	getLocalTime(&tmstruct, 1000);
	display.setCursor(50, 8);
	sprintf(time, "%02d:%02d:%02d", tmstruct.tm_hour, tmstruct.tm_min, tmstruct.tm_sec);
	display.print(time);
	// display.setCursor(50, 8);
	// display.print(hour());
	// display.print(":");
	// display.print(minute());
	// display.print(":");
	// display.println(second());
	// display.setTextAlignment(TEXT_ALIGN_LEFT);
	// display.drawString(50, 6, String(hour()) + ":" + String(minute()) + ":" + String(second()));
	display.display();
}
#endif


void setup()
{
	int mode;
	byte *ptr;

	pinMode(0, INPUT_PULLUP);
	pinMode(PTT_PIN, INPUT_PULLUP);
	pinMode(LED_RX, OUTPUT);
	pinMode(LED_TX, OUTPUT);
	pinMode(MIC_PIN, INPUT);
	pinMode(39, INPUT);
	pinMode(34, INPUT);
	pinMode(35, INPUT);
	pinMode(25, INPUT);
	pinMode(23, INPUT);
	pinMode(19, INPUT);
	pinMode(18, INPUT);
	pinMode(5, INPUT);
	pinMode(15, INPUT);
	
	digitalWrite(39, INPUT_PULLDOWN);
	Serial.begin(115200);

#ifdef OLED
	Wire.begin();
	Wire.setClock(100000L);

	// by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
	display.begin(SSD1306_SWITCHCAPVCC, 0x3C, false); // initialize with the I2C addr 0x3C (for the 128x64)
	// Initialising the UI will init the display too.
	// display.init();
	// // clear the display
	// display.clear();
	// display.flipScreenVertically();
	// display.setTextAlignment(TEXT_ALIGN_LEFT);
	// display.drawString(128, 54, String(millis()));
	//  write the buffer to the display
	display.clearDisplay();
	display.setTextSize(1);
	display.setTextColor(WHITE);
	display.setCursor(50, 30);
	display.print("BOOT");
	// topBar(-100);
	display.display();
#endif


	// Initialize the I2S peripheral
	I2S_Init(I2S_MODE_DAC_BUILT_IN, I2S_BITS_PER_SAMPLE_16BIT);


	// esp_task_wdt_init(TWDT_TIMEOUT_S, false);
	Serial.println();
	Serial.println("M17 Analog Gateway V" + String(VERSION));
	Serial.println("Push BOOT after 3sec for Factory Default config.");


	if (!EEPROM.begin(EEPROM_SIZE))
	{
		Serial.println(F("failed to initialise EEPROM")); // delay(100000);
	}

	delay(1000);
	digitalWrite(LED_TX, HIGH);
	delay(1000);
	digitalWrite(LED_RX, HIGH);
	delay(1000);
	if (digitalRead(0) == LOW)
	{
		defaultConfig();
		Serial.println("Default configure!");
		while (digitalRead(0) == LOW)
		{
			delay(500);
			digitalWrite(LED_TX, LOW);
			digitalWrite(LED_RX, LOW);
			delay(500);
			digitalWrite(LED_TX, HIGH);
			digitalWrite(LED_RX, HIGH);
		}
	}
	digitalWrite(LED_TX, LOW);
	digitalWrite(LED_RX, LOW);

	//ตรวจสอบคอนฟิกซ์ผิดพลาด
	ptr = (byte *)&config;
	EEPROM.readBytes(1, ptr, sizeof(Configuration));
	uint8_t chkSum = checkSum(ptr, sizeof(Configuration));
	if (EEPROM.read(0) != chkSum)
	{
		Serial.println("Config EEPROM Error!");
		defaultConfig();
		digitalWrite(LED_TX, HIGH);
		digitalWrite(LED_RX, HIGH);
		delay(1000);
		digitalWrite(LED_TX, LOW);
		digitalWrite(LED_RX, LOW);
	}
	current_module = config.reflector_module;

	
	connect_status = DISCONNECTED;

	set_agc_config(agc_handle, 15, 1, -6);


	ns_inst = ns_create(NS_FRAME_LENGTH_MS); // 10ms at 160sample,16Khz sample rate
											 // vad_inst = vad_create(VAD_MODE_4, SAMPLE_RATE_HZ, VAD_FRAME_LENGTH_MS);
											 // aec_inst = aec_create(AEC_SAMPLE_RATE, AEC_FRAME_LENGTH_MS, AEC_FILTER_LENGTH);
											 // set_agc_config(agc_handle, 10, 1, -3);


	vad_inst = vad_create(VAD_MODE_3, SAMPLE_RATE_HZ, VAD_FRAME_LENGTH_MS); // Creates an instance to the VAD structure.

	// Init codec2
	mode = CODEC2_MODE_1600;
	codec2_1600 = codec2_create(mode);
	nsam = codec2_samples_per_frame(codec2_1600);
	nbit = codec2_bits_per_frame(codec2_1600);
	nbyte = (nbit + 7) / 8;
	// frames = bit_errors = bits_proc = 0;
	// nstart_bit = 0;
	// nend_bit = nbit - 1;

	codec2_set_natural_or_gray(codec2_1600, !natural);
	codec2_set_lpc_post_filter(codec2_1600, 1, 0, 0.8, 0.2);
	Serial.printf("Create CODEC2_1600 : nsam=%d ,nbit=%d ,nbyte=%d\n", nsam, nbit, nbyte);

	mode = CODEC2_MODE_3200;
	codec2_3200 = codec2_create(mode);
	nsam = codec2_samples_per_frame(codec2_3200);
	nbit = codec2_bits_per_frame(codec2_3200);
	nbyte = (nbit + 7) / 8;

	codec2_set_natural_or_gray(codec2_3200, !natural);
	// codec2_set_lpc_post_filter(codec2_3200, 1, 0, 0.9, 0.3);
	codec2_set_lpc_post_filter(codec2_3200, 1, 0, 1.0, 0.8);
	Serial.printf("Create CODEC2_3200 : nsam=%d ,nbit=%d ,nbyte=%d\n", nsam, nbit, nbyte);


#ifdef OLED
	display.clearDisplay();
	display.setTextSize(1);
	display.display();
#endif

	enableLoopWDT();
	// enableCore0WDT();
	enableCore1WDT();

	xTaskCreatePinnedToCore(
		taskNetwork,		/* Function to implement the task */
		"taskNetwork",		/* Name of the task */
		8192,				/* Stack size in words */
		NULL,				/* Task input parameter */
		1,					/* Priority of the task */
		&taskNetworkHandle, /* Task handle. */
		1);					/* Core where the task should run */

	xTaskCreatePinnedToCore(
		taskDSP,		/* Function to implement the task */
		"taskDSP",		/* Name of the task */
		32768,			/* Stack size in words */
		NULL,			/* Task input parameter */
		1,				/* Priority of the task */
		&taskDSPHandle, /* Task handle. */
		0);				/* Core where the task should run */
	xTaskCreatePinnedToCore(
		taskUI,		   /* Function to implement the task */
		"taskUI",	   /* Name of the task */
		4096,		   /* Stack size in words */
		NULL,		   /* Task input parameter */
		2,			   /* Priority of the task */
		&taskUIHandle, /* Task handle. */
		1);			   /* Core where the task should run */
}

bool firstRxDisp = false;
int btn_count = 0;

void loop()
{
	vTaskDelay(10 / portTICK_PERIOD_MS);
	// #ifdef SA818
	// 	if (SerialRF.available())
	// 	{
	// 		Serial.print(Serial.readString());
	// 	}
	// #endif
	if (digitalRead(0) == LOW)
	{
		idleTimeout = 11;
		btn_count++;
		if (btn_count > 1000) // Push BOOT 10sec
		{
			digitalWrite(LED_RX, HIGH);
			digitalWrite(LED_TX, HIGH);
		}
	}
	else
	{
		if (btn_count > 0)
		{
			// Serial.printf("btn_count=%dms\n", btn_count * 10);
			if (btn_count > 1000) // Push BOOT 10sec to Factory Default
			{
				digitalWrite(LED_RX, LOW);
				digitalWrite(LED_TX, LOW);
				defaultConfig();
				Serial.println("SYSTEM REBOOT NOW!");
				esp_restart();
			}
			else
			{
				voicTx = true;
			}
			btn_count = 0;
		}
	}
}

int timeHalfSec = 0;
long tickSec = 0;
bool firstIdle = true;
// uint8_t micCur = 0;
// uint8_t volCur = 0;
void taskUI(void *pvParameters)
{
	int voxCount = 0;
	String info = "";

	char text[50];

	idleTimeout = 30;
	// micCur = config.mic;
	// volCur = config.volume;

	vox = false;
	tx = false;
	// timerAlarmEnable(timer);
	int mic_level = 0;
	long voxtime = millis() + 10;

	for (;;)
	{
		// long now = millis();
		// wdtDisplayTimer = now;
		// Serial.print("task1 Uptime (ms): ");
		// Serial.println(millis());
		vTaskDelay(10 / portTICK_PERIOD_MS);
		if (linkToFlage)
		{
			delay(1000);
			linkToFlage = false;
			sprintf(text, "g %s  %c", config.reflector_name, current_module);
			Serial.println(text);
		}
		if (notLinkFlage)
		{
			delay(1000);
			notLinkFlage = false;
			sprintf(text, " n ");
			Serial.println(text);
		}
		if (voicTx)
		{
			delay(1000);
			voicTx = false;
			sprintf(text, "0123456789ABCDEFGHIJKLMNOPQRSTUVWXZ");
			Serial.println(text);
		}
		if (voiceIPFlage)
		{
			delay(1000);
			voiceIPFlage = false;
			sprintf(text, "IP %s", WiFi.localIP().toString().c_str());
			Serial.println(text);
		}

#ifdef OLED
		if (millis() > tickSec)
		{
			if (oledTimeout > 0)
			{
				idleTimeout = 1;
				if (oledTimeout == 1)
				{
					idleTimeout = 0;
					display.clearDisplay();
					display.display();
				}
				oledTimeout--;
			}
			tickSec = millis() + 1000;
			if (idleTimeout > 15)
			{
				if (WiFi.status() == WL_CONNECTED)
				{
					display.setCursor(0, 17);
					if (config.wifi_mode == WIFI_AP_FIX || config.wifi_mode == WIFI_AP_STA_FIX)
					{
						display.println(F("WiFi AP URL:"));
						display.print(F("http://"));
						display.println(local_IP.toString());
					}
					display.print(F("WiFi SSID: "));
					display.println(WiFi.SSID());
					display.println(F("WiFi STA URL:"));
					display.print(F("http://"));
					display.println(WiFi.localIP());
				}
			}
			else if (idleTimeout == 10)
			{
				callLastDisp();
			}
			else if (idleTimeout == 2)
			{
				idleTimeout = 1;
				oledTimeout = oled_timeout;
				if (oledTimeout == 0)
				{
					idleTimeout = 0;
					display.clearDisplay();
					display.display();
				}
			}
			if (idleTimeout > 0)
			{
				idleTimeout--;
				topBar(WiFi.RSSI());
			}
		}
#endif

		if (connect_status != DISCONNECTED)
		{
			// PTT KEY
			if ((digitalRead(PTT_PIN) == LOW) && (!rxRef))
			{
				// Start Transmit
				if (tx == false)
				{
					tx = true;
					firstTX = true;
					firstIdle = true;
					tx_cnt = 0;
					if (config.codec2_mode == CODEC2_MODE_3200)
						mode = CODEC2_MODE_3200;
					else
						mode = CODEC2_MODE_1600;
					audioq.clean();
					pcmq.clean();
					txstreamid++;

					Serial.println("<Start TX to Ref.>");
					digitalWrite(LED_RX, HIGH);
					digitalWrite(LED_TX, LOW);
#ifdef OLED
					
						display.fillRect(0, 16, 128, 48, BLACK);

						display.drawRect(0, 16, 128, 39, WHITE);
						display.fillRect(101, 16, 28, 9, WHITE);
						display.fillRect(0, 35, 21, 10, WHITE);
						display.fillRect(75, 35, 20, 9, WHITE);
						display.fillRect(0, 44, 13, 10, WHITE);
						display.fillRect(64, 44, 13, 10, WHITE);
						display.drawLine(101, 16, 101, 34, WHITE);
						display.drawLine(0, 34, 128, 34, WHITE);
						display.drawLine(0, 44, 128, 44, WHITE);

						display.setTextColor(BLACK);
						display.setCursor(103, 17);
						display.print("SSID");
						display.setCursor(1, 36);
						display.print("TYP");
						display.setCursor(76, 36);
						display.print("PKG");
						display.setCursor(1, 46);
						display.print("R1");
						display.setCursor(65, 46);
						display.print("R2");

						display.fillRect(2, 19, 3, 12, WHITE);
						display.drawCircle(9, 25, 6, WHITE);
						display.setTextColor(WHITE);
						display.setCursor(20, 18);
						display.setTextSize(2);
						display.print(config.mycall);
						display.setTextSize(1);
						display.setCursor(108, 26);
						display.print(String(config.mymodule));
						display.setCursor(23, 36);
						if (config.codec2_mode == CODEC2_MODE_3200)
							display.print("3200 F/R");
						else
							display.print("1600 V/D");
						display.setCursor(100, 36);
						display.print(String(txstreamid, HEX));

						display.setCursor(15, 46);
						display.print(String(config.reflector_name));

						display.setCursor(79, 46);
						display.print("-");
						// display.println(((String)urCall).substring(0, 6)+"-"+urCall.substring(urCall.length() - 1, urCall.length()));

						display.setCursor(0, 56);
						display.print("SND");
						display.drawRect(20, 56, 100, 8, WHITE);
					
#endif
				}
#ifdef OLED
					i = ppm_Level >> 6;

					if (i > 99)
						i = 99;
					if (i < 1)
						i = 1;
					display.fillRect(20, 56, i, 8, WHITE);
					display.fillRect(i + 20, 57, 99 - i, 6, BLACK);
					// for (i; i >0; i--) display.print(">");
					// display.setCursor(80, 56);
					// display.print(i);
					// display.print("dBV");
					display.display();
				
#endif
				tx = true;
				idleTimeout = 15;
				// idleTimeout = millis();
			}
			else
			{
				// End Transmit
				if (tx)
				{
					tx = false;
					// timerAlarmDisable(timer);
					Serial.println("<END TX>");
					digitalWrite(LED_TX, LOW);
					digitalWrite(LED_RX, LOW);
					// while (audioq.getCount() >= 16)
					// {
					// 	delay(20);
					// }
					delay(50);
					audioq.clean();
					pcmq.clean();
					terminateM17();


					dac_output_disable(DAC_CHANNEL_1);
					dac_output_disable(DAC_CHANNEL_2);
					dac_i2s_disable();

				}
				tx = false;
			}

			// End RX
			if (millis() > (RxTimeout + 800))
			{
				if (rxRef)
				{
					rxRef = false;
					firstRX = false;
					firstIdle = true;
					Serial.println("<END RX>");
					digitalWrite(LED_TX, LOW);
					digitalWrite(LED_RX, LOW);
				}
			}

			// Start RX
			if (firstRX)
			{
				firstRX = false;
				firstRxDisp = true;

				Serial.println("<Start RX From Ref.>");
				pkgListUpdate((char *)srcCall.c_str(), 0);
				digitalWrite(LED_TX, HIGH);
				digitalWrite(LED_RX, LOW);
			}

			if (rxRef)
			{
				idleTimeout = 15;
				if (firstRxDisp)
				{
					firstRxDisp = false;
					info = "DstID: " + urCall;
					info += "\nType: " + rptr1;
					info += "\nFrameID: " + String(frameid);
					info += "\nStreamID: " + String(streamid);
					Serial.println(info);
#ifdef OLED
					
						display.fillRect(0, 16, 128, 48, BLACK); // Clear Screen

						display.drawRect(0, 16, 128, 39, WHITE);
						display.fillRect(101, 16, 28, 9, WHITE);
						display.fillRect(0, 35, 21, 10, WHITE);
						display.fillRect(75, 35, 20, 9, WHITE);
						display.fillRect(0, 44, 13, 10, WHITE);
						display.fillRect(64, 44, 13, 10, WHITE);
						display.drawLine(101, 16, 101, 34, WHITE);
						display.drawLine(0, 34, 128, 34, WHITE);
						display.drawLine(0, 44, 128, 44, WHITE);

						// display.drawRect(0, 16, 128, 21, WHITE); //RPT RECT
						display.setTextColor(BLACK);
						display.setCursor(103, 17);
						display.print("SSID");
						display.setCursor(1, 36);
						display.print("TYP");
						display.setCursor(76, 36);
						display.print("PKG");
						display.setCursor(1, 46);
						display.print("R1");
						display.setCursor(65, 46);
						display.print("R2");

						display.fillRect(2, 22, 5, 8, WHITE);
						display.drawLine(12, 18, 12, 32, WHITE);
						display.drawLine(7, 22, 12, 18, WHITE);
						display.drawLine(7, 30, 12, 32, WHITE);
						display.setTextColor(WHITE);
						display.setCursor(20, 18);
						display.setTextSize(2);
						display.println(srcCall.substring(0, srcCall.length() - 1));
						display.setTextSize(1);

						display.setCursor(110, 26);
						display.print(srcCall.substring(srcCall.length() - 1, srcCall.length()));

						display.setCursor(23, 36);
						display.println(rptr1.substring(0, 8));
						display.setCursor(100, 36);
						display.print(String(streamid, HEX));
						display.setCursor(15, 46);
						display.print(String(config.reflector_name));
						display.setCursor(79, 46);
						display.println(((String)urCall).substring(0, 6) + "-" + urCall.substring(urCall.length() - 1, urCall.length()));

						display.setCursor(0, 56);
						display.print("SND");
						display.drawRect(20, 56, 100, 8, WHITE);
						display.display();
					

					dac_output_enable(DAC_CHANNEL_2);
					dac_i2s_enable();

#endif
				}
			}
		}
	}
}

void taskDSP(void *pvParameters)
{
	size_t bytesRead;
	Serial.println("Task DSP has been start");
	long ts = millis();
	for (;;)
	{
		// Serial.println(millis() - ts);
		vTaskDelay(1 / portTICK_PERIOD_MS);
		// ts = millis();
		process_audio();
		if (tx)
		{
			if (i2s_read(I2S_NUM_0, (char *)&pcm_out, (320 * sizeof(int16_t)), &bytesRead, portMAX_DELAY) == ESP_OK) // Block but yield to other tasks
			{
				if (bytesRead == (320 * sizeof(uint16_t)))
				{
					//  Serial.print(pcm_out[0]<<4);
					//  Serial.print(",");
					int k = 0;
					float *audiof = (float *)malloc(320 * sizeof(float));
					short *audio_in = (short *)malloc(320 * sizeof(short));
					short *audio_out = (short *)malloc(320 * sizeof(short));
					if (audiof == NULL || audio_in == NULL || audio_in == NULL)
						continue;

					for (int i = 0; i < 320; i += 2)
					{
						// inputSampleBuffer[i]>>=16; //Capture Left Channel
						// pcm_out[i] &= 0xfff; // Clear MSB bit
						pcm_out[i] <<= 4; // Up 12bit ADC -> 16bit

						// Auto DC offset
						offset_new += pcm_out[i];
						offset_count++;
						if (offset_count >= 320)
						{
							offset = offset_new / offset_count;
							offset_count = 0;
							offset_new = 0;
							// if (offset > 3000 || offset < 1500) // Over dc offset to default
							// 	offset = 2303;
							if (offset > 40000 || offset < 20000) // Over dc offset to default
								offset = 28160;
						}
						audiof[k] = (float)pcm_out[i] - (float)offset;
						ppmUpdate((int)audiof[k]);
						k++;
					}

					bp_filter16K(audiof, 160);
					for (int i = 0; i < 160; i++)
					{
						if (config.noise)
							audio_out[i] = (int16_t)audiof[i];
						else
							audio_in[i] = (int16_t)audiof[i];
					}
					free(audiof);

					if (config.noise)
					{
						// esp_agc_process(agc_handle, (short *)&audio_in[0], &audio_out[0], 160, 16000);	 // Audomatic gain control
						ns_process(ns_inst, audio_out, audio_in);
					}

					audio_resample((short *)audio_in, (short *)audio_out, SAMPLE_RATE, SAMPLE_RATE_CODEC2, 160, 80, 1, &resample); // Change Sample rate 16Khz->8Khz

					for (int i = 0; i < 80; i++)
					{
						adcR = (int16_t)audio_out[i];
						pcmq.push(&adcR);
					}
					free(audio_in);
					free(audio_out);
				}
			}
		}
		else
		{
			//รับเสียงจากเซิร์ฟเวอร์ M17 ออกลำโพง
			if (pcmq.getCount() >= 80)
			{
				while (pcmq.getCount() >= 80)
				{
					short *audio_in = (short *)malloc(160 * sizeof(short));
					short *audio_out = (short *)malloc(160 * sizeof(short));
					if (audio_in != NULL && audio_out != NULL)
					{
						for (int i = 0; i < 80; i++)
						{
							pcmq.pop(&adcR);
							audio_in[i] = (int16_t)adcR;
						}

						audio_resample((short *)audio_in, (short *)audio_out, SAMPLE_RATE_CODEC2, SAMPLE_RATE, 80, 160, 1, &resample); // Change Sample rate 8Khz->16Khz
						esp_agc_process(agc_handle, audio_out, audio_in, 160, SAMPLE_RATE);
						int k = 0;
						for (int i = 0; i < 160; i++)
						{
							float auf = (float)audio_in[i];
							ppmUpdate((int)audio_in[i]);
							auf *= 1.8;
							audio_in[i] = (short)auf;
							// Chanel Left
							pcm_out[k] = (uint16_t)((int)audio_in[i] + 32768); // Convert sign to unsign wave
							// Chanel Right
							pcm_out[k + 1] = 0;
							k += 2;
						}
						size_t write_size = 0;
						i2s_write(I2S_NUM_0, (char *)&pcm_out, (k * sizeof(uint16_t)), &write_size, portMAX_DELAY);
						free(audio_in);
						free(audio_out);
					}
				}
			}
			else
			{
					i2s_read(I2S_NUM_0, (char *)&pcm_out, (320 * sizeof(int16_t)), &bytesRead, 10); // Block but yield to other tasks
					if (bytesRead >= (320 * sizeof(uint16_t)))
					{
						// Serial.print(String(pcm_out[0])+",");
						for (int i = 0; i < (bytesRead / sizeof(uint16_t)); i += 2)
						{
							pcm_out[i] <<= 4;
							// Auto DC offset
							offset_new += pcm_out[i];
							offset_count++;
							if (offset_count >= 320)
							{
								offset = offset_new / offset_count;
								offset_count = 0;
								offset_new = 0;
								if (offset > 40000 || offset < 20000) // Over dc offset to default
									offset = 28160;
							}

							ppmUpdate((int)pcm_out[i] - offset);
						}
					}
				
			}
		}
	}
}

unsigned long int wifiTTL = 0;
uint8_t current_protocol;
esp_interface_t current_esp_interface;
wifi_interface_t current_wifi_interface;

esp_interface_t check_protocol()
{
	char error_buf1[100];

	tcpip_adapter_get_esp_if(&current_esp_interface);
	if (current_esp_interface == ESP_IF_WIFI_STA)
		Serial.println("Interface is ESP_IF_WIFI_STA");
	else if (current_esp_interface == ESP_IF_WIFI_AP)
		Serial.println("Interface is ESP_IF_WIFI_AP");
	else
		Serial.println("Unknown interface!!");
	current_wifi_interface = (wifi_interface_t) current_esp_interface;
	if (current_wifi_interface == WIFI_IF_STA)
		Serial.println("Interface is WIFI_IF_STA");
	else if (current_wifi_interface == WIFI_IF_AP)
		Serial.println("Interface is WIFI_IF_AP");
	else
		Serial.println("Unknown interface!!");
	esp_err_t error_code = esp_wifi_get_protocol(current_wifi_interface, &current_protocol);
	esp_err_to_name_r(error_code, error_buf1, 100);
	Serial.print("esp_wifi_get_protocol error code: ");
	Serial.println(error_buf1);
	Serial.print("Current protocol code is ");
	Serial.println(current_protocol);
	if ((current_protocol & WIFI_PROTOCOL_11B) == WIFI_PROTOCOL_11B)
		Serial.println("Protocol is WIFI_PROTOCOL_11B");
	if ((current_protocol & WIFI_PROTOCOL_11G) == WIFI_PROTOCOL_11G)
		Serial.println("Protocol is WIFI_PROTOCOL_11G");
	if ((current_protocol & WIFI_PROTOCOL_11N) == WIFI_PROTOCOL_11N)
		Serial.println("Protocol is WIFI_PROTOCOL_11N");
	if ((current_protocol & WIFI_PROTOCOL_LR) == WIFI_PROTOCOL_LR)
		Serial.println("Protocol is WIFI_PROTOCOL_LR");

	return current_esp_interface;
}

void taskNetwork(void *pvParameters)
{
	int c = 0;
	uint aprsFixLocTimeout = 0;
	// chipid = ESP.getEfuseMac();
	Serial.println("Task Network has been start");

	// 	802.11 B     esp_wifi_set_protocol(ifx, WIFI_PROTOCOL_11B)
	// 802.11 BG    esp_wifi_set_protocol(ifx, WIFI_PROTOCOL_11B|WIFI_PROTOCOL_11G)
	// 802.11 BGN   esp_wifi_set_protocol(ifx, WIFI_PROTOCOL_11B|WIFI_PROTOCOL_11G|WIFI_PROTOCOL_11N)
	// 802.11 BGNLR esp_wifi_set_protocol(ifx, WIFI_PROTOCOL_11B|WIFI_PROTOCOL_11G|WIFI_PROTOCOL_11N|WIFI_PROTOCOL_LR)
	// 802.11 LR    esp_wifi_set_protocol(ifx, WIFI_PROTOCOL_LR)
	if (config.wifi_mode == WIFI_AP_STA_FIX || config.wifi_mode == WIFI_AP_FIX)
	{ // AP=false
		// WiFi.mode(config.wifi_mode);
		if (config.wifi_mode == WIFI_AP_STA_FIX)
		{
			WiFi.mode(WIFI_AP_STA);
		}
		else if (config.wifi_mode == WIFI_AP_FIX)
		{
			WiFi.mode(WIFI_AP);
		}
		//กำหนดค่าการทำงานไวไฟเป็นแอสเซสพ้อย
		WiFi.softAP(config.wifi_ap_ssid, config.wifi_ap_pass); // Start HOTspot removing password will disable security
		WiFi.softAPConfig(local_IP, gateway, subnet);
		Serial.print("Access point running. IP address: ");
		Serial.print(WiFi.softAPIP());
		Serial.println("");
	}
	else if (config.wifi_mode == WIFI_STA_FIX)
	{
		WiFi.mode(WIFI_STA);
		WiFi.disconnect();
		delay(100);
		Serial.println(F("WiFi Station Only mode."));
	}
	else
	{
		WiFi.mode(WIFI_OFF);
		WiFi.disconnect(true);
		delay(100);
		Serial.println(F("WiFi OFF All mode."));
	}

	if (config.wifi_protocol != 1 && config.wifi_protocol != 3 && config.wifi_protocol != 7)
		config.wifi_protocol = 7;
	tcpip_adapter_get_esp_if(&current_esp_interface);
	current_wifi_interface = (wifi_interface_t) current_esp_interface;
	esp_wifi_set_protocol(current_wifi_interface, config.wifi_protocol);
	esp_err_t error_code = esp_wifi_get_protocol(current_wifi_interface, &current_protocol);
	// esp_err_to_name_r(error_code,error_buf1,100);
	// Serial.print("esp_wifi_get_protocol error code: ");
	// Serial.println(error_buf1);
	Serial.print("Current protocol code is ");
	Serial.println(current_protocol);
	if ((current_protocol & WIFI_PROTOCOL_11B) == WIFI_PROTOCOL_11B)
		Serial.println("Protocol is WIFI_PROTOCOL_11B");
	if ((current_protocol & WIFI_PROTOCOL_11G) == WIFI_PROTOCOL_11G)
		Serial.println("Protocol is WIFI_PROTOCOL_11G");
	if ((current_protocol & WIFI_PROTOCOL_11N) == WIFI_PROTOCOL_11N)
		Serial.println("Protocol is WIFI_PROTOCOL_11N");
	if ((current_protocol & WIFI_PROTOCOL_LR) == WIFI_PROTOCOL_LR)
		Serial.println("Protocol is WIFI_PROTOCOL_LR");

	webService();
	for (;;)
	{
		vTaskDelay(1 / portTICK_PERIOD_MS);
		serviceHandle();
		// long now = millis();
		// wdtNetworkTimer = now;
		if ((config.wifi) && (config.wifi_mode == WIFI_AP_STA_FIX || config.wifi_mode == WIFI_STA_FIX))
		{
			if (WiFi.status() != WL_CONNECTED)
			{
				unsigned long int tw = millis();
				if (tw > wifiTTL)
				{
					wifiTTL = tw + 60000;
					Serial.print(F("WiFi Connecting to "));
					Serial.println(config.wifi_ssid);
					WiFi.disconnect();
					WiFi.setHostname("M17Hotspot");

					WiFi.setTxPower((wifi_power_t)44);
					WiFi.begin(config.wifi_ssid, config.wifi_pass);
					// Wait up to 1 minute for connection...
					for (c = 0; (c < 30) && (WiFi.status() != WL_CONNECTED); c++)
					{
						Serial.write('.');
						vTaskDelay(1000 / portTICK_PERIOD_MS);
						rtc_wdt_feed();
						// for (t = millis(); (millis() - t) < 1000; refresh());
					}

					if (c >= 30)
					{ // If it didn't connect within 1 min
						Serial.println(F("Failed. Will retry..."));
					}
					else
					{
						beginM17();
						m17ConectTimeout = millis() + 10000;
#ifdef DEBUG
						Serial.println(F("WiFi connected"));
						Serial.print(F("Host IP address: "));
						Serial.println(WiFi.localIP());
						vTaskDelay(1000 / portTICK_PERIOD_MS);
#endif
						Serial.println(F("### You can config by websevice ###"));
						if (config.wifi_mode == WIFI_AP_STA_FIX)
						{
							Serial.print(F("WiFi AP URL: http://"));
							Serial.println(local_IP);
						}
						Serial.print(F("WiFi STA URL: http://"));
						Serial.println(WiFi.localIP());

						pingTimeout = millis() + 60000;
					}
				}
			}
			else
			{
				if (connect_status == DISCONNECTED)
				{
					// if (millis() > (m17ConectTimeout + 10000))
					// {

					Serial.printf("M17 Connecting to %s[%c]\n", config.reflector_host, current_module);
					process_connect();
					// 						}
					// #ifndef I2S_INTERNAL
					// 						else
					// 						{
					// 							timerAlarmDisable(timer); // Stop sample audio when M17 Disconnect
					// 						}
					// #endif
				}
				else
				{
					readyReadM17();
					transmitM17();
					pingTimeout = millis() + 600000;
				}
				if (millis() > (m17ConectTimeout + 30000))
				{
					disconnect_from_host();
					connect_status = DISCONNECTED;
				}

				if (millis() > NTP_Timeout)
				{
					NTP_Timeout = millis() + 86400000;
					// Serial.println(F("Config NTP"));
					// setSyncProvider(getNtpTime);
					configTime(0 , 0, "203.150.19.19", "203.150.19.26");
					// topBar(WiFi.RSSI());
					vTaskDelay(3000 / portTICK_PERIOD_MS);
					time_t systemTime;
					time(&systemTime);
					setTime(systemTime);
					if (systemUptime == 0)
					{
						systemUptime = now();
					}
				}

				if (millis() > pingTimeout)
				{
					pingTimeout = millis() + 600000;
#ifdef DEBUG
					Serial.println("Ping Gateway to " + WiFi.gatewayIP().toString());
#endif
					if (ping_start(WiFi.gatewayIP(), 2, 0, 0, 5) == true)
					{
#ifdef DEBUG
						Serial.println(F("Ping Success!!"));
#endif
					}
					else
					{
#ifdef DEBUG
						Serial.println(F("Ping Fail!"));
#endif
						WiFi.disconnect();
						wifiTTL = 0;
					}
				}
			}
		} // wifi config
	}	  // Loop for
}

void frmUpdate(String str)
{

	i2s_adc_disable(I2S_NUM_0);
	dac_i2s_disable();
	ns_destroy(ns_inst);

#ifdef OLED
	display.clearDisplay();
	display.setCursor(20, 20);
	display.println("UPDATE FIRMWARE!");
	display.setCursor(25, 30);
	display.println("File Name: ");
	display.print(str);
	display.display();
#endif
	esp_agc_close(agc_handle);
	// wdtDisplayTimer = millis();
	// wdtSensorTimer = millis();
	disableCore0WDT();
	disableCore1WDT();
	disableLoopWDT();
	vTaskSuspend(taskDSPHandle);
	vTaskSuspend(taskUIHandle);

}
