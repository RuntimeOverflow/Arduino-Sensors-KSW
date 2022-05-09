#pragma once

#include <Ticker.h>
#include <U8g2lib.h>

#include "Constants.h"
#include "DisplayModule.h"
#include "Errors.h"
#include "ValueProvider.h"

class Actuator;

class ActuatorDescriptor {
public:
	Actuator *(*_create)(void);
	Actuator *create(void);

	ValueTypeFlags (*getRequiredValueTypes)(void);
	ValueTypeFlags (*getOptionalValueTypes)(void);
};

class Actuator {
public:
	ActuatorDescriptor descriptor;

	virtual ~Actuator();

	static ActuatorDescriptor getDescriptor(void);

	virtual Error initialize(void);
	virtual Error actuate(RequestedValueProviders providers);
};

/**********************\
| Individual Actuators |
\**********************/

// -------- ScreenActuator -------- \\;

class ScreenActuator : Actuator {
private:
	U8G2_SSD1306_64X48_ER_F_HW_I2C *u8g2;
	bool displayInverted = false;
	uint32_t lastDisplayInversion = 0;

	Screen **screens = nullptr;
	unsigned screenCount = 0;

public:
	static ActuatorDescriptor getDescriptor(void);

	virtual ~ScreenActuator();

	virtual Error initialize(void);
	virtual Error actuate(RequestedValueProviders providers);
};

// -------- BeepActuator -------- \\;

class BeepActuator : Actuator {
private:
	std::function<void(void)> callback;
	Ticker ticker;

	unsigned char highestFiredAlarm = 0;
	unsigned beepsLeft = 0;

public:
	virtual ~BeepActuator();

	static ActuatorDescriptor getDescriptor(void);

	virtual Error initialize(void);
	virtual Error actuate(RequestedValueProviders providers);

	void beep(uint32_t frequency, unsigned count);
};

// -------- LightActuator -------- \\;

class LightActuator : Actuator {
private:
	unsigned char state = 0;

public:
	static ActuatorDescriptor getDescriptor(void);

	virtual Error initialize(void);
	virtual Error actuate(RequestedValueProviders providers);
};