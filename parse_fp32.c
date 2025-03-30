#include <stdio.h>

#include "fp32.h"

int main(void) {
    float value = -3.14f;

    // Parse bits from -3.14f
    Binary32 bin32 = fp32_cvt_to_binary32(value);
    printf("%f\n", value);
    printf("  -> sign = %x, exp = %d, mantissa = %x\n",
        bin32.sign, bin32.exp, bin32.mantissa
    );

    // Reproduce -3.14f from bit expression
    uint8_t sign = 1;
    int8_t exp = 1;
    uint32_t mantissa = 0x48f5c3;
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
