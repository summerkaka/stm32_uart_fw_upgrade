
#include <fcntl.h> 
#include <getopt.h>  /* File control definitions */
#include <libgen.h>  /* for basename() */
#include <sys/select.h>
#include <sys/types.h>
#include <stdio.h>   /* Standard input/output definitions */
#include <string.h>  /* String function definitions */
#include <stdint.h>  /* for uintxx_t typedef */
#include <stdlib.h>  /* for exit(FAILURE) */
#include <termios.h> /* POSIX terminal control definitions */
#include <unistd.h>  /* read,write,sleep */

#include "command.h"
#include "tty_ops.h"

#define BUFSIZE 128

uint8_t wbuf[BUFSIZE] = {0xa5, 0x5a, 0};
uint8_t rbuf[BUFSIZE] = {0,};
uint8_t buffer[BUFSIZE] = {0,};
int fd_tty = -1;

struct option long_options[] = {
    { "help",   no_argument,        0, 'h' },
    { "file",   required_argument,  0, 'f' },
};

static void usage_printf(char *prg)
{
    fprintf(stderr,
            "Usage: %s [<can-interface>] [Options] <can-msg>\n"
            "<can-msg> can consist of up to 8 bytes given as a space separated list\n"
            "Options:\n"
            " -d, --netdev name like ifconfig -a shows\n"
            " -f  --hex file    hex file path\n"
            " -h, --help        this help\n",
            prg);
}

void error_handler(uint8_t cmd)
{
    printf("error at cmd: %d\n", cmd);
    exit(EXIT_FAILURE);
}

// standard command-response flow
void base_cmd_cycle(uint8_t cmd_num, uint16_t time_out)
{
    int8_t ret;

    ret = write_command_l(wbuf, 4, k_u8, cmd_num, k_u8 ,0x00);
    if (tty_send_command(fd_tty, wbuf, ret, 1000) <= 0)
        error_handler(cmd_num);
    else {
        ret = tty_get_response(fd_tty, rbuf, 1, time_out);
        if (ret < 0 || rbuf[2+IDX_CMD] != (cmd_num | 0x80) || rbuf[2+IDX_DATA] != 0)
            error_handler(cmd_num | 0x80);
    }
}

// data inquiry-answer flow
void data_cmd_cycle(uint8_t cmd_num, uint8_t nbytes, uint16_t time_out)
{
    int8_t ret;

    ret = write_command_l(wbuf, 4, k_u8, cmd_num, k_u8 ,0x00);
    if (tty_send_command(fd_tty, wbuf, ret, 1000) <= 0)
        error_handler(cmd_num);
    else {
        ret = tty_get_response(fd_tty, rbuf, nbytes, time_out);
        if (ret < 0 || rbuf[2+IDX_CMD] != (cmd_num | 0x80) || nbytes != rbuf[2+IDX_LEN])
            error_handler(cmd_num | 0x80);
    }
}

int main(int argc, char *argv[]) 
{
    char *hex_path;
    int fd_hex=-1, ret=0, count=0, i=0;
    uint32_t flash_start_addr=0, flash_end_addr=0, base_addr=0, line_num=0, length, address, type;


    while ((ret = getopt_long(argc, argv, "hf:", long_options, NULL)) != -1) {
        switch (ret) {
        case 'h':
            usage_printf(basename(argv[0]));
            exit(0);
        case 'f':
            hex_path = optarg;
            break;
        default:
            fprintf(stderr, "Unknown option %c\n", ret);
            exit(EXIT_FAILURE);
            break;
        }
    }

    if ((fd_tty = tty_init("/dev/ttyUSB0")) == -1) {
        exit(EXIT_FAILURE);
    }

    // open stream file to read hex file line by line
    if ((fd_hex = open(hex_path, O_RDONLY)) == -1) {
        printf("hex file open failed\n");
        exit(EXIT_FAILURE);
    }
    FILE *fp = fdopen(fd_hex, "r");
    
    // CMD_PING
    do {
        data_cmd_cycle(CMD_UPDATE_PING, 1, 5000);

        if (rbuf[2+IDX_DATA] == 0xa5) {
            printf("mcu is in boot loader\n");
            break;
        } else if (rbuf[2+IDX_DATA] == 0x01) {
            printf("mcu is in app\n");
            base_cmd_cycle(CMD_JUMPTOBL, 5000);
            sleep(2);
        } else {
            error_handler(CMD_UPDATE_PING);
        }
        count++;
    } while (count <= 3);

    if (count >= 3)
        exit(EXIT_FAILURE);
    
    // CMD_ASK_APP_AREA
    data_cmd_cycle(CMD_ASK_APPAREA, 8, 5000);
    flash_start_addr = GetWordH(rbuf+2+IDX_DATA);
    flash_end_addr = GetWordH(rbuf+2+4+IDX_DATA);
    

    // CMD_PROGRAM_START
    base_cmd_cycle(CMD_PROGRAM_START, 20000);        // leave enough time for mcu flash erasing
    printf("mcu is unlocked and erased\n");

    // read hex file line by line to program flash
    do {
        fgets((char*)buffer, BUFSIZE, fp);
        printf("line %d: %s", line_num++, buffer);
        if(ferror(fp)) {
            printf("Error read line from hex file\n");
            exit(EXIT_FAILURE);
        }

        // read header ":"
        if (*buffer != 58) {
            printf("line sof is not right\n");
            exit(EXIT_FAILURE);
        }

        // read length
        sscanf((const char*)&buffer[1], "%2x", &length);
        printf("length is %d, ", length);

        // read address
        sscanf((const char*)&buffer[3], "%4x", &address);
        address += base_addr;
        printf("address is %x, ", address);

        // read  type
        sscanf((const char*)&buffer[7], "%2x", &type);
        printf("type is %d\n", type);

        switch(type) {
        case 0x00:
            // CMD_DLD, send start address and data length first
            if (address < flash_start_addr || address > flash_end_addr) {
                printf("exceed app flash area, wrong hex file\n");
                exit(EXIT_FAILURE);
            }
            ret = write_command_l(wbuf, 8, k_u8, CMD_DLD, k_u8, 0x08, k_u32, address, k_u32, length);
            if (tty_send_command(fd_tty, wbuf, ret, 1000) <= 0)
                error_handler(CMD_DLD);
            else {
                ret = tty_get_response(fd_tty, rbuf, 1, 500);
                if (ret < 0 || rbuf[2+IDX_CMD] != (CMD_DLD | 0x80))
                    error_handler(CMD_DLD | 0x80);
            }

            // CMD_SEND_DATA
            ret = write_command_l(wbuf, 4, k_u8, CMD_SEND_DATA, k_u8, length);
            ret -= 4;
            for (i=0; i<length; i++) {
                sscanf((const char*)&buffer[9] + 2*i, "%02hhx", (unsigned char*)&wbuf[i+ret]);
            }
            WriteShortL(&wbuf[ret+length], 0x0000);
            WriteShortH(&wbuf[ret+length+2], 0x0d0a);

            if (tty_send_command(fd_tty, wbuf, ret+length+4, 1000) <= 0)
                error_handler(CMD_SEND_DATA);
            else {
                ret = tty_get_response(fd_tty, rbuf, 1, 500);
                if (ret < 0 || rbuf[2+IDX_CMD] != (CMD_SEND_DATA | 0x80) || rbuf[2+IDX_DATA] != 0x00)
                    error_handler(CMD_SEND_DATA | 0x80);
            }
            break;
        case 0x01:
            // CMD_WRITE_CRC
            base_cmd_cycle(CMD_WRITECRC, 5000);
            printf("write CRC to mcu flash ok\n");
            
            // CMD_PROGRAM_END
            base_cmd_cycle(CMD_PROGRAM_END, 5000);
            printf("program end, lock mcu flash ok\n");
            
            // CMD_JUMPTOAPP
            base_cmd_cycle(CMD_JUMPTOAPP, 5000);
            printf("mcu jumping to app\n");
            break;
        case 0x02:
            break;
        case 0x03:
            break;
        case 0x04:
            sscanf((const char*)(buffer+9), "%4x", &base_addr);
            base_addr <<= 16;
            printf("base_address is 0x%x\n", base_addr);
            break;
        case 0x05:
            break;
        default : break;
        }
    } while (!feof(fp) && type != 0x01);

    fclose(fp);
    close(fd_hex);
    close(fd_tty);

    printf("reach eof of hex\n");
    exit(EXIT_SUCCESS);
}




