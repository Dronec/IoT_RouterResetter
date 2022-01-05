#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "LittleFS.h"
#include <AsyncElegantOTA.h>
#include <ESP8266HTTPClient.h>
#include <DefsWiFi.h>

#define NoResetTime 600000   // 10 min no reset
#define DefaultOffTime 10000 // 10 sec off time

#define ToleranceLimit 120000 // 2 min until reboot

const char *ssid = WIFISSID_1;
const char *password = WIFIPASS_1;

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

WiFiEventHandler e1;

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
unsigned long execCount = 0;
unsigned long lastExternalPingErrorTime = 0;

HTTPClient http;
WiFiClient client;

void onSTAGotIP(WiFiEventStationModeGotIP ipInfo)
{
  Serial.printf("Connected: %s\n", WiFi.localIP().toString().c_str());
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
    state += "0";
  else
    state += "1";
  if (pp2Enabled)
    state += "0";
  else
    state += "1";
  ws.textAll(state);
}

void setup()
{
  delay(10);
  Serial.begin(115200);

  Serial.printf("Connecting to: %s\n",ssid);

  WiFi.begin(ssid, password);

  e1 = WiFi.onStationModeGotIP(onSTAGotIP);

  initLittleFS();

  timer = millis();
}

bool CheckInternet()
{

  http.begin(client, "http://www.msftncsi.com/ncsi.txt");
  int httpCode = http.GET();

  // httpCode will be negative on error
  if (httpCode > 0)
  {
    // HTTP header has been send and Server response header has been handled
    Serial.printf("[HTTP] GET... code: %d\n", httpCode);

    // file found at server
    if (httpCode == HTTP_CODE_OK)
    {
      Serial.println(http.getString());
      return true;
    }
  }
  else
  {
    Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    return false;
  }
  http.end();
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
  if (timer + NoResetTime < millis())
  {
    if (lastExternalPingErrorTime > 0 && lastExternalPingErrorTime + ToleranceLimit < millis())
    {
      switchRelay(1, false);
      Serial.println("Resetting Power Point #1");
      lastExternalPingErrorTime = 0;
      timer = 0;
    }
    ws.cleanupClients();
    notifyClients();

    if (execCount % 60 == 0)
    {
      if (CheckInternet() && lastExternalPingErrorTime > 0)
      {
        lastExternalPingErrorTime = 0;
        Serial.println("Error count reset.");
      }

      if (!CheckInternet() && lastExternalPingErrorTime == 0)
      {
        Serial.println("Starting error count...");
        lastExternalPingErrorTime = millis();
      }
    }
  }
  delay(1000);
  execCount++;
}