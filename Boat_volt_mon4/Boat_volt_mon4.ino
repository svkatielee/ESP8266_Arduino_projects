/*
 *
  * Copyright (c) 2018, Larry G Littlefield - LGL - KB7KMO - larryl@tcls.com
  *
  * See Blog at: http://KB7KMO.blogspot.com
  *
  * All rights reserved. 
*/ 
 /* 
 Batt_volt_mon4 Is to monitor boat house battery with ADS1115 A to D xonverter
 and send mqtt to op2e
 Also does OTA:
    on pot run Arduino IDE compile, then Sketch->Export Comp Bin
    then scp ~/Arduino/Boat_volt_mon2/Boat_volt_mon2.ino.d1_mini.bin rop:/var/www/html/bin/
    then the next cycle it will update then reboot and run new code.

 built a new board with connector for buck, ADC and Wemos D1 Mini. soldered in the trimmer pot       
    new board, so new rev 2.0

    refactored to tabs, added a status web page so gRev to 0.3.x
    finish refactor 0.4.1
    average current 0.4.2   ! didn't work! removed
    0.4.3 clean up  webpage 
    0.4.4 adjust VCC below 5V, add copyright
          reversed shunt wires so (* -1)
    0.4.5 reduce Buck from 5.25 to 5.02
          reset amp hours daily
    0.4.6 consolidate the mqttpub, more with callback, init ahour-10.0
    0.4.7 changed gain of current back to GAIN_ONE to see if fix the spurious reads
    0.4.8 changed circuit to add RC filter on current, changed current correction and multip
    0.4.9 removed RC filterset gain amd multiplier back
    0.5.0 change asd to use median value, 2 second cycle, 10 second mqtt, lots debug
*/
#define COPYRIGHT1 PSTR("\
     /* \
      * Copyright (c) 2018, Larry G. Littlefield - LGL - KB7KMO - larryl@tcls.com\
      * \
      * See Blog at: <a href='http://KB7KMO.blogspot.com'>http://KB7KMO.blogspot.com</a>\
      * \
      * All rights reserved. \
      */ \
    ")
const char *gRev = "bat-0.5.0";  // Software Revision Code

#include <ESP8266WiFi.h>
int mystate=3;
char msg[50];
char v_str[15], c_str[15], ah_str[15]; 
char c3[15], c4[15];
long curr_in[9]; byte cnt=0;
float volt, current, ahour=0.0, last_cur=0.0;
unsigned int op_hour=1;
int16_t adc1, adc2;
unsigned long ads_time = 1000;
unsigned long ads_dur = 0;
unsigned long last_ads = millis();
unsigned long mqtt_time = 10000;
unsigned long last_mqtt = millis();
unsigned long last_ota = millis();
unsigned long ota = 10000;
//unsigned long loop_count = 0; 
//unsigned long loop_count_avg =0;


#define DO_OTA
#ifdef DO_OTA
  #include "OTA.h"
#endif // DO_OTA

#define DO_PAGE
#ifdef DO_PAGE
  #include "webpage.h"
#endif // DO_PAGE

#define DO_ADS
#ifdef DO_ADS
  #include "ads.h"
#endif //DO_ADS

#define DO_MQTT
#ifdef DO_MQTT
  #include "mqtt.h"
#endif // DO_MQTT



// Static IP details...
IPAddress ip(192, 168, 11, 63);
IPAddress gateway(192, 168, 11, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress dns(192, 168, 11, 1);
const char* ssid     = "tcls";
const char* password = "svkatielee";


void setup() {
  
  byte ret_code;
  
  pinMode(LED_BUILTIN, OUTPUT);     // Initialize the LED_BUILTIN pin as an output
  digitalWrite(LED_BUILTIN, HIGH);
  Serial.begin(115200);
  Serial.setDebugOutput(true);

  Serial.println("--> Connecting Wifi");
    byte timeout = 0;  
  
    WiFi.config(ip, gateway, subnet, dns);
    delay(100);
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      delay(200);
      Serial.print(".");
      if (timeout++ >= 15) {
        Serial.println("--> ERROR: Wifi commection timeout");
        break;
      }
    }
    digitalWrite(LED_BUILTIN, LOW);
    
    Serial.println();
    Serial.println(WiFi.macAddress());
    Serial.println(WiFi.localIP());
    #ifdef DO_PAGE
      setup_webpage();
    #endif // DO_PAGE
    #ifdef DO_MQTT
      client.setServer(mqtt_server, 1883);
      client.setCallback(op2eTime_callback);
      mqtt_connect();
    #endif // DO_MQTT 
    #ifdef DO_ADS
      setup_ads();
    #endif // DO_ADS
  
} //end setup
 
// the loop function runs over and over again forever
void loop() {
//  loop_count++;
  #ifdef DO_OTA
    if ( (ota + last_ota) < millis() ){
      last_ota = millis();
      loop_OTA();
    }
  #endif //DO_OTA
  
  #ifdef DO_ADS
    if ( (last_ads + ads_time) < millis() ){
      last_ads = millis();
      loop_ads();
    }
  #endif //DO_ADS

  client.loop();
  #ifdef DO_MQTT
    if ( (last_mqtt + mqtt_time ) < millis() ){
      last_mqtt = millis();
      mqtt_connect();
      loop_mqtt();
    }
    
  #endif // DO_MQTT 

  // Blink the LED
  digitalWrite(LED_BUILTIN, HIGH);   // Turn the LED on 
  delay(100);                       // Wait for a  bit
  digitalWrite(LED_BUILTIN, LOW);  // Turn the LED off by making the voltage HIGH

  Serial.print("mqtt.state=");
  Serial.println((int)client.state());
  
  #ifdef DO_PAGE
    loop_webpage();
  #endif //DO_PAGE
  
} // End
