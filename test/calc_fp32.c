#include <assert.h>
#include <float.h>
#include <stdio.h>

#include "fp32.h"

int main(void) {
    // Addition
    // 1000: sign=0, exp=136, mantissa=0x7a0000
    // 0.5: sign=0, exp=126, mantissa=0x0
    // 1000 + 0.5 = 1000.5: sign=0, exp=136, mantissa=0x7a2000
    Binary32 a = fp32_to_binary32(1000);
    Binary32 b = fp32_to_binary32(0.5);
    Binary32 c = fp32_add(a, b);
    printf("%f + %f = %f (sign: %u, exp: %u, mantissa: 0x%x)\n",
        fp32_to_float(a), fp32_to_float(b), fp32_to_float(c),
        c.sign, c.exp, c.mantissa
    );
    assert(1000.f + 0.5f == fp32_to_float(c));

    // Carry
    // 1000 + 1000 = 2000: sign=0, exp=137, mantissa=0x7a0000
    a = fp32_to_binary32(1000);
    b = fp32_to_binary32(1000);
    c = fp32_add(a, b);
    printf("%f + %f = %f (sign: %u, exp: %u, mantissa: 0x%x)\n",
        fp32_to_float(a), fp32_to_float(b), fp32_to_float(c),
        c.sign, c.exp, c.mantissa
    );
    assert(1000.f + 1000.f == fp32_to_float(c));

    // Negative values
    a = fp32_to_binary32(-42.195);
    b = fp32_to_binary32(42);
    c = fp32_add(a, b);
    printf("%f + %f = %f (sign: %u, exp: %u, mantissa: 0x%x)\n",
        fp32_to_float(a), fp32_to_float(b), fp32_to_float(c),
        c.sign, c.exp, c.mantissa
    );
    assert(-42.195f + 42.f == fp32_to_float(c));

    // Round even
    // 8388609: exp=150, 0.5: exp=126
    // Guard bit=1, ulp=1 -> round up
    a = fp32_to_binary32(8388609);
    b = fp32_to_binary32(0.5);
    c = fp32_add(a, b);
    printf("%f + %f = %f (sign: %u, exp: %u, mantissa: 0x%x)\n",
        fp32_to_float(a), fp32_to_float(b), fp32_to_float(c),
        c.sign, c.exp, c.mantissa
    );
    assert(8388609.f + 0.5f == fp32_to_float(c));

    // Overflow: return inf
    a = fp32_to_binary32(FLT_MAX);
    b = fp32_to_binary32(1.70141183e+38);
    c = fp32_add(a, b);
    printf("%f + %f = %f (sign: %u, exp: %u, mantissa: 0x%x)\n",
        fp32_to_float(a), fp32_to_float(b), fp32_to_float(c),
        c.sign, c.exp, c.mantissa
    );
    // No assertion, because an exception will occur

    // Subtract
    a = fp32_to_binary32(1.0);
    b = fp32_to_binary32(2.0);
    c = fp32_sub(a, b);
    printf("%f - %f = %f (sign: %u, exp: %u, mantissa: 0x%x)\n",
        fp32_to_float(a), fp32_to_float(b), fp32_to_float(c),
        c.sign, c.exp, c.mantissa
    );
    assert(1.0f - 2.0f == fp32_to_float(c));

    return 0;
}
