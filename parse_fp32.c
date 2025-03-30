#include <stdio.h>

#include "fp32.h"

int main(void) {
    // Get binary32 bits from -3.14f
    float value = -3.14f;
    Binary32 bin32 = fp32_cvt_to_binary32(value);

    printf("%f\n", value);
    printf("  -> sign = %x, exp = %d, mantissa = %x\n",
        bin32.sign, bin32.exp, bin32.mantissa
    );

    // Get 42.195f from binary32 bits
    // 42.195
    // -> 0b'1010 10.00 1100 0111 1010 1110 (= 42.1949996948242)
    // -> 0b'1.010 1000 1100 0111 1010 1110 * 2^5
    // therefore:
    // - sign = 0
    // - exp = 5 (127 + 5 = 132 in excess notation)
    // - mantissa = 0b'010 1000 1100 0111 1010 1110 = 0x28c7ae
    uint8_t sign = 1;
    int8_t exp = 5;
    uint32_t mantissa = 0x28c7ae;
    float f = fp32_cvt_to_float(
        (Binary32){
            .sign = sign,
            .exp = exp,
            .mantissa = mantissa
        }
    );

    printf("sign = %x, exp = %d, mantissa = %x\n", sign, exp, mantissa);
    printf("  -> %f\n", f);

    return 0;
}
