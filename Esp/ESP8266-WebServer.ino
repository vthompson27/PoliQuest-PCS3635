//============================================
//ESP8266 Web Server (HTML + CSS + JavaScript)
//============================================
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include "webpageCode.h"
#include "gamemode1Code.h"
//------------------------------------------
ESP8266WebServer server(80);
const char* ssid = "";
const char* password = "";
//------------------------------------------
void webpage()
{
  server.send(200,"text/html", webpageCode);
}

void gamemode1()
{
  server.send(200,"text/html", gamemode1Code);
}

//=================================================================
void setup()
{
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while(WiFi.status()!=WL_CONNECTED){delay(500);Serial.print(".");}
  Serial.println();
  Serial.print("IP Address: "); Serial.println(WiFi.localIP());

  server.on("/", webpage);
  server.on("/mode1", gamemode1);

  server.begin();
}
//=================================================================
void loop()
{
  server.handleClient();
}