#include "Actuator.h"

#include <Arduino.h>

#include "Constants.h"
#include "Icons.h"

extern DisplayModuleDescriptor (*displayModuleDescriptors[])(void);
extern const unsigned DISPLAY_MODULE_COUNT;

Actuator *ActuatorDescriptor::create() {
	if(!_create) return nullptr;

	Actuator *actuator = _create();
	actuator->descriptor = *this;

	return actuator;
}

Actuator::~Actuator() = default;

ActuatorDescriptor Actuator::getDescriptor(void) {
	ActuatorDescriptor descriptor;

	descriptor._create = [](void) -> Actuator * {
		return nullptr;
	};

	descriptor.getRequiredValueTypes = [](void) -> ValueTypeFlags {
		return VT_NONE;
	};

	descriptor.getOptionalValueTypes = [](void) -> ValueTypeFlags {
		return VT_NONE;
	};

	return descriptor;
}

Error Actuator::initialize(void) {
	return E_ABSTRACT_METHOD_CALL;
}

Error Actuator::actuate(RequestedValueProviders providers) {
	return E_ABSTRACT_METHOD_CALL;
}

/**********************\
| Individual Actuators |
\**********************/

// -------- ScreenActuator -------- \\;

ActuatorDescriptor ScreenActuator::getDescriptor(void) {
	ActuatorDescriptor descriptor;

	descriptor._create = [](void) -> Actuator * {
		return new ScreenActuator;
	};

	descriptor.getRequiredValueTypes = [](void) -> ValueTypeFlags {
		return VT_NONE;
	};

	descriptor.getOptionalValueTypes = [](void) -> ValueTypeFlags {
		ValueTypeFlags flags = VT_NONE;

		for(unsigned i = 0; i < DISPLAY_MODULE_COUNT; i++) {
			DisplayModuleDescriptor desc = displayModuleDescriptors[i]();

			flags |= desc.getOptionalValueTypes();
			flags |= desc.getRequiredValueTypes();
		}

		return flags;
	};

	return descriptor;
}

ScreenActuator::~ScreenActuator() {
	for(unsigned i = 0; i < screenCount; i++) delete screens[i];
	delete[] screens;
}

Error ScreenActuator::initialize(void) {
	if(!u8g2.begin()) return E_NO_ACTUATOR;

	u8g2.enableUTF8Print();
	u8g2.setFont(u8g2_font_tom_thumb_4x6_tf);

	return E_OK;
}

Error ScreenActuator::actuate(RequestedValueProviders providers) {
	if(!screenCount) {
		Screen *all_screens[DISPLAY_MODULE_COUNT];

		for(unsigned i = 0; i < DISPLAY_MODULE_COUNT; i++) {
			DisplayModuleDescriptor desc = displayModuleDescriptors[i]();
			ValueTypeFlags required = desc.getRequiredValueTypes();
			bool satified = true;

			while(required) {
				ValueTypeIndex providerIndex = valueTypeToIndex((ValueType)required);

				if(!providers[providerIndex]) {
					satified = false;
					break;
				}

				required &= (ValueTypeFlags)~indexToValueType(providerIndex);
			}

			if(satified) {
				all_screens[screenCount] = new Screen(&u8g2, desc.create());
				screenCount++;
			}
		}

		screens = new Screen *[screenCount];
		memcpy(screens, all_screens, sizeof(Screen *) * screenCount);
	}

	u8g2.clearBuffer();

	if(screenCount > 0) {
		screens[0]->draw(providers);

		u8g2.setCursor(0, 48 - u8g2.getMaxCharHeight() + u8g2.getAscent());
		u8g2.print("<");

		if(screens[0]->title) {
			u8g2.setCursor(32 - u8g2.getStrWidth(screens[0]->title) / 2, 48 - u8g2.getMaxCharHeight() + u8g2.getAscent());
			u8g2.print(screens[0]->title);
		}

		u8g2.setCursor(64 - u8g2.getStrWidth(">"), 48 - u8g2.getMaxCharHeight() + u8g2.getAscent());
		u8g2.print(">");
	}

	u8g2.sendBuffer();

	if(millis() - lastDisplayInversion > 86400000) {
		lastDisplayInversion = millis();
		u8g2_SendF(u8g2.getU8g2(), "c", displayInverted ? 0xa6 : 0xa7);
		displayInverted = !displayInverted;
	}

	return E_OK;
}

// -------- BeepActuator -------- \\;

BeepActuator::~BeepActuator() {
	// TODO: Properly clean
	if(callback) callback();
}

ActuatorDescriptor BeepActuator::getDescriptor(void) {
	ActuatorDescriptor descriptor;

	descriptor._create = [](void) -> Actuator * {
		return new BeepActuator;
	};

	descriptor.getRequiredValueTypes = [](void) -> ValueTypeFlags {
		return VT_CO2;
	};

	descriptor.getOptionalValueTypes = [](void) -> ValueTypeFlags {
		return VT_NONE;
	};

	return descriptor;
}

Error BeepActuator::initialize(void) {
	pinMode(BUZZER_PIN, OUTPUT);

	return E_OK;
}

Error BeepActuator::actuate(RequestedValueProviders providers) {
	co2_t *co2 = providers[valueTypeToIndex(VT_CO2)]->as<co2_t>()->value();
	if(co2) {
		if(*co2 > CO2_HIGH_THRESHOLD) {
			if(highestFiredAlarm < 2) {
				highestFiredAlarm = 2;

				beep(BEEP_FREQUENCY, 3);
			}
		} else if(*co2 > CO2_MEDIUM_THRESHOLD) {
			if(highestFiredAlarm < 1) {
				highestFiredAlarm = 1;

				beep(BEEP_FREQUENCY, 2);
			}
		} else if(*co2 < CO2_LOW_THRESHOLD) {
			highestFiredAlarm = 0;
		}
	}

	return E_OK;
}

void BeepActuator::beep(uint32_t frequency, unsigned count) {
	Ticker::callback_function_t **tickerCallback = new Ticker::callback_function_t *;
	*tickerCallback = new std::function([this, tickerCallback, frequency](void) mutable {
		analogWriteRange(256);
		analogWriteFreq(frequency);
		analogWrite(BUZZER_PIN, BEEP_ON);

		this->callback = [this, tickerCallback](void) {
			analogWrite(BUZZER_PIN, BEEP_OFF);
			this->ticker.detach();
			this->callback = nullptr;

			if(beepsLeft && --beepsLeft) {
				this->callback = **tickerCallback;
				ticker.once_ms(BEEP_DURATION, callback);
			} else {
				delete *tickerCallback;
				delete tickerCallback;
			}
		};

		ticker.once_ms(BEEP_DURATION, callback);
	});

	if(!ticker.active()) {
		beepsLeft = count;

		(**tickerCallback)();
	}
}

// -------- LightActuator -------- \\;

ActuatorDescriptor LightActuator::getDescriptor(void) {
	ActuatorDescriptor descriptor;

	descriptor._create = [](void) -> Actuator * {
		return new LightActuator;
	};

	descriptor.getRequiredValueTypes = [](void) -> ValueTypeFlags {
		return VT_CO2;
	};

	descriptor.getOptionalValueTypes = [](void) -> ValueTypeFlags {
		return VT_NONE;
	};

	return descriptor;
}

Error LightActuator::initialize(void) {
	pinMode(LED_GREEN, OUTPUT);
	pinMode(LED_YELLOW, OUTPUT);
	pinMode(LED_RED, OUTPUT);

	digitalWrite(LED_GREEN, LS_OFF);
	digitalWrite(LED_YELLOW, LS_OFF);
	digitalWrite(LED_RED, LS_OFF);

	return E_OK;
}

Error LightActuator::actuate(RequestedValueProviders providers) {
	const LED ledMap[] = {LED_GREEN, LED_YELLOW, LED_RED};

	co2_t *co2 = providers[valueTypeToIndex(VT_CO2)]->as<co2_t>()->value();
	if(co2) {
		unsigned char lightFlags = 1 << ((*co2 >= 800) + (*co2 >= 1200));

		for(unsigned i = 0; i < 3; i++) {
			digitalWrite(ledMap[i], (lightFlags & 1) ? LS_ON : LS_OFF);
			lightFlags >>= 1;
		}
	}

	return E_OK;
}