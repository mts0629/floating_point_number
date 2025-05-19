#include "fp32.h"

#include <stdbool.h>

#define MAX(a, b) (((a) >= (b)) ? (a) : (b))
#define MIN(a, b) (((a) <= (b)) ? (a) : (b))
#define ABS(a) (((a) >= 0) ? (a) : -(a))

#define N_MANTISSA 23
#define N_ROUND_BITS 3

// Buffer used for "type punning" between float/uint32
typedef union {
    float fp32;
    uint32_t u32;
} Fp32Buffer;

Binary32 fp32_to_binary32(const float value) {
    Fp32Buffer buf = {.fp32 = value};

    return (Binary32){.sign = ((buf.u32 & 0x80000000) >> 31) & 0x1,
                      .exp = ((buf.u32 & 0x7f800000) >> N_MANTISSA) & 0xff,
                      .mantissa = buf.u32 & 0x7fffff};
}

float fp32_to_float(const Binary32 binary32) {
    Fp32Buffer buf = {.u32 = ((binary32.sign & 0x1) << 31) |
                             (binary32.exp << N_MANTISSA) |
                             (binary32.mantissa & 0x7fffff)};

    return buf.fp32;
}

static inline uint32_t get_mantissa_with_hidden_bit(const Binary32 binary32) {
    return (0x1 << N_MANTISSA) | (binary32.mantissa & 0x7fffff);
}

static uint8_t get_sticky_bit(const uint32_t mantissa, const uint8_t shift) {
    uint32_t mask;
    if (shift < 3) {
        // No sticy bit
        return 0x0;
    }
    if (shift > (N_MANTISSA + N_ROUND_BITS)) {
        // All bits are considered in the sticky bit
        mask = 0x7ffffff - 1;
    } else {
        mask = ((0x1 << (shift - (N_ROUND_BITS - 1))) - 1) - 1;
    }

    return (((mantissa & mask) > 0) ? 0x1 : 0x0);
}

static uint8_t get_msb_digit(const uint32_t mantissa) {
    uint8_t msb_digit = 31;
    while (msb_digit > 0) {
        if (((mantissa & (0x1 << msb_digit)) >> msb_digit) == 0x1) {
            break;
        }
        msb_digit--;
    }

    return msb_digit;
}

Binary32 fp32_add(const Binary32 a, const Binary32 b) {
    // Mantissas with a hidden bit and round bits
    uint32_t a_man = get_mantissa_with_hidden_bit(a) << N_ROUND_BITS;
    uint32_t b_man = get_mantissa_with_hidden_bit(b) << N_ROUND_BITS;

    // Pre-shift of mantissas: adjust exp. scale to a large one
    uint8_t sum_exp = MAX(a.exp, b.exp);
    if (a.exp > b.exp) {
        uint8_t shift = a.exp - b.exp;
        uint8_t s = get_sticky_bit(b_man, shift);
        b_man >>= MIN(
            shift, (N_MANTISSA + N_ROUND_BITS));  // Avoid too much shift down
        b_man &= 0x7fffffe;
        b_man |= s;
    } else if (b.exp > a.exp) {
        uint8_t shift = b.exp - a.exp;
        uint8_t s = get_sticky_bit(a_man, shift);
        a_man >>= MIN(
            shift, (N_MANTISSA + N_ROUND_BITS));  // Avoid too much shift down
        a_man &= 0x7fffffe;
        a_man |= s;
    }

    // Add mantissas
    int32_t sa = (a.sign == 0x1) ? -(int32_t)a_man : (int32_t)a_man;
    int32_t sb = (b.sign == 0x1) ? -(int32_t)b_man : (int32_t)b_man;
    int32_t sum_man = sa + sb;

    // Signature bit
    uint8_t sign = (sum_man > 0) ? 0x0 : 0x1;

    // Mantissa is an absolute value
    uint32_t norm_man = (uint32_t)ABS(sum_man);

    // Normalize mantissa: make b23 as MSB by scaling
    while (true) {
        uint8_t msb_digit = get_msb_digit(norm_man);

        if (msb_digit > (N_MANTISSA + N_ROUND_BITS)) {
            uint8_t shift = msb_digit - (N_MANTISSA + N_ROUND_BITS);
            uint8_t s = get_sticky_bit(norm_man, shift);
            norm_man >>= shift;
            norm_man &= 0x7fffffe;
            norm_man |= s;

            // Overflow: return +-inf
            if (((int)sum_exp + shift) > 255) {
                return (Binary32){sign, 255, 0x0};
            }

            sum_exp += shift;
        } else if (msb_digit < (N_MANTISSA + N_ROUND_BITS)) {
            uint8_t shift = (N_MANTISSA + N_ROUND_BITS) - msb_digit;
            norm_man <<= shift;

            // Underflow: return a subnormal number
            if (((int)sum_exp - shift) < 0) {
                return (Binary32){sign, 0, (norm_man & 0x7fffff)};
            }

            sum_exp -= shift;
        }

        // Round-even by using guard bit, round bit and sticky bit
        uint8_t lsb = (norm_man & 0x8) >> N_ROUND_BITS;
        uint8_t g = (norm_man & 0x4) >> 2;  // Guard bit
        uint8_t r = (norm_man & 0x2) >> 1;  // Round bit
        uint8_t s = norm_man & 0x1;         // Sticky bit
        uint8_t round_up = g & (lsb | r | s);
        norm_man += (round_up << N_ROUND_BITS);

        if (get_msb_digit(norm_man) == (N_MANTISSA + N_ROUND_BITS)) {
            break;
        }
    }

    return (Binary32){sign, sum_exp, ((norm_man >> N_ROUND_BITS) & 0x7fffff)};
}

Binary32 fp32_sub(const Binary32 a, const Binary32 b) {
    Binary32 neg_b = b;
    neg_b.sign = (neg_b.sign == 0x1) ? 0x0 : 0x1;

    return fp32_add(a, neg_b);
}
