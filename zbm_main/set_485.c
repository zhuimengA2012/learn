#include "set_485.h"

int fd_ser;

/*********串口初始化函数*********************************************/
void uart_init(int baudrate)
{	
	
	if (open_port(1, baudrate, 8, "1", 'N') < 0) {
		
		printf("open port error!\n");
		return;
	}

	return;
}

int open_port(int comport, int baudrate, int databit, const char *stopbit, char parity)
{
	char   *pcomport;
	int    retval;

	switch (comport) 
	{
	case 0:
		pcomport = "/dev/ttySAC0";
		break;
	case 1:
		pcomport = "/dev/ttyO1";
		break;
	case 2:
		pcomport = "/dev/ttySAC2";
		break;
	case 3:
		pcomport = "/dev/ttySAC3";
		break;
	default:
		pcomport = "/dev/ttySAC1";
		break;
	}

	fd_ser = open(pcomport, O_RDWR | O_NOCTTY);			//| O_NONBLOCK
	if (-1 == fd_ser) {
		
		perror("open fd_ser");
		return -1;
	}
	retval = tcgetattr(fd_ser, &termios_old);       /* save old termios value */
	if (-1 == retval) {
		perror("tcgetattr");
		return -1;
	}
	retval = setportattr(baudrate, databit, stopbit, parity);
	if (-1 == retval) {
		return -1;
	}

	return retval;
}

	/* get serial port baudrate */
int getbaudrate()
{
	return (_BAUDRATE(cfgetospeed(&termios_new)));
}

	/* set serial port baudrate by use of file descriptor fd_ser */
void setbaudrate(int baudrate)
{
	termios_new.c_cflag = BAUDRATE(baudrate);  /* set baudrate */
}

void setdatabit(int databit)
{
	termios_new.c_cflag &= ~CSIZE;
	switch (databit) 
	{
	case 8:
		termios_new.c_cflag |= CS8;
		break;
	case 7:
		termios_new.c_cflag |= CS7;
		break;
	case 6:
		termios_new.c_cflag |= CS6;
		break;
	case 5:
		termios_new.c_cflag |= CS5;
		break;
	default:
		termios_new.c_cflag |= CS8;
		break;
	}

	return;
}

void setstopbit(const char *stopbit)
{
	if (0 == strcmp(stopbit, "1")) {
		
		termios_new.c_cflag &= ~CSTOPB; /* 1 stop bit */
	}
	else if (0 == strcmp(stopbit, "1.5")) {
		
		termios_new.c_cflag &= ~CSTOPB; /* 1.5 stop bits */
	}
	else if (0 == strcmp(stopbit, "2")) {
		
		termios_new.c_cflag |= CSTOPB;  /* 2 stop bits */
	}
	else {

		termios_new.c_cflag &= ~CSTOPB; /* 1 stop bit */
	}

	return;
}

void setparitycheck(char parity)
{
	switch (parity) 
	{
	case 'N':                  /* no parity check */
		termios_new.c_cflag &= ~PARENB;
		break;
	case 'E':                  /* even */
		termios_new.c_cflag |= PARENB;
		termios_new.c_cflag &= ~PARODD;
		break;
	case 'O':                  /* odd */
		termios_new.c_cflag |= PARENB;
		termios_new.c_cflag |= ~PARODD;
		break;
	default:                   /* no parity check */
		termios_new.c_cflag &= ~PARENB;
		break;
	}

	return;
}

int setportattr(int baudrate, int databit, const char *stopbit, char parity)
{
	bzero(&termios_new, sizeof(termios_new));
	cfmakeraw(&termios_new);
	setbaudrate(baudrate);
	termios_new.c_cflag |= CLOCAL | CREAD;      /* | CRTSCTS */
	setdatabit(databit);
	setparitycheck(parity);
	setstopbit(stopbit);
	termios_new.c_oflag = 0;
	termios_new.c_lflag |= 0;
	termios_new.c_oflag &= ~OPOST;
	//    termios_new.c_cc[VTIME] = 0;        /* unit: 1/10 second. */
	//    termios_new.c_cc[VMIN] = 1;       /* minimal characters for reading */
	tcflush(fd_ser, TCIFLUSH);
	
	return (tcsetattr(fd_ser, TCSANOW, &termios_new));
}

int BAUDRATE(int baudrate)
{
	switch (baudrate) 
	{
	case 2400:
		return (B2400);
	case 9600:
		return (B9600);
	case 19200:
		return (B19200);
	case 38400:
		return (B38400);
	case 57600:
		return (B57600);
	case 115200:
		return (B115200);
	default:
		return (B9600);
	}
	
	return 0;
}

int _BAUDRATE(int baudrate)  
{
	/* reverse baudrate */
	switch (baudrate) 
	{
	case B2400:
		return (2400);
	case B9600:
		return (9600);
	case B19200:
		return (19200);
	case B38400:
		return (38400);
	case B57600:
		return (57600);
	case B115200:
		return (115200);
	default:
		return (9600);
	}

	return 0;
}

void  close_port()
{
	tcsetattr(fd_ser, TCSADRAIN, &termios_old);
	close(fd_ser);

	return;
}