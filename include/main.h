#ifndef MAIN_H
#define MAIN_H

#define VERSION "0.1"

#define DEBUG

//#define SDCARD

#define IMPLEMENTATION FIFO

#include <Arduino.h>
#include <cppQueue.h>
#include <codec2.h>
#include "soc/rtc_wdt.h"

#include "HardwareSerial.h"
#include "EEPROM.h"

enum M17Flags
{
	DISCONNECTED = 1 << 0,
	CONNECTING = 1 << 1,
	M17_AUTH = 1 << 2,
	M17_CONF = 1 << 3,
	M17_OPTS = 1 << 4,
	CONNECTED_RW = 1 << 5,
	CONNECTED_RO = 1 << 6
};

typedef struct Reflector_Struct
{
	char host[30];
	char name[8];
	uint16_t port;
} Reflector;

typedef struct Config_Struct
{
	char wifi_ssid[30];
	char wifi_pass[30];
	Reflector current_reflector;
	char current_module;
	char mycall[10];
	char mymodule;
	int codec2_mode;
} Configuration;

void saveEEPROM();
void defaultConfig();

void connectToWiFi(const char *ssid, const char *pwd);

#endif