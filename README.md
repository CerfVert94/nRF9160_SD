# nRF9160_SD
A sample code to read SD card, using microcontroller nRF9160.<br>
The code is based on the Arduino SD library (https://github.com/arduino-libraries/SD), and it is modified and redistributed under the GPL-3.0 license.<br>
The library is rewritten in C and adapted to nRF9610, using Segger Embedded Studio.<br>
But as long as the SPI protocol can be accordingly modified, it can be used for any other microcontrollers.<br>
(For example, the code has been modified for STM32 series microcontrollers and it worked well.)<br>

For a fast implementation, the code was written without any consideration in regards to readability or whatever it helps readers to understand the code.<br>
Write functionality is unsupported as well.<br>

It urgently needs a refactoring, a clean-up and pinout description to make understand how it works,<br>
but this is unlikely to happen as the author doesn't have the microcontroller to test it in his possession anymore.<br>
