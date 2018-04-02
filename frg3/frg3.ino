/*
 * fridge.ino 
 * Copyright (c) Dec 4, 2017. @ Larry Littlefield www.TCLS.com kb7kmo.blogspot.com
 * GPL v2 
 * https://github.com/svkatielee/
 */
 
/* Arduino IDE 64bit rev 1.8.5  esp8266 ver. 2.4.0-rc2
 * 
 * This program should read a temprature sensor, 
 *     https://github.com/adafruit/MAX31850_DallasTemp/issues
 *     http://mydomotic.blogspot.tw/2015/10/configure-ds18b20-resolution-and-save.html
 *     OneWire https://www.pjrc.com/teensy/td_libs_OneWire.html by Jim Studt loaded via lib manager
 *     https://milesburton.com/Dallas_Temper…  loaded via lib manager*   
 * periodically send to mqtt
 *     https://pubsubclient.knolleary.net/ loaded via library manager
 * with deep sleep inbetween.
 *     https://github.com/esp8266/Arduino/issues/1488
 * allow OTA update
 *     http://arduino-esp8266.readthedocs.io/en/latest/ota_updates/readme.html
 * RTC DS3231
 *     https://github.com/Makuna/Rtc/tree/master/examples/DS3231_Simple install via library mgr
 * 
 * For Wemos D1 mini ESP82666:
 *  wire jumper from GPIO16 (D0) to reset - might should be resist6or 1k seems ok
 *     else problems with flashing 
 *     (later changed to a slide switch for direct flashing vs. deep sleep wake)
 *  jumper A0 to 3.3v out because D1 mini has a voltage divider in A0 already.    
 *  
 * Dalas temp waterproof 3 wires: OneWire interface
 *   red to 3.3v666
 *   black ground
 *   yellow D1
 *   D1 -> 4.7k ->3.3v
 *   
 * Add OTA  
 *   no password, set hostname 
 */

 const char *gRev = "frg3-0.9.4";  // Software Revision Code, increment for OTA
/*
*  0.9.0 First try new refactored
*  0.9.1 add OTA back in
*    
 */
//#define DEBUG_ESP_HTTP_UPDATE
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>

#define SLEEPTIME 900 * 1000000      // seconds 10 000 000  600=10 minutes

// Static IP details...
IPAddress ip(192, 168, 11, 61);
IPAddress gateway(192, 168, 11, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress dns(192, 168, 11, 1);
const char* ssid     = "tcls";
const char* password = "svkatielee";

#include <OneWire.h>
OneWire  ds(13);  // on GPIO16 (a 4.7K resistor is necessary) For DS18B20 Dallas tempreature
#include <DallasTemperature.h>
DallasTemperature sensors(&ds);
DeviceAddress fridgeTemp;

#include <Wire.h> // For RTC timestamps6
#include <RtcDS3231.h>
RtcDS3231<TwoWire> Rtc(Wire);

IPAddress mqtt_server(192, 168, 11, 30);
WiFiClient espClient;
PubSubClient client(espClient);
char msg[50];
String clientId = "Fridge";   // Create a client ID

ADC_MODE(ADC_VCC);

// OTA config --------------------------------
void checkForOTA() {
  t_httpUpdate_return ret = ESPhttpUpdate.update("192.168.11.30", 80, "/serv.php", gRev ); // only md5sum, not use version
        
  // It does NOT return from successfull update!!
  snprintf (msg, 50, "Update successful");
  switch(ret) {
       case HTTP_UPDATE_FAILED:
            Serial.printf("HTTP_UPDATE_FAILD Error (%d): %s", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
            Serial.println();
            snprintf (msg, 50, "HTTP_UPDATE_FAILD Error (%d): %s", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str()); 
            break;

       case HTTP_UPDATE_NO_UPDATES:
            Serial.println("HTTP_UPDATE_NO_UPDATES");
            snprintf (msg, 50, "No newer update available");
            break;

       case HTTP_UPDATE_OK:
            Serial.println("HTTP_UPDATE_OK");
            snprintf (msg, 50, "Update successful");
            break;
        }
}

// setup ------------------------------------------------------
void setup()
{
  unsigned long startTime = millis();
  byte ret_code;
  
  Serial.begin(115200);
  Serial.setDebugOutput(false);

  Serial.println();
  Serial.println("--> Awake!");

  Serial.println("--> Connecting Wifi");
 // if (strcmp (WiFi.SSID().c_str(), ssid) != 0) {
    byte timeout = 0;  
  
    WiFi.config(ip, gateway, subnet, dns);
    delay(100);
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
      if (timeout++ >= 15) {
        Serial.println("--> ERROR: Wifi commection timeout");
        break;
      }
    }
  //}
  Serial.println();

   // check for OTA update
   checkForOTA();
   yield();
   delay(100);

// initialize the Dallas Temp stuff, start conversion 
  sensors.begin();
  if (!sensors.getAddress(fridgeTemp, 0)) Serial.println("Unable to find address for Device 0"); 

  Serial.print("Dallas Temp Device 0 Address: ");
  Serial.print(IPAddress(fridgeTemp));
  Serial.print("   Device 0 Resolution: ");
  Serial.print(sensors.getResolution(fridgeTemp), DEC); 
  Serial.println();
  
  sensors.requestTemperatures(); // Send the command to start temperatures conversion

 //  RTC
  Rtc.Begin();

  
  Serial.print("compiled: ");
  Serial.print(__DATE__);
  Serial.println(__TIME__);


    RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
//    printDateTime(compiled);
    Serial.println();

    if (!Rtc.IsDateTimeValid()) {
        Serial.println("RTC lost confidence in the DateTime!");
        Rtc.SetDateTime(compiled);
    }

    if (!Rtc.GetIsRunning()) {
        Serial.println("RTC was not actively running, starting now");
        Rtc.SetIsRunning(true);
    }

    RtcDateTime now = Rtc.GetDateTime();
    if (now < compiled) {
        Serial.println("RTC is older than compile time!  (Updating DateTime)");
        Rtc.SetDateTime(compiled);
    }

    // never assume the Rtc was last configured by you, so
    // just clear them to your needed state
    Rtc.Enable32kHzPin(false);
    Rtc.SetSquareWavePin(DS3231SquareWavePin_ModeNone); 
   
  ///==========
  yield();

  Serial.println("Sensor reads");
  //uint32_t timestamp = now.TotalSeconds();  // since 1/1/20006
  uint32_t timestamp = now.Epoch32Time();
  //uint32_t timestamp = millis();
  
  long val=ESP.getVcc();   
  long battery = map((val),2621,  4416, 2500, 4200);

  float tempC = sensors.getTempC(fridgeTemp);
  float tempF = DallasTemperature::toFahrenheit(tempC); // Converts tempC to Fahrenheit =====================
  int temp = int(tempF);


      Serial.print("timestamp: ");
      Serial.print(timestamp);
      Serial.print(", battery: ");
      Serial.print(battery);
      Serial.print(", Temp: ");
      Serial.println(temp);

    // Connect to mosquitto server//==================
    client.setServer(mqtt_server, 1883);
    String clientId = "Fridge";
    while (!client.connected()) {
        Serial.print("Attempting MQTT connection...");
        if (client.connect(clientId.c_str())) {
          Serial.println("connected");
        } else {
          Serial.print("failed, rc=");
          Serial.print(client.state());
      
          if(client.state() == -2 ) {
            Serial.println("MQTT:  WiFi not connected.");
          }
        } 
    }
    
    yield();
    
    Serial.println("Sending ");  
    snprintf (msg, 75, "{\"time\": %d, \"vcc\": %d, \"Temp\": %d}", timestamp, battery , temp); //==================
    if (!client.publish("tcls/Fridge/temp", msg, true) ) {
        Serial.println("Failed to publish !!!");
    } else {
      Serial.print("Publish message: "); Serial.println(msg); 
    }
    yield();
    
    int cycle = millis() - startTime;
    snprintf (msg, 75, "{\"Revision\": %s, \"Duration\": %d}", gRev, cycle); 
    if (!client.publish("tcls/Fridge/rev", msg, true) ) {
        Serial.println("Failed to publish !!!");
    } else {
        Serial.print("Publish message: "); Serial.println(msg); 
    }
    delay(100);
    yield();
    Serial.println("--> Disconnecting MQTT");
    client.disconnect(); 

   
    Serial.print("Duration: "); Serial.println(millis() - startTime);
  
    Serial.println("--> Deep sleep.");
  Serial.println("before sleep W/O RF");
  ESP.deepSleep(SLEEPTIME, WAKE_RFCAL);
  delay(10);

} //setup


void loop()
{
  Serial.println("--> Never gets here!");
}




 
