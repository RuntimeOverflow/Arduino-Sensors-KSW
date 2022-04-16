#include "Actuator.h"
#include "Sensor.h"

struct SensorMapGenerationOutput {
	Sensor **sensorMap = nullptr;
	Sensor **unusedSensors = nullptr;
	unsigned unusedSensorsLength = 0;
};

struct FilteredActuators {
	Actuator **matchingActuators = nullptr;
	unsigned matchingActuatorsCount = 0;
	Actuator **rejectedActuators = nullptr;
	unsigned rejectedActuatorsCount = 0;
	ValueTypeFlags requiredFlags = VT_NONE;
	ValueTypeFlags optionalFlags = VT_NONE;
};

SensorMapGenerationOutput generateSensorMap(Sensor *sensors[], int count, ValueTypeFlags neededFlags);
FilteredActuators getAvailableActuatorsForSensorMap(Actuator *actuators[], int count, Sensor **sensorMap);