

#include <PubSubClient.h>

IPAddress mqtt_server(192, 168, 11, 30);
WiFiClient espClient;
PubSubClient client(espClient);

String clientId = "Batt";   // Create a client ID
//client.setServer(mqtt_server, 1883);

void op2eTime_callback(char* topic, byte* payload, unsigned int length) {
  if((int)payload >= 18 && ahour >= 0) {
    ahour = 0;
  } 
}

void mqtt_connect(){
    byte timeout = 0;
    while (!client.connected()) {
        Serial.print("Attempting MQTT connection...");
        if (client.connect(clientId.c_str())) {
          Serial.println("connected");
        } else {
          Serial.print("failed, rc=");
          Serial.println(client.state());
      
          if(client.state() == -2 ) {
            Serial.println("MQTT:  WiFi not connected.");
          }
        }
        if (timeout++ >= 15) {
          Serial.println("--> ERROR: Wifi commection timeout");
          break; 
        }
    }
    delay(100);

    snprintf (msg, 75, "{\"Revision\": %s", gRev); 
    if (!client.publish("tcls/Battery/rev", msg, true) ) {
        Serial.println("Failed to publish !!!");
    } else {
        Serial.print("Publish message: "); Serial.println(msg); 
    }
    delay(100);
    client.subscribe("op2e/Time"); //to get time from op2e
    
    yield();
}

void loop_mqtt(){
    snprintf (msg, 75, "{\"Battery\": %s}", v_str); 
    if (!client.publish("tcls/Battery/volt", msg, true) ) {
        Serial.println("Failed to publish !!!");
    } else {
      Serial.print("Publish message: "); Serial.println(msg); 
    }
    yield();
        snprintf (msg, 75, "{\"Current\": %s, \"AHour\": %s}", c_str, ah_str); 
    if (!client.publish("tcls/Battery/current", msg, true) ) {
        Serial.println("Failed to publish !!!");
    } else {
      Serial.print("Publish message: "); Serial.println(msg); 
    }
    yield();


}
    
