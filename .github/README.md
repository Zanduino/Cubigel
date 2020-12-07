[![License: GPL v3](https://zanduino.github.io/Badges/GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0) [![Build](https://github.com/Zanduino/Cubigel/workflows/Build/badge.svg)](https://github.com/Zanduino/Cubigel/actions?query=workflow%3ABuild)] [![c++ Format](https://github.com/Zanduino/Cubigel/workflows/c++%20Format/badge.svg)](https://github.com/Zanduino/Cubigel/actions?query=workflow%3A%22c%2B%2B+Format%22) [![Wiki](https://zanduino.github.io/Badges/Documentation-Badge.svg)](https://github.com/Zanduino/Cubigel/wiki) [![Doxygen](https://github.com/Zanduino/Cubigel/workflows/Doxygen/badge.svg)](https://Zanduino.github.io/Cubigel/html/index.html)

 # Cubigel library
<img src="https://github.com/SV-Zanshin/Cubigel/blob/master/Images/HuayiCompressor.png" width="175" align="right"/> *Arduino* library for communicating with any compressor in the [Cubigel family](http://www.huayicompressor.es/) which uses their proprietery [FDC1](https://www.sv-zanshin.com/r/manuals/cubigel_fdc1_communication_protocol.pdf) communication protocol. The library allows reading the programmed compressor settings as well as the data sentences that are sent twice a second from the compressor.
Currently the library is limited to accessing 2 devices simultaneously, but it can easily be modified to support more; this limit was chosen to minimize memory usage and simplify implementation; particularly as most installations will have at most 2 compressors - one for the refrigerator and one for the freezer.
The library collects data in the background (piggybacking off the [TIMER0_COMPA](https://learn.adafruit.com/multi-tasking-the-arduino-part-2/timers) interrupt) and does not require manual polling to function, freeing up the Arduino/Atmel to perform other tasks. The data sentences containing RPM and amperage values are averaged automatically so that the correct value since the last reading is always returned regardless of how long it takes between library calls to retrieve the data.

## Communication Protocol
The manufacturer has published several documents regarding communicating with the FDC1 controller on their website. The main FDC1 document is [GD30FDC User Manual](http://www.huayicompressor.es/phocadownload/user-manuals/user_manual_gd30fdc.pdf) and the definition of the communication protocol can be found at [FDC1 Communication Protocol](https://www.sv-zanshin.com/r/manuals/cubigel_fdc1_communication_protocol.pdf)

## Cubigel Documentation
The supporting documents from Cubigel have been copied into the [documents](https://github.com/Zanduino/Cubigel/tree/master/documents) folder

Details of library and methods are described on the [GitHub Cubigel Wiki](https://github.com/SV-Zanshin/Cubigel/wiki) pages along withe example sketches.

[![Zanshin Logo](https://zanduino.github.io/Images/zanshinkanjitiny.gif) <img src="https://zanduino.github.io/Images/zanshintext.gif" width="75"/>](https://www.sv-zanshin.com)
