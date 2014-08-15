int16_t M_max_x, M_x, M_y, M_z;
long stepperNorthPos;
void FindNorth()
{
    // 转两圈
    stepper.rotate(Stepper_Total_Num * 2);
    while (stepper.isDone() == false)
    {
        getHeading();
        setNorthPos();
        stepper.run();
    }
    delay(1000);
    stepper.rotate(stepperNorthPos);
    while (stepper.isDone() == false)
    {
        stepper.run();
    }
}

void getHeading()
{
    int8_t ret = mag.readRaw(&M_x, &M_y, &M_z);

    switch (ret)
    {
    case HMC5833L_ERROR_GAINOVERFLOW:
        Serial.println("Gain Overflow");
        return;
    case 0:
        // success
        break;
    default:
        Serial.println("Failed to read Magnetometer");
        return;
    }
}

void setNorthPos()
{
    if (M_x > M_max_x)
    {
        M_max_x = M_x;
        stepperNorthPos = stepper.getStepsGone();
    }
}