This project utilizes two ESP32 devices:
* LILYGO T-Disply S3
* 2  XIAO ESP32C3

The ESP32C3 takes temperatures from 2 Dallas DS18b temperature probes and sends the value to the T-Display S3 through ESP NOW.

In order to utilize the one-to-many communciation, the MAC address of each receiver must be known by the transmitter.
