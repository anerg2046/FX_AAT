/****************************************************

  开源跟踪天线接收端代码

  TCM3105硬件通讯版
  Copyright (C) 2013  Coeus Rowe <r.anerg@gmail.com>

  ****************************************************/
//
// ***************************************************
// ***************波特率的设置************************
//
// 地面GPS模块的波特率
#define GPS_BAUD            (38400)
//
// FSK模块传输波特率，1200/2400可选
// 发射接收必须相同，1200稳定性更好，但速度略慢
#define FSK_BAUD            (2400)
//
//
// ***************************************************
// *******************舵机设置************************
//
// 定义舵机控制端口
#define SERVO_V_PIN         (2)
//

#include <EEPROM.h>
#include "EEPROMAnything.h"
struct config_t
{
    int servo_min_pulse;
    int servo_max_pulse;
    int servo_rev;
} sys_cfg;


//
// ***************************************************
// ***************步进电机设置************************
//
// 步进电机一圈总步数，查询你购买的电机的参数
// 一般会有‘步矩角’，这个值等于 （360/步矩角）
#define STEPPER_STEP_NUM    (48)
//
// 步进电机减速比，湛江的是1:50减速
#define STEPPER_RATE        (50)
//
// 定义步进电机方向控制端口
#define STEPPER_DIR         (4)
//
// 定义步进电机步数控制端口
#define STEPPER_STEP        (3)
//
// 步进电机反向，取值0、1
// 根据你的安装来调整，如果运行方向是反的，就改为1
#define STEPPER_REV         0
//
// 步进电机移动速度
#define STEPPER_SPEED       (3000.0)
//
// ***************************************************
// *****************LED引脚定义***********************
//
#define ledPIN              (13)
static bool ledSTAT = LOW;
//
// ***************************************************
// ****************加载类库***************************
//
// 步进电机控制库
#include "FxStepper.h"
FxStepper stepper(STEPPER_STEP, STEPPER_DIR, STEPPER_SPEED);

//
// GPS库
#include "TinyGPS.h"
TinyGPS gps;
unsigned long fix_age;
//
// 指南针模块
#include <Wire.h>
#include "HMC5883Llib.h"
Magnetometer mag;
bool MagAvailable = true;
//
#include <Servo.h>
// 俯仰舵机，对应飞机高度
Servo SERVO_V;
// ***************************************************
// ****************一些变量设置***********************
//
// 机械部分转一圈需要的步数，已通过easydriver细分8步以后的值
long Stepper_Total_Num;

struct Dgree_T
{
    float H;//水平角度，相对于北，正北为0度
    float V;//俯仰角度，相对于垂直面，垂直为0度
};
struct Dgree_T cur_dgree = {0, 0};//当前应该转到的角度
struct Dgree_T lst_dgree = {0, 0};//上一次的角度

/* 定义坐标数据结构 */
struct Location
{
    float lat;//纬度
    float lng;//经度
    float alt;//高度
    int snum;//星数
};
//当前飞机坐标
struct Location current_loc = {0, 0, 0, 0};
//家坐标
struct Location home_loc = {0, 0, 0, 0};
//飞机GPS数据是否有新的
static char GPS_UPDATE = 0;
//家的GPS数据是否有更新
static char Home_GPS_UPDATE = 0;
static int Home_GPS_Count = 0;

static char _DEBUG = 1;


void setup()
{

    pinMode(ledPIN, OUTPUT);
    digitalWrite(ledPIN, LOW);

    Serial.begin(38400);

    Serial2.begin(GPS_BAUD);
    Serial2.print("$PUBX,41,1,0007,0003,4800,0*13\r\n");
    delay(20);
    Serial2.begin(4800);
    Serial2.println("$PUBX,40,GLL,0,0,0,0*5C");
    Serial2.println("$PUBX,40,GSA,0,0,0,0*4E");
    Serial2.println("$PUBX,40,GSV,0,0,0,0*59");
    Serial2.println("$PUBX,40,VTG,0,0,0,0*5E");

    load_cfg();

    Serial3.begin(FSK_BAUD);

    Stepper_Total_Num = STEPPER_STEP_NUM * STEPPER_RATE * 8;
    stepper.start();

    if (mag.begin() != 0)
    {
        MagAvailable = false;
        Serial.println("Error connecting to Magnetometer");
    }
    else
    {
        mag.setGain(HMC5833L_GAIN_1090);
        Serial.println("Success connecting to Magnetometer");
    }
    if (MagAvailable == true)
    {
        if (_DEBUG != 1)
        {
            FindNorth();
        }
    }

    SERVO_V.attach(SERVO_V_PIN);
    delay(2000);
    lst_dgree.V = (float)SERVO_V.read();
    Servo_Move();

    Serial.println("AAT RX start...");

}

void load_cfg()
{
    EEPROM_readAnything(0, sys_cfg);
    if (sys_cfg.servo_min_pulse < 0)
    {
        sys_cfg.servo_min_pulse = 1000;
        sys_cfg.servo_max_pulse = 2800;
        sys_cfg.servo_rev = 0;
        EEPROM_writeAnything(0, sys_cfg);
    }
}

void loop()
{
    if (Home_GPS_UPDATE == 0)
    {
        read_Home_GPS();
    }
    else
    {
        read_GPS();
        if (GPS_UPDATE == 1)
        {
            calc_Angle();//计算出角度
            Track_Main();
        }

        GPS_UPDATE = 0;
        Home_GPS_Count++;
        if (Home_GPS_Count > 10)
        {
            read_Home_GPS();
            Home_GPS_Count = 0;
        }
    }

    if (stepper.isDone() == false)
    {
        stepper.run();
    }

    if (Serial.available())
    {
        configSet();
    }
}


const char *command_msg;
void configSet()
{
    String command = "";
    while (Serial.available() > 0)
    {
        char c = Serial.read();
        if (c != 0x0A)
        {
            command += c;
        }
        delay(10);
    }
    if (command == "read")
    {

        String _msg = "$FXAAT,";
        _msg += sys_cfg.servo_min_pulse;
        _msg += ",";
        _msg += sys_cfg.servo_max_pulse;
        _msg += ",";
        _msg += sys_cfg.servo_rev;
        _msg += ",END$";
        command_msg = _msg.c_str();
        Serial.write(command_msg);
        Serial.write(char(0X0A));
    }
    else if (command == "goTo0")
    {
        SERVO_V.writeMicroseconds(sys_cfg.servo_min_pulse);
    }
    else if (command == "goTo90")
    {
        SERVO_V.writeMicroseconds((sys_cfg.servo_max_pulse - sys_cfg.servo_min_pulse) / 2);
    }
}