/****************************************************

  废墟版跟踪天线接收端代码
  Copyright (C) 2013  Coeus Rowe

  ****************************************************/
void Track_Main()
{
    Servo_Move();

    float dif = (cur_dgree.H - lst_dgree.H) * 100;
    if (abs(dif) > 20)
    {

        long step_val = 1;

        if (dif < 0)
        {
            step_val *= -1;
        }
        if (abs(dif) > 18000)
        {
            step_val *= -1;
            dif = 36000.0 - abs(dif);
        }
        step_val *= abs((long)(Stepper_Total_Num * dif / 36000.0));
        if (STEPPER_REV == 1)
        {
            step_val *= -1;
        }
        stepper.rotate(step_val);
        lst_dgree.H = cur_dgree.H;
    }
}

void Servo_Move()
{
    cur_dgree.V = constrain(cur_dgree.V, 0, 90);//只允许在0-90度移动
    Serial.println(cur_dgree.V);
    if (sys_cfg.servo_rev == 1)
    {
        cur_dgree.V = 180.0 - cur_dgree.V;
    }

    long microV;
    microV = map(cur_dgree.V * 100, 0, 18000, sys_cfg.servo_min_pulse, sys_cfg.servo_max_pulse);
    SERVO_V.writeMicroseconds(microV);

    lst_dgree.V = cur_dgree.V;

}
