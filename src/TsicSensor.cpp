/*
 * Copyright (c) 2020, TrippleFox
 * All rights reserved.
 * 
 * This source code is licensed under the MIT-style license found in the
 * LICENSE file in the root directory of this source tree. 
 */

#include "Arduino.h"
#include "TsicSensor.h"

namespace {

// the sensor data array...
TsicData* sensorData[4] = {nullptr,nullptr,nullptr,nullptr}; 

// the interrupt service routines...
void IRAM_ATTR isr1(){ if(sensorData[0]!=nullptr) sensorData[0]->isr(); }
void IRAM_ATTR isr2(){ if(sensorData[1]!=nullptr) sensorData[1]->isr(); }
void IRAM_ATTR isr3(){ if(sensorData[2]!=nullptr) sensorData[2]->isr(); }
void IRAM_ATTR isr4(){ if(sensorData[3]!=nullptr) sensorData[3]->isr(); }

using ISR = void(*)(void);
ISR isrs[4] = {isr1, isr2, isr3, isr4};
} 

// ------------------------------------------------------------------------------------------
// * Creates and initializes a new sensor instance with the given type and input/vcc pins. 
// * Use "TsicExternalVcc" for the "vcc_pin" parameter if the sensor is connected directly to Vcc.
// * A maximum of 4 instances can be operated at the same time.
// * Returns a pointer to a sensor instance (or "nullptr" if the operation failed). 
// ------------------------------------------------------------------------------------------
TsicSensor* TsicSensor::create(byte input_pin, byte vcc_pin, TsicType type)
{
  TsicSensor* newSensor = nullptr;
  for(int idx=0; idx<4; idx++)
  {
     if(sensorData[idx]==nullptr)
     {
        TsicData* newData = new TsicData(input_pin, vcc_pin, (byte)type, idx);    
        sensorData[idx]= newData;    
        attachInterrupt(digitalPinToInterrupt(input_pin), isrs[idx], CHANGE);   // start reading...     
        newSensor = new TsicSensor(newData);
        idx=5;
     }
  }  
  return newSensor;
}

// ---------------------------------
// the (private) constructor 
// ---------------------------------
TsicSensor::TsicSensor(TsicData* data)
{
   m_data = data;
}

// ---------------------------------
// the destructor 
// ---------------------------------
TsicSensor::~TsicSensor()
{
  detachInterrupt(digitalPinToInterrupt(m_data->signalPin));
  sensorData[m_data->sensorIdx] = nullptr;
  delete m_data;
  m_data = nullptr;
}

// -------------------------------------------------------------------------------------------------------
// * If the sensor is connected directly to Vcc, it sends its values 10 times per second.
// * This function returns "true" if the received temperature value has changed since the last call to
// * one of the "*getTemp*()" functions.
// -------------------------------------------------------------------------------------------------------
bool TsicSensor::newValueAvailable()
{
  if(m_data!=nullptr)
     return m_data->valueAvailable;
  return false;
}

// -------------------------------------------------------------------------------------------------------
// * Returns the latest temperature value in °C (waits up to 100ms for sensor initialization, if needed).
// * Returns "-273.15" if the read was not successful.
// -------------------------------------------------------------------------------------------------------
float TsicSensor::getTempCelsius()
{
  float result = -273.15;
  tryGetTempValue(&result,TsicScale::Celsius);
  return result;
}

// -------------------------------------------------------------------------------------------------------
// * Returns the latest temperature value in °F (waits up to 100ms for sensor initialization, if needed).
// * Returns "-459.67" if the read was not successful.
// -------------------------------------------------------------------------------------------------------
float TsicSensor::getTempFahrenheit()
{
   float result = -459.67;   
   tryGetTempValue(&result,TsicScale::Fahrenheit);  
   return result;
}

// -------------------------------------------------------------------------------------------------------
// * Returns the latest temperature value in °K (waits up to 100ms for sensor initialization, if needed).
// * Returns "0" if the read was not successful.
// -------------------------------------------------------------------------------------------------------
float TsicSensor::getTempKelvin()
{
  float result = 0;
  tryGetTempValue(&result,TsicScale::Kelvin);
  return result;
}

// -------------------------------------------------------------------------------------------------------
// * Gets the latest temperature value in °C/°F/°K. 
// * (waits up to 100ms for sensor initialization, if needed).
// * Returns "true" if the read was successful, "false" otherwise.
// -------------------------------------------------------------------------------------------------------
bool TsicSensor::tryGetTempValue(float* value, TsicScale scale)
{ 
    if((m_data==nullptr)||(value==nullptr))
       return false;
       
    // if needed, wait for initialization of the sensor...
    if(!m_data->isInitialized)
    {        
      m_data->powerOn();            // only switches ON if a vcc_pin was specified...
      unsigned int timeout=0;
      while(!m_data->isInitialized) // it takes up to 65-85ms to start up the sensor 
      {
         delayMicroseconds(25);
         if(timeout++>4000)         // -> timeout after 100ms
            return false;
      }
    }
    m_data->powerOff();             // only switches OFF if a vcc_pin was specified...

    unsigned int raw = m_data->rawTemp; // get latest (raw) temperature value
    m_data->valueAvailable = false;     // reset signals...
    m_data->parityError = false;

    // Temperature is calculated from raw  sensor value with:
    // temp = (HT-LT) * raw_value / adc_max + LT   
    
    float htlt; float adcMax; float lt; 
    m_data->getParams((byte)scale, &htlt, &adcMax, &lt); // get the sensor type specific parameters
    
    *value = htlt*raw/adcMax+lt;  
    return true;
}

