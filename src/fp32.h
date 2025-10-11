#ifndef FP32_H
#define FP32_H

#include <stdint.h>

// Bit expression of float based on IEEE binary32
typedef struct {
    uint8_t sign;       // 1 bit, 1 for negative
    uint8_t exp;        // 8 bit ([-126, 127], biased by 127)
    uint32_t mantissa;  // 23 bit by hidden bit convension
} Fp32;

// Convert float to Fp32
Fp32 fp32_from_float(const float value);

// Convert Fp32 to float
float fp32_to_float(const Fp32 fp32);

// Add 2 Fp32 values: a + b
Fp32 fp32_add(const Fp32 a, const Fp32 b);

// Subtract 2 Fp32 values: a - b
Fp32 fp32_sub(const Fp32 a, const Fp32 b);

// Multiply 2 Fp32 values: a * b
Fp32 fp32_mul(const Fp32 a, const Fp32 b);

// Divide 2 Fp32 values: a / b
Fp32 fp32_div(const Fp32 a, const Fp32 b);

#endif  // FP32_H
