#include "mystring.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
int ftoaEngine(float value, char *buffer, int presc);
char *uint16ToString(uint16_t n, char *buffer)
{
    uint8_t d4, d3, d2, d1, q, d0;

    d1 = (n >> 4) & 0xF;
    d2 = (n >> 8) & 0xF;
    d3 = (n >> 12) & 0xF;

    d0 = 6 * (d3 + d2 + d1) + (n & 0xF);
    q = (d0 * 0xCD) >> 11;
    d0 = d0 - 10 * q;

    d1 = q + 9 * d3 + 5 * d2 + d1;
    q = (d1 * 0xCD) >> 11;
    d1 = d1 - 10 * q;

    d2 = q + 2 * d2;
    q = (d2 * 0x1A) >> 8;
    d2 = d2 - 10 * q;

    d3 = q + 4 * d3;
    d4 = (d3 * 0x1A) >> 8;
    d3 = d3 - 10 * d4;

    char *ptr = buffer;
    uint8_t flag = 0;
    if (d4)
    {
        *ptr++ = (d4 + '0');
        flag = 1;
    }
    if (d3 || flag)
    {
        *ptr++ = (d3 + '0');
        flag = 1;
    }
    if (d2 || flag)
    {
        *ptr++ = (d2 + '0');
        flag = 1;
    }
    if (d1 || flag)
    {
        *ptr++ = (d1 + '0');
        flag = 1;
    }
    *ptr++ = (d0 + '0');
    *ptr = 0;

    while (buffer[0] == '0')
        ++buffer;
    return buffer;
}

typedef struct
{
    uint32_t quot;
    uint8_t rem;
}divmod10_t;
const uint32_t table2[] =
{
    0xF0BDC21A,
    0x3DA137D5,
    0x9DC5ADA8,
    0x2863C1F5,
    0x6765C793,
    0x1A784379,
    0x43C33C19,
    0xAD78EBC5,
    0x2C68AF0B,
    0x71AFD498,
    0x1D1A94A2,
    0x4A817C80,
    0xBEBC2000,
    0x30D40000,
    0x7D000000,
    0x20000000,
    0x51EB851E,
    0xD1B71758,
    0x35AFE535,
    0x89705F41,
    0x232F3302,
    0x5A126E1A,
    0xE69594BE,
    0x3B07929F,
    0x971DA050,
    0x26AF8533,
    0x63090312,
    0xFD87B5F2,
    0x40E75996,
    0xA6274BBD,
    0x2A890926,
    0x6CE3EE76
};
inline static divmod10_t divmodu10(uint32_t n)
{
    divmod10_t res;
    res.quot = n >> 1;
    res.quot += res.quot >> 1;
    res.quot += res.quot >> 4;
    res.quot += res.quot >> 8;
    res.quot += res.quot >> 16;
    uint32_t qq = res.quot;
    res.quot >>= 3;
    res.rem = (uint8_t)(n - ((res.quot << 1) + (qq & ~7ul)));
    if(res.rem > 9)
    {
        res.rem -= 10;
        res.quot++;
    }
    return res;
}

inline static char * utoa_fast_div(uint32_t value, char *bufferEnd)
{
    *bufferEnd = 0;
    divmod10_t res;
    res.quot = value;
    do
    {
        res = divmodu10(res.quot);
        *--bufferEnd = res.rem + '0';
    }
    while (res.quot);
    return bufferEnd;
}

static inline uint32_t MantissaMul(uint32_t mantissa, uint32_t multiplier)
{
    mantissa <<= 8;
    return ((uint64_t)mantissa * multiplier) >> 32;
}


int ftoaEngine(float value, char *buffer, int presc)
{
    union union_type
    {
        float f;
        uint32_t u32;
    }cnvt;
    cnvt.f=value;
    uint32_t uvalue = cnvt.u32;
    uint16_t uvalue_hi16 = (uint16_t) (uvalue >> 16);
    uint8_t exponent = (uint8_t) (uvalue_hi16 >> 7);
    uint32_t fraction = (uvalue & 0x00ffffff) | 0x00800000;
    char *ptr = buffer;

    if(uvalue & 0x80000000)
        *ptr++ = '-';
    else
        *ptr++ = '+';

    if(exponent == 0) // don't care about a subnormals
    {
        ptr[0] = '0';
        ptr[1] = 0;
        return 0xff;
    }

    if(exponent == 0xff)
    {
        if(fraction & 0x007fffff)
        {
            ptr[0] = 'n';
            ptr[1] = 'a';
            ptr[2] = 'n';
            ptr[3] = 0;
        }
        else
        {
            ptr[0] = 'i';
            ptr[1] = 'n';
            ptr[2] = 'f';
            ptr[3] = 0;
        }
        return 0xff;
    }

    *ptr++ = '0';

    int exp10 = ((((exponent>>3))*77+63)>>5) - 38;
    uint32_t t = MantissaMul(fraction, table2[exponent / 8]) + 1;
    uint_fast8_t shift = 7 - (exponent & 7);
    t >>= shift;

    uint8_t digit = t >> 24;
    digit >>= 4;
    while(digit == 0)
    {
        t &= 0x0fffffff;
        //t <<= 1;
        //t += t << 2;
        t *= 10;
        digit = (uint8_t)(t >> 28);
        exp10--;
    }

    for(uint_fast8_t i = presc+1; i > 0; i--)
    {
        digit = (uint8_t)(t >> 28);
        *ptr++ = digit + '0';
        t &= 0x0fffffff;
        t *= 10;
        //t <<= 1;
        //t += t << 2;
    }
    // roundup
    if(buffer[presc+2] >= '5')
        buffer[presc+1]++;
    ptr[-1] = 0;
    ptr-=2;
    for(uint_fast8_t i = presc + 1; i > 1; i--)
    {
        if(buffer[i] > '9')
        {
            buffer[i]-=10;
            buffer[i-1]++;
        }
    }
    while(ptr[0] == '0')
    {
        *ptr-- = 0;
    }
    return exp10;
}

char * floatToString(float value, char *result)
{
    uint8_t precision = 4;
    char *out_ptr = result;
    const int bufferSize = 10;
    char buffer[bufferSize+1];
// получили цифры
    int exp10 = ftoaEngine(value, buffer, precision);
// если там inf или nan - выводим как есть.
   if(exp10 == 0xff)
    {
        uint8_t digits = (uint8_t)strlen(buffer);
        uint_fast8_t i = 0;
        if(buffer[0] == '+')
            i = 1;

        for(; i < digits; i++)
            *out_ptr++ = buffer[i];
        *out_ptr = 0;
        return result;
    }
// если был перенос старшей цифры при округлении
    char *str_begin = &buffer[2];
    if(buffer[1] != '0')
    {
        exp10++;
        str_begin--;
    }
// количество значащих цифр <= precision
    uint_fast8_t digits = (uint_fast8_t)strlen(str_begin);

    uint_fast8_t intDigits=0, leadingZeros = 0;
    if((uint8_t)abs(exp10) >= precision)
    {
        intDigits = 1;
    }else if(exp10 >= 0)
    {
        intDigits = exp10+1;
        exp10 = 0;
    }else
    {
        intDigits = 0;
        leadingZeros = -exp10 - 1;
        exp10 = 0;
    }
    if(buffer[0]=='-')
        *out_ptr++='-';
    uint_fast8_t fractDigits = digits > intDigits ? digits - intDigits : 0;
    //FieldFill(fractPartSize + intPartSize, IOS::right);
 // целая часть
    if(intDigits)
    {
        uint_fast8_t count = intDigits > digits ? digits : intDigits;
        while(count--)
            *out_ptr++ = *str_begin++;
        int_fast8_t tralingZeros = intDigits - digits;
        while(tralingZeros-- > 0)
            *out_ptr++ ='0';
    }
    else
        *out_ptr++ = '0';
// дробная часть
    if(fractDigits)
    {
        *out_ptr++ = '.';
        while(leadingZeros--)
            *out_ptr++ = '0';
        while(fractDigits--)
            *out_ptr++ = *str_begin++;
    }
    // десятичная экспонента
    if(exp10 != 0)
    {
        *out_ptr++ = 'e';
        uint_fast8_t upow10;
        if(exp10 < 0)
        {
            *out_ptr++ = '-';
            upow10 = -exp10;
        }
        else
        {
            *out_ptr++ = '+';
            upow10 = exp10;
        }
        char *powPtr = utoa_fast_div(upow10, buffer + bufferSize);
        while(powPtr < buffer + bufferSize)
        {
            *out_ptr++ = *powPtr++;
        }
    }
    *out_ptr = 0;
    return result;
}
