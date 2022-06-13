#pragma once

#include <Arduino.h>

// -------- GLOBAL -------- \\;

#define VERSION 1

// -------- Network -------- \\;

#define MULTICAST_IP "239.255.32.16"
#define UDP_PORT 1337
#define SERVER_IP "10.96.30.10"
#define HTTP_PORT 80

// -------- Measurement Types -------- \\;

typedef int16_t co2_t;
typedef float pressure_t;
typedef float altitude_t;
typedef float temperature_t;
typedef float humidity_t;

// -------- MHZ19XSensor -------- \\;

#define MHZ19X_TX_PIN D6
#define MHZ19X_RX_PIN D7

// -------- BMESensor -------- \\;

#define BME_SCK 13
#define BME_MISO 12
#define BME_MOSI 11
#define BME_CS 10
#define SEALEVELPRESSURE_HPA 1013.25

// -------- BeepActuator -------- \\;

#define BUZZER_PIN D8
#define BEEP_FREQUENCY 1000
#define BEEP_DURATION 500
#define BEEP_ON 127
#define BEEP_OFF 0

// -------- LightActuator -------- \\;

enum LED {
	LED_GREEN = D3,
	LED_YELLOW = D4,
	LED_RED = D5,
};

enum LED_STATE {
	LS_OFF = HIGH,
	LS_ON = LOW,
};

// -------- CO2 States -------- \\;

#define CO2_LOW_THRESHOLD 800
#define CO2_MEDIUM_THRESHOLD 1000
#define CO2_HIGH_THRESHOLD 1200