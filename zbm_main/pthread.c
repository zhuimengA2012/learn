#include "head.h"
#include "485.h"
#include "set_485.h"


struct VedioData VD;
extern const unsigned short mProductType;		 //裂缝：0x5380 ,沉降：0x5381 , 倾斜：0x5383
extern unsigned char err;
unsigned char *y_data;
int flag, time_flag = 0;
struct timeval time_start, time_mid, time_end;
int pthread_flag = 0;
extern int orl;
int time_count = 0;
extern int Err_count;
int save_image_flag = 0;
int num = 0;
extern sqlite3 *sqlp;

/************************线程相关自定义函数*******************************/
int api_get_thread_policy(pthread_attr_t *attr)
{
	int policy;
	int rs = pthread_attr_getschedpolicy(attr, &policy);
	assert(rs == 0);

	switch (policy)
		{
	case SCHED_FIFO:
		printf("policy = SCHED_FIFO\n");
		break;
	case SCHED_RR:
		printf("policy = SCHED_RR\n");
		break;
	case SCHED_OTHER:
		printf("policy = SCHED_OTHER\n");
		break;
	default:
		printf("policy = UNKNOWN\n");
		break; 
		}
	return policy;
}

void api_show_thread_priority(pthread_attr_t *attr, int policy)
{
	int priority = sched_get_priority_max(policy);
	assert(priority != -1);
	printf("max_priority = %d\n", priority);
	priority = sched_get_priority_min(policy);
	assert(priority != -1);
	printf("min_priority = %d\n", priority);

	return;
}

int api_get_thread_priority(pthread_attr_t *attr)
{
	struct sched_param param;
	int rs = pthread_attr_getschedparam(attr, &param);
	assert(rs == 0);
	printf("priority = %d\n", param.__sched_priority);

	return param.__sched_priority;
}

void api_set_thread_policy(pthread_attr_t *attr, int policy)
{
	int rs = pthread_attr_setschedpolicy(attr, policy);
	assert(rs == 0);
	api_get_thread_policy(attr);
	
	return;
}


void sighandle(int signo)
{
	int ret;
	long int my_s, my_us;
	char buf[64] =  {0};
	time_t my_rtc_time;
	unsigned char *pointer;
	time_t t;
	struct tm *my_time;
	char my_picture_time[32] = {0};
	float my_picture_value_x = 0, my_picture_value_y = 0;
	char image_name [64] = {0};
	int ser_num = 0;
	
	switch (signo) {
	case SIGALRM:
			/*
		if (0 == flag) {
			ret = gettimeofday(&time_start, NULL);
			my_s = time_start.tv_sec - time_end.tv_sec;
			my_us = time_start.tv_usec - time_end.tv_usec;
		}else {
			ret = gettimeofday(&time_end, NULL);
			my_s = time_end.tv_sec - time_start.tv_sec;
			my_us = time_end.tv_usec - time_start.tv_usec;
		}
		flag ^= 1;	*/
		/*****************将系统时间存入文件中,便于提取关机时间******************/
		keep_alive();
		if ((time_count++) % 10 == 1) {
			
			my_rtc_time = time(NULL);
			bzero(buf, sizeof(buf));
			sprintf(buf, "%s", ctime(&my_rtc_time));
#ifdef DEBUG
			printf("--------------------------------------time = %s\n", buf);
#endif
			fseek(fd_time, 0, SEEK_SET);
			fwrite(buf, 1, strlen(buf) - 1, fd_time);
			fflush(fd_time);
		}
		if (time_count % TIME_N_PIC == 0) {
			
			save_image_flag = 1;
		}
		
		if (0 == time_flag){
			
			time_flag = 1;  //保证v4l2_grab()只能进入一次
			VD.flag = 0;
			/*****************拍图片函数*******************/
			ret = v4l2_grab();	//与底层交互读出图片缓存
			if (-1 == ret) {
				printf("v4l2_grab error\n");
				time_flag = 0;
				return ;
			}
			
			pointer = buffers[0].start;
			yuyv_2_gray(y_data, pointer);
			if (1 == save_image_flag) {
				/*****************定时间存储图片*********************/
				save_image_flag = 0;
				zbm_ini_read(INI, "ZBM_PIC", "ser_num", &ser_num);
				VD = libzbmTest(y_data, 0, 1, ser_num + 1, 0);
				zbm_ini_write(INI, "ZBM_PIC", "ser_num", ser_num + 1);
				
				t = time(NULL);
				zbm_ini_write(TIME_INI, "COUNT_TIME", "time", (int)t);
				sync();
				time_count = TIME_N_PIC + 1;
				my_time = localtime(&t);
				if (mProductType == 0x5380 ) {
					
					sprintf(image_name, "%d%02d%02d-%0.2f-%d.jpg", 1900 + my_time->tm_year, 1 + my_time->tm_mon, my_time->tm_mday, VD.value, ser_num + 1);
				}else if (mProductType == 0x5381) {
				
					sprintf(image_name, "%d%02d%02d-%0.2f-%d.jpg", 1900 + my_time->tm_year, 1 + my_time->tm_mon, my_time->tm_mday, VD.value_x, ser_num + 1);
				}else{
					
					sprintf(image_name, "%d%02d%02d-%0.2f-%0.2f-%d.jpg", 1900 + my_time->tm_year, 1 + my_time->tm_mon, my_time->tm_mday, VD.value_x, VD.value_y, ser_num + 1);	
				}
				zbm_sqlite_insert(sqlp, "image", image_name);
				zbm_sqlite_del_picture(sqlp, "image");
			}else {
				
				VD = libzbmTest(y_data, 0, 1, 0, 0);
			}
			
			err = VD.err;
			ret = data_filter();
			if (-1 == ret) {			//读数错误
				
				printf("data error\n");
				printf("Err_count = %d\n", Err_count);
				num--;
			}
			
			num++;
			printf("---num = %d---\n", num);
			
			if (5 == (num % 50) && 1 == VD.err) {
			
				printf("num to 50 and save data\n");
				set_data();				//备份有效数据
				printf("num to 50 and save data after\n");
			}
			if (mProductType == 0x5380 || mProductType == 0x5381) {
			
				if (VD.flag == 255) { 
					
					close_v4l2();
					exit(1);		  //退出程序
				}	
			}
			if (access("/dev/video0", F_OK) == -1) {  
			
				perror("access"); 
				close_v4l2();
				exit(1);
			}
		//	printf("s = %ldus = %ld\n", my_us > 0 ? my_s : my_s - 1, my_us > 0 ? my_us : 1000000 + my_us);
			if (1 == time_flag){
				
				time_flag = 0;
			}
		}
		
		break;
	case SIGUSR1:
		printf("come in SIGUSR1\n");
		break;
	default:
		printf("Catch a error\n");
		break;
	}
	return;
}

/*******************************线程处理函数***************************************/
void *thread_485(void *args)  
{
	int read_id;

	uart_init(9600);
	COMM_Init();
	zbm_ini_read(INI, "ZBM_CONFIG", "ID", &read_id);
	reg[REG_ADDR] = (unsigned char)read_id;
	
	printf("mProductType= %x\n", mProductType);
	printf("ID = %d\n", reg[REG_ADDR]);
	
	//signal(SIGUSR1, sighandle);
	//kill(getpid(), SIGUSR1);    在需要的地方发信号
	
	while (1) {
		signal_handler_IO();
		usleep(100);
	}

	pthread_exit(NULL);
	return NULL;
}