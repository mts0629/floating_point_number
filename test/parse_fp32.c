#include <assert.h>
#include <stdio.h>

#include "fp32.h"

int main(void) {
    // Get binary32 bits from -3.14f
    float f = -3.14f;
    Fp32 bin32 = fp32_from_float(f);

    printf("%f: sign = %x, exp = %u, mantissa = 0x%x\n", f, bin32.sign,
           bin32.exp, bin32.mantissa);
    assert(-3.14f == fp32_to_float(bin32));

    // Get 42.195f from binary32 bits
    // 42.195
    // -> 0b'1010 10.00 1100 0111 1010 1110 (= 42.1949996948242)
    // -> 0b'1.010 1000 1100 0111 1010 1110 * 2^5
    // therefore:
    // - sign = 0
    // - exp = 132 = 5 + 127 in excess (/biased) notation
    // - mantissa = 0b'010 1000 1100 0111 1010 1110 = 0x28c7ae
    uint8_t sign = 0;
    uint8_t exp = 5 + 127;
    uint32_t mantissa = 0x28c7ae;
    f = fp32_to_float((Fp32){sign, exp, mantissa});

    printf("%f: sign = %x, exp = %u, mantissa = 0x%x\n", f, sign, exp,
           mantissa);

    return 0;
}
