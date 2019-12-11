
/**
  ******************************************************************************
  * @file    Project/src/command.c
  * @author  xiatian
  * @version V0.00
  * @date    2019-12-03
  * @brief   to support usart command
  ******************************************************************************
  */


/* Includes ------------------------------------------------------------------*/
#include "include.h"
#include "stdarg.h"


/* Private macro -------------------------------------------------------------*/
#define UART        &huart1
#define BUF_SIZE    64
#define IDX_CMD     0
#define IDX_LEN     1
#define IDX_DATA    2


/* Private typedef -----------------------------------------------------------*/
typedef enum {
    kConsoleIdle = 0,
    kConsoleReady,
    kConsoleRecv,
    kConsoleCplt
} UartSts_t;

typedef struct {
    uint32_t address;
    uint8_t length;
    uint64_t data;
} stProgramFlash;


/* Private function prototypes -----------------------------------------------*/


/* Private variables ---------------------------------------------------------*/
bool uart_listen = false;
static UartSts_t uart_sts = kConsoleIdle;
uint8_t cmd_recv[BUF_SIZE] = {0};
uint8_t cmd_exec[BUF_SIZE] = {0};
uint8_t cmd_transmit[BUF_SIZE] = {0xA5, 0x5A};
static uint8_t* const cmd_answer = cmd_transmit + 2;
static uint8_t widx = 0;
static uint8_t RxByte = 0;

//flash w/r related
static uint32_t                     page_err = 0;
static FLASH_EraseInitTypeDef       erase_init;
static stProgramFlash               flash;


/* Code begin ----------------------------------------------------------------*/
static void Console_Listen(void)
{
    uart_listen = HAL_UART_Receive_IT(UART, &RxByte, 1) == HAL_OK;
}

static uint16_t crc_calc(uint8_t *msg)
{
    return 0;
}

static bool crc_check(uint8_t *msg)
{
    return true;
}

static uint8_t answer(uint8_t num, ...)
{
    va_list valist;
    int8_t i = 0, widx = 0;
    dtype_t dtype = k_u8;
    uint64_t data;
    union_data32_t data32;
    union_data64_t data64;

    va_start(valist, num);

    for (; i < num; i++) {
        if (i % 2 == 0) {
            dtype = va_arg(valist, dtype_t);
        } else {
            switch (dtype) {
            case k_u8:
                cmd_answer[widx++] = va_arg(valist, int);
                break;
            case k_u16:
                data = va_arg(valist, int);
                WriteShortH(cmd_answer+widx, data);
                widx += 2;
                break;
            case k_u32:
                data = va_arg(valist, int);
                WriteWordH(cmd_answer+widx, data);
                widx += 4;
                break;
            case k_u64:
                data = va_arg(valist, uint64_t);
                WriteDWordH(cmd_answer+widx, data);
                widx += 8;
                break;
            case k_f32:
                data32.d_float = va_arg(valist, double);
                WriteWordH(cmd_answer+widx, data32.d_uint32);
                widx += 4;
                break;
            case k_f64:
                data64.d_double = va_arg(valist, double);
                WriteDWordH(cmd_answer+widx, data64.d_uint64);
                widx += 8;
                break;
            default : break;
            }
        }
    }
    va_end(valist);
    WriteShortH(&cmd_answer[widx], 0x0000); // write crc
    widx += 2;
    WriteShortH(&cmd_answer[widx], 0x0d0a); // write frame-end-symbol
    widx += 2;
    i = HAL_UART_Transmit(UART, cmd_transmit, widx + 2, 0x1000);
    memset(cmd_answer, 0, widx);
    return i;
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    switch (uart_sts) {
    case kConsoleIdle:
        if (RxByte == 0xA5)
            uart_sts = kConsoleReady;
        break;
    case kConsoleReady:
        if (RxByte == 0x5A)
            uart_sts = kConsoleRecv;
        else
            uart_sts = kConsoleIdle;
        break;
    case kConsoleRecv:
        cmd_recv[widx++] = RxByte;
        if (widx > IDX_LEN && widx >= cmd_recv[IDX_LEN] + 2) {
            uart_sts = kConsoleCplt;
            widx = 0;
        } else if (widx >= BUF_SIZE) {
            cmd_recv[IDX_LEN] = 0;
            uart_sts = kConsoleIdle;
            widx = 0;
        }
        break;
    case kConsoleCplt:      // 'lock' status. in main loop it handle the cmd and unlock the sts to 'idle'
        break;
    default : break;
    }

    Console_Listen();
}

// static void transmit_answer(void)
// {
//     uint16_t crc = crc_calc(cmd_answer);
//     uint8_t payload_len = IDX_DATA + cmd_answer[IDX_LEN];  // length exclude crc, start-flag, end-flag

//     if (payload_len >= BUF_SIZE - 6)
//         goto end;

//     WriteShortH(&cmd_answer[payload_len], crc);
//     WriteShortH(&cmd_answer[payload_len+2], 0x0a0d);
//     HAL_UART_Transmit(UART, cmd_transmit, payload_len + 6, 0xffff);
// end:
//     memset(cmd_answer, 0, payload_len+4);
// }

void CommandHandler(void)
{
    uint8_t i = 0, ret = HAL_ERROR, cmd_ans;
    uint8_t length;

    if (uart_sts == kConsoleCplt) {
        HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
        memcpy(cmd_exec, cmd_recv, cmd_recv[IDX_LEN]+2);
        cmd_recv[IDX_LEN] = 0;
        uart_sts = kConsoleIdle;

        if (crc_check(cmd_exec) == false)
            goto FINISH;

        cmd_ans = cmd_exec[IDX_CMD] | 0x80;

        switch (cmd_exec[IDX_CMD]) {
        case CMD_PING:
            answer(6, k_u8, cmd_ans, k_u8, 0x01, k_u8, 0x01);
            break;
        case CMD_FWUPDATE_PING:
            answer(6, k_u8, cmd_ans, k_u8, 0x01, k_u8, 0xa5);
            break;
        case CMD_ASK_APPAREA:
            answer(8, k_u8, cmd_ans, k_u8, 0x08, k_u32, FLASH_USER_START_ADDR, k_u32, FLASH_USER_END_ADDR);
            break;
        case CMD_PROGRAM_START:
            if ((ret = HAL_FLASH_Unlock()) == HAL_OK) {
                erase_init.TypeErase   = FLASH_TYPEERASE_SECTORS;
                erase_init.Sector = FLASH_SECTOR_2;
                erase_init.NbSectors = 6;   
                ret = HAL_FLASHEx_Erase(&erase_init, &page_err);
            }
            answer(6, k_u8, cmd_ans, k_u8, 0x01, k_u8, ret);
            break;
        case CMD_DLD:
            flash.address = GetWordL(&cmd_exec[IDX_DATA]);
            flash.length = GetWordL(&cmd_exec[IDX_DATA+4]);
            if (flash.address < FLASH_USER_START_ADDR || flash.address + flash.length > FLASH_USER_END_ADDR)
                ret = 0x01;
            else
                ret = 0x00;
            answer(6, k_u8, cmd_ans, k_u8, 0x01, k_u8, ret);
            break;
        case CMD_SEND_DATA:
            i = 0;
            length = cmd_exec[IDX_LEN];
            if (length != flash.length) {
                answer(6, k_u8, cmd_ans, k_u8, 0x01, k_u8, 0x01);
            }
            do {
                if (length - i >= 4) {
                    flash.data = GetWordL(&cmd_exec[IDX_DATA+i]);
                    ret = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, flash.address+i, flash.data);
                    i += 4;
                } else if (length - i >= 2) {
                    flash.data = GetShortL(&cmd_exec[IDX_DATA+i]);
                    ret = HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, flash.address + i, flash.data);
                    i += 2;
                } else if (length - i == 1) {
                    flash.data = cmd_exec[IDX_DATA];
                    flash.data |= 0xff00;
                    ret = HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, flash.address + i, flash.data);
                    i++;
                }
            } while (length > i && ret == HAL_OK);
            if (ret != HAL_OK) {
                HAL_FLASH_Lock();
            }
            flash.address = (ret == HAL_OK) ? flash.address + i : flash.address;
            answer(6, k_u8, cmd_ans, k_u8, 1, k_u8, ret);
            break;
        case CMD_RESET:
            answer(6, k_u8, cmd_ans, k_u8, 0x01, k_u8, 0x00);
            NVIC_SystemReset();
            break;
        case CMD_WRITECRC:
            flash.data = GetWordL(&cmd_exec[IDX_DATA]);
            flash.address = FLASH_USER_END_ADDR - 4 + 1;                            // 0x0803ffff -4+1
            ret = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, flash.address, flash.data);
            answer(6, k_u8, cmd_ans, k_u8, 0x01, k_u8, ret);
            break;
        case CMD_PROGRAM_END:
            ret = HAL_FLASH_Lock();
            answer(6, k_u8, cmd_ans, k_u8, 0x01, k_u8, ret);
            break;
        case CMD_JUMPTOAPP:
            if (((*(__IO uint32_t *)APP_ADDRESS) & 0x2FFE0000) == 0x20000000) {
                HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);
                answer(6, k_u8, cmd_ans, k_u8, 0x01, k_u8, 0);
                update_request = 0xaaaaaaaa;
                __set_MSP(*(__IO uint32_t*)APP_ADDRESS);
                ((void(*)(void))(*(__IO uint32_t*)(APP_ADDRESS + 4)))();
            }else {
                answer(6, k_u8, cmd_ans, k_u8, 0x01, k_u8, 0x01);
            }
            break;
        default : break;
        }

FINISH:
        uart_sts = kConsoleIdle;
    }

    if (uart_listen == false)
        Console_Listen();
}
