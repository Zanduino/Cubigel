/*******************************************************************************************************************
** This file defines the Cubigel class, see the "Cubigel.h" file for program documentation & version information. **
**                                                                                                                **
** This program is free software: you can redistribute it and/or modify it under the terms of the GNU General     **
** Public License as published by the Free Software Foundation, either version 3 of the License, or (at your      **
** option) any later version. This program is distributed in the hope that it will be useful, but WITHOUT ANY     **
** WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the   **
** GNU General Public License for more details. You should have received a copy of the GNU General Public License **
** along with this program.  If not, see <http://www.gnu.org/licenses/>.                                          **
**                                                                                                                **
*******************************************************************************************************************/
#include <SoftwareSerial.h>                                                   // Include the software serial lib  //
#include "Cubigel.h"                                                          // Include the header file          //
CubigelClass* CubigelClass::ClassPtr;                                         // Declare Class Reference pointer  //
/*******************************************************************************************************************
** The class constructor is overloaded to allow from 1 to CUBIGEL_MAX_DEVICES devices to be monitored at the same **
** time. Data is read using either the SoftwareSerial or the HardwareSerial calls, and each device needs to have  **
** been instantiated in order to pass in the appropriate pointer to the SoftwareSerial instance.                  **
** The Arduino design method doesn't allow interrupts to be attached to class members. The interrupt ISR is       **
** attached to the local static function, which in turn uses a pointer to the class with an offset to the appro-  **
** priate function to call the correct function.                                                                  **
*******************************************************************************************************************/
CubigelClass::CubigelClass( SoftwareSerial *ser1) {                           // Constructor 1 Software port      //
  ClassPtr      = this;                                                       // pointer to current instance      //
  devices[0].serialSW = ser1;                                                 // point to the appropriate port    //
  devices[0].serialSW->begin(CUBIGEL_BAUD_RATE);                              // Set baud rate to Cubigel speed   //
  _deviceCount = 1;                                                           // Store the number of devices      //
  StartTimer();                                                               // Enable timer interrupts          //
  setMode(0,MODE_SETTINGS);                                                   // Retrieve settings on first call  //
} // of class constructor                                                     //                                  //
                                                                              //----------------------------------//
CubigelClass::CubigelClass( HardwareSerial *ser1) {                           // Constructor 1 Hardware port      //
  ClassPtr      = this;                                                       // pointer to current instance      //
  devices[0].serialHW = ser1;                                                 // point to the appropriate port    //
  devices[0].serialHW->begin(CUBIGEL_BAUD_RATE);                              // Set baud rate to Cubigel speed   //
  _deviceCount = 1;                                                           // Store the number of devices      //
  StartTimer();                                                               // Enable timer interrupts          //
  setMode(0,MODE_SETTINGS);                                                   // Retrieve settings on first call  //
} // of class constructor                                                     //----------------------------------//
CubigelClass::CubigelClass( HardwareSerial *ser1,                             // Overloaded Class constructor     //
                            SoftwareSerial *ser2) {                           // with 2 devices - HW then SW      //
  ClassPtr      = this;                                                       // pointer to current instance      //
  devices[0].serialHW = ser1;                                                 // point to the appropriate port    //
  devices[0].serialHW->begin(CUBIGEL_BAUD_RATE);                              // Set baud rate to Cubigel speed   //
  devices[1].serialSW = ser2;                                                 // point to the appropriate port    //
  devices[1].serialSW->begin(CUBIGEL_BAUD_RATE);                              // Set baud rate to Cubigel speed   //
  _deviceCount = 2;                                                           // Store the number of devices      //
  StartTimer();                                                               // Enable timer interrupts          //
  setMode(0,MODE_SETTINGS);                                                   // Retrieve settings on first call  //
  setMode(1,MODE_SETTINGS);                                                   // Retrieve settings on first call  //
} // of class constructor                                                     //----------------------------------//
CubigelClass::CubigelClass( SoftwareSerial *ser1,                             // Overloaded Class constructor     //
                            HardwareSerial *ser2) {                           // with 2 devices - SW then HW      //
  ClassPtr      = this;                                                       // pointer to current instance      //
  devices[0].serialSW = ser1;                                                 // point to the appropriate port    //
  devices[0].serialSW->begin(CUBIGEL_BAUD_RATE);                              // Set baud rate to Cubigel speed   //
  devices[1].serialHW = ser2;                                                 // point to the appropriate port    //
  devices[1].serialHW->begin(CUBIGEL_BAUD_RATE);                              // Set baud rate to Cubigel speed   //
  _deviceCount = 2;                                                           // Store the number of devices      //
  StartTimer();                                                               // Enable timer interrupts          //
  setMode(0,MODE_SETTINGS);                                                   // Retrieve settings on first call  //
  setMode(1,MODE_SETTINGS);                                                   // Retrieve settings on first call  //
} // of class constructor                                                     //----------------------------------//
CubigelClass::CubigelClass( HardwareSerial *ser1,                             // Overloaded Class constructor     //
                            HardwareSerial *ser2) {                           // with 2 devices - HW then HW      //
  ClassPtr      = this;                                                       // pointer to current instance      //
  devices[0].serialHW = ser1;                                                 // point to the appropriate port    //
  devices[0].serialHW->begin(CUBIGEL_BAUD_RATE);                              // Set baud rate to Cubigel speed   //
  devices[1].serialHW = ser2;                                                 // point to the appropriate port    //
  devices[1].serialHW->begin(CUBIGEL_BAUD_RATE);                              // Set baud rate to Cubigel speed   //
  _deviceCount = 2;                                                           // Store the number of devices      //
  StartTimer();                                                               // Enable timer interrupts          //
  setMode(0,MODE_SETTINGS);                                                   // Retrieve settings on first call  //
  setMode(1,MODE_SETTINGS);                                                   // Retrieve settings on first call  //
} // of class constructor                                                     //----------------------------------//
/*******************************************************************************************************************
** function StartTimer() is called as part of class instantiation to enable internal timing. The code uses the    **
** Timer0 interrupt (also used by the millis() function) which is an 8 bit register with a clock divisor of 64    **
** which triggers it to overflow at a rate of 976.5625Hz, or roughly every millisecond. We set TIMER0_COMPA_vect  **
** to 0x01 which triggers when the value is equal to 64. This gives us an identical trigger speed to the millis() **
** function but at a different trigger point.                                                                     **
*******************************************************************************************************************/
void CubigelClass::StartTimer() {                                             //                                  //
  cli();                                                                      // Disable interrupts               //
  OCR0A   = 0x40;                                                             // Comparison register A to 64      //
  TIMSK0 |= _BV(OCIE0A);                                                      // TIMER0_COMPA trigger on 0x01     //
  sei();                                                                      // Enable interrupts                //
} // of method StartTimer()                                                   //                                  //
/*******************************************************************************************************************
** Define the ISR (Interrupt Service Routine) for the timer event to check to see if we have incoming data from a **
** Cubigel device. This definition is done as a static function which can be set directly as part of the Arduino  **
** IDE inside a class definition. It, in turn, redirects the interrupt to a class member function where the       **
** actual interrupt is handled                                                                                    **
*******************************************************************************************************************/
ISR(TIMER0_COMPA_vect) {CubigelClass::TimerISR();}                            // Call the ISR every millisecond   //
static void CubigelClass::TimerISR()     {ClassPtr->TimerHandler();}          // Redirect to real handler function//
/*******************************************************************************************************************
** function TimerHandler() is linked to the millis() timer 0 interrupt. This is called every millisecond and we   **
** check to see if anything has arrived in the receive buffers. Each devices' buffer is checked and any that have **
** data in them are processed using the "processDevice"function, which does the heavy lifting.                    **
*******************************************************************************************************************/
void CubigelClass::TimerHandler() {                                           //                                  //
  for (uint8_t idx=0;idx<_deviceCount;idx++) {                                // For each defined device          //
    if (devices[idx].serialSW) {                                              // if we are using software serial, //
      if (devices[idx].serialSW->available()) processDevice(idx);             // check to see if there is any data//
    } else {                                                                  // otherwise we are using HW serial //
      if (devices[idx].serialHW->available()) processDevice(idx);             // check to see if there is any data//
    } // of if-then-else software or hardware serial                          //                                  //
  } // of for-next each defined device loop                                   //                                  //
} // of method TimerHandler()                                                 //                                  //
/*******************************************************************************************************************
** function readValues() is called to process the collected readings. The return value is the number of readings  **
** taken and the parameters are updated (pass by reference) with the current value. The default settings is that  **
** the statistics are reset after this call, but the optional resetReading parameter can override this setting    **
*******************************************************************************************************************/
uint16_t CubigelClass::readValues(const uint8_t idx,uint16_t &RPM,uint16_t &mA,//                                 //
                                  const bool resetReadings) {                 //                                  //
  if (idx>=_deviceCount) return 0;                                            // just return nothing if invalid   //
  cli();                                                                      // Disable interrupts               //
  RPM = devices[idx].totalRPM/devices[idx].readings;                          // set the averaged RPM value       //
  mA  = devices[idx].totalmA/devices[idx].readings;                           // set the averaged mA value        //
  uint16_t tempReadings = devices[idx].readings;                              // copy the readings value          //
  if (resetReadings) {                                                        // Reset the readings if so desired //
    devices[idx].readings       = 0;                                          // Set back to 0                    //
    devices[idx].totalRPM       = 0;                                          // Set back to 0                    //
    devices[idx].totalmA        = 0;                                          // Set back to 0                    //
    devices[idx].errorStatus    = 0;                                          // Set back to 0                    //
    devices[idx].commsErrors    = 0;                                          // Set back to 0                    //
  } // of if-then values are to be reset                                      //                                  //
  sei();                                                                      // Enable interrupts                //
  return tempReadings;                                                        // Return the number of readings    //
} // of method readValues                                                     //                                  //
/*******************************************************************************************************************
** function readValues() is called to return the collected statistics for a device. By default the readings are   **
** reset after this call, but optionally they can be retained.                                                    **
*******************************************************************************************************************/
uint16_t CubigelClass::readValues(const uint8_t idx,uint16_t &RPM,uint16_t &mA,//                                 //
                                  uint16_t &commsErrors, uint16_t errorStatus,//                                  //
                                  const bool resetReadings) {                 //                                  //
  if (idx>=_deviceCount) return 0;                                            // just return nothing if invalid   //
  cli();                                                                      // Disable interrupts               //
  RPM = devices[idx].totalRPM/devices[idx].readings;                          // set the averaged RPM value       //
  mA  = devices[idx].totalmA/devices[idx].readings;                           // set the averaged mA value        //
  commsErrors = devices[idx].commsErrors;                                     // set the number of comms errors   //
  errorStatus = devices[idx].errorStatus;                                     // set the Cubigel error status     //
  uint16_t tempReadings = devices[idx].readings;                              // copy the readings value          //
  if (resetReadings) {                                                        // Reset the readings if so desired //
    devices[idx].readings       = 0;                                          // Set back to 0                    //
    devices[idx].totalRPM       = 0;                                          // Set back to 0                    //
    devices[idx].totalmA        = 0;                                          // Set back to 0                    //
    devices[idx].errorStatus    = 0;                                          // Set back to 0                    //
    devices[idx].commsErrors    = 0;                                          // Set back to 0                    //
  } // of if-then values are to be reset                                      //                                  //
  sei();                                                                      // Enable interrupts                //
  return tempReadings;                                                        // Return the number of readings    //
} // of method readValues                                                     //                                  //
/*******************************************************************************************************************
** function setMode() is called to set which mode the Cubigel outputs data in. The default mode, MODE_DEFAULT,    **
** outputs the message type 76 which contains the compressor speed and current consumption.                       **
*******************************************************************************************************************/
void  CubigelClass::setMode(const uint8_t idx, const uint8_t mode) {          // Set Cubigel FDC1 mode            //
  uint8_t      modeByte = 0;                                                  // Byte to set state, default is 0  //
  if (mode==1) modeByte = 192;                                                // Mode 0 is the default            //
  if (idx>=_deviceCount) return;                                              // just return nothing if invalid   //
  if (devices[idx].serialSW) {                                                // if we are using software serial, //
    devices[idx].serialSW->write((uint8_t)72);                                // Write control information        //
    devices[idx].serialSW->write((uint8_t)80);                                // Write control information        //
    devices[idx].serialSW->write(modeByte);                                   // Write control type byte          //
    devices[idx].serialSW->write((uint8_t) 0);                                // Write control information        //
    devices[idx].serialSW->write((uint8_t) 0);                                // Write control information        //
    devices[idx].serialSW->write((uint8_t) 0);                                // Write control information        //
    devices[idx].serialSW->write((uint8_t)15);                                // Write control information        //
  } else {                                                                    //----------------------------------//
    devices[idx].serialHW->write((uint8_t)72);                                // Write control information        //
    devices[idx].serialHW->write((uint8_t)80);                                // Write control information        //
    devices[idx].serialHW->write(modeByte);                                   // Write control type byte          //
    devices[idx].serialHW->write((uint8_t) 0);                                // Write control information        //
    devices[idx].serialHW->write((uint8_t) 0);                                // Write control information        //
    devices[idx].serialHW->write((uint8_t) 0);                                // Write control information        //
    devices[idx].serialHW->write((uint8_t)15);                                // Write control information        //
  } // of if-then-else software or hardware serial is use                     //                                  //
} // of method setMode()                                                      //                                  //
/*******************************************************************************************************************
** function processDevice() is called when there is data received on the port for a specific device. The device   **
** number is passed in as a parameter and the rest of the logic is independent of which device it is.             **
*******************************************************************************************************************/
void CubigelClass::processDevice(const uint8_t idx) {                         //                                  //
  if (devices[idx].serialSW) {                                                // if we are using software serial, //
    devices[idx].buffer[devices[idx].index++]=devices[idx].serialSW->read();  // Read next byte into buffer       //
  } else {                                                                    //----------------------------------//
    devices[idx].buffer[devices[idx].index++]=devices[idx].serialHW->read();  // Read next byte into buffer       //
  } // of if-then-else software or hardware serial is use                     //                                  //
  if ( (devices[idx].index==1 && devices[idx].buffer[0]!=27) ||               // If the first byte isn't a 27 and //
       (devices[idx].index==2 && !(devices[idx].buffer[1]==76 ||              // the 2nd byte neither 76 nor 80   //
        devices[idx].buffer[1]==80))) {                                       // then reset the index to beginning//
    devices[idx].index = 0;                                                   // 27 then reset the index          //
    devices[idx].commsErrors++;                                               // increment the error counter      //
  } else {                                                                    // otherwise check next character   //
    if (devices[idx].buffer[1]==76 && devices[idx].index==8) {                // We have a complete 76 sentence   //
      if ((devices[idx].buffer[0] ^ devices[idx].buffer[2] ^                  // Two checksums are made with XOR  //
          devices[idx].buffer[4]) == devices[idx].buffer[6] &&                // from bytes 0,2,4 and 1,3,5 to    //
         (devices[idx].buffer[1] ^ devices[idx].buffer[3] ^                   // match bytes 6 and 7 respectively //
         devices[idx].buffer[5]) == devices[idx].buffer[7]) {                 // Ignore sentence on bad checksum  //
        devices[idx].readings++;                                              // increment the counter            //
        if (devices[idx].offTime>=devices[idx].onTime&&devices[idx].buffer[2]!=0){// Set the off and on times     //
          devices[idx].onTime      = millis();                                // when state of compressor changes //
          devices[idx].timeChanged = true;                                    // Set the change flag              //
        } else {                                                              //                                  //
          if (devices[idx].onTime>=devices[idx].offTime&&devices[idx].buffer[2]==0){// Set the off and on times   //
            devices[idx].offTime     = millis();                              // then set the time and            //
            devices[idx].timeChanged = true;                                  // Set the change flag              //
          } // of if-then the device turned off                               //                                  //
        } // of if-then the device turned on                                  //                                  //
        if (devices[idx].buffer[2]!=0) {                                      // Compressor running if non-zero   //
          devices[idx].totalRPM += (devices[idx].buffer[2]*256)+devices[idx].buffer[3]; // Add in RPM             //
          devices[idx].totalmA  += ((uint32_t)devices[idx].buffer[4]*256+devices[idx].buffer[5])*1000/3160;// mA  //
        } else {                                                              // otherwise system off, check for  //
          devices[idx].errorStatus != devices[idx].buffer[5];                 // OR the alarm codes together      //
        } // of if-then-else the fridge is on                                 //                                  //
      } else {                                                                //                                  //
        devices[idx].commsErrors++;                                           // Add to number of errors detected //
      } // of if-then-else checksums match                                    //                                  //
      devices[idx].index = 0;                                                 // Reset index for next frame       //
    } else {                                                                  //                                  //
      if (devices[idx].buffer[1]==80 && devices[idx].index==22) {             // We have a complete 80 sentence   //
        if ((                        72  ^ devices[idx].buffer[2]  ^          // Two checksums are made by XORing //
             devices[idx].buffer[4]  ^ devices[idx].buffer[6]  ^              // all the odd bytes and the even   //
             devices[idx].buffer[8]  ^ devices[idx].buffer[10] ^              // bytes to compare to the last 2   //
             devices[idx].buffer[12] ^ devices[idx].buffer[14] ^              // sentence values. Note that the   //
             devices[idx].buffer[16] ^ devices[idx].buffer[18])               // initial byte is "72" and not the //
             == devices[idx].buffer[20] &&                                    // actual 27 for purposes of check- //
             (devices[idx].buffer[1] ^ devices[idx].buffer[3] ^               // sum computation                  //
             devices[idx].buffer[5] ^ devices[idx].buffer[7] ^                //                                  //
             devices[idx].buffer[9] ^ devices[idx].buffer[11] ^               //                                  //
             devices[idx].buffer[13] ^ devices[idx].buffer[15] ^              //                                  //
             devices[idx].buffer[17] ^ devices[idx].buffer[19])               //                                  //
             == devices[idx].buffer[21]) {                                    //                                  //
          devices[idx].minSpeed  = devices[idx].buffer[2]*256+devices[idx].buffer[3]; // Get minimum RPM          //
          devices[idx].maxSpeed  = devices[idx].buffer[4]*256+devices[idx].buffer[5]; // Get maximum RPM          //
          devices[idx].cutOut12V = ((uint32_t)devices[idx].buffer[ 8]*256+devices[idx].buffer[ 9])*1000/1187;// 12V cutout volts//
          devices[idx].cutIn12V  = ((uint32_t)devices[idx].buffer[10]*256+devices[idx].buffer[11])*1000/1187;// 12V cutin volts //
          devices[idx].cutOut24V = ((uint32_t)devices[idx].buffer[12]*256+devices[idx].buffer[13])*1000/1187;// 24V cutout volts//
          devices[idx].cutIn24V  = ((uint32_t)devices[idx].buffer[14]*256+devices[idx].buffer[15])*1000/1187;// 24V cutin volts //
          devices[idx].cutOut42V = ((uint32_t)devices[idx].buffer[16]*256+devices[idx].buffer[17])*1000/1187;// 42V cutout volts//
          devices[idx].cutIn42V  = ((uint32_t)devices[idx].buffer[18]*256+devices[idx].buffer[19])*1000/1187;// 42V cutin volts //
          devices[idx].modeByte  = devices[idx].buffer[7];                    // Save bit register settings       //
          setMode(idx,MODE_DEFAULT);                                          // Reset device to default mode     //
        } // of if-then the checksum computation was successful               //                                  //
        devices[idx].index = 0;                                               // Reset index for next frame       //
      } // of if-then we have a type 80 sentence                              //                                  //
    } // of if-then we have a complete sentence                               //                                  //
  } // of if-then-else we are at the first byte                               //                                  //
} // of method ProcessDevice                                                  //                                  //
/*******************************************************************************************************************
** Function readTiming() is called to return the given device's last state change from ON-OFF or OFF-ON.          **
*******************************************************************************************************************/
bool CubigelClass::readTiming(const uint8_t idx, uint32_t &onTime,            //                                  //
                              uint32_t &offTime) {                            //                                  //
  bool tempTimeChanged     = devices[idx].timeChanged;                        // Store the current value to return//
  onTime                   = devices[idx].onTime;                             //                                  //
  offTime                  = devices[idx].offTime;                            //                                  //
  devices[idx].timeChanged = false;                                           // Reset the change flag if set     //
  return(tempTimeChanged);                                                    //                                  //
} // of method ReadTiming                                                     //                                  //
/*******************************************************************************************************************
** Function requestSettings() is called to retrieve a settings sentence from the given device and to store it in  **
** the memory structure.                                                                                          **
*******************************************************************************************************************/
void CubigelClass::requestSettings(const uint8_t idx) {                       //                                  //
  setMode(idx,MODE_SETTINGS);                                                 // Change to get a settings sentence//
} // of method reqeustSettings                                                //                                  //
/*******************************************************************************************************************
** Function readSettings() is called to return the given device's settings. These are read once during class      **
** instantiation and stored                                                                                       **
*******************************************************************************************************************/
void CubigelClass::readSettings(const uint8_t idx, uint16_t &compMin,         // Request a settings sentence      //
                                uint16_t &compMax, uint16_t &out12V,          //                                  //
                                uint16_t &in12V, uint16_t &out24V,            //                                  //
                                uint16_t &in24V,uint16_t &out42V,             //                                  //
                                uint16_t &in42V, uint8_t &mode) {             //                                  //
  compMin = devices[idx].minSpeed;                                            // Read values from structure into  //
  compMax = devices[idx].maxSpeed;                                            // return variables                 //
  out12V  = devices[idx].cutOut12V;                                           //                                  //
  in12V   = devices[idx].cutIn12V;                                            //                                  //
  out24V  = devices[idx].cutOut24V;                                           //                                  //
  in24V   = devices[idx].cutIn24V;                                            //                                  //
  out42V  = devices[idx].cutOut42V;                                           //                                  //
  in42V   = devices[idx].cutIn42V;                                            //                                  //
  mode    = devices[idx].modeByte;                                            //                                  //
} // of method ReadSettings                                                   //                                  //
