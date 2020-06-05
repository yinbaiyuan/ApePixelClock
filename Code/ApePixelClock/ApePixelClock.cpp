#include "ApePixelClock.h"
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>
#include "Fonts/ApcFont35.h"
#include <ArduinoJson.h>
#include <vector>

#define   ABTN_PIN      D6
#define   BBTN_PIN      D7
#define   LED_PIN       D5

#define   BRIGHTNESS    50
#define   FRAMES_PER_SECOND 60

#define   MATRIX_WIDTH  32
#define   MATRIX_HEIGHT 8

//ESP8266WiFiMulti        WiFiMulti;
RTC_DS1307              rtc;
Apc_AreaMatrix           matrix = Apc_AreaMatrix(MATRIX_WIDTH, MATRIX_HEIGHT, 1, 1, LED_PIN,
                                  NEO_MATRIX_BOTTOM + NEO_MATRIX_RIGHT + NEO_MATRIX_COLUMNS + NEO_MATRIX_ZIGZAG,
                                  NEO_GRB + NEO_KHZ800);

char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

std::vector<ApcEffectDef *>           apcEffects;
std::vector<ApcScheduleCallbackDef> apcScheduleCallbacks;
int                                 apcEffectPointer;
unsigned long                       preCheckTime = 0;
unsigned long                       callbackCheckTime = 0;
const int                           API_TIMEOUT = 2000;  //keep it long if you want to receive headers from client
const int                           httpsPort = 443;
bool                                autoChange = true;
ApePixelClock::ApePixelClock() {

}
void ApePixelClock::initRTC()
{
  Serial.println("INIT RTC");
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }
  if (! rtc.isrunning()) {
    Serial.println("RTC is NOT running, let's set the time!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  } else
  {
    Serial.println("rtc work normally");
  }
  if (this->isWifiConnected())
  {
    Serial.println("Internet is connected,update date and time");
    String url = "http://worldtimeapi.org/api/ip";
    int errCode = 0;
    String res = this->httpRequest(url, &errCode);
    if (errCode == 0)
    {
      DynamicJsonDocument doc(1024);
      deserializeJson(doc, res);
      JsonObject obj = doc.as<JsonObject>();
      uint32_t secondStamp = obj["unixtime"].as<String>().substring(0, 10).toInt();
      int timeZone = obj["utc_offset"].as<String>().substring(0, 3).toInt();
      Serial.println(timeZone);
      rtc.adjust(DateTime(secondStamp + timeZone * 60 * 60));
      Serial.println("Update succeed！！！");
    }

  } else
  {
    Serial.println("wifi is not connected");
  }
}

void ApePixelClock::apcLoop()
{
  this->btnCheckAction();
  this->renderCheck();
//  this->apcCallbackAction();
}



void ApePixelClock::systemInit(const char* wifi_ssid, const char* wifi_password, const int waitingTime)
{
  apcEffectPointer = 0;

  pinMode(ABTN_PIN, INPUT);
  pinMode(BBTN_PIN, INPUT);

  matrix.begin();
  matrix.setTextWrap(false);
  matrix.setBrightness(BRIGHTNESS);
  matrix.setTextColor(matrix.Color(255, 255, 255));

  Serial.begin(115200);
  while (!Serial) {
    ;
  }

  WiFi.mode(WIFI_STA);
  WiFi.begin(wifi_ssid, wifi_password);
//  WiFiMulti.addAP(wifi_ssid, wifi_password);
  WiFi.setAutoReconnect(true);
  for (uint8_t t = waitingTime; t > 0; t--) {
    Serial.printf("[SETUP] Connecting Wifi %d sec ...\n", t);
    Serial.flush();
    this->showInfo(String("Conn Wifi"));
    //delay(1000);
  }
  this->initRTC();
}

bool ApePixelClock::isWifiConnected()
{
  return WiFi.isConnected();
}

String ApePixelClock::httpsRequest(const String& url, int* errCode)
{
  String urlTemp = url;
  urlTemp.replace("https://", "");
  int splitIndex = urlTemp.indexOf('/');
  String server = urlTemp.substring(0, splitIndex);
  String api = urlTemp.substring(splitIndex);
  //  Serial.printf("server:%c,api:%c", server, api);
  Serial.print("API ");
  Serial.println(api);
  String payload;
  Serial.print("Connecting to ");
  Serial.print(server);
  BearSSL::WiFiClientSecure client;
  client.setInsecure();
  client.setTimeout(API_TIMEOUT);
  int retries = 6;
  while (!client.connect(server, httpsPort) && (retries-- > 0)) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println();
  if (!client.connected()) {
    Serial.println("Failed to connect, going back to sleep");
    *errCode = -1;
    client.stop();
  } else
  {
    Serial.print("Request resource: ");
    client.print(String("GET ") + api +
                 " HTTP/1.1\r\n" +
                 "Host: " + server + "\r\n" +
                 "Connection: close\r\n\r\n");
    int timeout = 5 * 10; // 5 seconds
    while (!client.available() && (timeout-- > 0)) {
      delay(100);
    }
    if (!client.available()) {
      Serial.println("No response, going back to sleep");
      *errCode = -2;
      client.stop();
    } else
    {
      while (client.available()) {
        payload += char(client.read());
      }
      payload = payload.substring(payload.indexOf('{'));
      Serial.println("\nclosing connection");
      delay(100);
      client.stop();
    }
  }

  return payload;
}

String ApePixelClock::httpRequest(const String& url, int* errCode)
{
  String payload;
  WiFiClient client;
  HTTPClient http;
  Serial.print("[HTTP] begin...\n");
  if (http.begin(client, url)) {  // HTTP
    Serial.print("[HTTP] GET...\n");
    int httpCode = http.GET();
    *errCode = httpCode;
    if (httpCode > 0) {
      Serial.printf("[HTTP] GET... code: %d\n", httpCode);
      if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
        *errCode = 0;
        payload = http.getString();
      }
    } else {
      Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();
  } else {
    Serial.printf("[HTTP} Unable to connect\n");
  }
  return payload;
}



void ApePixelClock::addApcEffect(ApcEffectDef* apcEffect)
{
  int apcEffectAreaCount = 0;
  for (int i = 0; i < MAX_APCEffECTAREA_COUNT; i++)
  {
    ApcEffectAreaDef& apcEffectArea = apcEffect->areaDef[i];
    if (apcEffectArea.frameCount > 0 && apcEffectArea.frameRefreshTime > 0)
    {
      apcEffectArea.currentFrameCount = 0;
      apcEffectArea.currentRefreshTime = 0;
      apcEffectAreaCount++;
    }
  }
  apcEffect->areaCount = apcEffectAreaCount;
  apcEffect->currentAreaIndex = 0;
  apcEffect->currentChangeTime = apcEffect->autoChangeTime;
  apcEffects.push_back(apcEffect);
}

void ApePixelClock::addApcScheduleCallback(unsigned long callbackTime , ApcScheduleCallback scheduleCallback)
{
  ApcScheduleCallbackDef apcScheduleCallback;
  apcScheduleCallback.callbackTime = callbackTime;
  apcScheduleCallback.currentRefreshTime = callbackTime;
  apcScheduleCallback.callbackFunc = scheduleCallback;
  apcScheduleCallbacks.push_back(apcScheduleCallback);
}

void ApePixelClock::btnCheckAction()
{
  static bool aBtnCheckState = true;
  if (aBtnCheckState)
  {
    if (digitalRead(ABTN_PIN) == HIGH) {
      aBtnCheckState = false;
      this->aBtnPressed();
      delay(20);
    }
  } else
  {
    if (digitalRead(ABTN_PIN) == LOW) {
      aBtnCheckState = true;
    }
  }

  static bool bBtnCheckState = true;
  if (bBtnCheckState)
  {
    if (digitalRead(BBTN_PIN) == HIGH) {
      bBtnCheckState = false;
      this->bBtnPressed();
      delay(20);
    }
  } else
  {
    if (digitalRead(BBTN_PIN) == LOW) {
      bBtnCheckState = true;
    }
  }
}
void ApePixelClock::aBtnPressed()
{
  Serial.println("a btn pressed!!!");
  this->apcEffectChangeAction();
  autoChange = false;
}
void ApePixelClock::bBtnPressed()
{
  Serial.println("b btn pressed!!!");
  autoChange = true;
  this->showInfo("Auto");
}

void    ApePixelClock::apcEffectChangeAction()
{
  apcEffectPointer++;
  if (apcEffectPointer >= apcEffects.size()) {
    apcEffectPointer = 0;
  }
  ApcEffectDef *apcEffect = apcEffects[apcEffectPointer];
  this->apcEffectRefresh(apcEffect);
}

void     ApePixelClock::apcEffectRefresh(ApcEffectDef *apcEffect)
{
  apcEffect->currentChangeTime = apcEffect->autoChangeTime;
  for (int i = 0; i < apcEffect->areaCount; i++)
  {
    ApcEffectAreaDef& apcEffectArea = apcEffect->areaDef[i];
    apcEffectArea.currentFrameCount = 0;
    apcEffectArea.currentRefreshTime = 0;
  }
}

void    ApePixelClock::apcCallbackAction()
{
  for (int i = 0; i < apcScheduleCallbacks.size(); i++)
  {
    ApcScheduleCallbackDef& apcScheduleCallback = apcScheduleCallbacks[i];
    if (millis() - callbackCheckTime > 0)
    {
      apcScheduleCallback.currentRefreshTime -= (millis() - callbackCheckTime);
    }
    if (apcScheduleCallback.currentRefreshTime <= 0)
    {
      apcScheduleCallback.callbackFunc();
      apcScheduleCallback.currentRefreshTime = apcScheduleCallback.callbackTime;
    }
  }
  callbackCheckTime = millis();
}

String  ApePixelClock::timeString(bool showColon)
{
  char timeChar[6];
  DateTime now = rtc.now();
  if (showColon) {
    sprintf(timeChar, "%02d:%02d", now.hour(), now.minute());
  }
  else {
    sprintf(timeChar, "%02d %02d", now.hour(), now.minute());
  }
  return String(timeChar);
}

void ApePixelClock::renderCheck()
{
  if (apcEffectPointer >= apcEffects.size()) return;
  ApcEffectDef* apcEffect = apcEffects[apcEffectPointer];
  int  areaCount = apcEffect->areaCount;
  for (int i = 0; i < areaCount; i++)
  {
    apcEffect->currentAreaIndex = i;
    ApcEffectAreaDef &apcEffectArea = apcEffect->areaDef[i];
    if (millis() - preCheckTime > 0)
    {
      apcEffectArea.currentRefreshTime -= (millis() - preCheckTime);
    }
    if (apcEffectArea.currentRefreshTime <= 0)
    {
      this->renderAction(apcEffect);
      apcEffectArea.currentFrameCount++;
      if (apcEffectArea.currentFrameCount >= apcEffectArea.frameCount) {
        apcEffectArea.currentFrameCount = 0;
      }
      apcEffectArea.currentRefreshTime = apcEffectArea.frameRefreshTime;
    }
  }
  if (autoChange)
  {
    if (millis() - preCheckTime > 0)
    {
      apcEffect->currentChangeTime -= (millis() - preCheckTime);
    }
    if (apcEffect->currentChangeTime <= 0)
    {
      this->apcEffectChangeAction();
      matrix.fillScreen(matrix.Color(0, 0, 0));
    }
  }
  preCheckTime = millis();
  matrix.show();
}

void ApePixelClock::renderAction(ApcEffectDef *apcEffect, bool needArea)
{ 
  ApcEffectAreaDef &apcEffectArea = apcEffect->areaDef[apcEffect->currentAreaIndex];
  matrix.offsetX = apcEffectArea.x;
  matrix.offsetY = apcEffectArea.y;
  matrix.areaWidth = apcEffectArea.width;
  matrix.areaHeight = apcEffectArea.height;
  for (int i = 0; i < apcEffectArea.width; i++)
  {
    for (int j = 0; j < apcEffectArea.height; j++)
    {
      matrix.drawPixel(i, j, 0);
    }
  }
  apcEffect->callbackFunc(apcEffect->currentAreaIndex, apcEffectArea.currentFrameCount, matrix);
}

void ApePixelClock::showInfo(String msg, unsigned long delayTime)
{
  matrix.offsetX = 0;
  matrix.offsetY = 0;
  matrix.areaWidth = MATRIX_WIDTH;
  matrix.areaHeight = MATRIX_HEIGHT;
  matrix.fillScreen(matrix.Color(0, 0, 0));
  matrix.setFont(&ApcFont35);
  matrix.setTextColor(matrix.Color(255, 255, 255));
  int strLength = msg.length();
  matrix.setCursor(this->textCenterX(strLength,4,8), 7);
  matrix.print(msg);
  matrix.show();
  delay(delayTime);
  if (apcEffectPointer >= apcEffects.size()) return;
  ApcEffectDef *apcEffect = apcEffects[apcEffectPointer];
  this->apcEffectRefresh(apcEffect);
  matrix.fillScreen(matrix.Color(0, 0, 0));
  this->renderAction(apcEffect);
}
int     ApePixelClock::textCenterX(int strLength, int charWidth, int maxCharCount)
{
   if (strLength > maxCharCount)strLength = maxCharCount;
   return (maxCharCount - strLength) * charWidth / 2;
}
