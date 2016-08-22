#include <stdio.h>
#include <stdlib.h>
#include <linux/rtc.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

/********************设置rtc时间**********************/
int rtc_set_time(int *rtctime)
{
	int retval;
	int fd_rtc;
	struct rtc_time rtc_tm;
 
	fd_rtc = open("/dev/misc/rtc", O_RDWR);	//打开RTC驱动
	if (-1 == fd_rtc) {
		
		perror("/dev/misc/rtc");
		return -1;
	}
	
	/************ Set the RTC time/date ****************/
	rtc_tm.tm_year  = rtctime[0] - 1900;	//2016-1900=116
	rtc_tm.tm_mon   = rtctime[1] - 1;		//当前月份-1,如7月,输入7-1=6
	rtc_tm.tm_mday  = rtctime[2];			//日	
	rtc_tm.tm_hour  = rtctime[3];			//时
	rtc_tm.tm_min   = rtctime[4];			//分
	rtc_tm.tm_sec   = rtctime[5];			//秒

	retval = ioctl(fd_rtc, RTC_SET_TIME, &rtc_tm);	//设置RTC时间
	if (-1 == retval) {

		perror("ioctl");
		return -1;
	}

	close(fd_rtc);	
	system("hwclock -s");		//将系统时钟和硬件RTC时间同步
	
	return 0;	
}

/********************读取rtc时间**********************/
int *rtc_read_time(void)
{
	int retval;
	int fd_rtc;
	struct rtc_time rtc_tm;
	int *rtctime;

	rtctime = (int *)malloc(6 * sizeof(int));
	fd_rtc = open("/dev/misc/rtc", O_RDONLY);	//打开RTC驱动
	if (-1 == fd_rtc) {
		
		perror("/dev/misc/rtc");
		return NULL;
	}

	/**************** Read the RTC time/date ***************/
	retval = ioctl(fd_rtc, RTC_RD_TIME, &rtc_tm);
	if (-1 == retval) {

		perror("ioctl");
		return NULL;
	}

	rtctime[0] = rtc_tm.tm_year + 1900;
	rtctime[1] = rtc_tm.tm_mon + 1;
	rtctime[2] = rtc_tm.tm_mday;
	rtctime[3] = rtc_tm.tm_hour;
	rtctime[4] = rtc_tm.tm_min;
	rtctime[5] = rtc_tm.tm_sec;
#if 0
	printf("Current RTC date/time is %d-%d-%d,%02d:%02d:%02d\n",
		rtc_tm.tm_mday, rtc_tm.tm_mon + 1, rtc_tm.tm_year + 1900, rtc_tm.tm_hour, rtc_tm.tm_min, rtc_tm.tm_sec);
#endif
	close(fd_rtc);	

	return rtctime;
}