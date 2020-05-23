/*
 * Copyright (c) 2020, TrippleFox
 * All rights reserved.
 * 
 * This source code is licensed under the MIT-style license found in the
 * LICENSE file in the root directory of this source tree. 
 */

#ifndef TSIC_data_h
#define TSIC_data_h

#include "Arduino.h"

// -------------------------------------------------------------------------------
// internal data class used to hold the working variables of a sensor instance
// -------------------------------------------------------------------------------

class TsicData {
  public:
      TsicData(byte signal_pin, byte vcc_pin, byte sensor_type, byte idx);
      void powerOn();
      void powerOff();
      void getParams(byte scale, float* htlt, float* adcMax, float* lt);
      volatile bool isInitialized;
      volatile bool valueAvailable;
      volatile bool parityError;      
      volatile unsigned long rawTemp;
      byte signalPin;
      byte sensorIdx;
      void isr();
  private:      
      byte sensorType;
      byte vccPin;
      byte pinState;      
      unsigned long strobeTime;
      unsigned long prevTime;
      unsigned long diffTime;
      unsigned long currentTime;
      unsigned long bitBuffer;
      unsigned long tmp;
      byte parity;
      int bitPos = -1;           
};

#endif /* TSIC_data */
