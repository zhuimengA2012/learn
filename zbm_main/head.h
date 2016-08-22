#ifndef __HEAD__
#define __HEAD__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sched.h>
#include <assert.h>
#include <jpeglib.h> 
#include <linux/videodev2.h>
#include <sys/types.h>  
#include <sys/stat.h>   
#include <fcntl.h>    
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/signal.h>
#include <linux/watchdog.h>
#include <sys/time.h> 
#include <time.h>
#include <assert.h> 
#include <sqlite3.h>
#include <md5.h>

/*************main���*****************/
#define  DEBUG 1
void watchdog_init(void);
void keep_alive(void);

/************v4l���************/
extern int driver_fd;

#define USB_CMOS_OPEN 		ioctl(driver_fd,0,5)
#define USB_CMOS_CLOSE		ioctl(driver_fd,1,5)

#define LED_ON				ioctl(driver_fd,1,0)
#define LED_OFF				ioctl(driver_fd,0,0)

#define Rs485_Send 			ioctl(driver_fd,0,4)
#define Rs485_Receive		ioctl(driver_fd,1,4)

#define FILE_VIDEO     "/dev/video0"

int  IMAGEWIDTH;
int  IMAGEHEIGHT;

struct VedioData {
	
	int		flag;
	int		scale;
	int		zero;
	float	value;
	int	err;
	char   version[64];
	float   value_x;
	float   value_y;
	short mProductType;
	int width;
	int high;
};
struct ProcesData {			//�洢��б���ݵĽṹ��
	
	float  val_x;
	float  val_y;
	float  avg_x;
	float  avg_y;
	};

struct buffer  
{  
	void *start;  
	unsigned int length;  
};  
extern struct buffer *buffers;
FILE* fd_time;

/********************���ݴ�����ĺ���*********************/
int data_filter(void);

	/***********���κ���************/
void show_info(const char *argv []);

	/*********�߳���غ���*********/
int api_get_thread_policy(pthread_attr_t *attr);
void api_show_thread_priority(pthread_attr_t *attr, int policy);
int api_get_thread_priority(pthread_attr_t *attr);
void api_set_thread_policy(pthread_attr_t *attr, int policy);
void *thread_485(void *args);
void *thread_grab(void *args);
void *thread_picture(void *args);
void sighandle(int signo);

/*******************v4l��غ���***********************/
int init_v4l2(int type);
int v4l2_grab(void);
int save_shm_picture(void);
void yuyv_2_rgb888(int num);
void save_yuyv_jpeg(void);

/**********************��������ĺ���*********************/
void driver_init(void);

/*****************��̬������ĺ���********************/
extern struct VedioData libzbmTest(unsigned char *ydata, int type, int flag, int scale, int zero);
extern struct ProcesData DataProcess(struct ProcesData PD);
extern struct VedioData kalman_push(int a, double value);

/********************�붯̬����غ�*************************/
//#define VERSION		 "1.2.3.5"			//232�����ѯ���İ汾��
#define NAME		"zbm_main"
#define TYPE_VERSION	 1
#define TYPE_CLARITY	 2
#define TYPE_PIC_THR	 3
#define TYPE_IMAGE		 4
#define TYPE_M_Pro		 250

/********************ini�����ļ����**************************/
#define INI "zbm_config.ini"
#define TIME_INI "time_count.ini"
void zbm_ini_creat(char str []);
void zbm_ini_read(char str [], char heading [], char key [], int *value);
void zbm_ini_write(char str [], char heading [], char key [], int value);
void read_val_data(char str [], char heading [], char key [], double *value);
void write_val_data(char str [], char heading [], char key [], double value);
int set_time_count();

/***********************com_driver.c***************************************/
void set_data(void);

/***************************rtc���************************************/
int rtc_set_time(int *rtctime);
int *rtc_read_time(void);

/*****************************��ͼƬ��غ����ͱ���*********************************************/
#define   TIME_N   15      //TIME_NΪ��ʱ���ٷ�����һ��ͼƬ
#define   PIC_NUM   100
int TIME_N_PIC;
#define   MYSQL   "/opt/ZBM/etc/picture.db"
int callback(void *p, int c, char **val, char **name);
int zbm_sqlite_creat_table(sqlite3 *sqlp, char *table_name);
int zbm_sqlite_insert(sqlite3 *sqlp, char *table_name, char *image_name);
int zbm_sqlite_delete(sqlite3 *sqlp, char *table_name, char *image_name);
int zbm_sqlite_delete_table(sqlite3 *sqlp, char *table_name);
int zbm_sqlite_find(sqlite3 *sqlp, char *table_name, char *time2_start, char *time2_end);
int zbm_sqlite_del_picture(sqlite3 *sqlp, char *table_name);
int zbm_sqlite_time_get_picture(sqlite3 *sqlp, char *table_name, char *time2);

/*********************************ͼƬ����*************************************************/
int picture_trans(char *image_name);
int picture_find_trans();
char image_trans[32];
char ** image_result;
unsigned char *file_all;
int size_picture;
int image_row, image_colunm;
char image_time_start[16];
char image_time_end[16];
#define PIC_ACK  0X01
#define PIC_NAK  0X02
#define PIC_CAN  0X04

#endif