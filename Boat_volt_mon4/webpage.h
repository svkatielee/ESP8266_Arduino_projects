


#include <ESP8266WebServer.h>
#include <WiFiClient.h>

ESP8266WebServer server(80);
char gTmpBuf[32+2];  // Generic Temp Buffer
#define sF(x) (String) F(x)                // Used as an F() is being used as the first Element 
                                           // of a Multi-Element Expression
#define FMT(x) strcpy_P(gFmtBuf, PSTR(x))  // Used with printf() for the format string
    // USE WITH CAUTION !
char gFmtBuf[64+2];   

//==============================================================================
// Provides UpTime in Seconds
//
unsigned long
upTime()
{
  return millis() / 1000;
}
//==============================================================================
String upTimeStr()
{
//    char buf[PBUFSIZE];
    
    int uptimesec = upTime();
    int uptimemin = uptimesec / 60;
    int uptimehr  = uptimemin / 60;
    int uptimeday = uptimehr  / 24;
    yield();

    uptimesec %= 60;
    uptimemin %= 60;
    uptimehr  %= 24;

    snprintf( gTmpBuf, sizeof(gTmpBuf),
        FMT("%d Days, %02d:%02d:%02d"), uptimeday, uptimehr, uptimemin, uptimesec );
    yield();
    
    return gTmpBuf;
} 


//==============================================================================
void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += ( server.method() == HTTP_GET ) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";

  for ( uint8_t i = 0; i < server.args(); i++ ) {
    message += " " + server.argName ( i ) + ": " + server.arg ( i ) + "\n";
  }

  server.send ( 404, "text/plain", message );
}


//==============================================================================
void handleRoot() {     

  String mesg,myip;
  
  mesg = "<!DOCTYPE html>"
        "<html>"
        "<head>"
        "<title>Battery Status</title>"
        "<meta http-equiv=\"refresh\" content=\"3\" />"
        "<meta charset=\"utf-8\" />"
        "<meta name=viewport content=\"width=device-width, initial-scale=1\">"
        "<meta http-equiv='Pragma' content='no-cache'>"
        "<link rel='shortcut icon' href='http://espressif.com/favicon.ico'>"
        "<style>"
        "  body {background-color: #ddddee;}"
        "</style> "
        "</head>"
        "<body>"
        "<center>"
        "<h1>Battery Status</h1>"
       
        "<br>";
 
  
  yield();
  
  mesg += "    Uptime: <b>"       + String(upTimeStr()) + F("</b><br>");
  mesg += "    My IPA: <b>" + WiFi.localIP().toString() + F("</b><br>");
  mesg += F("<p>");
  mesg += "    Battery Volts=" + String(v_str) + F("</b><br>");
  
  mesg += F("<p>");
  mesg += "Current: " + String(c_str) + F("</b><br>");
  //mesg += "Curr_in: " + String(curr_in[0]) + " " + String(curr_in[1]) + " " + String(curr_in[2]) + " " + String(curr_in[3]) + " " + String(curr_in[4]) + F("</b><br>");
  mesg += F("<p>");
  mesg += "Amp Hours: " +  String(ah_str) + F("</b><br>"); 
  //mesg += "Avg loop count: " + String(loop_count_avg) + F("</b><br>"); 
  mesg += F("<p>");
  mesg += F("<p>");
  mesg += F("<p>");
  mesg += "mystate=" + String(mystate) + F("</b><br>");
  mesg += F("<p>"); 
  mesg += "adc1: " + String(adc1)+ "  adc2: " + String(adc2)+ " op_hour: " + String(op_hour) + F("</b><br>"); 
  mesg += "Revision: " + String(gRev) +  "<br>";
  mesg += F("Copyright (c) 2018, Larry G. Littlefield<br>LGL - KB7KMO - larryl@tcls.com");
  mesg += F("See Blog at: <a href='http://KB7KMO.blogspot.com'>http://KB7KMO.blogspot.com</a>");
  mesg += F("<p>");
  mesg += F("Powered by: <a href='http://espressif.com/'>Esp8266</a>");
  
  mesg += "</center>"
         "</body>"
         "</html>";
  yield();      
#ifdef DEBUG
  Serial.print ("in handleRootNormal\n");
#endif                                        
  server.send ( 200, "text/html", mesg );
}



void setup_webpage(){
  server.on ( "/", handleRoot );
  server.onNotFound ( handleNotFound );
  server.begin();
}

void loop_webpage(){
  server.handleClient();            // Check for webpage request
}
