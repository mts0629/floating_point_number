#ifndef FP32_H
#define FP32_H

#include <stdint.h>

// Buffer to convert float/uint32 expression
typedef union {
    float fp32;
    uint32_t u32;
} Fp32Buffer;

// Bit expression of float based on IEEE binary32
typedef struct {
    uint8_t sign;       // 1 bit, 1 for negative
    uint8_t exp;        // 8 bit ([-126, 127], biased by 127)
    uint32_t mantissa;  // 23 bit by hidden bit convension
} Binary32;

// Biased exponent for Binary32
#define FP32_EXP(exp) { (exp) + 127 }

// Convert float to Binary32
Binary32 fp32_cvt_to_binary32(const float value);

// Convert Binary32 to float
float fp32_cvt_to_float(const Binary32 binary32);

// Add 2 Binary32 values: a + b
Binary32 fp32_add_binary32(const Binary32 a, const Binary32 b);

#endif // FP32_H
