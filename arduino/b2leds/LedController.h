#ifndef __LEDCONTROLLER__
#define __LEDCONTROLLER__

#include "Arduino.h"

#define RED "red"
#define BLUE "blue"
#define GREEN "green"

class LedController
{
  public:
  static const int FrameSize = 12;
  LedController(int redPin, int greenPin, int bluePin);
    
  int getLevelForColor( String color );
  float getRatioForColor( String color );
  void colorSet(int red = -1, int green = -1, int blue = -1);
  void setColorToLevel(String color, int level);

  void turnOn();
  void turnOff();

  int RED_LED;
  int GREEN_LED;
  int BLUE_LED;

  protected:
    LedController();

  private:
    int currentR = 0;
    int currentG = 0;
    int currentB = 0;
};



#endif
