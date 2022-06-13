#pragma once

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#include <Adafruit_BME280.h>
#include <Adafruit_SHT31.h>
#include <Adafruit_SHTC3.h>
#include <ErriezMHZ19B.h>
#include <SoftwareSerial.h>
#pragma GCC diagnostic pop

#include "Constants.h"
#include "Errors.h"
#include "ValueProvider.h"

class Sensor;

class SensorDescriptor {
public:
	Sensor *(*_create)(void);
	Sensor *create(void);

	ValueTypeFlags (*getProvidableValueTypes)(void);
};

class Sensor {
private:
public:
	SensorDescriptor descriptor;

	static SensorDescriptor getDescriptor(void);

	virtual ~Sensor();

	virtual Error initialize(void);
	virtual ValueProvider<> *getProviderForValueType(ValueType value);
};

/********************\
| Individual Sensors |
\********************/

// -------- MHZ19XSensor -------- \\;

class MHZ19XSensor : public Sensor {
private:
	SoftwareSerial mhzSerial;
	ErriezMHZ19B mhz19x;

	int16_t readCO2(void);
	bool isReady(void);

public:
	static SensorDescriptor getDescriptor(void);

	virtual ~MHZ19XSensor();
	MHZ19XSensor() : mhzSerial(MHZ19X_TX_PIN, MHZ19X_RX_PIN), mhz19x(&mhzSerial){};

	virtual Error initialize(void);
	virtual ValueProvider<> *getProviderForValueType(ValueType value);
};

// -------- BMESensor -------- \\;

class BME280Sensor : public Sensor {
private:
	Adafruit_BME280 bme;

	float readPressure(void);
	float readAltitude(void);
	float readTemperature(void);
	float readHumidity(void);

public:
	static SensorDescriptor getDescriptor(void);

	virtual Error initialize(void);
	virtual ValueProvider<> *getProviderForValueType(ValueType value);
};

// -------- SHT31Sensor -------- \\;

class SHT31Sensor : public Sensor {
private:
	Adafruit_SHT31 sht31;

	float readTemperature(void);
	float readHumidity(void);

public:
	static SensorDescriptor getDescriptor(void);

	virtual Error initialize(void);
	virtual ValueProvider<> *getProviderForValueType(ValueType value);
};

// -------- SHTC3Sensor -------- \\;

class SHTC3Sensor : public Sensor {
private:
	Adafruit_SHTC3 shtc3;

	float readTemperature(void);
	float readHumidity(void);

public:
	static SensorDescriptor getDescriptor(void);

	virtual Error initialize(void);
	virtual ValueProvider<> *getProviderForValueType(ValueType value);
};
