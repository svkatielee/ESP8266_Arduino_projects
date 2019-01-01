/*
 * include code from
 ESP8266 Blink by Simon Peter
 Blink the blue LED on the ESP-01 module
 
 The blue LED on the ESP-01 module is connected to GPIO1 
 (which is also the TXD pin; so we cannot use Serial.print() at the same time)
 
 Note that this sketch uses LED_BUILTIN to find the pin with the internal LED

 Batt_volt_mon2 Is to monitor boat house battery with ADS1115 A to D xonverter
 and send mqtt to op2e
 Also does OTA:
    on pot run Arduino IDE compile, then Sketch->Export Comp Bin
    then scp ~/Arduino/Boat_volt_mon2/Boat_volt_mon2.ino.d1_mini.bin rop:/var/www/html/bin/
    then the next cycle it will update then reboot and run new code.

 built a new board with connector for buck, ADC and Wemos D1 Mini. soldered in the trimmer pot       
    new board, so new rev 2.0

    refactored to tabs, added a status web page so gRev to 0.3.x
    finish refactor 0.4.1
    average current 0.4.2

*/

const char *gRev = "bat-0.4.2";  // Software Revision Code

#include <ESP8266WiFi.h>

char msg[50];
char v_str[15], c_str[15], ah_str[15]; 
char c3[15], c4[15];
long curr_in[5]; byte cnt=0;
float current, ahour;
unsigned long ads_time = 1000;
unsigned long last_ads = millis();
unsigned long mqtt_time = 60000;
unsigned long last_mqtt = millis();
unsigned long last_ota = millis();
unsigned long ota = 10000;
unsigned long loop_count = 0; 
unsigned long loop_count_avg =0;

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
  Serial.begin(115200);
  Serial.setDebugOutput(true);

  Serial.println("--> Connecting Wifi");
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

    Serial.println();
    Serial.println(WiFi.macAddress());
    Serial.println(WiFi.localIP());
    #ifdef DO_PAGE
      setup_webpage();
    #endif // DO_PAGE
    #ifdef DO_MQTT
      client.setServer(mqtt_server, 1883);
    #endif // DO_MQTT 
    #ifdef DO_ADS
      setup_ads();
    #endif // DO_ADS
  
} //end setup
 
// the loop function runs over and over again forever
void loop() {
  loop_count++;
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


  #ifdef DO_MQTT
    if ( (last_mqtt + mqtt_time ) < millis() ){
      last_mqtt = millis();
      mqtt_connect();
      loop_mqtt();
    }
  #endif // DO_MQTT 

  // Blink the LED
  digitalWrite(LED_BUILTIN, LOW);   // Turn the LED on 
  delay(100);                       // Wait for a  bit
  digitalWrite(LED_BUILTIN, HIGH);  // Turn the LED off by making the voltage HIGH


  #ifdef DO_PAGE
    loop_webpage();
  #endif //DO_PAGE
  
} // End
