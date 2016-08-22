#include "head.h"

extern struct VedioData VD;
extern mProductVer;

/****************************�����򴫲Σ�ʵ����Ӧ�Ĺ���*********************************/

void show_info(const char *argv []) 
{
	int ret;
	unsigned char *pointer;
	unsigned char *y_data;
	char version [32];
#if 1
	if (0 == strncmp(argv[1], "--version", 3)) {
		/******************��ӡ�汾��Ϣ********************/
		sprintf(version, "%x.%x.%x.%x", (mProductVer >> 24) & 0xff, (mProductVer >> 16) & 0xff, (mProductVer >> 8) & 0xff, (mProductVer >> 0) & 0xff);
		printf("\n%s version: %s %s\n", NAME, version, " 2016.08.11  16:45");
		VD = libzbmTest(NULL,TYPE_VERSION, 1, 0, 0);
		printf("%s\n", VD.version);
		return ; 
	}else if (0 == strncmp(argv[1], "--clarity", 3)) {
		/******************����������********************/
		
		driver_init();
		if (init_v4l2(1) == -1) {   //type����1��ʱ����mjpg
			printf("v4l init error\n");
			exit(1);
		}
		while (1) {
			LED_ON;
			usleep(1000);
			LED_OFF;
			usleep(1000);
			
			v4l2_grab();
			save_shm_picture();
			VD = libzbmTest(NULL, TYPE_CLARITY, 1, 0, 0);
			printf("\nclarity:--%6f--\n", VD.value);
		}
	}else if (0 == strncmp(argv[1], "--gray", 6)) {
		
		return ;
	}else if (0 == strncmp(argv[1], "--threshold", 3)) {
		
		system("pic_threshold");
		return ;
	}else if (0 == strncmp(argv[1], "--help", 3)) {
		/******************��ӡ������Ϣ********************/
		
		printf("\n%-10s:display this help and exit\n", "--help");
		printf("%-10s:output version information and exit\n", "--version");
		printf("%-10s:you can adjust picture sharpness\n", "--clarity");
		printf("%-10s:you can obtain a picture and exit\n", "--image");
		printf("%-10s:you can obtain a picture of threshold\n", "--threshold");
		return ; 
	}else if (0 == strncmp(argv[1], "--image", 3)) {
		/******************����һ��ͼƬ������********************/
		
		system("pic_image");
		return ;
	}else if (0 == strncmp(argv[1], "--threshold", 11)) {
		
		return ; 
	}else {
				/******************����ָ�����********************/
		
		printf("\nUsage: %s [options]\n", NAME);
		printf("Try '--help' for more information\n");
	}
#endif
	return ;
}