#include <libini.h>
#include "head.h"


/******************ini文件配置信息***************************/
void zbm_ini_creat(char str[])
{
	ini_fd_t fd;
	char name [32];

	sprintf(name,"/opt/ZBM/etc/%s",str);
	fd = ini_open(name, "w", NULL);
	if (0 == fd){
		
		printf("creat -> open ini error\n");
		return ;
	}
	ini_flush(fd);
	ini_close(fd);
	sync();
	return ;
}

void zbm_ini_read(char str [], char heading [], char key [], int *value)
{
	ini_fd_t fd;
	char name[32];

	sprintf(name, "/opt/ZBM/etc/%s", str);
	fd = ini_open(name, "r", NULL);
	if (0 == fd) {
		
		printf("read -> open ini error\n");
		return ;
	}
	
	ini_locateHeading(fd, heading);
	ini_locateKey(fd, key);
	ini_readInt(fd, value);
	ini_flush(fd);
	ini_close(fd);
	return ;
}

void zbm_ini_write(char str [], char heading [], char key [], int value)
{
	ini_fd_t fd;
	char name[32];

	sprintf(name, "/opt/ZBM/etc/%s", str);
	fd = ini_open(name, "w", NULL);
	if (0 == fd) {
		
		printf("write -> open ini error\n");
		return ;
	}
	
	ini_locateHeading(fd, heading);
	ini_locateKey(fd, key);
	ini_writeInt(fd, value);
	ini_flush(fd);
	ini_close(fd);
	
	return ;
}

void read_val_data(char str [], char heading [], char key [], double *value)
{
	ini_fd_t fd;
	char name[32];

	sprintf(name, "/opt/ZBM/etc/%s", str);
	fd = ini_open(name, "r", NULL);
	if (0 == fd) {
		
		printf("read data -> open ini error\n");
		return ;
	}
	
	ini_locateHeading(fd, heading);
	ini_locateKey(fd, key);
	ini_readDouble(fd, value);
	ini_flush(fd);
	ini_close(fd);
	return ;
}

void write_val_data(char str [], char heading [], char key [], double value)
{
	ini_fd_t fd;
	char name[32];

	sprintf(name, "/opt/ZBM/etc/%s", str);
	fd = ini_open(name, "w", NULL);
	if (0 == fd) {
		
		printf("write data -> open ini error\n");
		return ;
	}
	
	ini_locateHeading(fd, heading);
	ini_locateKey(fd, key);
	ini_writeDouble(fd, value);
	ini_flush(fd);
	ini_close(fd);
	return ;
}

int set_time_count()
{
	int count = TIME_N_PIC;
	int my_time_count;
	time_t t;
	
	t = time(NULL);
	
	zbm_ini_read(TIME_INI, "COUNT_TIME", "time", &my_time_count);
	count = t - my_time_count;
	
	return count;
}