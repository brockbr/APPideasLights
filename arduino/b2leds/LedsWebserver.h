#ifndef __LEDSWEBSERVER__
#define __LEDSWEBSERVER__

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ESP8266WebServer.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>
#include <EEPROM.h>

#include "LedController.h"

// Values saved to EEPROM for persistent storage of settings
struct StoredSettings
  {
    char ssid[128];
    char pass[128];
    int offset = 0;
  };

void initWebserver(LedController* pLedController);
void sendBlank();
void serverHandleClient();
void handleRoot();
void handleConnect();
void handleNetworkStatus();
void handleStatus();
void startWebServer();
void startSoftAP();
StoredSettings getStoredSettings();

void connectToWifi( String wifiSSID, String wifiPassword );
void printWifiStatus();

String getTime();
void timeToVars();
unsigned long sendNTPpacket( IPAddress& address );
bool haveNetworkCredentials();

String getNetworkStatus();
String getStatus();
String getSoftAPStatus();
const String connectionHtml();
const String connectedHtml();
String redirect( String newIP );
void saveSettings( String ssid, String password, int offset );

#endif
