/*
 * Copyright (c) 2020, TrippleFox
 * All rights reserved.
 * 
 * This source code is licensed under the MIT-style license found in the
 * LICENSE file in the root directory of this source tree. 
 */
 
#include "Arduino.h"
#include "TsicData.h"

namespace {
  
  // sensor-type specific parameters used for temperature calculations 
  
  // TSIC_206/306 : LT = -50°C; HT = 150°C; 11-bit ADC
  // TSIC_316     : LT = -50°C; HT = 150°C; 14-bit ADC
  // TSIC_506     : LT = -10°C; HT = 60°C;  11-bit ADC
  // TSIC_516/716 : LT = -10°C; HT = 60°C;  14-bit ADC
    
  float adcMaxValue[4] = {2047,16383,2047,16383};
  float tempDiffMax[3][4] = {{200,200,70,70},{360,360,126,126},{200,200,70,70}};
  float tempLow[3][4] = {{-50,-50,-10,-10},{-58,-58,14,14},{223.15,223.15,263.15,263.15}};
}

// ---------------------------------
// the sensor data constructor
// ---------------------------------
TsicData::TsicData(byte signal_pin, byte vcc_pin, byte sensor_type, byte idx)
{
   sensorIdx = idx;
   signalPin = signal_pin;
   vccPin = vcc_pin;
   sensorType = sensor_type;
   
   isInitialized = false;
   valueAvailable = false;
   parityError = false;
   prevTime = micros();             
   
   pinMode(signalPin, INPUT);        // set signal input pin
   
   if(vccPin!=255)                   // set vcc pin (if configured)
   {
      pinMode(vccPin, OUTPUT);
      digitalWrite(vccPin, LOW);     // start switched OFF
   }
}

// ------------------------------------------------------------------
// switches the sensor power ON if a vcc pin was set in constructor
// ------------------------------------------------------------------
void TsicData::powerOn()
{ 
  if(vccPin!=255)
  {
     digitalWrite(vccPin, HIGH); // switch the vcc pin on
  }
}

// ------------------------------------------------------------------
// switches the sensor power OFF if a vcc pin was set in constructor
// ------------------------------------------------------------------
void TsicData::powerOff()
{
  if(vccPin!=255)
  {
     digitalWrite(vccPin, LOW); // switch sensor power off
     isInitialized = false;     // reset initialization flag
  }
}

// -------------------------------------------------------------------------------
// returns sensor type specific parameters for temperature calculation
// => temp_in_celsius = (HT-LT) * raw_value / adc_max + LT
// -------------------------------------------------------------------------------
void TsicData::getParams(byte scale, float* htlt, float* adcMax, float* lt)
{      
  *htlt = tempDiffMax[scale][sensorType];
  *adcMax = adcMaxValue[sensorType];
  *lt = tempLow[scale][sensorType];
}

// -------------------------------------------------------------------------------------
// interrupt routine (reads the temperature values of the sensor)
// -------------------------------------------------------------------------------------
// On an ESP32, this method runs for ~1µs at every signal edge, so it takes about 20µs 
// processor time to read a single temperature value. The TSIC sensor sends its values 
// with 10Hz, so we use about ~200µs processor time per second (=0.02%) per sensor.
// -------------------------------------------------------------------------------------
void IRAM_ATTR TsicData::isr() 
{   
    currentTime = micros();            // get current µs-timer value
    pinState = digitalRead(signalPin); // get new signal state

    // calculate the time since last interrupt (dont't forget to handle timer overrun...)
    if(currentTime>prevTime)
       diffTime = currentTime-prevTime;
    else
       diffTime = 1+(~prevTime)+currentTime;
      
    if(pinState) // signal is HIGH: now we can read the bit-value...
    {
      if(bitPos>0)  // ...but only after the first start-bit (we need the strobe-time to be set)...
      {         
         bitBuffer = bitBuffer<<1;
         if(diffTime<strobeTime) // get/store the bit value
         {
            bitBuffer |= 1;
            parity++;
         }
            
         if(bitPos==19) // now the bit-sequence is complete (16 data, 2 start, 2 parity)...
         {
            // return the received value...
            if ((parity % 2)==0)   // parity must be "even"
            {              
               tmp = (bitBuffer>>11)<<8|(0xFF&(bitBuffer>>1));    // get the value-bits out of the buffer...
               if((tmp!=rawTemp) || !isInitialized)    // inform only if the value has changed...
               {               
                  rawTemp = tmp;
                  isInitialized = true;
                  valueAvailable=true; 
               }
            }
            else
              parityError=true; // inform that there was a parity error...
            
            bitPos=-1; // now wait for a new sequence...
         }
        
       }
    }
    else // signal is LOW: start of a new bit...
    {
      if(diffTime>500) // yeah, we found the first start-bit!
      {
        bitPos=0;       // so, we can start reading a new sequence...
      }
      else
      {
        if(bitPos==0)   // after the first start-bit, we can determine the strobe length...
        {
           // reduce the strobe-time by 10µs, so the next start-bit is read 
           // as "low" and doesn't conflict with the parity calculation...
           
           strobeTime = (diffTime>>1)-10; 
           
           bitBuffer=0;     // reset buffer and parity...
           parity=0;
        }
        bitPos++; // increment sequence position
      }
      
      prevTime = currentTime; // memorize current timer value
    }
}

