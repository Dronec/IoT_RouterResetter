#include <UniversalTelegramBot.h>
#define TelegramAdminID 587564160

// Initialize Telegram BOT
String BOTtoken = "1219467580:AAGwWJRdAeMS_EdY4Z2hp02EmzMtDY4nv_k";
WiFiClientSecure clientTCP;
UniversalTelegramBot bot(BOTtoken, clientTCP);

void SendMessageTelegramAdmin(String message)
{
   bot.sendMessage(String(TelegramAdminID), message, "MarkdownV2");
}

void SendMessageTelegram(int chat_id, String message)
{
  bot.sendMessage(String(chat_id), message, "MarkdownV2");
}

String sendPhotoTelegram(int chatId){
  const char* myDomain = "api.telegram.org";
  String getAll = "";
  String getBody = "";

  Blink(true);
  delay(500);
  
  camera_fb_t * fb = NULL;
  fb = esp_camera_fb_get();  
  if(!fb) {
    Serial.println("Camera capture failed");
    delay(1000);
    ESP.restart();
    return "Camera capture failed";
  }  
  
  Blink(false);

  Serial.println("Connect to " + String(myDomain));

  if (clientTCP.connect(myDomain, 443)) {
    Serial.println("Connection successful");
    
    String head = "--RandomNerdTutorials\r\nContent-Disposition: form-data; name=\"chat_id\"; \r\n\r\n" + String(chatId) + "\r\n--RandomNerdTutorials\r\nContent-Disposition: form-data; name=\"photo\"; filename=\"esp32-cam.jpg\"\r\nContent-Type: image/jpeg\r\n\r\n";
    String tail = "\r\n--RandomNerdTutorials--\r\n";

    uint16_t imageLen = fb->len;
    uint16_t extraLen = head.length() + tail.length();
    uint16_t totalLen = imageLen + extraLen;
  
    clientTCP.println("POST /bot"+BOTtoken+"/sendPhoto HTTP/1.1");
    clientTCP.println("Host: " + String(myDomain));
    clientTCP.println("Content-Length: " + String(totalLen));
    clientTCP.println("Content-Type: multipart/form-data; boundary=RandomNerdTutorials");
    clientTCP.println();
    clientTCP.print(head);
  
    uint8_t *fbBuf = fb->buf;
    size_t fbLen = fb->len;
    for (size_t n=0;n<fbLen;n=n+1024) {
      if (n+1024<fbLen) {
        clientTCP.write(fbBuf, 1024);
        delay(50);
        fbBuf += 1024;
      }
      else if (fbLen%1024>0) {
        size_t remainder = fbLen%1024;
        clientTCP.write(fbBuf, remainder);
      }
    }  
    
    clientTCP.print(tail);
    
    esp_camera_fb_return(fb);
    
    int waitTime = 10000;   // timeout 10 seconds
    long startTimer = millis();
    boolean state = false;
    
    while ((startTimer + waitTime) > millis()){
      Serial.print(".");
      delay(100);     
      while (clientTCP.available()) {
        char c = clientTCP.read();
        if (state==true) getBody += String(c);        
        if (c == '\n') {
          if (getAll.length()==0) state=true; 
          getAll = "";
        } 
        else if (c != '\r')
          getAll += String(c);
        startTimer = millis();
      }
      if (getBody.length()>0) break;
    }
    clientTCP.stop();
    Serial.println(getBody);
  }
  else {
    getBody="Connected to api.telegram.org failed.";
    Serial.println("Connected to api.telegram.org failed.");
  }
  return getBody;
}