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
    int8_t exp;         // 8 bit
                        // (biased by +-127 while processing)
    uint32_t mantissa;  // 23 bit by hidden bit convension
} Binary32;

// Convert float to Binary32
Binary32 fp32_cvt_to_binary32(const float value);

// Convert Binary32 to float
float fp32_cvt_to_float(const Binary32 binary32);

#endif // FP32_H
