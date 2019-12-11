/**
  ******************************************************************************
  * @file    Project/inc/command.h
  * @author  xiatian
  * @version V0.00
  * @date    2019-12-03
  * @brief   command.h
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __COMMAND_H
#define __COMMAND_H
//#ifdef __cplusplus
// extern "C" {
//#endif


/* Includes ------------------------------------------------------------------*/


/* Exported macro ------------------------------------------------------------*/
// big endian
#define GetShortH(ptr)      (uint32_t)(*((uint8_t *)(ptr)    )) << 8  |             \
                            (uint32_t)(*((uint8_t *)(ptr) + 1))
// little endian
#define GetShortL(ptr)      (uint32_t)(*((uint8_t *)(ptr) + 1)) << 8  |             \
                            (uint32_t)(*((uint8_t *)(ptr)    ))
// big endian
#define GetWordH(ptr)       (uint32_t)(*((uint8_t *)(ptr)    )) << 24 |             \
                            (uint32_t)(*((uint8_t *)(ptr) + 1)) << 16 |             \
                            (uint32_t)(*((uint8_t *)(ptr) + 2)) << 8  |             \
                            (uint32_t)(*((uint8_t *)(ptr) + 3))
// little endian
#define GetWordL(ptr)       (uint32_t)(*((uint8_t *)(ptr) + 3)) << 24 |             \
                            (uint32_t)(*((uint8_t *)(ptr) + 2)) << 16 |             \
                            (uint32_t)(*((uint8_t *)(ptr) + 1)) << 8  |             \
                            (uint32_t)(*((uint8_t *)(ptr)    ))
// big endian
#define GetDWordH(ptr)      (uint64_t)(*((uint8_t *)(ptr)    )) << 56 |         \
                            (uint64_t)(*((uint8_t *)(ptr) + 1)) << 48 |         \
                            (uint64_t)(*((uint8_t *)(ptr) + 2)) << 40 |         \
                            (uint64_t)(*((uint8_t *)(ptr) + 3)) << 32 |         \
                            (uint64_t)(*((uint8_t *)(ptr) + 4)) << 24 |         \
                            (uint64_t)(*((uint8_t *)(ptr) + 5)) << 16 |         \
                            (uint64_t)(*((uint8_t *)(ptr) + 6)) << 8  |         \
                            (uint64_t)(*((uint8_t *)(ptr) + 7))
// little endian
#define GetDWordL(ptr)      (uint64_t)(*((uint8_t *)(ptr) + 7)) << 56 |         \
                            (uint64_t)(*((uint8_t *)(ptr) + 6)) << 48 |         \
                            (uint64_t)(*((uint8_t *)(ptr) + 5)) << 40 |         \
                            (uint64_t)(*((uint8_t *)(ptr) + 4)) << 32 |         \
                            (uint64_t)(*((uint8_t *)(ptr) + 3)) << 24 |         \
                            (uint64_t)(*((uint8_t *)(ptr) + 2)) << 16 |         \
                            (uint64_t)(*((uint8_t *)(ptr) + 1)) << 8  |         \
                            (uint64_t)(*((uint8_t *)(ptr)    ))

// big endian
#define WriteDWordH(ptr, value)     do {*(uint8_t *)(ptr)       = (value) >> 56;         \
                                        *(uint8_t *)((ptr) + 1) = (value) >> 48 & 0xff;  \
                                        *(uint8_t *)((ptr) + 2) = (value) >> 40 & 0xff;  \
                                        *(uint8_t *)((ptr) + 3) = (value) >> 32 & 0xff;  \
                                        *(uint8_t *)((ptr) + 4) = (value) >> 24 & 0xff;  \
                                        *(uint8_t *)((ptr) + 5) = (value) >> 16 & 0xff;  \
                                        *(uint8_t *)((ptr) + 6) = (value) >> 8  & 0xff;  \
                                        *(uint8_t *)((ptr) + 7) = (value)       & 0xff;  \
                                    } while (0)

// little endian
#define WriteDWordL(ptr, value)     do {*(uint8_t *)(ptr)       = (value)       & 0xff;  \
                                        *(uint8_t *)((ptr) + 1) = (value) >> 8  & 0xff;  \
                                        *(uint8_t *)((ptr) + 2) = (value) >> 16 & 0xff;  \
                                        *(uint8_t *)((ptr) + 3) = (value) >> 24 & 0xff;  \
                                        *(uint8_t *)((ptr) + 4) = (value) >> 32 & 0xff;  \
                                        *(uint8_t *)((ptr) + 5) = (value) >> 40 & 0xff;  \
                                        *(uint8_t *)((ptr) + 6) = (value) >> 48 & 0xff;  \
                                        *(uint8_t *)((ptr) + 7) = (value) >> 56 & 0xff;  \
                                    } while (0)

// big endian
#define WriteWordH(ptr, value)      do {*(uint8_t *)(ptr)       = (value) >> 24;        \
                                        *(uint8_t *)((ptr) + 1) = (value) >> 16 & 0xff; \
                                        *(uint8_t *)((ptr) + 2) = (value) >> 8 & 0xff;  \
                                        *(uint8_t *)((ptr) + 3) = (value) & 0xff;       \
                                    } while (0)
// little endian
#define WriteWordL(ptr, value)      do {*(uint8_t *)((ptr) + 3) = (value) >> 24;        \
                                        *(uint8_t *)((ptr) + 2) = (value) >> 16 & 0xff; \
                                        *(uint8_t *)((ptr) + 1) = (value) >> 8 & 0xff;  \
                                        *(uint8_t *)(ptr)       = (value) & 0xff;       \
                                    } while (0)
// big endian
#define WriteShortH(ptr, value)     do {*(uint8_t *)(ptr)       = (value) >> 8;   \
                                        *(uint8_t *)((ptr) + 1) = value & 0xff; \
                                    } while (0)
// little endian
#define WriteShortL(ptr, value)     do {*(uint8_t *)((ptr) + 1) = (uint8_t)((value) >> 8);  \
                                        *(uint8_t *)(ptr)       = (uint8_t)(value);         \
                                    } while (0)



/* Exported types ------------------------------------------------------------*/
typedef enum {
    k_u8 = 1,
    k_u16,
    k_u32,
    k_u64,
    k_f32,
    k_f64
} dtype_t;

typedef union {
    uint32_t d_uint32;
    float    d_float;
} union_data32_t;

typedef union {
    uint64_t d_uint64;
    double   d_double;
} union_data64_t;


/* Exported constants --------------------------------------------------------*/
#define CMD_PING            0x00
#define CMD_FWUPDATE_PING   0x70
#define CMD_DLD             0x71
#define CMD_SEND_DATA       0x72
#define CMD_RESET           0x73
#define CMD_BIOS_PING       0x74
#define CMD_ASK_APPAREA     0x7A
#define CMD_JUMPTOAPP       0x7B
#define CMD_JUMPTOBL        0x7C
#define CMD_WRITECRC        0x7D
#define CMD_PROGRAM_START   0x7E
#define CMD_PROGRAM_END     0x7F


/* Exported variables ------------------------------------------------------- */
extern float fw_version;
extern uint32_t update_request;


/* Exported functions ------------------------------------------------------- */
void CommandHandler(void);



//#ifdef __cplusplus
//}
//#endif

#endif /*__CONSOLE_H */
