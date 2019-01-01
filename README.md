README.md for github.com

#This repo is for saving my 'mostly working' ESP projects


## frg3

Monitor refrigerator temperature and battery with
- DS18B20
- one wire
- OTA
- deepsleep
- mqtt to local mosquitto server
- static IP4

## Boat_volt_mon

Monitors the house bank of batteries in my sailboat to mqtt
- mqtt to local mosquitto server
- static IP4
- OTA

## Boat_volt_mon2
- lots of changes but more in mon4

## Boat_volt_mon4
- refactored entire program
-- conditional include modules
-- modules in tabs (Arduino IDE tabs)
- OTA http pull mode
- MQTT to local server
- ADS1115 A to D differential and direct
- hosts its own status webpage
