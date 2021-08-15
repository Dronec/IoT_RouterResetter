#include "esp_timer.h"
#include "img_converters.h"
#include "Arduino.h"
#include "fb_gfx.h"
#include "Esp32MQTTClient.h"

// Local includes
#include "./libraries/Blink.h"
#include "./libraries/httpcameraserver.h"
#include "./libraries/camera.h"
#include "./libraries/Network.h"

static const char* connectionString = "HostName=myiothubkirrawee.azure-devices.net;DeviceId=SecurityCamera_Home;SharedAccessKey=DtZXV/1GUT8MK9HbNZyenVKgbrA/3J9vzg0TGxLUvLU=";

static bool hasIoTHub = false;
static bool hasWiFi = false;
String sendPhotoTelegram();

//motion sensor
  #define KEY_PIN       3
//
  #define TelegramAdminID 587564160

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

   if (strcmp(action, "takephoto") == 0)
  {
    sendPhotoTelegram(chatId);
  }
}
void loop() {
  Esp32MQTTClient_Check();
   if(digitalRead(KEY_PIN)==LOW)
  {
    Serial.print(F("Motion detected!"));
    sendPhotoTelegram(TelegramAdminID);
    while(digitalRead(KEY_PIN)==LOW);
  }
  delay(1000);
}