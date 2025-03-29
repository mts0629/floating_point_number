#include <stdint.h>
#include <stdio.h>

// Buffer to convert float/uint32 expression
typedef union {
    float fp32;
    uint32_t u32;
} Fp32Buffer;

// Bit expression of float based on IEEE binary32
typedef struct {
    uint8_t sign;       // 1 bit
    uint8_t exp;
    uint32_t mantissa;  // 23 bit
} Fp32BitExpr;

Fp32BitExpr get_fp32_bits(const float value) {
    Fp32Buffer buf = { .fp32 = value };

    Fp32BitExpr bits;
    bits.sign     = ((buf.u32 & 0x80000000) >> 31) & 0x1;
    bits.exp      = ((buf.u32 & 0x7f800000) >> 23) & 0xff;
    bits.mantissa = buf.u32 & 0x007fffff;

    return bits;
}

float cvt_fp32_bits(const Fp32BitExpr bits) {
    Fp32Buffer buf = {
        .u32 = ((bits.sign & 0x1) << 31) |
               (bits.exp << 23) |
               (bits.mantissa & 0x7fffff)
    };

    return buf.fp32;
}

int main(void) {
    float value = -3.14f;

    // Parse bits from -3.14f
    Fp32BitExpr bits = get_fp32_bits(value);
    printf("%f\n", value);
    printf("  sign = %x\n", bits.sign);
    printf("  exp = %u\n", bits.exp);
    printf("  mantissa = %x\n", bits.mantissa);

    // Reproduce -3.14f from bits
    float f = cvt_fp32_bits(
        (Fp32BitExpr){
            .sign = 1,
            .exp = 128,
            .mantissa = 0x48f5c3
        }
    );
    printf("%f\n", f);

    return 0;
}
