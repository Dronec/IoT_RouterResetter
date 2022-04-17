#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "LittleFS.h"
#include <AsyncElegantOTA.h>
#include <ESP8266HTTPClient.h>
#include <DefsWiFi.h>
#include <Arduino_JSON.h>

#define NoResetTime 600000   // 10 min no reset
#define DefaultOffTime 10000 // 10 sec off time

#define loopDelay 5000 // 5 seconds between loops

#define ToleranceLimit 120000 // 2 min until reboot

const char *ssid[2] = {WIFISSID_1, WIFISSID_2};
const char *password[2] = {WIFIPASS_1, WIFIPASS_2};
const char *softwareVersion = "0.8";

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

WiFiEventHandler e1;

int relayNumber = 0;

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

unsigned long timer[] = {0, 0};
unsigned long pp1offtime = 0;
unsigned long pp2offtime = 0;
unsigned long execCount = 0;
unsigned long lastExternalPingErrorTime[] = {0, 0};

HTTPClient http;
WiFiClient client;

void onSTAGotIP(WiFiEventStationModeGotIP ipInfo)
{
  Serial.printf("Connected: %s\n", WiFi.localIP().toString().c_str());
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

void initWebServer()
{
  Serial.println("Web server initialized.");
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
    JSONVar webmsg = JSON.parse((char *)data);
    // values
    // if (webmsg.hasOwnProperty("currentCamera"))
    // {
    //   currentCamera = atoi(webmsg["currentCamera"]);
    //   EnableCamera(currentCamera);
    // }
    // if (webmsg.hasOwnProperty("frontCamTimeout"))
    //   frontCamTimeout = atoi(webmsg["frontCamTimeout"]);
    // if (webmsg.hasOwnProperty("serialOutput"))
    //   serialOutput = atoi(webmsg["serialOutput"]);
    // if (webmsg.hasOwnProperty("rearCamMode"))
    //   rearCamMode = atoi(webmsg["rearCamMode"]);
    // if (webmsg.hasOwnProperty("loopDelay"))
    //   loopDelay = atoi(webmsg["loopDelay"]);
    // if (webmsg.hasOwnProperty("canInterface"))
    //   canInterface = atoi(webmsg["canInterface"]);
    // if (webmsg.hasOwnProperty("canSpeed"))
    //   canSpeed = atoi(webmsg["canSpeed"]);
    // checkboxes
    if (webmsg.hasOwnProperty("relay1"))
      switchRelay(1, webmsg["relay1"]);
    if (webmsg.hasOwnProperty("relay2"))
      switchRelay(2, webmsg["relay2"]);

    if (webmsg.hasOwnProperty("command"))
    {
      int command = atoi(webmsg["command"]);
      if (command == 0)
        ESP.restart();
    }
    notifyClients(getOutputStates());
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
void notifyClients(String state)
{
  ws.textAll(state);
}
String getOutputStates()
{
  JSONVar myArray;
  // sending stats
  myArray["stats"]["ssid"] = ssid[relayNumber];
  myArray["stats"]["softwareVersion"] = softwareVersion;
  myArray["stats"]["lastErrorpp1"] = lastExternalPingErrorTime[0];
  myArray["stats"]["lastErrorpp2"] = lastExternalPingErrorTime[1];
  myArray["stats"]["uptime"] = millis() / 1000;
  myArray["stats"]["ram"] = (int)ESP.getFreeHeap();

  // // sending values
  // myArray["settings"]["currentCamera"] = currentCamera;
  // myArray["settings"]["frontCamTimeout"] = frontCamTimeout;
  // myArray["settings"]["rearCamMode"] = rearCamMode;
  // myArray["settings"]["serialOutput"] = serialOutput;
  // myArray["settings"]["loopDelay"] = loopDelay;
  // myArray["settings"]["canSpeed"] = canSpeed;
  // myArray["settings"]["canInterface"] = canInterface;

  // sending checkboxes
  myArray["checkboxes"]["relay1"] = pp1Enabled;
  myArray["checkboxes"]["relay2"] = pp2Enabled;

  String jsonString = JSON.stringify(myArray);
  return jsonString;
}

void setup()
{
  delay(10);
  Serial.begin(115200);

  e1 = WiFi.onStationModeGotIP(onSTAGotIP);
  Serial.printf("Connecting to: %s\n", ssid[relayNumber]);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid[relayNumber], password[relayNumber]);
  delay(DefaultOffTime);
  initLittleFS();
  initWebServer();
  timer[relayNumber] = millis();
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

  if (timer[relayNumber] + NoResetTime < millis())
  {
    if (lastExternalPingErrorTime[relayNumber] > 0 && lastExternalPingErrorTime[relayNumber] + ToleranceLimit < millis())
    {
      switchRelay(relayNumber + 1, false);
      Serial.printf("Resetting Power Point #%d\n", relayNumber + 1);
      lastExternalPingErrorTime[relayNumber] = 0;
      timer[relayNumber] = 0;
    }
    ws.cleanupClients();
    notifyClients(getOutputStates());

    if (execCount % 12 == 0)
    {
      Serial.printf("Disconnecting from: %s\n", ssid[relayNumber]);
      WiFi.mode(WIFI_OFF);
      relayNumber = abs(relayNumber - 1);
      Serial.printf("Connecting to: %s\n", ssid[relayNumber]);
      WiFi.mode(WIFI_STA);
      WiFi.begin(ssid[relayNumber], password[relayNumber]);
      delay(loopDelay);

      bool checkNow = CheckInternet();

      if (checkNow && lastExternalPingErrorTime[relayNumber] > 0)
      {
        lastExternalPingErrorTime[relayNumber] = 0;
        Serial.printf("Error count reset for relay #%d.\n", relayNumber + 1);
      }

      if (!checkNow && lastExternalPingErrorTime[relayNumber] == 0)
      {
        Serial.printf("Starting error count for relay #%d...\n", relayNumber + 1);
        lastExternalPingErrorTime[relayNumber] = millis();
      }
    }
  }
  if (execCount % 12 != 0)
    delay(loopDelay);
  execCount++;
  notifyClients(getOutputStates());
}