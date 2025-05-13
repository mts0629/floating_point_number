#include <stdio.h>
#include <float.h>

#include "fp32.h"

int main(void) {
    // 1000: sign=0, exp=136, mantissa=0x7a0000
    Binary32 a = fp32_to_binary32(1000);
    // 0.5: sign=0, exp=126, mantissa=0x0
    Binary32 b = fp32_to_binary32(0.5);
    // 1000 + 0.5 = 1000.5: sign=0, exp=136, mantissa=0x7a2000
    Binary32 c = fp32_add(a, b);
    printf("%f + %f = %f (sign: %u, exp: %u, mantissa: 0x%x)\n",
        fp32_to_float(a), fp32_to_float(b), fp32_to_float(c),
        c.sign, c.exp, c.mantissa
    );

    // Carry
    a = fp32_to_binary32(1000);
    b = fp32_to_binary32(1000);
    // 1000 + 1000 = 2000: sign=0, exp=137, mantissa=0x7a0000
    c = fp32_add(a, b);
    printf("%f + %f = %f (sign: %u, exp: %u, mantissa: 0x%x)\n",
        fp32_to_float(a), fp32_to_float(b), fp32_to_float(c),
        c.sign, c.exp, c.mantissa
    );

    // Negative values
    a = fp32_to_binary32(-42.195);
    b = fp32_to_binary32(42);
    c = fp32_add(a, b);
    printf("%f + %f = %f (sign: %u, exp: %u, mantissa: 0x%x)\n",
        fp32_to_float(a), fp32_to_float(b), fp32_to_float(c),
        c.sign, c.exp, c.mantissa
    );

    // Round
    a = fp32_to_binary32(8388609); // exp: 23
    b = fp32_to_binary32(0.5); // exp: -1
    c = fp32_add(a, b); // Guard bit=1, ulp=1 -> round
    printf("%f + %f = %f (sign: %u, exp: %u, mantissa: 0x%x)\n",
        fp32_to_float(a), fp32_to_float(b), fp32_to_float(c),
        c.sign, c.exp, c.mantissa
    );

    // Overflow: return +-inf
    a = fp32_to_binary32(FLT_MAX);
    b = fp32_to_binary32(1.70141183e+38);
    c = fp32_add(a, b);
    printf("%f + %f = %f (sign: %u, exp: %u, mantissa: 0x%x)\n",
        fp32_to_float(a), fp32_to_float(b), fp32_to_float(c),
        c.sign, c.exp, c.mantissa
    );

    return 0;
}
