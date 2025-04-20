#include <stdio.h>

#include "fp32.h"

int main(void) {
    // 1000: 0x447a0000, sign=0, exp=136, mantissa=0x7a0000
    Binary32 a = fp32_cvt_to_binary32(1000);
    // 0.5: 0x3f000000, sign=0, exp=126, mantissa=0x0
    Binary32 b = fp32_cvt_to_binary32(0.5);

    // 1000.5: 0x447a2a00, sign=0, exp=136, mantissa=0x7a2000
    Binary32 c = fp32_add_binary32(a, b);
    printf("%f + %f = %f (sign: %u, exp: %u, mantissa: 0x%x)\n",
        fp32_cvt_to_float(a), fp32_cvt_to_float(b), fp32_cvt_to_float(c),
        c.sign, c.exp, c.mantissa
    );

    a = fp32_cvt_to_binary32(1000);
    b = fp32_cvt_to_binary32(1000);

    // 2000: 0x44fa0000, sign=0, exp=137, mantissa=0x7a0000
    c = fp32_add_binary32(a, b);
    printf("%f + %f = %f (sign: %u, exp: %u, mantissa: 0x%x)\n",
        fp32_cvt_to_float(a), fp32_cvt_to_float(b), fp32_cvt_to_float(c),
        c.sign, c.exp, c.mantissa
    );

    return 0;
}
