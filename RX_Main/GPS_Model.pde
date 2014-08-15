/****************************************************

  开源跟踪天线接收端代码 -- GPS数据读取模块

  TCM3105硬件通讯版
  Copyright (C) 2013  Coeus Rowe <r.anerg@gmail.com>

  ****************************************************/
String GPS_MSG, R_XOR;
int _XOR, _do_XOR, _buf_len;

void read_Home_GPS()
{
    // if (_DEBUG == 1)
    // {
    //     Home_GPS_UPDATE = 1;
    //     home_loc.lat = 39.9111713;
    //     home_loc.lng = 116.5097933;
    //     home_loc.alt = 10;
    //     home_loc.snum = 10;
    //     digitalWrite(ledPIN, HIGH);
    //     return;
    // }

    while (Serial2.available())
    {
        char c = Serial2.read();
        Serial.print(c);
        if (gps.encode(c))
        {
            gps.f_get_position(&home_loc.lat, &home_loc.lng, &fix_age);
            home_loc.alt = gps.f_altitude();
            home_loc.snum = gps.satellites();
            if (fix_age != TinyGPS::GPS_INVALID_AGE)
            {
                if (home_loc.snum > 6)
                {
                    Home_GPS_UPDATE = 1;
                    digitalWrite(ledPIN, HIGH);
                    // Serial.println("=============OK===========");
                }
            }
            return;
        }
    }
}

void read_GPS()
{
    while (Serial3.available())
    {
        char c = Serial3.read();
        // Serial.print(c);
        if (GPS_Encode(c))
        {
            Save_Location();
            GPS_UPDATE = 1;
            ledSTAT = !ledSTAT;
            digitalWrite(ledPIN, ledSTAT);
            // Serial.println("=============OK===========");
            return;
        }
    }
}

bool GPS_Encode(char c)
{
    if (c == '$')
    {
        GPS_MSG = "";
        R_XOR = "";
        _XOR = 0;
        _do_XOR = 1;
        _buf_len = 0;
    }

    if (_buf_len > 200)
    {
        return false;
    }

    if (c == '*')
    {
        _do_XOR = 0;
    }

    if (c == 0x0A)
    {
        return String(_XOR, HEX) == R_XOR;
    }

    GPS_MSG += c;

    if (_do_XOR == 0 && c != '*')
    {
        R_XOR += c;
    }

    if (_do_XOR == 1 && c != '$')
    {
        __XOR(c);
    }

    _buf_len++;
    return false;
}

void __XOR(char c)
{
    _XOR ^= c;
}

void Save_Location()
{
    int pos = -1;//字符串查找位置
    int part = 0;//gps分段位置
    String part_info;//gps分段数据

    do
    {
        pos = GPS_MSG.indexOf(',');

        if (pos != -1)
        {
            part_info = GPS_MSG.substring(0, pos);
            part_info.trim();
            GPS_MSG = GPS_MSG.substring(pos + 1, GPS_MSG.length());

            switch (part)
            {
            case 1:
                current_loc.lat = float(part_info.toInt()) / 1000000;
                break;

            case 2:
                current_loc.lng = float(part_info.toInt()) / 1000000;
                break;

            case 3:
                current_loc.alt = float(part_info.toInt()) / 100;
                break;

            case 4:
                current_loc.snum = part_info.toInt();
                break;
            }

            part++;
        }
    }
    while (pos >= 0);

}