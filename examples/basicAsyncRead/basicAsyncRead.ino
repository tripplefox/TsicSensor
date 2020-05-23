/*
   This is a very basic example how to use the TsicSensor library
   to read temperature values from a TSIC sensor.
   
   Tip: Open the "SerialPlotter" tool of the Arduino IDE and see the temperature as graph...
*/

#include <TsicSensor.h>

TsicSensor* sensor1;

float temperature;

void setup() 
{
   Serial.begin(115200);

   // This creates/initializes a TSIC_506 sensor connected to GPIO 16.
   // -----------------------------------------------------------------------------------
   // The sensor is configured with "TsicExternalVcc", so it has a permanent external 
   // power source. The sensor values are then read in the background. 
   // (we can check for new values with the "newValueAvailable()" function...)
   
   sensor1 = TsicSensor::create(16, TsicExternalVcc, TsicType::TSIC_506);
}


void loop() 
{
	// we are now waiting for new (changed) temperature values ...
	if(sensor1->newValueAvailable())
    {      
	  // Get the temperature in °Celsius.
      temperature = sensor1->getTempCelsius();
    }

    // output the temperature value to the serial port...
    Serial.println(temperature);
    delay(100);
}
