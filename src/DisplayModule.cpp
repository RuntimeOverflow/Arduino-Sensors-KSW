#include "DisplayModule.h"

#include "Constants.h"
#include "Icons.h"

Screen::~Screen() {
	if(displayModule) delete displayModule;
}

void Screen::draw(RequestedValueProviders providers) {
	displayModule->draw(this, providers);
}

void Screen::setTitle(const char *title) {
	memcpy(this->title, title, 24);
}

unsigned Screen::getStringWidth(const char *str) {
	return u8g2->getStrWidth(str);
}

void Screen::drawString(unsigned x, unsigned y, const char *str) {
	u8g2->setCursor(offsetX + x, offsetY + y + u8g2->getAscent());
	u8g2->print(str);
}

void Screen::drawImage(unsigned x, unsigned y, unsigned width, unsigned height, const unsigned char *img) {
	u8g2->drawXBMP(offsetX + x, offsetY + y, width, height, img);
}

DisplayModule *DisplayModuleDescriptor::create() {
	if(!_create) return nullptr;

	DisplayModule *displayModule = _create();
	displayModule->descriptor = *this;

	return displayModule;
}

DisplayModule::~DisplayModule() = default;

DisplayModuleDescriptor DisplayModule::getDescriptor(void) {
	DisplayModuleDescriptor descriptor;

	descriptor._create = [](void) -> DisplayModule * {
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

Error DisplayModule::initialize(void) {
	return E_ABSTRACT_METHOD_CALL;
}

Error DisplayModule::draw(Screen *screen, RequestedValueProviders providers) {
	return E_ABSTRACT_METHOD_CALL;
}

/***************************\
| Individual DisplayModules |
\***************************/

// -------- TemperatureDisplayModule -------- \\;

DisplayModuleDescriptor TemperatureDisplayModule::getDescriptor(void) {
	DisplayModuleDescriptor descriptor;

	descriptor._create = [](void) -> DisplayModule * {
		return new TemperatureDisplayModule;
	};

	descriptor.getRequiredValueTypes = [](void) -> ValueTypeFlags {
		return VT_TEMPERATURE;
	};

	descriptor.getOptionalValueTypes = [](void) -> ValueTypeFlags {
		return VT_NONE;
	};

	return descriptor;
}

Error TemperatureDisplayModule::initialize(void) {
	return E_OK;
}

Error TemperatureDisplayModule::draw(Screen *screen, RequestedValueProviders providers) {
	screen->setTitle("Temperature");

	temperature_t *temp = providers[valueTypeToIndex(VT_TEMPERATURE)]->as<temperature_t>()->value();

	if(temp) {
		screen->drawImage(48, 0, 16, 32, icon_temperature);
		screen->drawString(0, 0, (String(*temp) + " °C").c_str());
	} else {
		screen->drawString(0, 0, "Loading...");
	}

	return E_OK;
}