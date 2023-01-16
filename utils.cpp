#include "everything.h"

float trim(float minv, float iv, float maxv) {
    if (iv < minv) return minv;
    if (iv > maxv) return maxv;
    return iv;
}

float trim_mid(float neg, float iv, float pos) {
    //return iv;
    if (0 < iv && iv < pos) return pos;
    if (neg < iv && iv < 0) return neg;
    return iv;
}
