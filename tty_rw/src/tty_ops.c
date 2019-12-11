
#include <errno.h>
#include <sys/select.h>
#include <stdio.h>   /* Standard input/output definitions */
#include <stdint.h>
#include <string.h>
#include <fcntl.h>   /* File control definitions */
#include <termios.h> /* POSIX terminal control definitions */
#include <unistd.h>
#include "command.h"


static int8_t check_integrity(uint8_t *pdata)
{
    uint8_t idx_crc;
    if (pdata[0] != 0xa5 || pdata[1] != 0x5a) 
        return -1;
    
    idx_crc = 2 + IDX_DATA + pdata[IDX_LEN+2];
    
    if (pdata[idx_crc+2] != 0x0d || pdata[idx_crc+3] != 0x0a) 
        return -1;
    
    return 0;
}

int tty_init(char *tty_dev)
{
    struct termios tty0;
    memset(&tty0,0,sizeof(tty0));

    ////////////////////////////  ttyusbfd setting  ////////////////////////////////////////
    int usbfd = open(tty_dev, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (usbfd == -1){
        printf("%s open failed\n", tty_dev);
        return -1;
    }
    
    // Error Handling 
    if (tcgetattr(usbfd, &tty0) != 0 ) {
        printf("error: tcgetattr %s\n", tty_dev);
        return -1;
    }

    // Set Baud Rate 
    cfsetospeed (&tty0, (speed_t)B115200);

    // Setting other Port Stuff 
    tty0.c_cflag     |=  CREAD | CLOCAL;     // turn on READ & ignore ctrl lines
    tty0.c_cflag     &=  ~PARENB;            // Make 8n1
    tty0.c_cflag     &=  ~CSTOPB;
    tty0.c_cflag     &=  ~CSIZE;
    tty0.c_cflag     |=  CS8;
    tty0.c_cflag     &=  ~CRTSCTS;           // no flow control

    // Make raw 
    cfmakeraw(&tty0);

    // Flush Port, then applies attributes 
    tcflush(usbfd, TCIFLUSH);
    if (tcsetattr(usbfd, TCSANOW, &tty0) != 0) {
       printf("error: tcsetattr %s\n", tty_dev);
       return -1;
    }
    
    return usbfd;
}

int tty_send_command(int fd, uint8_t *pdata, uint8_t send_nbytes, uint16_t time_out)
{
    fd_set wfds;
    struct timeval tv;
    int ret;
    uint32_t time_us = time_out * 1000;

    FD_ZERO(&wfds);
    FD_SET(fd, &wfds);  // add tty_fd to wfds
    tv.tv_sec = time_us / 1000000;      // 1
    tv.tv_usec = time_us % 1000000;     // set select() timeout to x(s) + y(us)

    ret = select(fd+1, NULL, &wfds, NULL, &tv); 
        
    if (ret < 0) {
        printf("select() fail\n");
    } else if (ret == 0) {
        printf("select write timeout\n");
    } else if (FD_ISSET(fd, &wfds)) {
        ret = write(fd, pdata, send_nbytes);
        printf("send : ");
        for (int i=0; i<ret; i++) 
            printf("%02x ", pdata[i]);
        printf("\n");
    }

    return ret;
}

int tty_get_response(int fd, uint8_t *pdata, uint8_t need_nbytes, uint16_t time_out)  // time_out : ms
{
    fd_set rfds;
    struct timeval tv;
    int ret=1, count=0;
    uint32_t time_us = time_out * 1000;
    uint8_t total_bytes = need_nbytes + 8;

    FD_ZERO(&rfds);
    FD_SET(fd, &rfds);  // add tty_fd to rfds
    tv.tv_sec = time_us / 1000000;
    tv.tv_usec = time_us % 1000000;

    while (count < total_bytes && ret > 0) {
        ret = select(fd+1, &rfds, NULL, NULL, &tv); 
            
        if (ret < 0) {
            printf("select() fail\n");
        } else if (ret == 0) {
            printf("select read timeout\n");
        } else if (FD_ISSET(fd, &rfds)) {
            ret = read(fd, &pdata[count], total_bytes-count);
            if (ret == -1)
                printf("read fail: %s\n", strerror(errno));
            else
                count += ret;
        }
    }

    //ret = (ret < 8 || check_integrity(pdata) == -1) ? -1 : ret;
    printf("get %dbytes: ", count);
    for (int i=0; i<count; i++)
        printf("%02x ", pdata[i]);
    printf("\n");
    
    return ret;
}

