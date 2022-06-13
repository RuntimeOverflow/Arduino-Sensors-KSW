#include "Scheduler.h"

#include <alloca.h>

SensorMapGenerationOutput generateSensorMap(Sensor *sensors[], int count, ValueTypeFlags neededFlags) {
	ValueTypeFlags flags = VT_NONE;
	bool *usedMap = (bool *)alloca(count * sizeof(bool));

	SensorMapGenerationOutput output;
	output.sensorMap = new Sensor *[VT_INDEXED_ARRAY_COUNT];
	for(unsigned i = 0; i < VT_INDEXED_ARRAY_COUNT; i++) output.sensorMap[i] = nullptr;
	output.unusedSensors = new Sensor *[VT_INDEXED_ARRAY_COUNT];

	ValueTypeFlags flagsBySensor[count];
	for(int i = 0; i < count; i++) {
		flagsBySensor[i] = sensors[i] ? sensors[i]->descriptor.getProvidableValueTypes() : VT_NONE;
		usedMap[i] = false;
	}

	while((flags & neededFlags) != neededFlags) {
		int newFlagsCount = 0;
		int index = -1;

		for(int i = 0; i < count; i++) {
			int _newFlagsCount = __builtin_popcount(flagsBySensor[i] & neededFlags & ~flags);

			if(_newFlagsCount > newFlagsCount) {
				newFlagsCount = _newFlagsCount;
				index = i;
			}
		}

		if(!newFlagsCount) break;
		else {
			ValueTypeFlags sensorFlags = flagsBySensor[index] & neededFlags;

			flags |= sensorFlags;

			usedMap[index] = true;

			while(sensorFlags) {
				ValueTypeIndex providerIndex = valueTypeToIndex((ValueType)sensorFlags);
				output.sensorMap[providerIndex] = sensors[index];
				sensorFlags &= (ValueTypeFlags)~indexToValueType(providerIndex);
			}
		}
	}

	unsigned unusedIndex = 0;
	for(int i = 0; i < count; i++)
		if(!usedMap[i]) output.unusedSensors[unusedIndex++] = sensors[i];
	output.unusedSensorsLength = unusedIndex;

	return output;
}

FilteredActuators getAvailableActuatorsForSensorMap(Actuator *actuators[], int count, Sensor **sensorMap) {
	bool *matchMap = (bool *)alloca(count * sizeof(bool));
	int availableActuatorCount = 0;

	struct FilteredActuators filtered;

	for(int i = 0; i < count; i++) {
		ValueTypeFlags requiredValues = actuators[i]->descriptor.getRequiredValueTypes();

		bool satisfiesRequirements = true;

		while(requiredValues) {
			ValueTypeIndex providerIndex = valueTypeToIndex((ValueType)requiredValues);

			if(!sensorMap[providerIndex]) {
				satisfiesRequirements = false;
				break;
			}

			requiredValues &= (ValueTypeFlags)~indexToValueType(providerIndex);
		}

		matchMap[i] = satisfiesRequirements;
		availableActuatorCount += satisfiesRequirements;

		if(satisfiesRequirements) {
			filtered.requiredFlags |= actuators[i]->descriptor.getRequiredValueTypes();
			filtered.optionalFlags |= actuators[i]->descriptor.getOptionalValueTypes();
		}
	}

	filtered.matchingActuatorsCount = availableActuatorCount;
	filtered.rejectedActuatorsCount = count - availableActuatorCount;

	filtered.matchingActuators = new Actuator *[availableActuatorCount];
	filtered.rejectedActuators = new Actuator *[filtered.rejectedActuatorsCount];

	unsigned availableIndex = 0;
	unsigned unavailableIndex = 0;
	for(int i = 0; i < count; i++) {
		if(matchMap[i]) filtered.matchingActuators[availableIndex++] = actuators[i];
		else filtered.rejectedActuators[unavailableIndex++] = actuators[i];
	}

	return filtered;
}