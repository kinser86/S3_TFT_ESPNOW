## Overview
This example explores two methods of transmitting/receiving data via ESPNOW. The ESP32C3 takes temperatures from 2 Dallas DS18B20 temperature probes and sends the value to the T-Display S3 through ESP NOW.
  
### This first example transmits data from 1 device and is received by 1 device.  
  * transmitterOne
  * receiverTdisplayS3
#### This project utilizes two ESP32 devices:
* Transmitter
  * XIAO ESP32C3
* Receiver
  * LILYGO T-Disply S3 
### The second example transmits data from 1 device and is received by 2 devices.  
  * transmitterOneToMany
  * receiverTdisplayS3
  * recieverEsp32C3  

#### This project utilizes three ESP32 devices:
* Transmitter
  * XIAO ESP32C3
* Receiver
  * LILYGO T-Disply S3
  * XIAO ESP32C3 Seeed Studio Round Display


Note: In order to utilize the one-to-many communciation, the MAC address of each receiver must be known by the transmitter.
