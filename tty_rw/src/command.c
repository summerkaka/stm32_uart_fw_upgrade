#include <stdarg.h>
#include <stdint.h>
#include <sys/types.h>
#include "command.h"

int8_t write_command_h(uint8_t *pbuf, uint8_t argc, ...)
{
    va_list valist;
    int8_t i = 0, widx = 0;
    dtype_t dtype = k_u8;
    uint64_t data;
    union_data32_t data32;
    union_data64_t data64;

    va_start(valist, argc);

    WriteShortH(pbuf, 0xa55a);
    widx += 2;

    for (; i < argc; i++) {
        if (i % 2 == 0) {
            dtype = va_arg(valist, dtype_t);
        } else {
            switch (dtype) {
            case k_u8:
                pbuf[widx++] = va_arg(valist, int);
                break;
            case k_u16:
                data = va_arg(valist, int);
                WriteShortH(pbuf+widx, data);
                widx += 2;
                break;
            case k_u32:
                data = va_arg(valist, int);
                WriteWordH(pbuf+widx, data);
                widx += 4;
                break;
            case k_u64:
                data = va_arg(valist, uint64_t);
                WriteDWordH(pbuf+widx, data);
                widx += 8;
                break;
            case k_f32:
                data32.d_float = va_arg(valist, double);
                WriteWordH(pbuf+widx, data32.d_uint32);
                widx += 4;
                break;
            case k_f64:
                data64.d_double = va_arg(valist, double);
                WriteDWordH(pbuf+widx, data64.d_uint64);
                widx += 8;
                break;
            default : break;
            }
        }
    }
    va_end(valist);
    WriteShortH(&pbuf[widx], 0x0000); // write crc
    widx += 2;
    WriteShortH(&pbuf[widx], 0x0d0a); // write frame-end-symbol
    widx += 2;
    return widx;
}

int8_t write_command_l(uint8_t *pbuf, uint8_t argc, ...)
{
    va_list valist;
    int8_t i = 0, widx = 0;
    dtype_t dtype = k_u8;
    uint64_t data;
    union_data32_t data32;
    union_data64_t data64;

    va_start(valist, argc);

    WriteShortH(pbuf, 0xa55a);
    widx += 2;

    for (; i < argc; i++) {
        if (i % 2 == 0) {
            dtype = va_arg(valist, dtype_t);
        } else {
            switch (dtype) {
            case k_u8:
                pbuf[widx++] = va_arg(valist, int);
                break;
            case k_u16:
                data = va_arg(valist, int);
                WriteShortL(pbuf+widx, data);
                widx += 2;
                break;
            case k_u32:
                data = va_arg(valist, int);
                WriteWordL(pbuf+widx, data);
                widx += 4;
                break;
            case k_u64:
                data = va_arg(valist, uint64_t);
                WriteDWordL(pbuf+widx, data);
                widx += 8;
                break;
            case k_f32:
                data32.d_float = va_arg(valist, double);
                WriteWordL(pbuf+widx, data32.d_uint32);
                widx += 4;
                break;
            case k_f64:
                data64.d_double = va_arg(valist, double);
                WriteDWordL(pbuf+widx, data64.d_uint64);
                widx += 8;
                break;
            default : break;
            }
        }
    }
    va_end(valist);
    WriteShortH(&pbuf[widx], 0x0000); // write crc
    widx += 2;
    WriteShortH(&pbuf[widx], 0x0d0a); // write frame-end-symbol
    widx += 2;
    return widx;
}


