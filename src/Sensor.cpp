#include "Sensor.h"

#include "Constants.h"

Sensor *SensorDescriptor::create() {
	if(!_create) return nullptr;

	Sensor *sensor = _create();
	sensor->descriptor = *this;

	return sensor;
}

Sensor::~Sensor() {}

SensorDescriptor Sensor::getDescriptor(void) {
	SensorDescriptor descriptor;

	descriptor._create = [](void) -> Sensor * {
		return nullptr;
	};

	descriptor.getProvidableValueTypes = [](void) -> ValueTypeFlags {
		return VT_NONE;
	};

	return descriptor;
}

Error Sensor::initialize() {
	return E_ABSTRACT_METHOD_CALL;
}

ValueProvider<> *Sensor::getProviderForValueType(ValueType value) {
	return nullptr;
}

/********************\
| Individual Sensors |
\********************/

// -------- MHZ19XSensor -------- \\;

SensorDescriptor MHZ19XSensor::getDescriptor(void) {
	SensorDescriptor descriptor;

	descriptor._create = [](void) -> Sensor * {
		return new MHZ19XSensor;
	};

	descriptor.getProvidableValueTypes = [](void) -> ValueTypeFlags {
		return VT_CO2;
	};

	return descriptor;
}

MHZ19XSensor::~MHZ19XSensor() {
	delete mhz19x;
	mhzSerial->end();
	delete mhzSerial;
}

Error MHZ19XSensor::initialize(void) {
	mhzSerial = new SoftwareSerial(MHZ19X_TX_PIN, MHZ19X_RX_PIN);
	mhz19x = new ErriezMHZ19B(mhzSerial);

	mhzSerial->begin(9600);

	bool detected = false;
	unsigned char counter = 0;
	while(!(detected = mhz19x->detect()) && (counter++) < 10) delay(500);
	if(!detected) return E_NOT_FOUND;

	mhz19x->setRange5000ppm();
	mhz19x->setAutoCalibration(true);

	return E_OK;
}

ValueProvider<> *MHZ19XSensor::getProviderForValueType(ValueType value) {
	switch(value) {
		case VT_CO2:
			return reinterpret_cast<ValueProvider<> *>(new ValueProvider<co2_t, MHZ19XSensor>(this, &MHZ19XSensor::readCO2, &MHZ19XSensor::isReady));
		default:
			return nullptr;
	}
}

int16_t MHZ19XSensor::readCO2(void) {
	return mhz19x->readCO2();
}

bool MHZ19XSensor::isReady(void) {
	return mhz19x->isReady() && !mhz19x->isWarmingUp();
}

// -------- BMESensor -------- \\;

SensorDescriptor BME280Sensor::getDescriptor(void) {
	SensorDescriptor descriptor;

	descriptor._create = [](void) -> Sensor * {
		return new BME280Sensor;
	};

	descriptor.getProvidableValueTypes = [](void) -> ValueTypeFlags {
		return VT_PRESSURE | VT_ALTITUDE | VT_TEMPERATURE | VT_HUMIDITY;
	};

	return descriptor;
}

BME280Sensor::~BME280Sensor() {
	delete bme;
}

Error BME280Sensor::initialize(void) {
	bme = new Adafruit_BME280;

	if(!bme->begin(0x76)) return E_GENERIC;

	return E_OK;
}

ValueProvider<> *BME280Sensor::getProviderForValueType(ValueType value) {
	switch(value) {
		case VT_PRESSURE:
			return reinterpret_cast<ValueProvider<> *>(new ValueProvider<pressure_t, BME280Sensor>(this, &BME280Sensor::readPressure, nullptr));
		case VT_ALTITUDE:
			return reinterpret_cast<ValueProvider<> *>(new ValueProvider<altitude_t, BME280Sensor>(this, &BME280Sensor::readAltitude, nullptr));
		case VT_TEMPERATURE:
			return reinterpret_cast<ValueProvider<> *>(new ValueProvider<temperature_t, BME280Sensor>(this, &BME280Sensor::readTemperature, nullptr));
		case VT_HUMIDITY:
			return reinterpret_cast<ValueProvider<> *>(new ValueProvider<humidity_t, BME280Sensor>(this, &BME280Sensor::readHumidity, nullptr));
		default:
			return nullptr;
	}
}

float BME280Sensor::readPressure(void) {
	return bme->readPressure();
}

float BME280Sensor::readAltitude(void) {
	return bme->readAltitude(SEALEVELPRESSURE_HPA);
}

float BME280Sensor::readTemperature(void) {
	return bme->readTemperature();
}

float BME280Sensor::readHumidity(void) {
	return bme->readHumidity();
}

// -------- SHT31Sensor -------- \\;

SensorDescriptor SHT31Sensor::getDescriptor(void) {
	SensorDescriptor descriptor;

	descriptor._create = [](void) -> Sensor * {
		return new SHT31Sensor;
	};

	descriptor.getProvidableValueTypes = [](void) -> ValueTypeFlags {
		return VT_TEMPERATURE | VT_HUMIDITY;
	};

	return descriptor;
}

SHT31Sensor::~SHT31Sensor() {
	delete sht31;
}

Error SHT31Sensor::initialize(void) {
	sht31 = new Adafruit_SHT31;

	if(!sht31->begin(0x44)) return E_GENERIC;

	return E_OK;
}

ValueProvider<> *SHT31Sensor::getProviderForValueType(ValueType value) {
	switch(value) {
		case VT_TEMPERATURE:
			return reinterpret_cast<ValueProvider<> *>(new ValueProvider<temperature_t, SHT31Sensor>(this, &SHT31Sensor::readTemperature, nullptr));
		case VT_HUMIDITY:
			return reinterpret_cast<ValueProvider<> *>(new ValueProvider<humidity_t, SHT31Sensor>(this, &SHT31Sensor::readHumidity, nullptr));
		default:
			return nullptr;
	}
}

float SHT31Sensor::readTemperature(void) {
	return sht31->readTemperature();
}

float SHT31Sensor::readHumidity(void) {
	return sht31->readHumidity();
}

// -------- SHTC3Sensor -------- \\;

SensorDescriptor SHTC3Sensor::getDescriptor(void) {
	SensorDescriptor descriptor;

	descriptor._create = [](void) -> Sensor * {
		return new SHTC3Sensor;
	};

	descriptor.getProvidableValueTypes = [](void) -> ValueTypeFlags {
		return VT_TEMPERATURE | VT_HUMIDITY;
	};

	return descriptor;
}

SHTC3Sensor::~SHTC3Sensor() {
	delete shtc3;
}

Error SHTC3Sensor::initialize(void) {
	shtc3 = new Adafruit_SHTC3;

	if(!shtc3->begin()) return E_GENERIC;

	return E_OK;
}

ValueProvider<> *SHTC3Sensor::getProviderForValueType(ValueType value) {
	switch(value) {
		case VT_TEMPERATURE:
			return reinterpret_cast<ValueProvider<> *>(new ValueProvider<temperature_t, SHTC3Sensor>(this, &SHTC3Sensor::readTemperature, nullptr));
		case VT_HUMIDITY:
			return reinterpret_cast<ValueProvider<> *>(new ValueProvider<humidity_t, SHTC3Sensor>(this, &SHTC3Sensor::readHumidity, nullptr));
		default:
			return nullptr;
	}
}

float SHTC3Sensor::readTemperature(void) {
	sensors_event_t temperature;
	shtc3->getEvent(nullptr, &temperature);

	return temperature.temperature;
}

float SHTC3Sensor::readHumidity(void) {
	sensors_event_t humidity;
	shtc3->getEvent(&humidity, nullptr);

	return humidity.relative_humidity;
}