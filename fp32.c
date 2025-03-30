#include "fp32.h"

Binary32 fp32_cvt_to_binary32(const float value) {
    Fp32Buffer buf = { .fp32 = value };

    return (Binary32) {
        .sign = ((buf.u32 & 0x80000000) >> 31) & 0x1,
        .exp = (int8_t)((((buf.u32 & 0x7f800000) >> 23) & 0xff) - 127),
        .mantissa = buf.u32 & 0x007fffff
    };
}

float fp32_cvt_to_float(const Binary32 bin32) {
    Fp32Buffer buf = {
        .u32 = ((bin32.sign & 0x1) << 31) |
               (((uint8_t)bin32.exp + 127) << 23) |
               (bin32.mantissa & 0x7fffff)
    };

    return buf.fp32;
}
