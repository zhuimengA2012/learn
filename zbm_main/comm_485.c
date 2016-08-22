#include "485.h"
#include "set_485.h"
#include "head.h"

const unsigned int   mFactoryKey = 0x66554433; //������Կ
unsigned int   mFactoryKey1;
unsigned int   mFactoryKey_temp;
const unsigned int  mProductVer = 0x01020309;	//�̼��汾1.2.3.9    //485�����ѯ���İ汾��
unsigned int  mProductVer1;
unsigned short mProductType = 0x00;  //��б��0x5383 ,�ѷ죺0x5380 ,������0x5381
unsigned short mProductType1;
const unsigned short mProtocolVer = 0x0103;  //Э��汾 1.3
unsigned short mProtocolVer1;

int dataCap_flag;      //�������������һ����Ч���ݱ�־
int dataErr_flag;      //ͼ��ʶ����������10�α�־

unsigned int   ProductSN;   //Serial Number
unsigned int   ProductSN1;
unsigned char reg[128];
int priority_flag = 0;

unsigned char mFactoryMod;
extern struct ProcesData PD;
extern struct VedioData VD;
extern FILE* fd_485_note;
extern FRAME rx_frame;
extern int wdog_fd;
extern int save_image_flag;
extern sqlite3 *sqlp;

/********************485�ӿڳ�ʼ��*************************/
void CMD_Init(void)
{

	unsigned short sh;   //protocol  version
	unsigned short sh1;

	dataCap_flag = 0;
	dataErr_flag = 0;
		
	mFactoryMod = 0;
	reg[REG_ADDR] = 1;
	mFactoryKey_temp = 0;
	mFactoryKey1 = htonl(mFactoryKey);
	mProductVer1 = htonl(mProductVer);
	mProtocolVer1 = htons(mProtocolVer);
	mProductType1 = htons(mProductType);
	
	/**********��������������ȼ�����*************/
	priority_flag = 0;
	zbm_ini_read(INI, "ZBM_CONFIG", "SN", &ProductSN);
	printf("SN = %d\n", ProductSN);
	ProductSN1 = htonl(ProductSN);
	memcpy(reg + REG_PRODUCT_SN, &ProductSN1, 4);
	//���ݸ���ʾ��
	
	return;
}

/********************************�����ļ��Ĵ�С*********************************/
unsigned long get_file_size(const char *path)    
{    
	unsigned long filesize = 0;        
	struct stat statbuff;    
	
	if (stat(path, &statbuff) < 0) {    
		
		return filesize;    
	}else {    
		
		filesize = statbuff.st_size;    
	}    
	
	return filesize;    
} 

/********************��¼ָ��������Ϣ,priority��ֵԽ�����ȼ�Խ��***************************/
void cmd_note(int priority)
{
	time_t my_time;
	unsigned char buf_time[128] = {0};
	int i = 0;
	unsigned long size_485_note = 0;
	
	size_485_note = get_file_size("/opt/ZBM/proc/485cmd_note");
	if (size_485_note >= 20971520) {      //20971520��20M
		fclose(fd_485_note);
		system("mv /opt/ZBM/proc/485cmd_note /opt/ZBM/proc/485cmd_note.old");
		fd_485_note = fopen("/opt/ZBM/proc/485cmd_note", "a+");
		if (NULL == fd_485_note) {
		
			perror("fd_485_note");
			close_v4l2();
			exit(1);
		}
	}
	if (priority > priority_flag) {
		time(&my_time);
		sprintf(buf_time, "time:%s", ctime(&my_time));
		
		fwrite(buf_time, 1, strlen(buf_time) - 1, fd_485_note);
		
		fprintf(fd_485_note, "%s", " -- cmd:");
		for (i = 0;i < rx_frame.cnt;i++) {
		
			fprintf(fd_485_note,"%02x", rx_frame.buf[i]);
		}
		fprintf(fd_485_note, "\n");
		fflush(fd_485_note);
		//sync();     //sync�˴������ã��ȽϺ�ʱ��
	}
	
	return ;
}

unsigned char CMD(unsigned char cmd, unsigned char *rx_data_buf, unsigned char rx_data_cnt, unsigned char *tx_data_buf, unsigned char *tx_data_cnt)
{
	unsigned char rtn = ERR_OK;
	switch (cmd)
	{
			//��ѡ����
	case 0x01:
		rtn = CMD_QueryData(rx_data_buf, rx_data_cnt, tx_data_buf, tx_data_cnt);
		rtn = data_validity_check(rx_data_buf, rx_data_cnt, tx_data_buf, tx_data_cnt);
		break;
	case 0x02:
		rtn = CMD_QueryAllData(rx_data_buf, rx_data_cnt, tx_data_buf, tx_data_cnt); //��ѯ������ȫ������
		break;
	case 0x03:
		rtn = CMD_QueryRelativeFlag(rx_data_buf, rx_data_cnt, tx_data_buf, tx_data_cnt);
		break;
	case 0x04:
		rtn = CMD_SetRelativeFlag(rx_data_buf, rx_data_cnt, tx_data_buf, tx_data_cnt);
		break;
	case 0x05:
		rtn = CMD_QueryAddr(rx_data_buf, rx_data_cnt, tx_data_buf, tx_data_cnt);
		break;
	case 0x06:
		rtn = CMD_SetAddr(rx_data_buf, rx_data_cnt, tx_data_buf, tx_data_cnt);
		break;
	case 0x07:
		rtn = CMD_SetBaud(rx_data_buf, rx_data_cnt, tx_data_buf, tx_data_cnt);
		break;
	case 0x10:
		rtn = CMD_QueryProductType(rx_data_buf, rx_data_cnt, tx_data_buf, tx_data_cnt); //��ѯ����������
		break;
	case 0x11:
		rtn = CMD_SetProductSN(rx_data_buf, rx_data_cnt, tx_data_buf, tx_data_cnt);
		break;
	case 0x0A:
		rtn = CMD_QueryProtocolVer(rx_data_buf, rx_data_cnt, tx_data_buf, tx_data_cnt);
		break;
	case 0x0B:
		rtn = CMD_QueryProductVer(rx_data_buf, rx_data_cnt, tx_data_buf, tx_data_cnt);  //��ѯ�̼��汾��
		break;
	case 0x0C:
		rtn = CMD_QueryProductSN(rx_data_buf, rx_data_cnt, tx_data_buf, tx_data_cnt);   //��ѯ���к�
		break;		
	case 0x0D:
		rtn = CMD_SaveConfig(rx_data_buf, rx_data_cnt, tx_data_buf, tx_data_cnt);
		break;
	case 0x0E:
		rtn = CMD_Reboot(rx_data_buf, rx_data_cnt, tx_data_buf, tx_data_cnt);
		break;
	case 0x0F:
		rtn = CMD_LoadFactory(rx_data_buf, rx_data_cnt, tx_data_buf, tx_data_cnt);
		break;
	case 0x50:
		rtn = CMD_GetPic(rx_data_buf, rx_data_cnt, tx_data_buf, tx_data_cnt);
		break;
//����Ϊ������ָ��
	case 0xE0: 
		rtn = CMD_SetFactoryKey(rx_data_buf, rx_data_cnt, tx_data_buf, tx_data_cnt); //���빤����Կ
		break;
	case 0xE1:
		rtn = CMD_QueryConfig(rx_data_buf, rx_data_cnt, tx_data_buf, tx_data_cnt); //��ȡ������Ϣ
		break;
	case 0xE2:
		rtn = CMD_WriteConfig(rx_data_buf, rx_data_cnt, tx_data_buf, tx_data_cnt); //д��������Ϣ
		break;
	case 0xE3:
		rtn = CMD_QueryCalibration(rx_data_buf, rx_data_cnt, tx_data_buf, tx_data_cnt); //��ȡ�궨��Ϣ
		break;
	case 0xE4:
		rtn = CMD_WriteCalibration(rx_data_buf, rx_data_cnt, tx_data_buf, tx_data_cnt); //д��궨��Ϣ
		break;
	case 0xE5:
		rtn = CMD_FindPic_num(rx_data_buf, rx_data_cnt, tx_data_buf, tx_data_cnt); 
		break;
	case 0xE6:
		rtn = CMD_FindPic(rx_data_buf, rx_data_cnt, tx_data_buf, tx_data_cnt); //��ѯͼƬ��Ϣ
		break;
	case 0xE7:
		rtn = CMD_Msg_Pic(rx_data_buf, rx_data_cnt, tx_data_buf, tx_data_cnt); 
		break;
	case 0xE8:
		rtn = CMD_UpLoadPic(rx_data_buf, rx_data_cnt, tx_data_buf, tx_data_cnt); 
		break;
	case 0xEE:
		rtn = CMD_Debug(rx_data_buf, rx_data_cnt, tx_data_buf, tx_data_cnt);
		break;
	case 0xEF:
		rtn = CMD_QueryWorkMode(rx_data_buf, rx_data_cnt, tx_data_buf, tx_data_cnt);
		break;
	case 0xF0:
		rtn = CMD_Bootloader(rx_data_buf, rx_data_cnt, tx_data_buf, tx_data_cnt);
		break;
	default:
		rtn = ERR_UNKNOW_CMD;
		break;
	}
	return rtn;
}

/**********************************��ѯ�������������ݲ��ж�****************************************/
unsigned char CMD_QueryData(unsigned char *rx_data_buf, unsigned char rx_data_cnt, unsigned char *tx_data_buf, unsigned char *tx_data_cnt)
{
	unsigned char l2b_tmp1[4];
	unsigned char l2b_tmp2[4];
	int num;
	long int my_s, my_us;
	
	
	if (rx_data_cnt != 1) {
		
		return ERR_PARAM;
	}
	tx_data_buf[0] = rx_data_buf[0]; //��������ID��
	
	cmd_note(5);
	switch (rx_data_buf[0])
	{
	case 0://0������
		if (mProductType == 0x5383 || mProductType == 0x5381)	//��б��0x5383  ���� ��5381
		{
			memcpy(l2b_tmp1, &PD.avg_x, 4);
			memcpy(l2b_tmp2, &PD.avg_y, 4);
			for (num = 0;num < 4;num++) {
				
				reg[REG_DATA_DISTANCE + num] = l2b_tmp1[3 - num];
				reg[REG_DATA_DISTANCE + 4 + num] = l2b_tmp2[3 - num];
			}  
			memcpy(tx_data_buf + 1, reg + REG_DATA_DISTANCE, 4 + 4);
			*tx_data_cnt = 4 + 1 + 4;
		}else{
			
			memcpy(l2b_tmp1, &PD.val_x, 4);
			for (num = 0;num < 4;num++) {
				
				reg[REG_DATA_DISTANCE + num] = l2b_tmp1[3 - num];
			}  
			memcpy(tx_data_buf + 1, reg + REG_DATA_DISTANCE, 4);
			*tx_data_cnt = 4 + 1;
		}
		break;	
	case 1://1������
		if (mProductType == 0x5383 || mProductType == 0x5381)	//��б��0x5383
		{
			memcpy(l2b_tmp1, &PD.val_x, 4);
			memcpy(l2b_tmp2, &PD.val_y, 4);
			for (num = 0;num < 4;num++) {
				
				reg[REG_DATA_DISTANCE + num] = l2b_tmp1[3 - num];
				reg[REG_DATA_DISTANCE + 4 + num] = l2b_tmp2[3 - num];
			}  
			memcpy(tx_data_buf + 1, reg + REG_DATA_DISTANCE, 4 + 4);
			*tx_data_cnt = 4 + 1 + 4;
			break;
		}else {
			
			memcpy(tx_data_buf + 1, reg + REG_DATA_DISTANCE, 4);
			*tx_data_cnt = 4 + 1;
			return ERR_PARAM;
		}
	default:
		if (mProductType == 0x5383 || mProductType == 0x5381)	//��б��0x5383
		{
			memcpy(tx_data_buf + 1, reg + REG_DATA_DISTANCE, 4 + 4);
			*tx_data_cnt = 4 + 1 + 4;
			return ERR_PARAM;
		}
		else {
			memcpy(tx_data_buf + 1, reg + REG_DATA_DISTANCE, 4);
			*tx_data_cnt = 4 + 1;
			return ERR_PARAM;
		}
	}
	
	if (dataErr_flag == 0  && dataCap_flag == 1) {
		
		return ERR_OK;
	}
	else
		return ERR_PARAM;
	return 0;
}
unsigned char data_validity_check(unsigned char *rx_data_buf, unsigned char rx_data_cnt, unsigned char *tx_data_buf, unsigned char *tx_data_cnt)
{
	unsigned char rtn = 0, num;
	unsigned char l2b_tmp1[4];
	unsigned char l2b_tmp2[4];
	if (1 == err) {
		
		rtn = ERR_OK;	//	������ʵ��Ч
	}else {
		rtn = ERR_DATA;	//	���ݿ��õ�����ʵ
		if (PD.val_x == 0) {
			read_data();
		}else {
			cache_data_x = PD.avg_x;
			cache_data_y = PD.avg_y;
			cache_data = PD.val_x;
		}
		
		memcpy(l2b_tmp1, &cache_data_x, 4);
		memcpy(l2b_tmp2, &cache_data_y, 4);
		if (mProductType == 0x5383 || mProductType == 0x5381)	//��б��0x5383
		{
			for (num = 0;num < 4;num++) {
				
				reg[REG_DATA_DISTANCE + num] = l2b_tmp1[3 - num];
				reg[REG_DATA_DISTANCE + 4 + num] = l2b_tmp2[3 - num];
			}	
			memcpy(tx_data_buf + 1, reg + REG_DATA_DISTANCE, 4 + 4);
			*tx_data_cnt = 4 + 1 + 4;

			if (cache_data_x == 0 && cache_data_y == 0) {
				rtn = ERR_NO_DATA;	//	������Ч
			}
		}else {
				
			memcpy(l2b_tmp1, &cache_data, 4);
			for (num = 0;num < 4;num++) {
				
				reg[REG_DATA_DISTANCE + num] = l2b_tmp1[3 - num];
			}  
			memcpy(tx_data_buf + 1, reg + REG_DATA_DISTANCE, 4);
			*tx_data_cnt = 4 + 1;

			if (cache_data == 0) {
				
				rtn = ERR_NO_DATA;	//	������Ч
			}
		}
		
	}

	return rtn;
}

/*****************************************video�а�ť�ֱ��Ӧָ��**********************************************/

/**************************************��ѯ������ȫ������***************************************/
unsigned char CMD_QueryAllData(unsigned char *rx_data_buf, unsigned char rx_data_cnt, unsigned char *tx_data_buf, unsigned char *tx_data_cnt)
{
	//���������ʲô���ܣ�ȫ����Ϣ��
	unsigned char temp1[4];
	unsigned char temp2[4];
	unsigned char temp3[4];
	unsigned char temp4[4];
	
	unsigned char copytemp1[4];
	unsigned char copytemp2[4];
	unsigned char copytemp3[4];
	unsigned char copytemp4[4];
	int num;
	
	if (rx_data_cnt != 0) {
		
		return ERR_PARAM;
	}
	cmd_note(5);
	if (mProductType == 0x5383)	//��б��0x5383
	{
		memcpy(temp1, &PD.avg_x, 4);
		memcpy(temp2, &PD.avg_y, 4);
		memcpy(temp3, &PD.val_x, 4);
		memcpy(temp4, &PD.val_y, 4);
		
		for (num = 0;num < 4;num++) {
			
			copytemp1[num] = temp1[3 - num];
			copytemp2[num] = temp2[3 - num];
			copytemp3[num] = temp3[3 - num];
			copytemp4[num] = temp4[3 - num];
		}
		memcpy(tx_data_buf, copytemp1, 4);
		memcpy(tx_data_buf + 4, copytemp2, 4);
		memcpy(tx_data_buf + 4 + 4, copytemp3, 4);
		memcpy(tx_data_buf + 4 + 4 + 4, copytemp4, 4);
		*tx_data_cnt = 4 + 4 * 3;
	}
	else {
		memcpy(temp1, &VD.value, 4);
		for (num = 0;num < 4;num++) {
			
			copytemp1[num] = temp1[3 - num];
		}  
		memcpy(tx_data_buf, copytemp1, 4);
		*tx_data_cnt = 4;
	}
	if (dataErr_flag == 0  && dataCap_flag == 1) {
		
		return ERR_OK;
	}
	else
		return ERR_PARAM;

	return 0;
}

/**************************************��ѯ��������־**************************************/		//��������־��ʲô��˼��
unsigned char CMD_QueryRelativeFlag(unsigned char *rx_data_buf, unsigned char rx_data_cnt, unsigned char *tx_data_buf, unsigned char *tx_data_cnt)
{
	if (rx_data_cnt != 0) {
		
		return ERR_PARAM;
	}
	cmd_note(5);
	tx_data_buf[0] = reg[REG_RELATIVE_FLAG];
	*tx_data_cnt = 1;

	return ERR_OK;
}

/********************************************������������־************************************/
unsigned char CMD_SetRelativeFlag(unsigned char *rx_data_buf, unsigned char rx_data_cnt, unsigned char *tx_data_buf, unsigned char *tx_data_cnt)
{
	if (rx_data_cnt != 1) {
		
		return ERR_PARAM;
	}
	cmd_note(5);
	if (rx_data_buf[0]) {
		
		reg[REG_RELATIVE_FLAG] = 1;
	}
	else {
		reg[REG_RELATIVE_FLAG] = 0;
	}
	tx_data_buf[0] = reg[REG_RELATIVE_FLAG];
	*tx_data_cnt = 1;
	return ERR_OK;
}

/***************************************�����豸ͨѶ��ַ******************************************/
unsigned char CMD_QueryAddr(unsigned char *rx_data_buf, unsigned char rx_data_cnt, unsigned char *tx_data_buf, unsigned char *tx_data_cnt)
{
	if (rx_data_cnt != 0) {
		
		return ERR_PARAM;
	}
	cmd_note(5);
	if (rx_data_buf[0] == 0xff || rx_data_buf[0] == 0) {
		
		return ERR_PARAM;
	}
	tx_data_buf[0] = reg[REG_ADDR];
	*tx_data_cnt = 1;
	
	return ERR_OK;
}

/**************************************�����豸ͨѶ��ַ*****************************************/
unsigned char CMD_SetAddr(unsigned char *rx_data_buf, unsigned char rx_data_cnt, unsigned char *tx_data_buf, unsigned char *tx_data_cnt)
{
	if (rx_data_cnt != 1) {
		
		return ERR_PARAM;
	}
	cmd_note(5);
	if (rx_data_buf[0] == 0xff || rx_data_buf[0] == 0) {
		
		return ERR_PARAM;
	}
	
	reg[REG_ADDR] = rx_data_buf[0];
	zbm_ini_write(INI, "ZBM_CONFIG", "ID", reg[REG_ADDR]);
	sync();
	tx_data_buf[0] = reg[REG_ADDR];
	*tx_data_cnt = 1;
	
	return ERR_OK;
}

/****************************************�����豸ͨѶ������**************************************/
unsigned char CMD_SetBaud(unsigned char *rx_data_buf, unsigned char rx_data_cnt, unsigned char *tx_data_buf, unsigned char *tx_data_cnt)
{
	if (rx_data_cnt != 1) {
		
		return ERR_PARAM;
	}
	cmd_note(5);
	if (rx_data_buf[0] > 4) {
		
		return ERR_PARAM;
	}
	reg[REG_BAUD] = rx_data_buf[0];
	tx_data_buf[0] = reg[REG_BAUD];
	*tx_data_cnt = 1;
	
	return ERR_OK;
}

/********************************************��ѯ����������************************************/
unsigned char CMD_QueryProductType(unsigned char *rx_data_buf, unsigned char rx_data_cnt, unsigned char *tx_data_buf, unsigned char *tx_data_cnt)
{
	if (rx_data_cnt != 0) {
		
		return ERR_PARAM;
	}
	cmd_note(5);
	memcpy(tx_data_buf, &mProductType1, 2);
	*tx_data_cnt = 2;
	
	return ERR_OK;
}

/********************************************���ò�ƷSN************************************/
unsigned char CMD_SetProductSN(unsigned char *rx_data_buf, unsigned char rx_data_cnt, unsigned char *tx_data_buf, unsigned char *tx_data_cnt)
{
	if (rx_data_cnt != 4) {
		
		return ERR_PARAM;
	}
	cmd_note(5);
	memcpy(tx_data_buf, rx_data_buf, 4);
	*tx_data_cnt = 4;     
	if (mFactoryMod == 1)   //mFactoryMod����1ʱ��ʲô�����
	{
		memcpy(&ProductSN1, rx_data_buf, 4);
		ProductSN = ntohl(ProductSN1);
		zbm_ini_write(INI, "ZBM_CONFIG", "SN", ProductSN);
		sync();
		return ERR_OK;
	}
	else {
		return ERR_WOKR_MODE;
	}
	return 0;
}

/*******************************************��ѯЭ��汾��****************************************/
unsigned char CMD_QueryProtocolVer(unsigned char *rx_data_buf, unsigned char rx_data_cnt, unsigned char *tx_data_buf, unsigned char *tx_data_cnt)
{
	if (rx_data_cnt != 0) {
		
		return ERR_PARAM;
	}
	cmd_note(5);
	memcpy(tx_data_buf, &mProtocolVer1, 2);
	*tx_data_cnt = 2;
	
	return ERR_OK;	
}

/*****************************************��ѯ�̼��汾��**********************************/
unsigned char CMD_QueryProductVer(unsigned char *rx_data_buf, unsigned char rx_data_cnt, unsigned char *tx_data_buf, unsigned char *tx_data_cnt)
{
	if (rx_data_cnt != 0) {
		
		return ERR_PARAM;
	}
	cmd_note(5);
	memcpy(tx_data_buf, &mProductVer1, 4);
	*tx_data_cnt = 4;
	
	return ERR_OK;
}

/***********************************************��ѯ���к�*************************************/
unsigned char CMD_QueryProductSN(unsigned char *rx_data_buf, unsigned char rx_data_cnt, unsigned char *tx_data_buf, unsigned char *tx_data_cnt)
{

	if (rx_data_cnt != 0) {
		
		return ERR_PARAM;
	}
	cmd_note(5);
		//memcpy(tx_data_buf,reg+REG_PRODUCT_SN,4);
	memcpy(tx_data_buf, &ProductSN1, 4);
	*tx_data_cnt = 4;
	
	return ERR_OK;
}

/***************************************�����趨��������**************************************/
unsigned char CMD_SaveConfig(unsigned char *rx_data_buf, unsigned char rx_data_cnt, unsigned char *tx_data_buf, unsigned char *tx_data_cnt)
{
	if (rx_data_cnt != 0) {
		
		return ERR_PARAM;
	}
	cmd_note(5);
	*tx_data_cnt = 0;
	return ERR_OK;
}

/******************************************�豸��������**********************************/
unsigned char CMD_Reboot(unsigned char *rx_data_buf, unsigned char rx_data_cnt, unsigned char *tx_data_buf, unsigned char *tx_data_cnt)
{
	if (rx_data_cnt != 0) {
		
		return ERR_PARAM;
	}
	cmd_note(5);
		//NVIC_SystemReset();
	system("reboot");
	*tx_data_cnt = 1;
	
	return ERR_OK;
}

/****************************************�ָ���������****************************************/
unsigned char CMD_LoadFactory(unsigned char *rx_data_buf, unsigned char rx_data_cnt, unsigned char *tx_data_buf, unsigned char *tx_data_cnt)
{
	if (rx_data_cnt != 0) {
		return ERR_PARAM;
	}
	cmd_note(5);
	*tx_data_cnt = 0;
	return ERR_OK;
}

/*******************485��ͼƬ**********************/
unsigned char CMD_GetPic(unsigned char *rx_data_buf, unsigned char rx_data_cnt, unsigned char *tx_data_buf, unsigned char *tx_data_cnt)
{
	if (rx_data_cnt != 0) {
		
		return ERR_PARAM;
	}
	cmd_note(5);
	save_image_flag = 1;
	return ERR_OK;
}

/******************************************�趨������Կ*************************************/
unsigned char CMD_SetFactoryKey(unsigned char *rx_data_buf, unsigned char rx_data_cnt, unsigned char *tx_data_buf, unsigned char *tx_data_cnt)
{
	if (rx_data_cnt != 4) {
		
		return ERR_PARAM;
	}
	cmd_note(5);
	memcpy(tx_data_buf, rx_data_buf, 4);
	*tx_data_cnt = 4;
	if (memcmp(rx_data_buf, &mFactoryKey1, 4) == 0) //ƥ��     
	{
		mFactoryMod = 1;
		return ERR_OK;
	}else {
		mFactoryMod = 0;
		memcpy(&mFactoryMod, rx_data_buf, 4);
		return ERR_PARAM;
	}
	return 0;
}

/*********************************************************************************************/
unsigned char CMD_QueryConfig(unsigned char *rx_data_buf, unsigned char rx_data_cnt, unsigned char *tx_data_buf, unsigned char *tx_data_cnt)
{
	if (!mFactoryMod) {
		
		return ERR_UNKNOW_CMD;
	}
	if (rx_data_cnt != 0) {
		
		return ERR_PARAM;
	}
	cmd_note(5);
	*tx_data_cnt = 1;
	return ERR_OK;
}

/*********************************************************************************************/
unsigned char CMD_WriteConfig(unsigned char *rx_data_buf, unsigned char rx_data_cnt, unsigned char *tx_data_buf, unsigned char *tx_data_cnt)
{
	if (!mFactoryMod) {
		
		return ERR_UNKNOW_CMD;
	}
	if (rx_data_cnt != 0) {
		
		return ERR_PARAM;
	}
	cmd_note(5);
	*tx_data_cnt = 1;
	return ERR_OK;
}

/*********************************************************************************************/
unsigned char CMD_QueryCalibration(unsigned char *rx_data_buf, unsigned char rx_data_cnt, unsigned char *tx_data_buf, unsigned char *tx_data_cnt)
{
	if (!mFactoryMod) {
		
		return ERR_UNKNOW_CMD;
	}
	if (rx_data_cnt != 0) {
		
		return ERR_PARAM;
	}
	cmd_note(5);
	*tx_data_cnt = 1;
	return ERR_OK;
}

/*********************************************************************************************/
unsigned char CMD_WriteCalibration(unsigned char *rx_data_buf, unsigned char rx_data_cnt, unsigned char *tx_data_buf, unsigned char *tx_data_cnt)
{
	if (!mFactoryMod) {
		
		return ERR_UNKNOW_CMD;
	}
	if (rx_data_cnt != 0) {
		
		return ERR_PARAM;
	}
	cmd_note(5);
	*tx_data_cnt = 1;
	return ERR_OK;
}

/**************************485��ѯͼƬ����*****************************/
unsigned char CMD_FindPic_num(unsigned char *rx_data_buf, unsigned char rx_data_cnt, unsigned char *tx_data_buf, unsigned char *tx_data_cnt)
{
	char buf_sql[128] = {0};
	char num = 0;

	if (!mFactoryMod) {
		
		return ERR_UNKNOW_CMD;
	}
	if (rx_data_cnt != 0) {
		
		return ERR_PARAM;
	}
	cmd_note(5);
	
	memcpy(&image_time_start[0], &rx_data_buf[0], 10);
	printf("image_time_start = %s\n", image_time_start);
	memcpy(&image_time_end[0], &rx_data_buf[10], 10);
	printf("image_time_end = %s\n", image_time_end);
	zbm_sqlite_find(sqlp, "image", image_time_start, image_time_end);
	if (image_row == 0){
		
		num = 0;
	}else{
		
		num = image_row - 1;
	}
	
	memcpy(&tx_data_buf[0], &num, 1);
	*tx_data_cnt = 1;
	
	return ERR_OK;
}

/**************************485��ѯͼƬ*****************************/
unsigned char CMD_FindPic(unsigned char *rx_data_buf, unsigned char rx_data_cnt, unsigned char *tx_data_buf, unsigned char *tx_data_cnt)
{
	int i, j, k = 0,cnt_num = 0;
	char num[3];
	int my_time = 0, pic_num = 0;
	
	if (!mFactoryMod) {
		
		return ERR_UNKNOW_CMD;
	}
	if (rx_data_cnt != 0) {
		
		return ERR_PARAM;
	}
	cmd_note(5);

	pic_num = rx_data_buf [0];
	
	for (j = (pic_num - 1) * 25;j < image_row;j++) {
		for (i = 2;i < 19;i++) {
		
			bzero(num, sizeof(num));
			memcpy(&num[0], &image_result[3 + (j + 1) * image_colunm][i], 2);
			i += 2;
			my_time = atoi(num); 
			tx_data_buf[k++] = my_time;
		}
		cnt_num++;   //�˴δ����ж�����ͼƬ
		if (cnt_num >= 25)
			break;
	}
	
	*tx_data_cnt = cnt_num * 6;
	
	return ERR_OK;
}

/*************************����ͼƬ��Ϣ********************************/
unsigned char CMD_Msg_Pic(unsigned char *rx_data_buf, unsigned char rx_data_cnt, unsigned char *tx_data_buf, unsigned char *tx_data_cnt)
{
	char buf_sql[128] = {0};
	char *errmsg;
	char ** result;
	int row, colunm;
	int fd_pic;
	char time_pic [6] = {0};
	unsigned char sout[MD5_HASHBYTES] = {0};  
	MD5_CTX ctx;
	
	if (!mFactoryMod) {
		
		return ERR_UNKNOW_CMD;
	}
	if (rx_data_cnt != 0) {
		
		return ERR_PARAM;
	}
	cmd_note(5);
	
	memset(&ctx, 0, sizeof(MD5_CTX));  
	sprintf(time_pic, "20%02d-%02d-%02d %02d:%02d:%02d", rx_data_buf [0], rx_data_buf [1], rx_data_buf [2], rx_data_buf [3], rx_data_buf [4], rx_data_buf [5]);
	zbm_sqlite_time_get_picture(sqlp, "image", time_pic);
	sprintf(buf_sql, "/opt/ZBM/pic/pic_save/%s", image_trans);
	size_picture = get_file_size(buf_sql);
	
	fd_pic = open(buf_sql, O_RDONLY);
	if (-1 == fd_pic) {
		
		perror("open picture");
		return ERR_PARAM;
	}
	file_all = (unsigned char *)malloc(size_picture);
	read(fd_pic, file_all, size_picture);
	/********************md5�����ı�У��**********************/
	bzero(sout, MD5_HASHBYTES);  
	MD5Init(&ctx);
	MD5Update(&ctx, (unsigned char *)file_all, size_picture);
	MD5Final((unsigned char *)sout, &ctx);

	memcpy(&tx_data_buf[0], &image_trans[0], strlen(image_trans));
	memcpy(&tx_data_buf[24], &size_picture, 4);
	memcpy(&tx_data_buf[28], &sout[0], 16);
	
	*tx_data_cnt = 44;
	close(fd_pic);
	return ERR_OK;
}

/*************************485�ϴ�ͼƬ********************************/
unsigned char CMD_UpLoadPic(unsigned char *rx_data_buf, unsigned char rx_data_cnt, unsigned char *tx_data_buf, unsigned char *tx_data_cnt)
{
	char buf_sql[128] = {0};
	unsigned short num; 
	int size_data = 0;
	
	if (!mFactoryMod) {
		
		return ERR_UNKNOW_CMD;
	}
	if (rx_data_cnt != 0) {
		
		return ERR_PARAM;
	}
	cmd_note(5);
	
	//time_flag = 2;      //��������ͼƬ�ʹ���ͼƬ
	
	memcpy(&num, &rx_data_buf[0],2);
	if (128 * (num - 1) > size_picture) {
		
		return ERR_PARAM;
	}
	if (128 * num < size_picture){
		
		size_data = 128;
	}else{
		
		size_data = size_picture - 128 * (num - 1);
	}
	switch (rx_data_buf[2]){
		
	case PIC_ACK:
		
		memcpy(&tx_data_buf[0], &file_all[(num - 1) * 128], size_data);
		*tx_data_cnt = size_data;
		break;
	case PIC_NAK:
		
		memcpy(&tx_data_buf[0], &file_all[(num - 1) * 128], size_data);
		*tx_data_cnt = size_data;
		break;
	case PIC_CAN:

		*tx_data_cnt = 0;
		break;
	default:
		printf("picture cmd is error\n");
		break;
	}
	
	if (128 * num >= size_picture || rx_data_buf[2] == PIC_CAN)	{
		
		free(file_all);
	}
	
	//time_flag = 0;     //��������ͼƬ������ʹ���
	
	return ERR_OK;
}


/*********************************************************************************************/
unsigned char CMD_Debug(unsigned char *rx_data_buf, unsigned char rx_data_cnt, unsigned char *tx_data_buf, unsigned char *tx_data_cnt)
{
	if (!mFactoryMod) {
		
		return ERR_UNKNOW_CMD;
	}
	if (rx_data_cnt != 0) {
		
		return ERR_PARAM;
	}
	cmd_note(5);
	*tx_data_cnt = 1;
	return ERR_OK;
}

/*********************************************************************************************/
unsigned char CMD_QueryWorkMode(unsigned char *rx_data_buf, unsigned char rx_data_cnt, unsigned char *tx_data_buf, unsigned char *tx_data_cnt)
{
	if (rx_data_cnt != 0) {
		
		return ERR_PARAM;
	}
	if (mFactoryMod == 1) {
		
		memcpy(tx_data_buf, &mFactoryMod, 4);
	}else {
		
		memcpy(tx_data_buf, &mFactoryMod, 4);
	}
	cmd_note(5);
	*tx_data_cnt = 4;
	return ERR_OK;
}

/****************************����Զ������ʱ�˳������򣬲�����bootloader**********************************/
unsigned char CMD_Bootloader(unsigned char *rx_data_buf, unsigned char rx_data_cnt, unsigned char *tx_data_buf, unsigned char *tx_data_cnt)
{
	if (!mFactoryMod) {
		
		return ERR_UNKNOW_CMD;
	}
	if (rx_data_cnt != 0) {
		
		return ERR_PARAM;
	}
	cmd_note(5);
	close_v4l2();
	system("bootloader &");
	exit(0);
	return ERR_OK;
}