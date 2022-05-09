#include <U8g2lib.h>

#include "ValueProvider.h"

class DisplayModule;

class Screen {
private:
	unsigned offsetX = 0;
	unsigned offsetY = 0;
	U8G2_SSD1306_64X48_ER_F_HW_I2C *u8g2;
	DisplayModule *displayModule;
	char *title;

	void draw(RequestedValueProviders providers);

	friend class ScreenActuator;

public:
	Screen(U8G2_SSD1306_64X48_ER_F_HW_I2C *u8g2, DisplayModule *displayModule) : u8g2(u8g2), displayModule(displayModule), displayWidth(u8g2->getDisplayWidth()), displayHeight(u8g2->getDisplayHeight()), stringHeight(u8g2->getMaxCharHeight()){};
	~Screen();

	void setTitle(const char *title);

	unsigned displayWidth;
	unsigned displayHeight;

	unsigned getStringWidth(const char *str);
	unsigned stringHeight;

	void drawString(unsigned x, unsigned y, const char *str);
	void drawImage(unsigned x, unsigned y, unsigned width, unsigned height, const unsigned char *img);
};

class DisplayModuleDescriptor {
public:
	DisplayModule *(*_create)(void);
	DisplayModule *create(void);

	ValueTypeFlags (*getRequiredValueTypes)(void);
	ValueTypeFlags (*getOptionalValueTypes)(void);
};

class DisplayModule {
public:
	DisplayModuleDescriptor descriptor;

	static DisplayModuleDescriptor getDescriptor(void);

	virtual ~DisplayModule();

	virtual Error initialize(void);
	virtual Error draw(Screen *screen, RequestedValueProviders providers);
};

/***************************\
| Individual DisplayModules |
\***************************/

// -------- TemperatureDisplayModule -------- \\;

class TemperatureDisplayModule : public DisplayModule {
public:
	DisplayModuleDescriptor descriptor;

	static DisplayModuleDescriptor getDescriptor(void);

	virtual Error initialize(void);
	virtual Error draw(Screen *screen, RequestedValueProviders providers);
};