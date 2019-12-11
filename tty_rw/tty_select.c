/* use fgets for getchar, do-while for ttyUSB read */

#include <sys/select.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>   /* Standard input/output definitions */
#include <string.h>  /* String function definitions */
#include <unistd.h>  /* UNIX standard function definitions */
#include <fcntl.h>   /* File control definitions */
#include <errno.h>   /* Error number definitions */
#include <termios.h> /* POSIX terminal control definitions */
// #include <termbits.h>
#include <stdlib.h>

#define BUFSIZE 128


int main(void) 
{
    char stdin_buf[BUFSIZE] = {0,};
    char wbuf[BUFSIZE];
    char rbuf[BUFSIZE];
    char byte = 0;
    int ret, i, len;
    fd_set rfds, wfds;
    struct termios tty0;
    struct timeval tv;
    
    memset(&tty0,0,sizeof(tty0));

    tv.tv_sec = 10;
    tv.tv_usec = 500; /* 设置select等待的最大时间为1秒加500毫秒 */ 
    
    ////////////////////////////  ttyUSB0 setting  ////////////////////////////////////////
    int USB0 = open("/dev/ttyUSB0", O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (USB0 == -1){
        printf("ttyUSB0 open failed\n");
        exit(EXIT_FAILURE);
    }
    
    // Error Handling 
    if (tcgetattr(USB0, &tty0) != 0 ) {
        printf("error: tcgetattr ttyUSB0\n");
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
    tcflush(USB0, TCIFLUSH);
    if (tcsetattr(USB0, TCSANOW, &tty0) != 0) {
       printf("error: tcsetattr");
    }
    
    /////////////////////////////  READ WRITE  //////////////////////////////////////////////////////////////////////
    printf("file descriptor USB0 %d, input what you want to send\n", USB0);
    FD_ZERO(&rfds);
    FD_ZERO(&wfds);
    FD_SET(USB0,&wfds);  // add USB0 to wfds
    FD_SET(USB0,&rfds);  // add USB0 to rfds
    
    while(1){
        // send command
        ret = select(USB0+1, NULL, &wfds, NULL, &tv); 

        if (ret <0) {
            printf("select() fail\n");
            continue;
        } else if (ret == 0) {
            printf("select write timeout\n");
            continue;
        } else {
            if (fgets(stdin_buf, BUFSIZE, stdin) != NULL) {
                len = strlen(stdin_buf) - 1;        // remove '\n' of terminal input
                
                if (len % 2 != 0) {
                    printf("string length should be even, wait for input again\n");
                    continue;
                }
                
                for (i=0; i<len; i+=2) {
                    sscanf(&stdin_buf[i], "%02hhx", &byte);
                    wbuf[i/2] = byte;
                }

                printf("std input length : %d\n", len);
                printf("std input : %s", stdin_buf);
                printf("translate stdin to hex: ");  
                for (i=0; i<len/2; i++) {
                    printf("0x%02x ", (unsigned char)wbuf[i]);
                }
                printf("\n");

                ret = write(USB0, wbuf, i); // can't use strlen(wbuf), for data maybe 0 as '\0'
                if (ret <= 0) {
                    printf("write to tty_usb fail, try again\n");
                    continue;
                } else {
                    printf("write %d bytes to tty\n", ret);
                }
            }
            
            // read response after sending command
            memset(rbuf, 0, BUFSIZE);
            ret = select(USB0+1, &rfds, NULL, NULL, &tv); 

            if (ret <0)
                printf("select() fail\n");
            else if (ret == 0)
                printf("select read timeout\n");
            else {
                printf("USB0 can read\n");
                printf("read %d bytes from tty\n", len = read(USB0, rbuf, BUFSIZ));
                if (len > 0) {
                    printf("response is: ");
                    for (i=0; i<len; i++)
                        printf("0x%02x ", (unsigned char)rbuf[i]);
                    printf("\n");
                }

            }
        }
    } 

    close(USB0);
    return 0;
}




