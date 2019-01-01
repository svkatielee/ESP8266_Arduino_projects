

#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>

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


// check for OTA update
void loop_OTA(){
  checkForOTA();
  yield();
}
