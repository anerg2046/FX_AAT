struct Dist_T
{
    double lat;//纬度距离
    double lng;//经度距离
    double line;//直线距离
} _dist;

void calc_Angle()
{
    cacl_Dist();
    if (_dist.line > 15)
    {
        //俯仰计算，暂不考虑飞机比aat低的情况，如：在山顶往下飞
        cur_dgree.V = atan((current_loc.alt - home_loc.alt) / _dist.line) * 180 / PI;
        //水平计算
        cur_dgree.H = TinyGPS::course_to(home_loc.lat, home_loc.lng, current_loc.lat, current_loc.lng);
    }
    // // 当距离大于300米，且水平要求转180度，则视为错误数据，忽略
    // if (_dist.line > 300 && abs(cur_dgree.H - lst_dgree.H) > 180)
    // {
    //     cur_dgree.H = lst_dgree.H;
    // }
}

void cacl_Dist()
{
    _dist.line = TinyGPS::distance_between(home_loc.lat, home_loc.lng, current_loc.lat, current_loc.lng);
}