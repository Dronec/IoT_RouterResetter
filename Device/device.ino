#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "LittleFS.h"
#include <AsyncElegantOTA.h>

const char *ssid = "ssid";
const char *password = "password";

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

// Hex command to send to serial for close relay
// Hex command to send to serial for close relay
byte relON[] = {0xA0, 0x01, 0x01, 0xA2};

// Hex command to send to serial for open relay
byte relOFF[] = {0xA0, 0x01, 0x00, 0xA1};

// Hex command to send to serial for close relay
byte rel2ON[] = {0xA0, 0x02, 0x01, 0xA3};

// Hex command to send to serial for open relay
byte rel2OFF[] = {0xA0, 0x02, 0x00, 0xA2};

bool pp1Enabled = true;
bool pp2Enabled = true;

// Initialize LittleFS
void initLittleFS()
{
  if (!LittleFS.begin())
  {
    Serial.println("An error has occurred while mounting LittleFS");
  }
  Serial.println("LittleFS mounted successfully");
}

void notifyClients(String state)
{
  ws.textAll(state);
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len)
{
  AwsFrameInfo *info = (AwsFrameInfo *)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT)
  {
    data[len] = 0;
    int relay = atoi((char *)data);
    if (relay == 1)
    {
      pp1Enabled = !pp1Enabled;
      if (pp1Enabled)
        Serial.write(relOFF, sizeof(relOFF));
      else
        Serial.write(relON, sizeof(relON));
    }
    if (relay == 2)
    {
      pp2Enabled = !pp2Enabled;
      if (pp2Enabled)
        Serial.write(rel2OFF, sizeof(rel2OFF));
      else
        Serial.write(rel2ON, sizeof(rel2ON));
    }
  }
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
             void *arg, uint8_t *data, size_t len)
{
  switch (type)
  {
  case WS_EVT_CONNECT:
    Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
    break;
  case WS_EVT_DISCONNECT:
    Serial.printf("WebSocket client #%u disconnected\n", client->id());
    break;
  case WS_EVT_DATA:
    handleWebSocketMessage(arg, data, len);
    break;
  case WS_EVT_PONG:
  case WS_EVT_ERROR:
    break;
  }
}

void initWebSocket()
{
  ws.onEvent(onEvent);
  server.addHandler(&ws);
}

void setup()
{
  delay(10);
  Serial.begin(115200);

  WiFi.begin(ssid, password);

  initLittleFS();
  initWebSocket();

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(LittleFS, "/index.html", "text/html", false); });

  server.serveStatic("/", LittleFS, "/");

  // Start ElegantOTA
  AsyncElegantOTA.begin(&server);

  // Start server
  server.begin();
}

void loop()
{
  // int val;
  /*
    // Check if a client has connected
    WiFiClient client = server.available();
    if (!client)
    {
      return;
    }

    // Wait until the client sends some data
    while (!client.available())
    {
      delay(100);
    }

    // Read the first line of the request
    String req = client.readStringUntil('\r');
    client.flush();

    // Match the request
    if (req.indexOf("/1/on") != -1)
    {
      Serial.write(relON, sizeof(relON));
      val = 1; // if you want feedback see below
    }
    else if (req.indexOf("/1/off") != -1)
    {
      Serial.write(relOFF, sizeof(relOFF));
      val = 0; // if you want feedback
    }
    else if (req.indexOf("/2/on") != -1)
    {
      Serial.write(rel2ON, sizeof(rel2ON));
      val = 1; // if you want feedback see below
    }
    else
    {
      if (req.indexOf("/2/off") != -1)
        Serial.write(rel2OFF, sizeof(rel2OFF));
      val = 0; // if you want feedback
    }

    client.flush();

    // only if you want feedback - see above
    // Prepare the response
    String s = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html>\r\nRelay is now ";
    s += (val) ? "on" : "off";
    s += "</html>\n";

    // Send the response to the client
    client.print(s);*/
  // delay(10000);
}