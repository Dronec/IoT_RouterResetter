#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "LittleFS.h"
#include <AsyncElegantOTA.h>

#define NoRebootTime 600000  // 10 min no reboot
#define DefaultOffTime 10000 // 10 sec off time

const char *ssid = "ssid";
const char *password = "password";

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

// Hex command to send to serial for close relay
// Hex command to send to serial for close relay
byte rel1ON[] = {0xA0, 0x01, 0x01, 0xA2};

// Hex command to send to serial for open relay
byte rel1OFF[] = {0xA0, 0x01, 0x00, 0xA1};

// Hex command to send to serial for close relay
byte rel2ON[] = {0xA0, 0x02, 0x01, 0xA3};

// Hex command to send to serial for open relay
byte rel2OFF[] = {0xA0, 0x02, 0x00, 0xA2};

bool pp1Enabled = true;
bool pp2Enabled = true;

unsigned long timer = 0;
unsigned long pp1offtime = 0;
unsigned long pp2offtime = 0;
// Initialize LittleFS
void initLittleFS()
{
  if (!LittleFS.begin())
  {
    Serial.println("An error has occurred while mounting LittleFS");
  }
  Serial.println("LittleFS mounted successfully");
}

void switchRelay(int relay, bool state)
{
  if (relay == 1)
  {
    if (state)
    {
      Serial.write(rel1OFF, sizeof(rel1OFF));
      pp1offtime = 0;
    }
    else
    {
      Serial.write(rel1ON, sizeof(rel1ON));
      pp1offtime = millis();
    }
    pp1Enabled = state;
  }
  if (relay == 2)
  {
    if (state)
    {
      Serial.write(rel2OFF, sizeof(rel2OFF));
      pp2offtime = 0;
    }
    else
    {
      Serial.write(rel2ON, sizeof(rel2ON));
      pp2offtime = millis();
    }
    pp2Enabled = state;
  }
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len)
{
  AwsFrameInfo *info = (AwsFrameInfo *)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT)
  {
    data[len] = 0;
    int relay = atoi((char *)data);
    if (relay == 1)
      switchRelay(relay, !pp1Enabled);
    if (relay == 2)
      switchRelay(relay, !pp2Enabled);
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
void notifyClients()
{
  String state;
  if (pp1Enabled)
    state+="0";
  else
    state+="1";
  if (pp2Enabled)
    state+="0";
  else
    state+="1";
  ws.textAll(state);
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
  timer = millis();
}

void loop()
{
  if (pp1offtime > 0 && pp1offtime + DefaultOffTime < millis())
  {
    switchRelay(1, true);
  }
  if (pp2offtime > 0 && pp2offtime + DefaultOffTime < millis())
  {
    switchRelay(2, true);
  }
  //if (timer + NoRebootTime < millis())
  //{
  //}
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
  ws.cleanupClients();
  notifyClients();
  delay(1000);
}