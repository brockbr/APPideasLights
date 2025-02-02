#include "LedsWebserver.h"

LedController* pLedController = NULL;

// Setup a wireless access point so the user can be prompted to connect to a network
String  ssidPrefix = "b2prototech-";
String ssid = "";
int serverPort = 80;
String softIP  = "";
bool wifiConnected = false;
const int MAX_WIFI_RETRIES = 60;              // The maximum number of times we'll try connecting to WiFi before reverting to AP mode

// Setup NTP parameters for syncing with network time
const int TIMEZONE_OFFSET = 3600;             // Number of seconds in an hour
int inputTimezoneOffset = 0;                  // Offset from UTC - will be input by the user
const char* ntpServerName = "time.nist.gov";  // NTP server to use
const int checkInterval = 60 * 30;            // how often (in seconds) to query the NTP server for updates (30 minutes)

// NTP parameters that probably shouldn't be changed
unsigned int localPort = 2390;                // local port to listen for UDP packets
IPAddress timeServerIP;                       // time.nist.gov NTP server address
const int NTP_PACKET_SIZE = 48;               // NTP time stamp is in the first 48 bytes of the message
byte packetBuffer[ NTP_PACKET_SIZE];          //buffer to hold incoming and outgoing packets

// A UDP instance to let us send and receive packets over UDP
WiFiUDP udp;

// Global time variables
String fullTime = String( "" );
String humanTime = String( "" );

int intTime = 0;
int currentHour = 0;
int currentMinute = 0;
int currentSecond = 0;
int currentMeridiem = 0;

int status = WL_IDLE_STATUS;
IPAddress ip;

ESP8266WebServer server( serverPort );

StoredSettings romSettings;

void initWebserver(LedController* pController)
{
  pLedController = pController;
}

void serverHandleClient()
{
  server.handleClient();
}

void saveSettings( String ssid, String password, int offset )
{
  uint addr = 0;

  strcpy( romSettings.ssid, ssid.c_str() );
  strcpy( romSettings.pass, password.c_str() );
  romSettings.offset = offset;

  EEPROM.begin( 512 );
  EEPROM.put( addr, romSettings );
  EEPROM.end();
}

StoredSettings getStoredSettings()
{
  uint addr = 0;
  EEPROM.begin( 512 );
  EEPROM.get( addr, romSettings );
  EEPROM.end();
  return romSettings;
}

void wipeSettings()
{
  struct StoredSettings
  {
    char ssid[128];
    char pass[128];
    int offset = 0;
  } clearSettings;

  strcpy( clearSettings.ssid, "b2prototech-changeme" );
  strcpy( clearSettings.pass, "apideas-changeme" );

  uint addr = 0;
  EEPROM.begin( 512 );
  EEPROM.put( addr, clearSettings );
  EEPROM.end();
}

void sendBlank()
{
  String sendValue = "";
  StaticJsonDocument<32> doc;
  doc[""] = "";

  serializeJson(doc, sendValue);
  
  server.send( 200, "text/html", sendValue );
}

void handleRoot()
{
  if( !wifiConnected )
  {
    Serial.println( "Connecting" );
    server.send( 200, "text/html", connectionHtml() );
  }
  else
  {
    Serial.println( "Already connected" );
    server.send( 200, "text/html", connectedHtml() );
  }
  delay( 100 );
}

void handleConnect()
{
  String hardSSID = server.arg( "ssid" );
  String hardPassword = server.arg( "password" );
  int tmpInputTimezoneOffset = server.arg( "timezone" ).toInt();

  if( tmpInputTimezoneOffset != 0 )
  {
    inputTimezoneOffset = tmpInputTimezoneOffset;
  }

  saveSettings( hardSSID, hardPassword, inputTimezoneOffset );

  if( !wifiConnected )
  {
    ssid = hardSSID;
    connectToWifi( ssid, hardPassword );
  }

  String newIp = WiFi.localIP().toString();
  server.send( 200, "text/html", redirect( newIp ) );

  // Get and show the time
  Serial.println( "Starting UDP" );
  udp.begin( localPort );
  fullTime = getTime();
  timeToVars();
  Serial.println( "Current time: " + humanTime );
  
  server.send( 200, "text/html", connectedHtml() );
  Serial.println( "Connected to WiFi" );
  delay( 100 );
}

void handleNetworkStatus()
{
   server.send( 200, "text/html", getNetworkStatus() );
}

void handleStatus()
{
   server.send( 200, "text/html", getStatus() );
}

void handleColorset()
{
  int red = server.arg( RED ).toInt();
  int green = server.arg( GREEN ).toInt();
  int blue = server.arg( BLUE ).toInt();

  pLedController->colorSet(red, green, blue);

  delay( 50 ); // give the web server a small amount of time to buffer and send

  sendBlank();
  server.send( 200, "text/html", "" ); // see if we can improve response times a little bit

}

void handleSetAnimation()
{
  String values = server.arg("values");

  if(values != NULL && values.length() > 0 && (values.length() % LedController::FrameSize) == 0){
    
  }
}

void startWebServer()
{
  server.on("/", handleRoot);
  server.on("/connect", handleConnect);
  server.on("/network-status", handleNetworkStatus);
  server.on("/status", handleStatus);
  server.on("/colorset", handleColorset);
  server.on("/setanimation", handleSetAnimation);

  server.begin();

}

void startSoftAP()
{
  String mac = WiFi.macAddress();
  String macPartOne = mac.substring( 12, 14 );
  String macPartTwo = mac.substring( 15 );

  ssid = ssidPrefix + macPartOne + macPartTwo; // "b2prototech-" followed by the last 4 characters of the device MAC address
  
  WiFi.softAP( ssid.c_str() );
  softIP = WiFi.softAPIP().toString();
 
  Serial.println( getSoftAPStatus() );
}


void connectToWifi( String wifiSSID, String wifiPassword )
{
  int attemptCount = 0;
  
  Serial.print( "Connecting to " );
  Serial.println( wifiSSID );
  
  WiFi.mode( WIFI_STA );
  WiFi.begin( wifiSSID.c_str(), wifiPassword.c_str() );

  while( WiFi.status() != WL_CONNECTED )
  {
      delay( 500 ); // delay one-half second between retries
      Serial.print( "." );

      // Reset EEPROM values and go into AP mode if the connection hasn't been established for a period of time. Default: 30 seconds
      if( attemptCount > MAX_WIFI_RETRIES )
      {
        //wipeSettings();
        startSoftAP();
        return;
      }
      attemptCount++;
  }
  Serial.println( "." );
  Serial.println( "Connected" );
  wifiConnected = true;
  printWifiStatus();
}

void printWifiStatus() 
{
  Serial.printf( "IP Address: %s", WiFi.localIP().toString().c_str());
}
 


String getTime()
{
  
  //Serial.println( "Getting time" );
  String returnValue = String();

  //get a random server from the pool
  WiFi.hostByName( ntpServerName, timeServerIP );

  sendNTPpacket( timeServerIP ); // send an NTP packet to a time server
  // wait to see if a reply is available
  delay( 2000 );
  
  int cb = udp.parsePacket();
  // loop until we have a good connection
  while( !cb ) 
  {
    //Serial.println( "Problem reading from NTP server. Retrying in 10 seconds..." );
    delay( 10000);
    //Serial.println( "Trying again..." );
    cb = udp.parsePacket();
  }

    // We've received a packet, read the data from it
    udp.read( packetBuffer, NTP_PACKET_SIZE ); // read the packet into the buffer

    //the timestamp starts at byte 40 of the received packet and is four bytes,
    // or two words, long. First, esxtract the two words:

    unsigned long highWord = word( packetBuffer[40], packetBuffer[41] );
    unsigned long lowWord = word( packetBuffer[42], packetBuffer[43] );
    // combine the four bytes (two words) into a long integer
    // this is NTP time (seconds since Jan 1 1900):
    unsigned long secsSince1900 = highWord << 16 | lowWord;

    // now convert NTP time into everyday time:
    // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
    const unsigned long seventyYears = 2208988800UL;
    // subtract seventy years:
    unsigned long epoch = secsSince1900 - seventyYears;

    epoch = epoch + (TIMEZONE_OFFSET * (inputTimezoneOffset + 1));

    int hour = ((epoch  % 86400L) / 3600);
    int minute = ((epoch  % 3600) / 60);
    int second = (epoch % 60);
    
    // make a 12 hour clock
    bool isPM = false;
    if( hour >= 12 )
    {
      isPM = true;
      if( hour > 12 )
      {
        hour = hour - 12;
      }
    }
    if( hour == 0 )
    {
      hour = 12;
    }
    
    returnValue = String( hour );
    returnValue = String( returnValue + " " );
    returnValue = String( returnValue + minute );
    returnValue = String( returnValue + " " );
    returnValue = String( returnValue + second );
    
    if( isPM )
    {
      returnValue = String( returnValue + " 1" );
    }
    else
    {
      returnValue = String( returnValue + " 0" );
    }

    humanTime = String( hour );
    humanTime = String( humanTime + ":" );
    humanTime = String( humanTime + minute );
    humanTime = String( humanTime + ":" );
    humanTime = String( humanTime + second );
    if( isPM )
    {
      humanTime = String( humanTime + " PM" );
    }
    else
    {
      humanTime = String( humanTime + " AM" );
    }

  return returnValue;
}

void timeToVars()
{
  int separator1 = fullTime.indexOf( ' ' );
  int separator2 = fullTime.indexOf( ' ', (separator1 + 1) );
  int separator3 = fullTime.indexOf( ' ', (separator2 + 1) );

  String hourAsString = fullTime.substring( 0, separator1 );
  String minuteAsString = fullTime.substring( separator1, separator2 );
  String secondAsString = fullTime.substring( separator2, separator3 );
  String meridiemAsString = fullTime.substring( separator3 );


  currentHour = hourAsString.toInt();
  currentMinute = minuteAsString.toInt();
  int second = secondAsString.toInt();

  currentSecond = second;
  if( currentSecond >= 60 )
  {
    currentSecond = (currentSecond % 60);
    currentMinute += 1;
  }

  if( currentMinute >= 60 )
  {
    currentMinute = (currentMinute % 60);
    currentHour += 1;
  }

  if( currentHour >= 12 )
  {
    currentMeridiem = 1;
    currentHour = (currentHour % 12);
    if( currentHour == 0 )
    {
      currentHour = 11;
    }
    currentHour += 1;
  }
  else
  {
    currentMeridiem = 0;
  }

  if( currentHour < 12 && currentMeridiem > 0 )
  {
    currentHour = currentHour + 12;
  }

  String stringMinute = String( currentMinute );
  if( currentMinute < 10 )
  {
    stringMinute = "0" + stringMinute;
  }

  String timeString = String( currentHour );
  timeString = timeString + stringMinute;
  intTime = timeString.toInt();
}

unsigned long sendNTPpacket( IPAddress& address )
{
  //Serial.println("sending NTP packet...");
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  udp.beginPacket(address, 123); //NTP requests are to port 123
  udp.write(packetBuffer, NTP_PACKET_SIZE);
  udp.endPacket();
}


bool haveNetworkCredentials()
{
  StoredSettings deviceSettings = getStoredSettings();

  // < 2 to catch empty string
  if( deviceSettings.ssid == NULL || 
      sizeof( deviceSettings.ssid ) < 2 || 
      String( deviceSettings.ssid ).equals( "b2prototech-changeme" ) )
  {
    return false; 
  }

  return true;
}

String getNetworkStatus()
{
  String returnValue = "";

  StaticJsonDocument<1024> doc;
  if( wifiConnected )
  {
    doc["soft"] = "disconnected";
    doc["hard"] = "connected";
    doc["ip"] = WiFi.localIP().toString();
  }
  else
  {
    doc["soft"] = "connected";
    doc["hard"] = "disconnected";
    doc["ip"] = softIP;
  }
  doc["ssid"] = ssid;

  serializeJson(doc, returnValue);

  return returnValue;
}

String getStatus()
{
  String returnValue = "";

  StaticJsonDocument<1024> doc;

  JsonArray v = doc.createNestedArray( "values" );
  JsonArray r = doc.createNestedArray( "ratios" );

  v.add( pLedController->getLevelForColor(RED));
  v.add( pLedController->getLevelForColor(GREEN));
  v.add( pLedController->getLevelForColor(BLUE));

  r.add( pLedController->getRatioForColor(RED));
  r.add( pLedController->getRatioForColor(GREEN));
  r.add( pLedController->getRatioForColor(BLUE));

  serializeJson(doc, returnValue);

  return returnValue;
}


String getSoftAPStatus()
{
  String returnValue = "\n";
  returnValue += "SSID: " + ssid + "\n";
  returnValue += "IP  : " + softIP + "\n";

  return returnValue;
}
const String connectionHtml()
{
  const String returnValue =
    "<!DOCTYPE HTML>"
    "<html>"
    "<head>"
    "<meta name = \"viewport\" content = \"width = device-width, initial-scale = 1.0, maximum-scale = 1.0, user-scalable=0\">"
    "<title>Connect to WiFi</title>"
    "<link href='https://fonts.googleapis.com/css?family=Open+Sans' rel='stylesheet' type='text/css'>"
    "<style>"
    "body { background-color: #D3E3F1; font-family: \"Open Sans\", sans-serif; Color: #000000; }"
    "#header { width: 100%; text-align: center; }"
    "input[type=text], input[type=password] { width: 50%; height: 30px; padding-left: 10px; }"
    "</style>"
    "</head>"
    "<body>"
    "<div id='header'><h3>Connect to your WiFi network</h3></div>"
    "<FORM action=\"/connect\" method=\"post\">"
    "<P>"
    "<p><INPUT type=\"text\" name=\"ssid\" placeholder=\"SSID\" autocomplete=\"off\" autocorrect=\"off\" autocapitalize=\"off\" spellcheck=\"false\"></p>"
    "<p><INPUT type=\"password\" name=\"password\" placeholder=\"Password\" autocomplete=\"off\" autocorrect=\"off\" autocapitalize=\"off\" spellcheck=\"false\"></p>"
    "<p><INPUT type=\"text\" name=\"timezone\" placeholder=\"Timezone Offset (e.g. -8)\" autocomplete=\"off\" autocorrect=\"off\" autocapitalize=\"off\" spellcheck=\"false\"></p><br>"
    "<p><INPUT type=\"submit\" value=\"CONNECT\"></p>"
    "</P>"
    "</FORM>"
    "</body>"
    "</html>";

  return returnValue;
}

const String connectedHtml()
{
  const String returnValue =
    "<!DOCTYPE HTML>"
    "<html>"
    "<head>"
    "<meta name = \"viewport\" content = \"width = device-width, initial-scale = 1.0, maximum-scale = 1.0, user-scalable=0\">"
    "<title>Connect to WiFi</title>"
    "<link href='https://fonts.googleapis.com/css?family=Open+Sans' rel='stylesheet' type='text/css'>"
    "<style>"
    "body { background-color: #D3E3F1; font-family: \"Open Sans\", sans-serif; Color: #000000; }"
    "#header { width: 100%; text-align: center; }"
    "</style>"
    "</head>"
    "<body>"
    "You are connected to WiFi"
    "</body>"
    "</html>";

  return returnValue;
}

String redirect( String newIP )
{
  String returnValue =
    "<!DOCTYPE HTML>"
    "<html>"
    "<head>"
    "<script type=\"text/javascript\">"
    " window.location = \"http://" + newIP + "/\""
    "</script>"
    "</head>"
    "<body>"
    "</body>"
    "</html>";

  return returnValue;
}
