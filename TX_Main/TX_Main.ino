/****************************************************

  开源跟踪天线发射端代码

  TCM3105硬件通讯版
  Copyright (C) 2013  Coeus Rowe <r.anerg@gmail.com>

  ****************************************************/
#define GPS_BAUD    (38400) //GPS模块的波特率
#define FSK_BAUD    (2400) //传输波特率，1200/2400可选
#define SOFT_RX     (2)
#define SOFT_TX     (3)

#include <SoftwareSerial.h>
SoftwareSerial mSerial(SOFT_RX, SOFT_TX);

#include "TinyGPS.h"
TinyGPS gps;
unsigned long fix_age;

/* 定义坐标数据结构 */
struct Location {
    long lat;//纬度
    long lng;//经度
    long alt;//高度
    int snum;//星数
};
//当前坐标
struct Location current_loc = {0, 0, 0, 0};
//GPS数据是否有新的
static char GPS_UPDATE = 0;
String GPS_MSG;
const char *msg;

void setup()
{
    Serial.begin(GPS_BAUD);
    delay(20);
    Serial.println("$PUBX,40,GLL,0,0,0,0*5C");
    Serial.println("$PUBX,40,GSA,0,0,0,0*4E");
    Serial.println("$PUBX,40,GSV,0,0,0,0*59");
    Serial.println("$PUBX,40,VTG,0,0,0,0*5E");
    mSerial.begin(FSK_BAUD);
    Serial.println("FX AAT TX Start...");
}

void loop()
{
    while (Serial.available()) 
    {
        if(gps.encode(Serial.read())) 
        {
            GPS_UPDATE = 1;
        }
    }


    if (GPS_UPDATE == 1) 
    {
        gps.get_position(&current_loc.lat, &current_loc.lng, &fix_age);
        current_loc.alt = gps.altitude();
        current_loc.snum = gps.satellites();

        if (fix_age != TinyGPS::GPS_INVALID_AGE) 
        {
            build_GPS_Info();
            msg = GPS_MSG.c_str();
            mSerial.write(msg);
            mSerial.write(char(0X0A));
            // Serial.println(GPS_MSG);
        }
    }
    GPS_UPDATE = 0;
}