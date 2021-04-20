#include "LedsOled.h"

Adafruit_SSD1306* display = NULL;

void setupDisplay()
{
  
  display = new Adafruit_SSD1306(128, 64);
  // Initialize with the I2C default display address of 0x3C
  display->begin(SSD1306_SWITCHCAPVCC, 0x3C);
  
  display->clearDisplay();
}

void clearDisplay()
{
  if(display != NULL){
    display->clearDisplay();
  }
}
bool validDisplay()
{
  if(display == NULL){
    Serial.println("No display configured!");
    return false;
  }
  
  return true;
}

void writeStringToDisplay(const char* message, bool clearFirst, float lineNumber)
{
  if(validDisplay()){
    if(clearFirst) display->clearDisplay();
    display->setTextSize(1);
    display->setTextColor(WHITE);
    display->setCursor(0, (int)(8.0 * lineNumber));
    display->println(message);
    display->display();
  }
}

void writeStringToDisplay(String msg, bool clearFirst, float lineNumber)
{
  writeStringToDisplay(msg.c_str(), clearFirst, lineNumber);
}

void writeStringToDisplay(String msg, String additionalMsg, bool clearFirst, float lineNumber)
{
  writeStringToDisplay((msg + additionalMsg).c_str(), clearFirst, lineNumber);
}
