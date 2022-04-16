#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

#include "Actuator.h"
#include "Network.h"
#include "Scheduler.h"
#include "Sensor.h"
#include "ValueProvider.h"

SensorDescriptor (*sensorDescriptors[])(void) = {MHZ19XSensor::getDescriptor, BME280Sensor::getDescriptor, SHT31Sensor::getDescriptor, SHTC3Sensor::getDescriptor};
#define SENSOR_COUNT std::size(sensorDescriptors)

ActuatorDescriptor (*actuatorDescriptors[])(void) = {ScreenActuator::getDescriptor, BeepActuator::getDescriptor, LightActuator::getDescriptor};
#define ACTUATOR_COUNT std::size(actuatorDescriptors)

static struct {
	Sensor **sensorMap = nullptr;
	ValueProvider<> **providerMap = nullptr;
	Actuator **availableActuators = nullptr;
	unsigned availableActuatorCount = 0;
} config;

Network net(WIFI_SSID);

void setup(void) {
	// TODO: Remove timeout needed to wait for the computer to connect to serial
	delay(1000);

	Serial.begin(9600);
	while(!Serial) {}

	Serial.printf("Initializing Sensors...\n");

	ValueTypeFlags requiredFlags = VT_NONE;
	ValueTypeFlags optionalFlags = VT_NONE;

	Sensor **sensors = new Sensor *[SENSOR_COUNT];
	for(unsigned i = 0; i < SENSOR_COUNT; i++) {
		Sensor *sensor = sensorDescriptors[i]().create();

		if(sensor->initialize() != E_OK) {
			delete sensor;
			sensor = nullptr;
		}

		sensors[i] = sensor;
	}

	Serial.printf("Initializing Actuators...\n");

	Actuator **actuators = new Actuator *[ACTUATOR_COUNT];
	for(unsigned i = 0; i < ACTUATOR_COUNT; i++) {
		Actuator *actuator = actuatorDescriptors[i]().create();

		if(actuator->initialize() != E_OK) {
			delete actuator;
			actuator = nullptr;
		}

		actuators[i] = actuator;

		requiredFlags |= actuators[i]->descriptor.getRequiredValueTypes();
		optionalFlags |= actuators[i]->descriptor.getOptionalValueTypes();
	}

	Serial.printf("Required Flags: %x\n", requiredFlags);
	Serial.printf("Optional Flags: %x\n", optionalFlags);

	Serial.printf("Generating Initial Sensor Map...\n");

	SensorMapGenerationOutput output = generateSensorMap(sensors, SENSOR_COUNT, requiredFlags);
	delete[] output.unusedSensors;

	Serial.printf("Evaluating Available Actuators...\n");

	FilteredActuators filtered = getAvailableActuatorsForSensorMap(actuators, ACTUATOR_COUNT, output.sensorMap);

	for(unsigned i = 0; i < filtered.rejectedActuatorsCount; i++) delete filtered.rejectedActuators[i];
	delete[] filtered.rejectedActuators;

	delete[] output.sensorMap;

	Serial.printf("Generating Final Sensor Map...\n");

	output = generateSensorMap(sensors, SENSOR_COUNT, filtered.requiredFlags | filtered.optionalFlags);

	for(unsigned i = 0; i < output.unusedSensorsLength; i++) delete output.unusedSensors[i];
	delete[] output.unusedSensors;

	delete[] sensors;

	Serial.printf("Creating Config...\n");

	config.sensorMap = output.sensorMap;
	config.availableActuators = filtered.matchingActuators;
	config.availableActuatorCount = filtered.matchingActuatorsCount;

	Serial.printf("Filling Provider Map...\n");

	config.providerMap = new ValueProvider<> *[VT_INDEXED_ARRAY_COUNT];

	for(unsigned i = 0; i < VT_INDEXED_ARRAY_COUNT; i++) {
		Serial.printf("Sensor: %u, %p\n", i, config.sensorMap[i]);

		config.providerMap[i] = config.sensorMap[i] ? config.sensorMap[i]->getProviderForValueType(indexToValueType(i)) : nullptr;

		Serial.printf("Provider: %u, %p\n", i, config.providerMap[i]);
	}
}

unsigned lastUpdateCheck = 0;

void loop(void) {
	if((millis() / (1000 * 60 * 60)) + 1 > lastUpdateCheck) {
		lastUpdateCheck++;

		if(!net.isConnected()) net.connect();

		if(net.isUpdateAvailable()) net.updateFirmware();
	}

	// TODO: Only read those sensors we actually need
	for(unsigned i = 0; i < VT_INDEXED_ARRAY_COUNT; i++) {
		if(config.providerMap[i]) {
			config.providerMap[i]->read();
		}
	}

	for(unsigned i = 0; i < config.availableActuatorCount; i++) {
		config.availableActuators[i]->actuate(config.providerMap);
	}
}