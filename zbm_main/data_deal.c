#include "head.h"
#include <math.h>

extern struct VedioData VD;
struct ProcesData PD;
extern const unsigned short mProductType;		 //�ѷ죺0x5380 ,������0x5381 , ��б��0x5383
int err;
extern int dataCap_flag; 
int Err_count;
int num_flag = 0;
struct VedioData DATA_CJ;

/*************************�Դ��㷨�еõ������ݽ��д���*******************************/
int data_filter(void)			
{
	float data = 0.0;


	if (VD.err == 1) {
		
		switch (mProductType) {             //�ѷ죺0x5380 ,������0x5381 , ��б��0x5383
		
		case 0x5380:
			printf("------mProductType=0x5380-liefeng------\n");
			if (-3.00 < VD.value && VD.value < 37.00) {
				
				dataCap_flag = 1;				//���������������һ����Ч���ݱ�־������־λ��1	
				PD.val_x = VD.value;
				printf("PD.val_x=%f\n", PD.val_x);
				Err_count = 0;
			}else {
			
				err = 0;			//������Ч���ñ�־λ
				printf("DATA overflow  Err_count\n");
				Err_count++;
				return -1;
			}
			break;
		case 0x5381:
			printf("------mProductType=0x5381-chenjiang------\n");
			if (50.00 < VD.value_x && VD.value_x < 190.00) {
#if 0
				/*****************ʹ�����ϳ���**********************/
				dataCap_flag = 1;			//���������������һ����Ч���ݱ�־������־λ��1
				PD.val_x = VD.value;
				printf("PD.val_x=%f\n", PD.val_x);
				Err_count = 0;
#else
				/*************�������³���***************/
				if (0 == num_flag) {
					
					DATA_CJ = kalman_push(num_flag, VD.value_x);
					num_flag = 1;
				}else {
					
					DATA_CJ = kalman_push(num_flag, VD.value_x);
				}
				
				dataCap_flag = 1;			//���������������һ����Ч���ݱ�־������־λ��1
				data = ceilf(DATA_CJ.value_x * 50);//��������0.02
				PD.val_x = (float)data / 50;
				data = ceilf(DATA_CJ.value_y * 50);
				PD.val_y = (float)data / 50;
				PD.avg_x = PD.val_x;
				PD.avg_y = PD.val_y;
				printf("PD.val_x = %f\n", PD.val_x);
				printf("PD.val_y = %f\n", PD.val_y);
				Err_count = 0;
#endif
			}else {
				
				err = 0;			//������Ч���ñ�־λ
				printf("DATA overflow  Err_count\n");
				Err_count++;
				return -1;
			}
			break;
		case 0x5383:
			printf("------mProductType=0x5383-qingxie------\n");
			if ((-60.0 < VD.value_x) && (VD.value_x < 60.0) && (-60.0 < VD.value_y) && (VD.value_y < 60.0)) {
				
				dataCap_flag = 1;		   //���������������һ����Ч���ݱ�־������־λ��1
				PD.val_x = VD.value_x;
				PD.val_y = VD.value_y;
				PD = DataProcess(PD);		//��ֵ�˲�
				
				printf("PD.val_x = %f\n", PD.val_x);
				printf("PD.val_y = %f\n", PD.val_y);
				Err_count = 0;			   //�������λ
			}else {
				
				err = 0;               //������Ч���ñ�־λ
				printf("DATA overflow  Err_count\n");
				Err_count++;
				return -1;
			}
			break;
		default:
			printf("Product Type ERROR\n");	
			break;
		}	
	}else {
		
		Err_count++;
		printf("VD.err = %d\n", VD.err);
		return -1;
	}
		
			
	return 0;
}