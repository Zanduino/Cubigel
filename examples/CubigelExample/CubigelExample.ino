/*! @file CubigelExample.ino

@section CubigelExample_intro_section Description
This program is a simple example for the Cubigel compressor communications library class. The class,
once instantiated, will automatically collect statistics in the background using an interrupt so
that any program using the library can do any processing it needs until such time as it requests the
most recent statistics. The system settings are only collected once when the class is instantiated,
any subsequent changes by another program will not get picked up.

The "SoftwareSerial library" (see https://www.arduino.cc/en/Reference/SoftwareSerial) can be used
for one device, but not for more. This is because the library only looks for pin change interrupts
on one port at a time, and since this library is designed to work in the background using it's own
interrupt to get triggered it cannot switch the SoftwareSerial active port around at the same time.
This means that if two (or potentially more) devices are to be monitored then all but one must use
the Arduino's UARTs. This, in turn, means that only such Atmel chips as the ATMega, which has 4
hardware UARTs, can be used if one wishes to monitor more than 2 devices.

@section CubigelExampleLicense License

This program is free software: you can redistribute it and/or modify it under the terms of the GNU
General Public License as published by the Free Software Foundation, either version 3 of the
License, or (at your option) any later version. This program is distributed in the hope that it will
be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details. You should have
received a copy of the GNU General Public License along with this program.  If not, see
<http://www.gnu.org/licenses/>.

@section CubigelExampleAuthor Author

Written by Arnd <Arnd@Zanduino.Com> at https://www.github.com/SV-Zanshin

@section CubigelExampleVersions Changelog

Version | Date       | Developer  | Comments
------- | ---------- | ---------- | ---------------------------------------------------------------
1.0.4   | 2020-12-07 | SV-Zanshin | Reformatted using doxygen comments
1.0.3   | 2020-06-28 | SV-Zanshin | Changed comment style to clang-format LLVM
1.0.2   | 2020-06-25 | SV-Zanshin | Fixed comments
1.0.1   | 2017-08-21 | SV-Zanshin | Fixed comments, removed extraneous code
1.0.0b  | 2017-02-21 | SV-Zanshin | Initial coding
*/

#include <Cubigel.h>         // Include Cubigel library
#include <SoftwareSerial.h>  // Software serial port emulation
///< Define the different types of devices
enum DeviceName { Fridge, Freezer, LastElement };
const uint32_t INTERVAL_MILLIS{60000};  ///< default 60s interval between readings
const uint8_t  FRIDGE_RX_PIN{52};       ///< Pin for fridge serial receive RX
const uint8_t  FRIDGE_TX_PIN{53};       ///< Pin for fridge serial transmit TX
SoftwareSerial FridgeSerial(FRIDGE_RX_PIN, FRIDGE_TX_PIN);  ///< Instantiate Fridge serial port
CubigelClass   Cubigel(&FridgeSerial, &Serial1);            ///< One soft and one hardware port
static char    sprintfBuffer[32];                           ///< Buffer for formatted text output

void setup() {
  /*!
    @brief    Arduino method called once at startup to initialize the system
    @details  This is an Arduino IDE method which is called first upon boot or restart. It is only
              called one time and then control goes to the main "loop()" method, from which
              control never returns
    @return   void
  */
  Serial.begin(115200);    // Initialize Serial I/O at speed
#ifdef __AVR_ATmega32U4__  // If a 32U4 processor, then wait 3s to init port
  delay(3000);
#endif
  while (!Serial) {};  // Give serial port time to start
  Serial.println(F("Cubigel example program [v1.0.4]"));
  delay(1100);  // 2 sentences get sent per second,
  delay(1100);  // Repeat for first data sentence
  Serial.println(
      F("_______________________________________________________________________________"));
  Serial.println(
      F("|        | Compressor |      12V      |      24V      |      42V      | Run   |"));
  Serial.println(
      F("| Device |  min/max   |  Out  /  In   |  Out  /  In   |  Out  /  In   | Mode  |"));
  Serial.println(
      F("|========|============|===============|===============|===============|=======|"));
  for (uint8_t idx = 0; idx < LastElement; idx++) {  // Loop for every device
    uint16_t compMin, compMax, out12V, in12V, out24V, in24V, out42V,
        in42V;     // declare temporary variables
    uint8_t mode;  // declare temporary variables
    Cubigel.readSettings(idx, compMin, compMax, out12V, in12V,
                         out24V,  // Get the most recent values
                         in24V, out42V, in42V, mode);
    if (idx == Fridge)
      Serial.print(F("| Fridge"));  // Display appropriate device name
    else if (idx == Freezer)
      Serial.print(F("| Freeze"));
    else
      Serial.print(F("| ??????"));
    sprintf(sprintfBuffer, " | %04d/%04d  | ", compMin, compMax);
    Serial.print(sprintfBuffer);
    sprintf(sprintfBuffer, "%2d.%03d/%2d.%03d | ", out12V / 1000, out12V % 1000, in12V / 1000,
            in12V % 1000);
    Serial.print(sprintfBuffer);
    sprintf(sprintfBuffer, "%2d.%03d/%2d.%03d | ", out24V / 1000, out24V % 1000, in24V / 1000,
            in24V % 1000);
    Serial.print(sprintfBuffer);
    sprintf(sprintfBuffer, "%2d.%03d/%2d.%03d |", out42V / 1000, out42V % 1000, in42V / 1000,
            in42V % 1000);
    Serial.print(sprintfBuffer);
    if ((mode & 12) == 12)
      Serial.print(F("Energy"));
    else if (mode & 8)
      Serial.print(F(" Sleep"));
    else if (mode & 4)
      Serial.print(F(" Smart"));
    else
      Serial.print(F("IN 1/2"));
    Serial.println(F(" |"));
  }  // of for-next loop for each device
  Serial.println(
      F("|________|____________|_______________|_______________|_______________|_______|"));
  Serial.println(F("\nRdgs  = Number of readings"));
  Serial.println(F("RPM   = Compressor speed"));
  Serial.println(F("mA    = milliAmps current consumption"));
  Serial.println(F("Cycle = Number of minutes ON/OFF cycle has lasted"));
  Serial.println(F("\n_________________________________________________"));
  Serial.println(F("| Fridge                | Freezer               |"));
  Serial.println(F("|-----------------------|-----------------------|"));
  Serial.println(F("| Rdgs RPM  mA    Cycle | Rdgs RPM  mA    Cycle |"));
  Serial.println(F("| ==== ==== ===== ===== | ==== ==== ===== ===== |"));
}  // of method "setup()"

void loop() {
  /*!
    @brief    Arduino method for the main program loop
    @details  This is the main program for the Arduino IDE, it is an infinite loop and keeps on
              repeating.
    @return   void
  */
  static unsigned long nextInterval  = 0;                   // store time for next display
  static uint32_t      tempStartTime = 0;                   // store the last compressor start
  static uint32_t      tempStopTime  = 0;                   // store the last compressor stop
  if (millis() > nextInterval) {                            // If it is time to display values
    uint16_t readings, RPM, mA, CommsErrors, CubigelError;  // Temporary variables
    for (uint8_t idx = 0; idx < LastElement; idx++) {       // Loop for every device enumerated
      readings = Cubigel.readValues(idx, RPM, mA, CommsErrors, CubigelError);  // Read and reset
      uint16_t cycleSecs = 0;                                      // store the cycle time
      if (Cubigel.readTiming(idx, tempStartTime, tempStopTime)) {  // Read the Start/Stop time
        if (tempStartTime > tempStopTime)  // When we have a start event, then
          cycleSecs = (tempStartTime - tempStopTime) / 60000;
        else
          cycleSecs = (tempStopTime - tempStartTime) / 60000;
      }                // of if-then the state has changed
      if (readings) {  // If anything has been read, show
        if (idx == 0)
          Serial.print(F("| "));
        else
          Serial.print(F(" | "));  // Print separator when necessary
        sprintf(sprintfBuffer, "%4d %4d %5d ", readings, RPM, mA);  // Format output string
        Serial.print(sprintfBuffer);                                // Print the formatted string
        if (cycleSecs == 0)
          Serial.print(F("     "));  // Print spaces if no cycle change
        else {
          sprintf(sprintfBuffer, "%5d", cycleSecs);  // Format output string
          Serial.print(sprintfBuffer);               // Print the formatted string
        }                                            // of if-then-else we have a cycle change
      } else {
        Serial.print(F("   no data   "));
      }  // of if-then we have measurements
    }    // of for-next loop through all devices
    Serial.println(F(" |"));
    nextInterval = millis() + INTERVAL_MILLIS;  // Set next interval value
  }                                             // of if-then time to display
}  // of method loop()
