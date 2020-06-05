#ifndef __APEPIXELCLOCK_H
#define __APEPIXELCLOCK_H

#include <Arduino.h>
#include "RTClib.h"
#include "Apc_AreaMatrix.h"

#define   MAX_APCEffECTAREA_COUNT  8


struct ApcEffectAreaDef
{
  int     x;              // from top-left to rigth-bottom
  int     y;
  int     width;
  int     height;
  int     frameCount;
  long    frameRefreshTime; //ms
  int     currentFrameCount;
  long    currentRefreshTime;
};



typedef void (*ApcEffectCallback)(unsigned int, unsigned int, Apc_AreaMatrix&);
typedef void (*ApcScheduleCallback)();

struct ApcEffectDef
{
  int                       areaCount;
  ApcEffectAreaDef          areaDef[MAX_APCEffECTAREA_COUNT];
  ApcEffectCallback         callbackFunc;
  int                       currentAreaIndex;
  long                      autoChangeTime;
  long                      currentChangeTime;
};
struct ApcScheduleCallbackDef
{
  long             callbackTime;
  long             currentRefreshTime;
  ApcScheduleCallback       callbackFunc;
};

class ApePixelClock
{
  public:
    ApePixelClock();
    void    systemInit(const char* wifi_ssid, const char* wifi_password, const int waitingTime = 10);
    bool    isWifiConnected();
    String  httpsRequest(const String& url, int* errCode);
    String  httpRequest(const String& url, int* errCode);
    void    apcLoop();
    void    addApcEffect(ApcEffectDef* apcEffect);
    void    addApcScheduleCallback(unsigned long callbackTime , ApcScheduleCallback scheduleCallback);
    String  timeString(bool showColon = true);
    void    showInfo(String msg, unsigned long delayTime = 1000);
    int     textCenterX(int strLength,int charWidth,int maxCharCount);
  private:
    void    renderCheck();
    void    renderAction(ApcEffectDef *apcEffect, bool needArea = true);
    void    btnCheckAction();
    void    aBtnPressed();
    void    bBtnPressed();
    void    apcEffectChangeAction();
    void    initRTC();
    void    apcCallbackAction();
    void    apcEffectRefresh(ApcEffectDef *apcEffect);
};






#endif // __APEPIXELCLOCK_H
