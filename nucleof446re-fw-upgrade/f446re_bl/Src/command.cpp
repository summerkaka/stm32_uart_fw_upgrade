
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
#include <vector>
#include <deque>
using namespace std;


/* Private macro -------------------------------------------------------------*/
#define UART        &huart1
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
static bool uart_listen = false;
struct Buf_t rx_buf;
static vector<char> cmd_recv;
static uint8_t parse_stat = 0;
static deque<vector<char>> packet_pool;

static uint8_t cmd_transmit[BUF_SIZE] = {0xA5, 0x5A};
static uint8_t* const cmd_answer = cmd_transmit + 2;
// static uint8_t widx = 0;
static uint8_t RxByte = 0;

bool test_mode = false;
extern uint32_t update_request;

//flash w/r related
static uint32_t                     page_err = 0;
static FLASH_EraseInitTypeDef       erase_init;
static stProgramFlash               flash;


/* Code begin ----------------------------------------------------------------*/
static void Console_Listen(void)
{
    uart_listen = HAL_UART_Receive_IT(UART, &RxByte, 1) == HAL_OK;
}

static uint16_t crc_calc(vector<char> &msg)
{
    return 0;
}

static bool crc_check(vector<char> &msg)
{
    return true;
}

static uint8_t answer(uint8_t num, ...)
{
    va_list valist;
    int8_t i = 0;
    int data;

    va_start(valist, num);
    for (; i < num; i++) {
        data = va_arg(valist, int);
        cmd_answer[i] = data;
    }
    va_end(valist);
    WriteShortH(&cmd_answer[i], 0x0000); // write crc
    i += 2;
    WriteShortH(&cmd_answer[i], 0x0d0a); // write frame-end-symbol
    i += 2;
    i= HAL_UART_Transmit(UART, cmd_transmit, i+2, 0xffff);
    return i;
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
     if (rx_buf.r_idx - rx_buf.w_idx == 1 || (rx_buf.r_idx == 0 && rx_buf.w_idx == BUF_SIZE-1)) { // buffer full
        return;
    }
    rx_buf.buf[rx_buf.w_idx++] = RxByte;
    rx_buf.w_idx = rx_buf.w_idx >= BUF_SIZE ? 0 : rx_buf.w_idx;

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


void CommandParse(void)
{
    while (rx_buf.r_idx != rx_buf.w_idx) {  // not empty

        if (parse_stat == 0) {
            if (rx_buf.buf[rx_buf.r_idx] == 0xa5) {
                parse_stat = 1;
            }
        } else if (parse_stat == 1) {
            parse_stat = rx_buf.buf[rx_buf.r_idx] == 0x5a ? 2 : 0;
        } else if (parse_stat == 2) {
            cmd_recv.push_back(rx_buf.buf[rx_buf.r_idx]);
            if (cmd_recv.size() > IDX_LEN && cmd_recv.size() == 2 + cmd_recv[IDX_LEN] + 4) { // cmd(1) + dlc(1) + payload + crc(2) + eof(2)
                if (cmd_recv.at(cmd_recv.size()-2) == 0x0d && cmd_recv.at(cmd_recv.size()-1) == 0x0a) {
                    packet_pool.push_back(cmd_recv);
                }
                parse_stat = 0;
                cmd_recv.clear();
            }
        }
        rx_buf.r_idx = ++rx_buf.r_idx >= BUF_SIZE ? 0 : rx_buf.r_idx;
    }
}


void CommandHandler(void)
{
    uint8_t ret=HAL_ERROR, cmd_ans, i=0, length=0;

    if (uart_listen == false)
        Console_Listen();

    if (packet_pool.empty())
        return;

    for (vector<char> cmd : packet_pool) {

        if (crc_check(cmd) == false)
            continue;

        cmd_ans = cmd[IDX_CMD] | 0x80;

        switch (cmd[IDX_CMD]) {
        case CMD_PING:
            answer(3, cmd_ans, 0x01, 0x01);
            break;
        case CMD_FWUPDATE_PING:
            answer(3, cmd_ans, 0x01, 0xa5);
            break;
        case CMD_ASK_APPAREA:
            answer(10, cmd_ans, 0x08, FLASH_USER_START_ADDR, FLASH_USER_START_ADDR>>8, FLASH_USER_START_ADDR>>16, FLASH_USER_START_ADDR>>24,
                                  FLASH_USER_END_ADDR, FLASH_USER_END_ADDR>>8, FLASH_USER_END_ADDR>>16, FLASH_USER_END_ADDR>>24);
            break;
        case CMD_PROGRAM_START:
            if ((ret = HAL_FLASH_Unlock()) == HAL_OK) {
                erase_init.TypeErase   = FLASH_TYPEERASE_SECTORS;
                erase_init.Sector = FLASH_SECTOR_2;
                erase_init.NbSectors = 6;
                ret = HAL_FLASHEx_Erase(&erase_init, &page_err);
            }
            answer(3, cmd_ans, 0x01, ret);
            break;
        case CMD_DLD:
            flash.address = GetWordL(&cmd[IDX_DATA]);
            flash.length = GetWordL(&cmd[IDX_DATA+4]);
            if (flash.address < FLASH_USER_START_ADDR || flash.address + flash.length > FLASH_USER_END_ADDR)
                ret = 0x01;
            else
                ret = 0x00;
            answer(3, cmd_ans, 0x01, ret);
            break;
        case CMD_SEND_DATA:
            i = 0;
            length = cmd[IDX_LEN];
            if (length != flash.length) {
                answer(3, cmd_ans, 0x01, 0x01);
            }
            do {
                if (length - i >= 4) {
                    flash.data = GetWordL(&cmd[IDX_DATA+i]);
                    ret = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, flash.address+i, flash.data);
                    i += 4;
                } else if (length - i >= 2) {
                    flash.data = GetShortL(&cmd[IDX_DATA+i]);
                    ret = HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, flash.address + i, flash.data);
                    i += 2;
                } else if (length - i == 1) {
                    flash.data = cmd[IDX_DATA];
                    flash.data |= 0xff00;
                    ret = HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, flash.address + i, flash.data);
                    i++;
                }
            } while (length > i && ret == HAL_OK);
            if (ret != HAL_OK) {
                HAL_FLASH_Lock();
            }
            flash.address = (ret == HAL_OK) ? flash.address + i : flash.address;
            answer(3, cmd_ans, 1, ret);
            break;
        case CMD_RESET:
            answer(3, cmd_ans, 0x01, 0x00);
            NVIC_SystemReset();
            break;
        case CMD_WRITECRC:
            flash.data = GetWordL(&cmd[IDX_DATA]);
            flash.address = FLASH_USER_END_ADDR - 4 + 1;                            // 0x0803ffff -4+1
            ret = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, flash.address, flash.data);
            answer(3, cmd_ans, 0x01, ret);
            break;
        case CMD_PROGRAM_END:
            ret = HAL_FLASH_Lock();
            answer(3, cmd_ans, 0x01, ret);
            break;
        case CMD_JUMPTOAPP:
            if (((*(__IO uint32_t *)APP_ADDRESS) & 0x2FFE0000) == 0x20000000) {
                HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);
                answer(3, cmd_ans, 0x01, 0);
                update_request = 0xaaaaaaaa;
                __set_MSP(*(__IO uint32_t*)APP_ADDRESS);
                ((void(*)(void))(*(__IO uint32_t*)(APP_ADDRESS + 4)))();
            }else {
                answer(3, cmd_ans, 0x01, 0x01);
            }
            break;
        default :
            answer(3, 0xff, 0x01, 0xff);
            break;
        }
        packet_pool.pop_front();
    }

    if (uart_listen == false)
        Console_Listen();
}
