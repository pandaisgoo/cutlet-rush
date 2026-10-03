#ifndef ANGLE_H_INCLUDED
#define ANGLE_H_INCLUDED

#include <cmath>
#include <cstdlib>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// 產生 [min, max] 的 double 隨機數
inline double rand_range(double min, double max) {
    double r = (double)rand() / RAND_MAX;
    return min + r * (max - min);
}

// 產生 [min, max] 的 int 隨機數
inline int rand_int(int min, int max) {
    return min + rand() % (max - min + 1);
}

// 度數轉弧度
inline double deg2rad(double deg) {
    return deg * M_PI / 180.0;
}

// 弧度轉度數
inline double rad2deg(double rad) {
    return rad * 180.0 / M_PI;
}

// 距離
inline double dist(double x1, double y1, double x2, double y2) {
    double dx = x2 - x1;
    double dy = y2 - y1;
    return sqrt(dx*dx + dy*dy);
}

#endif
