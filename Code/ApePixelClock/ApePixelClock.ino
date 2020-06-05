#include "ApePixelClock.h"
#include "Apc_AreaMatrix.h"
#include "Fonts/ApcFont35.h"
#include <ArduinoJson.h>

ApePixelClock apc;
#define WIFI_SSID       "DLINK-TEST"      //note:only support 2.4G wifi
#define WIFI_PASSWORD   "APE@2020"

void setup() {
  apc.systemInit(WIFI_SSID, WIFI_PASSWORD, 15);

  ApcEffectDef* apcEffect_TimeShow = new ApcEffectDef;
  memset(apcEffect_TimeShow, 0, sizeof(ApcEffectDef));
  apcEffect_TimeShow->areaDef[0] = {0, 0, 32, 8, 2, 1000};
  apcEffect_TimeShow->callbackFunc = apcEffect_TimeShow_callback;
  apcEffect_TimeShow->autoChangeTime = 5000;
  apc.addApcEffect(apcEffect_TimeShow);

  ApcEffectDef* apcEffect_youtubeSubscriberCount = new ApcEffectDef;
  memset(apcEffect_youtubeSubscriberCount, 0, sizeof(ApcEffectDef));
  apcEffect_youtubeSubscriberCount->areaDef[0] = {0, 0, 8, 8, 1, 99999999};
  apcEffect_youtubeSubscriberCount->areaDef[1] = {8, 0, 24, 8, 6, 1000};
  apcEffect_youtubeSubscriberCount->callbackFunc = apcEffect_youtubeSubscriberCount_callback;
  apcEffect_youtubeSubscriberCount->autoChangeTime = 5000;
  apc.addApcEffect(apcEffect_youtubeSubscriberCount);

  ApcEffectDef* apcEffect_biliSubscriberCount = new ApcEffectDef;
  memset(apcEffect_biliSubscriberCount, 0, sizeof(ApcEffectDef));
  apcEffect_biliSubscriberCount->areaDef[0] = {0, 0, 8, 8, 1, 99999999};
  apcEffect_biliSubscriberCount->areaDef[1] = {8, 0, 24, 8, 6, 1000};
  apcEffect_biliSubscriberCount->callbackFunc = apcEffect_biliSubscriberCount_callback;
  apcEffect_biliSubscriberCount->autoChangeTime = 5000;
  apc.addApcEffect(apcEffect_biliSubscriberCount);

  //  ApcEffectDef* apcEffect_temperature = new ApcEffectDef;
  //  memset(apcEffect_temperature, 0, sizeof(ApcEffectDef));
  //  apcEffect_temperature->areaDef[0] = {0, 0, 32, 8, 1, 99999999};
  //  apcEffect_temperature->callbackFunc = apcEffect_temperature_callback;
  //  apcEffect_temperature->autoChangeTime = 5000;
  //  apc.addApcEffect(apcEffect_temperature);

  ApcEffectDef* apcEffect_temperature = new ApcEffectDef;
  memset(apcEffect_temperature, 0, sizeof(ApcEffectDef));
  apcEffect_temperature->areaDef[0] = {7, 0, 8, 8, 1, 99999999};
  apcEffect_temperature->areaDef[1] = {17, 0, 8, 8, 1, 99999999};
  apcEffect_temperature->callbackFunc = apcEffect_eyes_callback;
  apcEffect_temperature->autoChangeTime = 5000;
  apc.addApcEffect(apcEffect_temperature);

  

  apc.addApcScheduleCallback(60 * 60 * 1000, apcEffect_updateYouTubeSubscriberCount_callback);
  apcEffect_updateYouTubeSubscriberCount_callback();

  apc.addApcScheduleCallback(60 * 60 * 1000, apcEffect_updatebiliSubscriberCount_callback);
  apcEffect_updatebiliSubscriberCount_callback();

  apc.addApcScheduleCallback(15 * 60 * 1000, apcEffect_wether_callback);
  apcEffect_wether_callback();
}

void loop() {
  apc.apcLoop();
}

void apcEffect_TimeShow_callback(unsigned int areaCount, unsigned int frameCount, Apc_AreaMatrix& matrix)
{
  if (areaCount == 0)
  {
    matrix.setFont(&ApcFont35);
    matrix.setTextColor(matrix.Color(255, 255, 255));
    matrix.setCursor(8, 7);
    matrix.print(apc.timeString(frameCount == 0));
  }
}
int youTubeSubscriberCount = 0;
void apcEffect_updateYouTubeSubscriberCount_callback()
{
  if (apc.isWifiConnected())
  {
    const String CHANNEL = "UCVP3cwbysoohuvQbSWN8RgA";
    const String APIKEY = "AIzaSyDj5ul8pz-S0aCMqTFZaORURuETaLqC7L4";
    const String API = "https://www.googleapis.com/youtube/v3/channels?part=statistics";
    String url = API + "&id=" + CHANNEL + "&key=" + APIKEY;
    int errCode = 0;
    String res = apc.httpsRequest(url, &errCode);
    if (errCode == 0)
    {
      DynamicJsonDocument doc(1024);
      deserializeJson(doc, res);
      JsonObject obj = doc.as<JsonObject>();
      youTubeSubscriberCount = obj["items"][0]["statistics"]["subscriberCount"].as<int>();
    }
  }
}

int biliSubscriberCount = 0;
void apcEffect_updatebiliSubscriberCount_callback()
{
  if (apc.isWifiConnected())
  {

    const String UID = "298146460";
    const String API = "https://api.bilibili.com/x/relation/stat?vmid=";
    String url = API + UID ;
    int errCode = 0;
    String res = apc.httpsRequest(url, &errCode);
    if (errCode == 0)
    {
      DynamicJsonDocument doc(1024);
      deserializeJson(doc, res);
      JsonObject obj = doc.as<JsonObject>();
      biliSubscriberCount = obj["data"]["follower"].as<int>();
    }
  }
}
float temperature = 0;
int humidity = 0;
void apcEffect_wether_callback()
{
  if (apc.isWifiConnected())
  {

    const String API = "http://www.weather.com.cn/data/sk/101010200.html";
    String url = API;
    int errCode = 0;
    String res = apc.httpRequest(url, &errCode);
    Serial.println(res);
    if (errCode == 0)
    {
      DynamicJsonDocument doc(1024);
      deserializeJson(doc, res);
      JsonObject obj = doc.as<JsonObject>();
      temperature = obj["weatherinfo"]["temp"].as<float>();
      humidity = obj["weatherinfo"]["SD"].as<float>();
    }
    Serial.printf("temperature:%f\n", temperature);
    Serial.printf("humidity:%d\n", humidity);
  }

}

unsigned long colorArr[3] = {0x000000, 0xFF0000, 0xFFFFFF};
unsigned char pixels[64] =
{
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00,
  0x01, 0x01, 0x01, 0x02, 0x01, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x02, 0x02, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x02, 0x02, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x02, 0x01, 0x01, 0x01, 0x01,
  0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

void apcEffect_youtubeSubscriberCount_callback(unsigned int areaCount, unsigned int frameCount, Apc_AreaMatrix& matrix)
{
  if (areaCount == 0)
  {
    matrix.drawColorIndexFrame(colorArr, 8, 8, pixels);
  } else if (areaCount == 1)
  {
    String countStr = String(youTubeSubscriberCount);
    unsigned int strLength = countStr.length();
    matrix.setFont(&ApcFont35);
    matrix.setTextColor(matrix.Color(255, 255, 255));
    matrix.setCursor(apc.textCenterX(strLength, 4, 6), 7);
    matrix.print(countStr);
  }
}


unsigned long biliColorArr[2] = {0x000000, 0x00A1F1};
unsigned char biliPixels[64] =
{
  0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
  0x00, 0x00, 0x01, 0x00, 0x00, 0x01, 0x00, 0x00,
  0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00,
  0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
  0x01, 0x00, 0x01, 0x00, 0x00, 0x01, 0x00, 0x01,
  0x01, 0x00, 0x01, 0x00, 0x00, 0x01, 0x00, 0x01,
  0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
  0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00
};


void apcEffect_biliSubscriberCount_callback(unsigned int areaCount, unsigned int frameCount, Apc_AreaMatrix& matrix)
{
  if (areaCount == 0)
  {
    matrix.drawColorIndexFrame(biliColorArr, 8, 8, biliPixels);
  } else if (areaCount == 1)
  {
    String countStr = String(biliSubscriberCount);
    unsigned int strLength = countStr.length();
    matrix.setFont(&ApcFont35);
    matrix.setTextColor(matrix.Color(255, 255, 255));
    matrix.setCursor(apc.textCenterX(strLength, 4, 6), 7);
    matrix.print(countStr);
  }
}

unsigned long eyesColorArr[2] = {0x000000, 0x00A1F1};
unsigned char eyesPixels[64] =
{

  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00,
  0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
  0x01, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x01,
  0x01, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x01,
  0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
  0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

void apcEffect_temperature_callback(unsigned int areaCount, unsigned int frameCount, Apc_AreaMatrix& matrix)
{
  if (areaCount == 0)
  {
    char wInfo[20];
    sprintf(wInfo, "%.1f C", temperature);
    String countStr = String(wInfo);
    unsigned int strLength = countStr.length();
    matrix.setFont(&ApcFont35);
    matrix.setTextColor(matrix.Color(255, 255, 255));
    matrix.setCursor(apc.textCenterX(strLength, 4, 10), 7);
    matrix.print(countStr);
  } else if (areaCount == 1)
  {
    matrix.drawColorIndexFrame(eyesColorArr, 8, 8, eyesPixels);
  }
}

void apcEffect_eyes_callback(unsigned int areaCount, unsigned int frameCount, Apc_AreaMatrix& matrix)
{
  if (areaCount == 0)
  {
    matrix.drawColorIndexFrame(eyesColorArr, 8, 8, eyesPixels);
  } else if (areaCount == 1)
  {
    matrix.drawColorIndexFrame(eyesColorArr, 8, 8, eyesPixels);
  }
}
