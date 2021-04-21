#include "LedController.h"

#define MAX_BRIGHTNESS 1024
#define OFF_BRIGHTNESS 0

LedController::LedController() : LedController(14, 15, 12)
{
}

LedController::LedController(int red, int green, int blue)
{
    RED_LED = red;
    GREEN_LED = green;
    BLUE_LED = blue;
}

int LedController::getLevelForColor(String color )
{
  if( color == RED )
  {
    return currentR;
  }
  else if( color == GREEN )
  {
    return currentG;
  }
  else if( color == BLUE )
  {
    return currentB;
  }
  else
  {
    return 0;
  }
}


float LedController::getRatioForColor(String color )
{
  int  currentValue = getLevelForColor( color );
  return (float) ((float)currentValue / (float)1024);
}

void LedController::colorSet(int red, int green, int blue)
{
  Serial.printf("Color Setting - Red: %d, Green %d, Blue %d\n", red, green, blue);
  float ratioRed = ((float)red / (float)100);
  float ratioGreen = ((float)green / (float)100);
  float ratioBlue = ((float)blue / (float)100);

  int requestLevelRed = ceil( ratioRed * MAX_BRIGHTNESS );
  int requestLevelGreen = ceil( ratioGreen * MAX_BRIGHTNESS );
  int requestLevelBlue = ceil( ratioBlue * MAX_BRIGHTNESS );

  setColorToLevel( RED, requestLevelRed );
  setColorToLevel( GREEN, requestLevelGreen );
  setColorToLevel( BLUE, requestLevelBlue );

}

void LedController::setColorToLevel(String color, int level) 
{ 
  if( color == "all" )
  {
    analogWrite( RED_LED, level );
    analogWrite( GREEN_LED, level );
    analogWrite( BLUE_LED, level );

    currentR = level;
    currentG = level;
    currentB = level;
    
    return;
  }

  if( color == RED )
  {
    analogWrite( RED_LED, level );
    currentR = level;
    return;
  }
  
  if( color == GREEN )
  {
    analogWrite( GREEN_LED, level );
    currentG = level;
    return;
  }
  
  if( color == BLUE )
  {
    analogWrite( BLUE_LED, level );
    currentB = level;
    return;
  }

}

void LedController::turnOn() 
{ 
  colorSet(100, 100, 100);
}

void LedController::turnOff() 
{
  colorSet(0, 0, 0);
}
