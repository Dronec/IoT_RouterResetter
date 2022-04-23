#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "LittleFS.h"
#include <AsyncElegantOTA.h>
#include <ESP8266HTTPClient.h>
#include <DefsWiFi.h>
#include <Arduino_JSON.h>

#define DefaultOffTime 10000 // 10 sec off time

#define loopDelay 1000       // 5 seconds between loops
#define checkIncrement 60000 // every time internet fails we add 1 minute

const char *ssid = WIFISSID_2;
const char *password = WIFIPASS_2;
const char *softwareVersion = "1.01";

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
bool fsMounted = false;

int fails = 0;
int reboots = 0;

unsigned long pp1offtime = 0;
unsigned long pp2offtime = 0;
unsigned long lastInternetTime;
unsigned long timer;

HTTPClient http;
WiFiClient client;

void onSTAGotIP(WiFiEventStationModeGotIP ipInfo)
{
  Serial.printf("%d: Connected: %s to %s\n", millis(), WiFi.localIP().toString().c_str(), ssid);
  initWebServer();
}

// Initialize LittleFS
void initLittleFS()
{
  if (!LittleFS.begin())
  {
    Serial.println("An error has occurred while mounting LittleFS");
  }
  else
  {
    Serial.println("LittleFS mounted successfully");
    fsMounted = true;
  }
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
      reboots++;
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
      reboots++;
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
      switch (command)
      {
      case 0:
        ESP.restart();
        break;
      case 1:
        timer = timer + checkIncrement * 15;
        break;
      }
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
char *millisToTime(unsigned long currentMillis)
{
  unsigned long seconds = currentMillis / 1000;
  unsigned long minutes = seconds / 60;
  unsigned long hours = minutes / 60;
  unsigned long days = hours / 24;
  currentMillis %= 1000;
  seconds %= 60;
  minutes %= 60;
  hours %= 24;
  static char buffer[50];
  if (days == 0 && hours == 0 && minutes == 0)
    sprintf(buffer, "%lu sec ", seconds);
  else if (days == 0 && hours == 0 && minutes > 0)
    sprintf(buffer, "%lu min %lu sec ", minutes, seconds);
  else if (days == 0 && hours > 0)
    sprintf(buffer, "%lu h %lu m %lu s ", hours, minutes, seconds);
  else
    sprintf(buffer, "%lud %luh %lum %lus ", days, hours, minutes, seconds);
  return buffer;
}
String getOutputStates()
{
  JSONVar myArray;
  // sending stats
  myArray["stats"]["ssid"] = ssid;
  myArray["stats"]["softwareVersion"] = softwareVersion;
  myArray["stats"]["lastInternetTime"] = millisToTime((millis() - lastInternetTime - loopDelay));
  myArray["stats"]["nextCheckIn"] = millisToTime((timer - millis() + loopDelay));
  myArray["stats"]["fails"] = fails;
  myArray["stats"]["reboots"] = reboots;
  myArray["stats"]["uptime"] = millisToTime(millis());
  myArray["stats"]["ram"] = (int)ESP.getFreeHeap();
  myArray["stats"]["frag"] = (int)ESP.getHeapFragmentation();

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
  Serial.begin(115200);
  lastInternetTime = 0;
  timer = millis() + checkIncrement * 5;
  initLittleFS();
  e1 = WiFi.onStationModeGotIP(onSTAGotIP);
  WiFi.begin(ssid, password);
  delay(DefaultOffTime);
  Serial.printf("%d: Network monitor restarted.\n", millis());
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
  // this part turns on relays after 10 seconds off
  if (pp1offtime > 0 && pp1offtime + DefaultOffTime < millis())
  {
    switchRelay(1, true);
  }
  if (pp2offtime > 0 && pp2offtime + DefaultOffTime < millis())
  {
    switchRelay(2, true);
  }
  //
  if (timer < millis())
  {
    bool checkNow = CheckInternet();
    if (checkNow)
    {
      fails = 0;
      timer = millis() + checkIncrement;
      lastInternetTime = millis();
    }
    else
    {
      fails++;
      timer = millis() + checkIncrement * (fails + 2);
    }

    if (fails == 2)
      switchRelay(2, false);
    if (fails > 2)
      switchRelay(1, false);
  }

  delay(loopDelay);
  ws.cleanupClients();
  notifyClients(getOutputStates());
}