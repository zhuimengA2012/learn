#ifndef __SET_COM__
#define __SET_COM__

#include <stdio.h>      
#include <stdlib.h>
#include <unistd.h> 
#include <signal.h>
#include <termios.h>    /*PPSIX终端控制定义*/
#include <sys/types.h>  
#include <sys/stat.h>   
#include <fcntl.h>   
#include <strings.h>

static struct termios termios_old, termios_new;
extern unsigned char reg[128];

/*************************函数声明***********************************/
void uart_init(int baudrate);
int open_port(int comport, int baudrate, int databit, const char *stopbit, char parity);
int getbaudrate();
void setbaudrate(int baudrate);
void setdatabit(int databit);
void setstopbit(const char *stopbit);
void setparitycheck(char parity);
int setportattr(int baudrate, int databit, const char *stopbit, char parity);
int BAUDRATE(int baudrate);
int _BAUDRATE(int baudrate);
void  close_port();

#endif