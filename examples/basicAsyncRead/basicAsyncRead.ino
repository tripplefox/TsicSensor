/*
   This is a very basic example how to use the TsicSensor library
   to read temperature values from TSIC sensors.
*/

#include <TsicSensor.h>

TsicSensor* sensor1;
TsicSensor* sensor2;

float temp1, temp2;

void setup() 
{
   Serial.begin(115200);

   // creates/initializes two TSIC_506 sensors connected on GPIO 16 and 17...
    
   sensor1 = TsicSensor::create(16, TsicExternalVcc, TsicType::TSIC_506);
   sensor2 = TsicSensor::create(17, TsicExternalVcc, TsicType::TSIC_506);
}


void loop() 
{
	// now read new sensor values (if available) and print them to Serial.
	// (just open the "SerialPlotter" tool of the Arduino IDE and see the values as graph...)
	
    if(sensor1->newValueAvailable())
    {      
      temp1 = sensor1->getTempCelsius();
    }
    if(sensor2->newValueAvailable())
    {      
      temp2 = sensor2->getTempCelsius();
    }

    Serial.print(temp1);
    Serial.print("\t");
    Serial.println(temp2);
    
    delay(100);
}
