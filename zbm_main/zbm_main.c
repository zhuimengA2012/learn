#include "head.h"
#include "485.h"
#include "set_485.h"
fsefsdf
int driver_fd;
extern int pthread_flag;
extern struct VedioData VD;
extern unsigned short mProductType;
int wdog_fd;
int orl = 0;
extern int time_count;
extern int save_image_flag;
extern unsigned char *y_data;
sqlite3 *sqlp = NULL;

/*************************led 485���� ����ͷ��س�ʼ��*******************************/
void driver_init(void)
{
	driver_fd = open("/dev/led", O_RDWR | O_NONBLOCK);
	if (driver_fd < 0) {
		
		perror("open device led");
		return ;
	}
}

/*********************************���Ź���ʼ��**************************************/
void watchdog_init(void)
{
	int timeout = 30;           //���ó�ʱʱ��Ϊ30��
	
	wdog_fd = open("/dev/watchdog", O_RDWR);  //Ĭ��Ϊ1����

	if (wdog_fd == -1) {
		
		perror("Watchdog device");
		return ;
	}
	ioctl(wdog_fd, WDIOC_SETOPTIONS, WDIOS_ENABLECARD); //ʹ�ܿ��Ź�
	ioctl(wdog_fd, WDIOC_SETTIMEOUT, &timeout);         //���ó�ʱʱ��
	
	return;
}
/*************************************ι��******************************************/
void keep_alive(void)
{

	ioctl(wdog_fd, WDIOC_KEEPALIVE, 0);    //ι��
//	printf("keep_alive\n");
	return ;
}

int main(int argc,const char *argv[])
{
	int ret;
	int wdog_fd;
	pthread_t id_picture, id_grab, id_485;	
	pthread_attr_t attr, attr2, attr3;       // �߳�����
	struct sched_param sched, sched2, sched3;  // ���Ȳ���
	int rs;
	int read_id_ini, read_sn_ini;
	int read_id;
	struct itimerval value;
	
	zbm_ini_creat(INI);   //��û��ini�ļ���ʱ����Դ���ini�ļ�
	zbm_ini_creat(TIME_INI);
	if (SQLITE_OK != sqlite3_open(MYSQL, &sqlp)) {
		puts("open file fail");
	}
	/*****************���㷨���л�ȡ����������********************/
	VD = libzbmTest(NULL, TYPE_M_Pro, 1, 0, 0);
	mProductType = VD.mProductType;
	IMAGEWIDTH = VD.width;
	IMAGEHEIGHT = VD.high;
	time_count = set_time_count();

	if (mProductType == 0x5380) {
		
		TIME_N_PIC = 46 * TIME_N;
	}else if (mProductType == 0x5381) {
		
		TIME_N_PIC = 15 * TIME_N;
	}else if (mProductType == 0x5383) {
		
		TIME_N_PIC = 80 * TIME_N;
	}
	if (time_count > TIME_N_PIC) {
			
		save_image_flag = 1;
	}
	/*********************���δ���*************************/
	if (2 <= argc) {
		show_info(argv);
		return 0;
	}
	printf("by libzbm return mProductType = %x \n", mProductType);
	printf("by libzbm return width = %d \n", IMAGEWIDTH);
	printf("by libzbm return high = %d \n", IMAGEHEIGHT);
	printf("time_count = %d\n", time_count);
	sleep(1);
	
	driver_init();        //���豸�ڵ㣬��ײ㽻��
	watchdog_init();	  //��ʼ�����Ź�
	pthread_flag = 0;
	VD.flag = 0;

	/*******************485�̳߳�ʼ��Ϊ������ȼ��߳�***********************/

	ret = pthread_create(&id_485, NULL, thread_485, NULL);
	if (ret != 0) {
		printf("thread_485 creat error!\n");
		return -1;
	}

	y_data = (unsigned char *)malloc(IMAGEHEIGHT * IMAGEWIDTH);
	if (-1 == init_v4l2(0)) {		 //typeΪ1��MJPG,typeΪ0��YUYV
		
		printf("v4l init error\n");
		close_v4l2();
		exit(1);
	}
	fd_time = fopen("/opt/ZBM/etc/time", "w+");
	if (NULL == fd_time){
		
		perror("time open");
		close_v4l2();
		exit(1);
	}
	zbm_sqlite_creat_table(sqlp, "image");
	signal(SIGALRM, sighandle);
	if (mProductType == 0x5380) {
		
		value.it_interval.tv_sec = 1;
		value.it_interval.tv_usec = 300000;
		value.it_value.tv_sec = 1;
		value.it_value.tv_usec = 300000;
	}
	else if (mProductType == 0x5381) {
		
		value.it_interval.tv_sec = 4;
		value.it_interval.tv_usec = 0;
		value.it_value.tv_sec = 4;
		value.it_value.tv_usec = 0;
	}
	else if (mProductType == 0x5383) {
		
		value.it_interval.tv_sec = 0;
		value.it_interval.tv_usec = 750000;
		value.it_value.tv_sec = 0;
		value.it_value.tv_usec = 750000;
	}
	setitimer(ITIMER_REAL, &value, NULL);
	
	pthread_join(id_485, NULL);
	return 0;
}