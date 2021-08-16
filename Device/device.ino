#include "esp_timer.h"
#include "img_converters.h"
#include "Arduino.h"
#include "fb_gfx.h"
#include "Esp32MQTTClient.h"

// Local includes
#include "./libraries/Blink.h"
#include "./libraries/Network.h"
#include "./libraries/httpcameraserver.h"
#include "./libraries/camera.h"

static const char* connectionString = "HostName=myiothubkirrawee.azure-devices.net;DeviceId=SecurityCamera_Home;SharedAccessKey=DtZXV/1GUT8MK9HbNZyenVKgbrA/3J9vzg0TGxLUvLU=";

static bool hasIoTHub = false;
static bool hasWiFi = false;
static bool webserverOn = false;
static bool motionSensorOn = true;

String sendPhotoTelegram();

//motion sensor
  #define KEY_PIN       3
//
void setup() {
  
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  // Inits

  BlinkInit();
  CameraInit();

  hasWiFi = WiFiInit();
  
if (!Esp32MQTTClient_Init((const uint8_t*)connectionString, true))
  {
    hasIoTHub = false;
    Serial.println("Initializing IoT hub failed.");
  }
  else
  {
    hasIoTHub = true;
    //Esp32MQTTClient_SetSendConfirmationCallback(SendConfirmationCallback);
    Esp32MQTTClient_SetMessageCallback(MessageCallback);
    //Esp32MQTTClient_SetDeviceTwinCallback(DeviceTwinCallback);
    //Esp32MQTTClient_SetDeviceMethodCallback(DeviceMethodCallback);
    Serial.println("Start sending events.");
  }

  Serial.print("Camera Stream Ready! Go to: http://");
  Serial.print(WiFi.localIP());
  
  // Start streaming web server
  if (webserverOn)
    startCameraServer();
}
static void MessageCallback(const char* payLoad, int size)
{
  Serial.println("Message callback:");
  Serial.println(payLoad);
  const char* action;
  int chatId;
  StaticJsonDocument<200> messageProperties;
  DeserializationError error = deserializeJson(messageProperties, payLoad);
  // Test if parsing succeeds.
  if (error) {
    Serial.print(F("Message deserialization failed:"));
    Serial.println(error.f_str());
    return;
  }
  chatId = messageProperties["ChatID"];
  action = messageProperties["Action"];

  if (strcmp(action, "status") == 0)
  {
    CheckUptime(chatId);
    CheckWebServer(chatId, false);
    CheckMotionSensor(chatId);
  }
  if (strcmp(action, "websrv") == 0)
  {
    webserverOn = !webserverOn;
    CheckWebServer(chatId, true);
  }
  if (strcmp(action, "msensor") == 0)
  {
    motionSensorOn = !motionSensorOn;
    CheckMotionSensor(chatId);
  }
   if (strcmp(action, "photo") == 0)
  {
    sendPhotoTelegram(chatId);
  }
}
void CheckMotionSensor(int chatId)
{
  String onoff;
  if (motionSensorOn)
    {
      onoff = "enabled";
    }
    else
    {
      onoff = "disabled";
    }
    String message = "*Motion sensor:* " + onoff;
    SendMessageTelegram(chatId, message);
}
void CheckWebServer(int chatId, bool act)
{
  String onoff;
  if (webserverOn)
      {
        if (act)
          startCameraServer();
        onoff = "started";
      }
      else
      {
        if (act)
          stopCameraServer();
        onoff = "stopped";
      }
  String message = "*Streaming:* " + onoff;
  SendMessageTelegram(chatId, message);
}
void CheckUptime(int chatId)
{
char timestring[25]; // for output
sprintf(timestring,"*Uptime:* %d min;", esp_timer_get_time()/60000000);
SendMessageTelegram(chatId, String(timestring));
}
/*
static void DeviceTwinCallback(DEVICE_TWIN_UPDATE_STATE updateState, const unsigned char *payLoad, int size)
{
  Serial.println("Twin payload received:");
  Serial.println((const char*)payLoad);

  StaticJsonDocument<200> twinProperties;
  DeserializationError error = deserializeJson(twinProperties, payLoad);
  // Test if parsing succeeds.
  if (error) {
    Serial.print(F("Twin deserialization failed:"));
    Serial.println(error.f_str());
    return;
  }
  bool webserverOnNew;
  bool motionSensorOnNew;
  webserverOnNew = twinProperties["desired"]["webserverOn"];
  if ((!webserverOn) && (webserverOnNew))
    {
      webserverOn = webserverOnNew;
      startCameraServer();
      SendMessageTelegramAdmin("*Streaming server started*");
    }
  else
  if ((webserverOn) && (!webserverOnNew))
    {
      webserverOn = webserverOnNew;
      stopCameraServer();
      SendMessageTelegramAdmin("*Streaming server stopped*");
    }

  motionSensorOnNew = twinProperties["desired"]["motionSensorOn"];

  if ((!motionSensorOn) && (motionSensorOnNew))
    {
      motionSensorOn = motionSensorOnNew;
      SendMessageTelegramAdmin("*Motion sensor enabled*");
    }
  else
  if ((motionSensorOn) && (!motionSensorOnNew))
    {
      motionSensorOn = motionSensorOnNew;
      SendMessageTelegramAdmin("*Motion sensor disabled*");
    }
}
*/
void loop() {

  Esp32MQTTClient_Check();
  
  if((motionSensorOn && digitalRead(KEY_PIN)==LOW))
  {
    Serial.print(F("Motion detected!"));
    SendMessageTelegramAdmin("*Motion detected*");
    sendPhotoTelegram(TelegramAdminID);
    while(digitalRead(KEY_PIN)==LOW);
  }
  delay(1000);
}