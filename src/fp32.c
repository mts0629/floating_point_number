#include <stdbool.h>

#include "fp32.h"

#define MAX(a, b) (((a) >= (b)) ? (a): (b))
#define MIN(a, b) (((a) <= (b)) ? (a): (b))
#define ABS(a) (((a) >= 0) ? (a): -(a))

Binary32 fp32_cvt_to_binary32(const float value) {
    Fp32Buffer buf = { .fp32 = value };

    return (Binary32) {
        .sign = ((buf.u32 & 0x80000000) >> 31) & 0x1,
        .exp = ((buf.u32 & 0x7f800000) >> 23) & 0xff,
        .mantissa = buf.u32 & 0x7fffff
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

static inline uint32_t get_mantissa_with_hidden_bit(const Binary32 a) {
    return (0x1 << 23) | (a.mantissa & 0x7fffff);
}

static inline uint8_t get_sticky_bit(const uint32_t mantissa, const uint8_t shift) {
    if (shift > (24 + 3)) {
        // Shift over 24 bits means all bits are considered in the sticky bit
        return (((mantissa & ((0x1 << (24 + 3)) - 1)) > 0x0) ? 0x1 : 0x0);
    }
    return (((mantissa & ((0x1 << (shift - 2)) - 1)) > 0x0) ? 0x1 : 0x0);
}

static uint8_t get_msb_digit(const uint32_t value) {
    uint8_t msb_digit = 31;
    while (msb_digit > 0) {
        if (((value & (1 << msb_digit)) >> msb_digit) == 0x1) {
            break;
        }
        msb_digit--;
    }

    return msb_digit;
}

Binary32 fp32_add_binary32(const Binary32 a, const Binary32 b) {
    // Mantissas with a hidden bit
    uint32_t a_man = get_mantissa_with_hidden_bit(a);
    uint32_t b_man = get_mantissa_with_hidden_bit(b);

    // Pre-shift of mantissas: adjust exp. scale to a large one
    uint8_t sum_exp = MAX(a.exp, b.exp);
    if (a.exp > b.exp) {
        uint8_t shift = a.exp - b.exp;
        a_man <<= 3;
        b_man >>= MIN((shift - 3), 24); // Avoid too much shift down
        b_man |= get_sticky_bit(b_man, shift);
    } else if (b.exp > a.exp) {
        uint8_t shift = b.exp - a.exp;
        a_man >>= MIN((shift - 3), 24); // Avoid too much shift down
        a_man |= get_sticky_bit(b_man, shift);
        b_man <<= 3;
    }

    // Add mantissas
    int32_t sa = (a.sign == 1) ? -(int32_t)a_man : (int32_t)a_man;
    int32_t sb = (b.sign == 1) ? -(int32_t)b_man : (int32_t)b_man;
    int32_t sum_man = sa + sb;

    // Signature bit
    uint8_t sign = (sum_man > 0) ? 0x0 : 0x1;

    // Mantissa is an absolute value
    uint32_t norm_man = (uint32_t)ABS(sum_man);

    // Normalize mantissa: make b23 as MSB by scaling
    while (true) {
        uint8_t msb_digit = get_msb_digit(norm_man);

        if (msb_digit > (23 + 3)) {
            uint8_t shift = msb_digit - (23 + 3);
            uint8_t s = get_sticky_bit(norm_man, shift);
            norm_man >>= shift;
            norm_man |= s;

            // Overflow: return +-inf
            if (((int)sum_exp + shift) > 255) {
                return (Binary32){ sign, 255, 0x0 };
            }

            sum_exp += shift;
        } else if (msb_digit < (23 + 3)) {
            uint8_t shift = (23 + 3) - msb_digit;
            norm_man <<= shift;

            // Underflow: return a subnormal number
            if (((int)sum_exp - shift) < 0) {
                return (Binary32){ sign, 0, (norm_man & 0x7fffff) };
            }

            sum_exp -= shift;
        }

        // Round-even by using guard bit, round bit and sticky bit
        uint8_t lsb = (norm_man & 0x8) >> 3;
        uint8_t g = (norm_man & 0x4) >> 2;  // Guard bit
        uint8_t r = (norm_man & 0x2) >> 1;  // Round bit
        uint8_t s = norm_man & 0x1;         // Sticky bit
        uint8_t round_up = g & (lsb | r | s);
        if (round_up == 0x1) {
            norm_man += (0x1 << 3);
        }

        if (get_msb_digit(norm_man) == (23 + 3)) {
            break;
        }
    }

    return (Binary32){ sign, sum_exp, ((norm_man >> 3) & 0x7fffff) };
}
