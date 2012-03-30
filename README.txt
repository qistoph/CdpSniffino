CdpSniffino

Arduino based CDP sniffer.

Currently this is a work in progress.
More details are on my blog http://qistoph.blogspot.com/2012/03/arduino-cdp-viewer.html

Requirements:
- Arduino Ethernet Shield
- Serial connection
- DebounceButton library (https://github.com/qistoph/ArduinoDebounceButton)

Received CDP packets are (partially) interpreted and extracted information is send on the serial connection.
Future plans include displaying the CDP information on an attached (I2C?) LCD.
