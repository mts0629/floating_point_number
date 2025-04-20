#include "fp32.h"

Binary32 fp32_cvt_to_binary32(const float value) {
    Fp32Buffer buf = { .fp32 = value };

    return (Binary32) {
        .sign = ((buf.u32 & 0x80000000) >> 31) & 0x1,
        .exp = ((buf.u32 & 0x7f800000) >> 23) & 0xff,
        .mantissa = buf.u32 & 0x007fffff
    };
}

float fp32_cvt_to_float(const Binary32 bin32) {
    Fp32Buffer buf = {
        .u32 = ((bin32.sign & 0x1) << 31) |
               (bin32.exp << 23) |
               (bin32.mantissa & 0x7fffff)
    };

    return buf.fp32;
}

Binary32 fp32_add_binary32(const Binary32 a, const Binary32 b) {
    // Signed mantissas...                              with a hidden bit
    int32_t a_man = (int32_t)((a.mantissa & 0x7fffff) | (0x1 << 23) | ((a.sign & 0x1) << 31));
    int32_t b_man = (int32_t)((b.mantissa & 0x7fffff) | (0x1 << 23) | ((b.sign & 0x1) << 31));

    // Adjust exp. scale to a large one
    uint8_t sum_exp = a.exp;
    if (a.exp > b.exp) {
        b_man >>= (a.exp - b.exp);
    } else if (b.exp > a.exp) {
        a_man >>= (b.exp - a.exp);
        sum_exp = b.exp;
    }

    // Add mantissas
    int32_t sum_man = a_man + b_man;

    // Signature bit
    uint8_t sign = ((uint32_t)sum_man & 0x80000000) >> 31;

    // Normalize summed mantissa: scale to make MSB (b23) to 1
    if (((sum_man & 0x1000000) >> 24) == 0x1) {
        sum_man >>= 1;
        sum_exp++;
        if (sum_exp == 255) {
            // Overflow: return +-inf
            return (Binary32){ sign, 255, 0x0 };
        }
    }
    while (((sum_man & 0x800000) >> 23) != 0x1) {
        sum_man <<= 1;
        sum_exp--;
        if (sum_exp == 0) {
            // Underflow, return a subnormal number
            return (Binary32){ sign, 0, ((uint32_t)sum_man & 0x7fffff) };
        }
    }

    // (Rounding by LSB should be done here)
    // (Loop normalization if a rounded value is subnormal)

    return (Binary32){ sign, sum_exp, (uint32_t)(sum_man & 0x7fffff) };
}
