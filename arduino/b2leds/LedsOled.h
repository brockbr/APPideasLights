#ifndef __LEDSOLED__
#define __LEDSOLED__

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

void setupDisplay();
void clearDisplay();
bool validDisplay();
void writeStringToDisplay(const char* message, bool clearFirst = true, float lineNumber = 3.0);
void writeStringToDisplay(String msg, bool clearFirst = true, float lineNumber = 3.0);
void writeStringToDisplay(String msg, String additionalMsg, bool clearFirst = true, float lineNumber = 3.0);
#endif
