/****************************************************

  开源跟踪天线发射端代码 - GPS数据处理模块

  TCM3105硬件通讯版
  Copyright (C) 2013  Coeus Rowe <r.anerg@gmail.com>

****************************************************/
void read_GPS()
{
    while (Serial.available()) {
        char c = Serial.read();

        if (gps.encode(c)) {
            GPS_UPDATE = 1;
        }
    }
}

void build_GPS_Info()
{
    GPS_MSG = "$FX,";
    GPS_MSG += current_loc.lat;
    GPS_MSG += ",";
    GPS_MSG += current_loc.lng;
    GPS_MSG += ",";
    GPS_MSG += current_loc.alt;
    GPS_MSG += ",";
    GPS_MSG += current_loc.snum;
    GPS_MSG += ",";
    int c = 0, XOR = 0;

    for (int i = 1; i < GPS_MSG.length(); i++) {
        c = (unsigned char)GPS_MSG[i];
        XOR ^= c;
    }

    GPS_MSG += "*";
    GPS_MSG += String(XOR, HEX);
}