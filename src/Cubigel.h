// clang-format off
/*! @file Cubigel.h

@mainpage Arduino Library to access and control a Cubigel compressor

@section Cubigel_section Description

This program defines the Cubigel class header. The Cubigel compressors that support the FDC1
protocol can be accessed using this library. The detailed protocol description can be downloaded
as a PDF file from the GitHub repository "documents" directory.

This library uses the Arduino SoftwareSerial library, and it is important to only use those pins
which support pin change interrupts for the fridge and freezer inputs. A good description of the
pins and their uses can be found at https://www.arduino.cc/en/Reference/AttachInterrupt.

While the Cubigel transmits data every 0.5 seconds, this library is designed to use its own
interrupt to read the data and to compile statistics until such time as the calling program is ready
to consume it. The Arduino SoftwareSerial library is used, but as it only supports a single port at
a time it will not work if more than one device uses the library. This library supports the use of
both hardware and software serial ports, and if more than one device is used then at most one port
can use the SoftwareSerial library/class. The pointer to the appropriate class instance is passed in
to this library during class instantiation and the constructor is overloaded to reflect the various
options available.

Although programming for the Arduino and in c/c++ is new to me, I'm a professional programmer and
have learned, over the years, that it is much easier to ignore superfluous comments than it is to
decipher non-existent ones; so both my comments and variable names tend to be verbose. There are
several parts of code which can be somewhat optimized, but in order to make the c++ code more
understandable by non-programmers some performance has been sacrificed for legibility and
maintainability.

@section doxygen doxygen configuration

This library is built with the standard "Doxyfile", which is located at
https://github.com/Zanduino/Common/blob/main/Doxygen. As described on that page, there are only 5
environment variables used, and these are set in the project's actions file, located at
https://github.com/Zanduino/Cubigel/blob/master/.github/workflows/ci-doxygen.yml
Edit this file and set the 5 variables -  PRETTYNAME, PROJECT_NAME, PROJECT_NUMBER, PROJECT_BRIEF
and PROJECT_LOGO so that these values are used in the doxygen documentation.
The local copy of the doxyfile should be in the project's root directory in order to do local
doxygen testing, but the file is ignored on upload to GitHub.

@section clang-format
Part of the GitHub actions for CI is running every source file through "clang-format" to ensure
that coding formatting is done the same for all files. The configuration file ".clang-format" is
located at https://github.com/Zanduino/Common/tree/main/clang-format and this is used for CI tests
when pushing to GitHub. The local file, if present in the root directory, is ignored when
committing and uploading.

@section license License

This program is free software: you can redistribute it and/or modify it under the terms of the GNU
General Public License as published by the Free Software Foundation, either version 3 of the
License, or (at your option) any later version. This program is distributed in the hope that it will
be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details. You should have
received a copy of the GNU General Public License along with this program.  If not, see
<http://www.gnu.org/licenses/>.

@section author Author

Written by Arnd <Arnd@Zanduino.Com> at https://www.github.com/SV-Zanshin

@section versions Changelog

Version | Date       | Developer  | Comments
------- | ---------- | ---------- | ---------------------------------------------------------------
1.0.4   | 2020-12-07 | SV-Zanshin | Converted to doxygen commenting
1.0.3   | 2020-09-30 | SV-Zanshin | Issue #2 - convert sources to clang-format compatibility
1.0.2   | 2017-08-21 | SV-Zanshin | Removed extraneous code, changed comments
1.0.1   | 2017-07-31 | SV-Zanshin | Prototypes contain optional parameter definitions, the functions no longer have them declared as non-Windows compilers fail when they do
1.0.0   | 2017-04-19 | SV-Zanshin | Cleaned up debugging code and ready for first release
1.0.b1  | 2017-04-18 | SV-Zanshin | Added hardware serial in addition tosoftware serial
1.0.b2  | 2017-02-21 | SV-Zanshin | Initial coding
*/
// clang-format on

#include "Arduino.h"                     // Arduino data type definitions
#include "SoftwareSerial.h"              // Software serial port emulation
#ifndef Cubigel_h                        // Guard code definition
  #define Cubigel_h                       ///< Define the name inside guard code
const uint16_t CUBIGEL_BAUD_RATE{1200};  ///< Cubigel has a fixed baud rate
const uint8_t  CUBIGEL_MAX_DEVICES{2};   ///< Max number of devices supported
const uint8_t  MODE_DEFAULT{0};          ///< Default output mode
const uint8_t  MODE_SETTINGS{1};         ///< Output settings mode
/*! @brief  this structure contains all of the variables stored per Cubigel device */
typedef struct {
  HardwareSerial *serialHW;     ///< Pointer to HardwareSerial
  SoftwareSerial *serialSW;     ///< Pointer to SoftwareSerial
  uint8_t         buffer[22];   ///< Data buffer for device
  uint8_t         index;        ///< Current index into data buffer
  uint16_t        readings;     ///< Number of readings stored
  uint32_t        totalRPM;     ///< Sum of all RPM values
  uint32_t        totalmA;      ///< Sum of all Milliamps values
  uint8_t         errorStatus;  ///< OR'd values of all errors found
  uint16_t        commsErrors;  ///< Number of communications errors
  uint16_t        minSpeed;     ///< Minimum speed setting
  uint16_t        maxSpeed;     ///< Maximum speed setting
  uint16_t        cutOut12V;    ///< 12V Cut out voltage
  uint16_t        cutIn12V;     ///< 12V Cut in  voltage
  uint16_t        cutOut24V;    ///< 24V Cut out voltage
  uint16_t        cutIn24V;     ///< 24V Cut in  voltage
  uint16_t        cutOut42V;    ///< 42V Cut out voltage
  uint16_t        cutIn42V;     ///< 42V Cut in  voltage
  uint8_t         modeByte;     ///< Mode setting switches
  bool            timeChanged;  ///< Set to true when turned ON/OFF
  uint32_t        onTime;       ///< Last millis() for an ON event
  uint32_t        offTime;      ///< Last millis() for an OFF event
} CubigelDataType;              ///< of CubigelDataType declaration

class CubigelClass {
  /*!
   * @class CubigelClass
   * @brief Main CubigelClass class for the Cubigel Compressor
   */
 public:                                                           // Publicly visible class members
  CubigelClass(SoftwareSerial *serial1);                           // 1 Software device
  CubigelClass(HardwareSerial *serial1);                           // 1 Hardware device
  CubigelClass(HardwareSerial *serial1, SoftwareSerial *serial2);  // 2 devices - HW then SW
  CubigelClass(SoftwareSerial *serial1, HardwareSerial *serial2);  // 2 devices - SW then HW
  CubigelClass(HardwareSerial *serial1, HardwareSerial *serial2);  // 2 devices - HW and HW
  uint16_t    readValues(const uint8_t idx, uint16_t &RPM, uint16_t &mA,  // Just return RPM and mA
                         const bool resetReadings = true);
  uint16_t    readValues(const uint8_t idx, uint16_t &RPM,
                         uint16_t & mA,  // return number of readings and
                         uint16_t & commsErrors,
                         uint16_t   errorStatus,  // update parameters with values
                         const bool resetReadings = true);
  void        readSettings(const uint8_t idx,
                           uint16_t &    compMin,  // Return the settings values
                           uint16_t &compMax, uint16_t &out12V, uint16_t &in12V, uint16_t &out24V,
                           uint16_t &in24V, uint16_t &out42V, uint16_t &in42V, uint8_t &mode);
  void        requestSettings(const uint8_t idx);  // Request a settings measurement
  bool        readTiming(const uint8_t idx, uint32_t &onTime, uint32_t &offTime);  //  changes
  static void TimerISR();                                 // Interim ISR calls real handler
 private:                                                 // Declare private class members
  void setMode(const uint8_t idx, const uint8_t mode);    // Set Cubigel FDC1 mode
  void StartTimer() const;                                // set the interrupt vector
  void processDevice(const uint8_t deviceNumber);         // read and store data for a device
  void TimerHandler();                                    // Called every millisecond for fade
  static CubigelClass *    ClassPtr;                      // store pointer to class itself
  bool                     _freezerPresent = false;       // Switch denoting if a freezer set
  uint8_t                  _deviceCount    = 0;           // Number of devices instantiated
  volatile CubigelDataType devices[CUBIGEL_MAX_DEVICES];  // Declare storage for max devices
};  // of class header definition for CubigelClass
#endif
