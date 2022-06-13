#include "Network.h"

#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <U8g2lib.h>  //TODO: Remove
#include <alloca.h>
#include <string.h>

#include "Constants.h"

Network::Network(char const *_ssid, char const *_password) {
	ssid = new char[strlen(_ssid) + 1];
	strcpy(ssid, _ssid);

	if(_password) {
		password = new char[strlen(_password) + 1];
		strcpy(password, _password);
	}
}

Network::~Network() {
	disconnect();

	delete ssid;
	delete password;
}

bool Network::isConnected() {
	return WiFi.status() == WL_CONNECTED;
}

Error Network::connect() {
	wl_status_t status = WiFi.begin(ssid, password);
	unsigned timeout = 0;
	while(WiFi.status() == WL_DISCONNECTED && timeout++ < 100) delay(50);
	return status == WL_CONNECTED ? E_OK : E_CONNECTION_FAILED;
}

void Network::disconnect() {
	WiFi.disconnect();
}

/*Error Network::sendUDPPacket(char const *address, unsigned port, char const *msg, size_t length) {
	if(length == UINT_MAX) length = strlen(msg);

	udp.beginPacket(address, port);
	udp.write(msg, length);
	bool status = udp.endPacket();

	return status ? E_OK : E_FAILED_TO_SEND;
}*/

Error Network::sendMulticastPacket(char const *address, unsigned port, char const *msg, size_t length) {
	if(length == UINT_MAX) length = strlen(msg);

	IPAddress addr;
	addr.fromString(address);
	udp.beginPacketMulticast(addr, port, WiFi.localIP(), 64);
	udp.write(msg, length);
	bool status = udp.endPacket();

	return status ? E_OK : E_FAILED_TO_SEND;
}

bool Network::isUpdateAvailable() {
#define xstr(s) str(s)
#define str(s) #s
	String version = xstr(VERSION);
#undef xstr
#undef str

	WiFiClient client;
	HTTPClient http;

	http.begin(client, SERVER_IP, HTTP_PORT, "/api/sensors/check_update?version=" + version);
	http.GET();

	bool hasUpdate = http.getString().toInt();

	http.end();

	return hasUpdate;
}

void Network::updateFirmware() {
	WiFiClient client;
	HTTPClient http;

	http.begin(client, SERVER_IP, HTTP_PORT, "/api/sensors/firmware");
	http.GET();

	Update.begin(http.getSize());

	U8G2_SSD1306_64X48_ER_F_HW_I2C u8g2(U8G2_R0);

	u8g2.begin();

	u8g2.enableUTF8Print();
	u8g2.setFont(u8g2_font_tom_thumb_4x6_tf);

	Update.onProgress([&u8g2](size_t current, size_t total) {
		// TODO: Replace with assertion
		u8g2.clearBuffer();

		u8g2.setCursor((u8g2.getDisplayWidth() - u8g2.getStrWidth("Updating")) / 2, u8g2.getDisplayHeight() / 2 - u8g2.getMaxCharHeight() + u8g2.getAscent() - 4);
		u8g2.print("Updating");

		unsigned progress = roundf(current / (double)total * 100);
		unsigned digits = 1 + (progress >= 10) + (progress >= 100);
		char *progressStr = (char *)alloca((4 + digits) * sizeof(char));

		progressStr[0] = '[';
		snprintf(progressStr + 1, digits + 1, "%u", progress);
		progressStr[digits + 1] = '%';
		progressStr[digits + 2] = ']';
		progressStr[digits + 3] = (char)NULL;

		u8g2.setCursor((u8g2.getDisplayWidth() - u8g2.getStrWidth(progressStr)) / 2, u8g2.getDisplayHeight() / 2 + u8g2.getAscent() + 4);
		u8g2.print(progressStr);

		u8g2.sendBuffer();
	});

	Update.writeStream(http.getStream());
	Update.end();

	http.end();

	u8g2.clearDisplay();

	ESP.restart();
	while(true) {}
}