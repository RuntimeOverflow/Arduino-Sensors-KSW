#include <WiFiUdp.h>

#include "Errors.h"

class Network {
private:
	char *ssid = nullptr;
	char *password = nullptr;
	WiFiUDP udp;

	// Error sendUDPPacket(char const *address, unsigned port, char const *msg, size_t length = UINT_MAX);
	Error sendMulticastPacket(char const *address, unsigned port, char const *msg, size_t length = UINT_MAX);

public:
	Network(char const *ssid, char const *password = nullptr);
	~Network();

	bool isConnected();
	Error connect();
	void disconnect();

	bool isUpdateAvailable();
	void updateFirmware();
};