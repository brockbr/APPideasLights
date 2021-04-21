#include <SPI.h>
#include <Wire.h>

#include "LedsWebserver.h"
#include "LedsOled.h"
#include "LedController.h"

/**
 * A sketch to control two strings of RGBW LED lights from one ESP8266.
 * 
 * This creates its own wireless access point with an SSID beginning with "b2prototech-"
 *   that you need to connect to for configuring this device to connect to your network
 *   
 * For complete assembly and usage information, visit this Instructable:
 * https://www.instructables.com/id/Wifi-Led-Light-Strip-Controller/
 * 
 * @author costmo
 * @since  20180902
 */



// For reference (pin mapping for ESP8266-12E):
// D0/GPIO16      = 16;
// D1/GPIO5       = 5;
// D2/GPIO4       = 4;
// D3/GPIO0       = 0;

// D4/GPIO2       = 2;

// D5/GPIO14      = 14;
// D6/GPIO12      = 12;
// D7/GPIO13      = 13;
// D8/GPIO15      = 15;


LedController ledController(14, 15, 12);

bool dnsUp = false;
MDNSResponder mdns;

void setupMDNS(String ipAddress)
{
  
  //writeStringToDisplay("Setting DNS...", true, 4.5);
  String fullMac = WiFi.macAddress();
  fullMac.replace(":", "");
  
  if (!mdns.begin((String("b2leds-") + fullMac).c_str())) {
    Serial.println("Error setting up MDNS responder!");
    //writeStringToDisplay("DNS Failed", false, 4.5);
    //while (1) {
    //  delay(1000);
    //}
    return;
  }
  
  Serial.println("mDNS responder started");  
  writeStringToDisplay("DNS up", false, 4.5);
  mdns.addService("http", "tcp", 80);
  dnsUp = true;
}

void setup() 
{
  writeStringToDisplay("Starting...");
  setupDisplay();

  Serial.begin( 57600 );
  delay( 100 );

  initWebserver(&ledController);
  
  clearDisplay();
  delay(500);
  
  writeStringToDisplay("Setting pin modes...");
  pinMode( ledController.RED_LED, OUTPUT );
  pinMode( ledController.GREEN_LED, OUTPUT );
  pinMode( ledController.BLUE_LED, OUTPUT);
 
  //wipeSettings();

  // Check to see if we have stored network credentials already
  if( !haveNetworkCredentials() )
  {
    // Start the local access point
    writeStringToDisplay("No creds - Starting AP...");
    startSoftAP();
  }
  else{
    StoredSettings deviceSettings = getStoredSettings();
    writeStringToDisplay(String("Connecting to: "), String(deviceSettings.ssid));

    connectToWifi( String( deviceSettings.ssid ), String( deviceSettings.pass ) );
    writeStringToDisplay(String("Connected: "), String(deviceSettings.ssid));
    
    writeStringToDisplay(WiFi.localIP().toString(), false, 4.5);
  }

  // Start the web server and get ready to handle incoming requests
  setupMDNS(WiFi.localIP().toString());
  startWebServer();

  ledController.turnOff();
}


void loop() 
{
  if(dnsUp) mdns.update();
  
  serverHandleClient();
}
