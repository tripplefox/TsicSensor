/*
 * Copyright (c) 2020, TrippleFox
 * All rights reserved.
 * 
 * This source code is licensed under the MIT-style license found in the
 * LICENSE file in the root directory of this source tree. 
 */

#ifndef TSIC_Sensor_h
#define TSIC_Sensor_h

#include "Arduino.h"
#include "data/TsicData.h"

enum class TsicScale : byte{
   Celsius = 0,
   Fahrenheit = 1,
   Kelvin,
};

enum class TsicType : byte{
   TSIC_206 = 0,
   TSIC_306 = 0,
   TSIC_316 = 1,
   TSIC_506 = 2,
   TSIC_516 = 3,
   TSIC_716 = 3
};

// use this constant for "vcc_pin" parameter if the sensor is connected directly to Vcc
const int TsicExternalVcc = 0xFF;

class TsicSensor {
  public:
      // -------------------------------------------------------------------------------------------------------
      // * Creates and initializes a new sensor instance with the given type and input/vcc pins. 
      // * Use "TsicExternalVcc" for the "vcc_pin" parameter if the sensor is connected directly to Vcc.
      // * A maximum of 4 instances can be operated at the same time.
      // * Returns a pointer to a sensor instance (or "nullptr" if the operation failed). 
      // -------------------------------------------------------------------------------------------------------
      static TsicSensor* create(byte input_pin, byte vcc_pin, TsicType type);

      // -------------------------------------------------------------------------------------------------------
      // * Returns the latest temperature value in °C (waits up to 100ms for sensor initialization, if needed).
      // * Returns "-273.15" if the read was not successful.
      // -------------------------------------------------------------------------------------------------------
      float getTempCelsius();

      // -------------------------------------------------------------------------------------------------------
      // * Returns the latest temperature value in °F (waits up to 100ms for sensor initialization, if needed).
      // * Returns "-459.67" if the read was not successful.
      // -------------------------------------------------------------------------------------------------------
      float getTempFahrenheit();
      
      // -------------------------------------------------------------------------------------------------------
      // * Returns the latest temperature value in °K (waits up to 100ms for sensor initialization, if needed).
      // * Returns "0.0" if the read was not successful.
      // -------------------------------------------------------------------------------------------------------
      float getTempKelvin();
      
      // -------------------------------------------------------------------------------------------------------
      // * Gets the latest temperature value in °C/°F/°K. 
      // * (waits up to 100ms for sensor initialization, if needed).
      // * Returns "true" if the read was successful, "false" otherwise.
      // -------------------------------------------------------------------------------------------------------
      bool tryGetTempValue(float* value, TsicScale scale);

      // -------------------------------------------------------------------------------------------------------
      // * If the sensor is connected directly to Vcc, it sends its values 10 times per second.
      // * This function returns "true" if the received temperature value has changed since the last call to
	  // * one of the "*getTemp*()" functions.
      // -------------------------------------------------------------------------------------------------------
      bool newValueAvailable();           

      ~TsicSensor();     
  private:      
      TsicSensor(TsicData* data);      
      TsicData* m_data;
};

#endif /* TSIC_Sensor_H */
