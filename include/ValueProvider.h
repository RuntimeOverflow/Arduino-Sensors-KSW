#pragma once

#include <type_traits>

#include "Errors.h"

class Sensor;

enum __attribute__((__packed__)) ValueType {
	VT_NONE = 0,

	VT_CO2 = 1 << 0,
	VT_PRESSURE = 1 << 1,
	VT_ALTITUDE = 1 << 2,
	VT_TEMPERATURE = 1 << 3,
	VT_HUMIDITY = 1 << 4,

	VT_END,
};

typedef std::underlying_type<ValueType>::type ValueType_t;
typedef ValueType ValueTypeFlags;
typedef unsigned char ValueTypeIndex;

#define VT_ALL (((VT_END - 1) << 1) - 1)
#define VT_INDEXED_ARRAY_COUNT ((ValueTypeIndex)(valueTypeToIndex((ValueType)(VT_END - 1)) + 1))

static constexpr inline ValueTypeIndex valueTypeToIndex(ValueType valueProvider) {
	return valueProvider ? __builtin_ctzll(valueProvider) : 0;
}

static constexpr inline ValueType indexToValueType(ValueTypeIndex index) {
	return (ValueType)(1 << index);
}

static constexpr inline ValueTypeFlags operator|(ValueTypeFlags a, ValueTypeFlags b) {
	return static_cast<ValueTypeFlags>(static_cast<ValueType_t>(a) | static_cast<ValueType_t>(b));
}

static constexpr inline ValueTypeFlags operator&(ValueTypeFlags a, ValueTypeFlags b) {
	return static_cast<ValueTypeFlags>(static_cast<ValueType_t>(a) & static_cast<ValueType_t>(b));
}

static constexpr inline ValueTypeFlags &operator|=(ValueTypeFlags &a, ValueTypeFlags b) {
	a = a | b;
	return a;
}

static constexpr inline ValueTypeFlags operator&=(ValueTypeFlags &a, ValueTypeFlags b) {
	a = a & b;
	return a;
}

template <typename T = char, typename U = Sensor>
class ValueProvider {
private:
	U *sensor;
	T(U::*fetchValue)
	(void);
	bool (U::*isReadyForFetching)(void);
	T *cachedValue = nullptr;

public:
	ValueProvider(U *sensor, T (U::*fetchValue)(void), bool (U::*isReadyForFetching)(void)) : sensor(sensor), fetchValue(fetchValue), isReadyForFetching(isReadyForFetching) {}

	T *value(void);

	template <typename S>
	ValueProvider<S> *as(void);

	virtual Error read(void);
};

typedef ValueProvider<> **RequestedValueProviders;

#include "ValueProvider_impl.h"