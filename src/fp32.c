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
    if (shift < N_ROUND_BITS) {
        // No sticy bit
        return 0x0;
    }

    uint32_t mask;
    if (shift > (N_MANTISSA + N_ROUND_BITS)) {
        // All bits are considered in the sticky bit
        mask = 0x7fffffe;
    } else {
        mask = ((0x1 << (shift - (N_ROUND_BITS - 1))) - 1) - 1;
    }

    return (((mantissa & mask) > 0) ? 0x1 : 0x0);
}

static uint8_t get_msb_digit(const uint32_t mantissa) {
    uint8_t msb_digit = 31;
    while (msb_digit > 0) {
        if (((mantissa & ((uint32_t)0x1 << msb_digit)) >> msb_digit) == 0x1) {
            break;
        }
        msb_digit--;
    }

    return msb_digit;
}

static void normalize(uint32_t *mantissa, int *exp) {
    uint32_t m = *mantissa;
    int e = *exp;

    while (true) {
        uint8_t msb_digit = get_msb_digit(m);

        if (msb_digit > (N_MANTISSA + N_ROUND_BITS)) {
            uint8_t shift = msb_digit - (N_MANTISSA + N_ROUND_BITS);
            uint8_t s = get_sticky_bit(m, shift);
            m >>= shift;
            m &= 0x7fffffe;
            m |= s;

            e += shift;
        } else if (msb_digit < (N_MANTISSA + N_ROUND_BITS)) {
            uint8_t shift = (N_MANTISSA + N_ROUND_BITS) - msb_digit;
            m <<= shift;

            e -= shift;
        }

        if (e >= 255) {
            // Overflow: +-inf
            e = 255;
            m = 0x0;
            break;
        } else if (e < 0) {
            // Underflow: a subnormal number
            e = 0;
            break;
        }

        // Round-even by using guard bit, round bit and sticky bit
        uint8_t lsb = (m & 0x8) >> N_ROUND_BITS;
        uint8_t g = (m & 0x4) >> 2;  // Guard bit
        uint8_t r = (m & 0x2) >> 1;  // Round bit
        uint8_t s = m & 0x1;         // Sticky bit
        uint8_t round_up = g & (lsb | r | s);
        m += (round_up << N_ROUND_BITS);

        // Finish when the mantissa is normalized
        if (get_msb_digit(m) == (N_MANTISSA + N_ROUND_BITS)) {
            break;
        }
    }

    // Dispose round bits and a hidden bit
    *mantissa = (m >> N_ROUND_BITS) & 0x7fffff;
    *exp = e;
}

Binary32 fp32_add(const Binary32 a, const Binary32 b) {
    // Mantissas with a hidden bit and round bits
    uint32_t a_man = get_mantissa_with_hidden_bit(a) << N_ROUND_BITS;
    uint32_t b_man = get_mantissa_with_hidden_bit(b) << N_ROUND_BITS;

    // Pre-shift of mantissas: adjust exp. scale to a large one
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

    // Normalize the mantissa
    uint32_t mantissa = (uint32_t)ABS(sum_man);
    int exp = MAX(a.exp, b.exp);
    normalize(&mantissa, &exp);

    return (Binary32){sign, (uint8_t)exp, mantissa};
}

Binary32 fp32_sub(const Binary32 a, const Binary32 b) {
    Binary32 neg_b = b;
    neg_b.sign = (neg_b.sign == 0x1) ? 0x0 : 0x1;

    return fp32_add(a, neg_b);
}

Binary32 fp32_mul(const Binary32 a, const Binary32 b) {
    // Mantissas with a hidden bit and round bits
    uint64_t a_man = get_mantissa_with_hidden_bit(a);
    uint64_t b_man = get_mantissa_with_hidden_bit(b);

    // Add two exponents
    int exp = a.exp + b.exp - 127;

    // Multiply mantissas
    uint64_t mul_man = a_man * b_man;

    // Right shift to dispose extra fraction bits,
    // considering with round bits and a hidden bit
    uint8_t shift = N_MANTISSA - N_ROUND_BITS;
    uint64_t mask = (((uint64_t)0x1 << (shift - (N_ROUND_BITS - 1))) - 1) - 1;
    uint8_t sticky_bit = ((mul_man & mask) > 0) ? 0x1 : 0x0;
    mul_man >>= shift;
    mul_man &= 0x7fffffe;
    mul_man |= sticky_bit;

    // Normalize the mantissa
    uint32_t mantissa = (uint32_t)mul_man;
    normalize(&mantissa, &exp);

    // Signature bit
    uint8_t sign = (a.sign == b.sign) ? 0x0 : 0x1;

    return (Binary32){sign, (uint8_t)exp, mantissa};
}

Binary32 fp32_div(const Binary32 a, const Binary32 b) {
    // Signature bit
    uint8_t sign = (a.sign == b.sign) ? 0x0 : 0x1;

    if ((a.exp == 0) && (a.mantissa == 0)) {
        return (Binary32){sign, 0, 0};
    }

    // Mantissas with a hidden bit and round bits
    uint64_t a_man = get_mantissa_with_hidden_bit(a) << N_ROUND_BITS;
    uint64_t b_man = get_mantissa_with_hidden_bit(b) << N_ROUND_BITS;

    // Subtract two exponents
    int exp = a.exp - b.exp + 127;

    // Divide mantissas
    uint64_t div_man = (a_man << (N_MANTISSA + N_ROUND_BITS)) / b_man;

    // Normalize the mantissa
    uint32_t mantissa = (uint32_t)div_man;
    normalize(&mantissa, &exp);

    return (Binary32){sign, (uint8_t)exp, mantissa};
}
