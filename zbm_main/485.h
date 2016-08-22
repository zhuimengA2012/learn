#ifndef __COMM_485__
#define __COMM_485__

#define ERR_OK			0x00	//����
#define ERR_MAC			0x21	//�������������豸�����쳣�����빩�粻�ȣ��¶ȹ��ߵ�
#define ERR_DATA		0x22	//���ݴ����㷨��ͼ��������󣬻ش���Ϊ��������
#define ERR_NO_DATA		0x24	//�豸���ϣ�����Ҳû������
#define ERR_UNKNOW_CMD	0x23	//δ֪����
#define ERR_PARAM		0x25	//��������
#define ERR_WOKR_MODE   0x26	//����ģʽ����

unsigned char err;

//#define ERR_PARAM 0x02 //��������
//#define ERR_UNKNOW_CMD 0x01 //δ֪����
#define ERR_NOT_READY  0x09
#define ERR_NO_MORE_DATA    0x0a
 

#define REG_PRODUCT_VER    0  //�̼��汾��
#define REG_PRODUCT_SN     2  //���к�
#define REG_PROTOCOL_VER   6  //ͨѶЭ��汾��
#define REG_ADDR           8  //�豸��ַ
#define REG_BAUD           9  //�豸ͨѶ������
#define REG_RELATIVE_FLAG  10 //�������־λ
#define REG_RELATIVE_ZERO  11 //������ƫ��
//������������
#define REG_DATA_DISTANCE  15 //������λ��,���궨���ʵ��ֵ
#define REG_DATA_TEMP      19 //�������¶�

float cache_data_x;				//��бx
float cache_data_y;				//��бy
float cache_data;					//�ѷ죬����

typedef union
{
	struct
	{ 
		unsigned short header;          //֡��ʼ��������һ֡���ݵĿ�ʼ���̶�Ϊ0xA5A5��
		unsigned char  packet_id;       //֡ID�������ж�����֡������֡�Ƿ�ƥ�䣬ÿ��ͨѶ���Զ���һ��
		unsigned char  source;		    //Դ��ַ��ͨѶ����֡���𷽵ĵ�ַ,������ַ�̶�Ϊ0x00��
		unsigned char  destination;	    //Ŀ���ַ��ͨѶ����֡���շ��ĵ�ַ��
		unsigned char  cmd;			    //ָ�����������������ִ֡�еĲ�����
		unsigned char  dlen;			//���������ȣ����������ȣ���ֵ��ָ����仯��
		unsigned char  data_buf[255];	//������������ָ��������ݺͳ��ȶ���Ӧ�ı仯����󳤶�Ϊ255���ֽڣ������������ĵ�һ���ֽ�Ϊ�������
		
		//����ʱ��ʱ����
		unsigned short  cnt;   //֡���ȼ���
		unsigned short *pCRC; //CRCָ��
		unsigned char  data_bnt;//����������
	};									//��������ʽԼ�����ȷ��͸�λ���ݣ�MSB�����ٷ��͵�λ���ݣ�LSB���������С������ȫ�����ñ�׼float���͡�
	unsigned char buf[280];
}FRAME;

extern FRAME tx_frame, rx_frame;
extern unsigned char reg[128];

/********************************data_485��غ���**************************************/
void COMM_Init(void);
void signal_handler_IO();
int read_port(unsigned char *data, int datalength);
void FramePase();
void FramePase_old();
unsigned short CRCCheck(unsigned char *str_, unsigned short crc_cd);
void SendData();
void uart_send_date(unsigned char *p_send, unsigned short send_num);
void PrintBuf(unsigned char *buf, int cnt);
void read_data(void);
void set_data(void);

/******************************comm_485��غ���****************************************/
void cmd_note(int priority);
unsigned char CMD(unsigned char cmd, unsigned char *rx_data_buf, unsigned char rx_data_cnt, unsigned char *tx_data_buf, unsigned char *tx_data_cnt);
unsigned char CMD_QueryData(unsigned char *rx_data_buf, unsigned char rx_data_cnt, unsigned char *tx_data_buf, unsigned char *tx_data_cnt);
unsigned char data_validity_check(unsigned char *rx_data_buf, unsigned char rx_data_cnt, unsigned char *tx_data_buf, unsigned char *tx_data_cnt);
unsigned char CMD_QueryAllData(unsigned char *rx_data_buf, unsigned char rx_data_cnt, unsigned char *tx_data_buf, unsigned char *tx_data_cnt);
unsigned char CMD_QueryRelativeFlag(unsigned char *rx_data_buf, unsigned char rx_data_cnt, unsigned char *tx_data_buf, unsigned char *tx_data_cnt);
unsigned char CMD_SetRelativeFlag(unsigned char *rx_data_buf, unsigned char rx_data_cnt, unsigned char *tx_data_buf, unsigned char *tx_data_cnt);
unsigned char CMD_QueryAddr(unsigned char *rx_data_buf, unsigned char rx_data_cnt, unsigned char *tx_data_buf, unsigned char *tx_data_cnt);
unsigned char CMD_SetAddr(unsigned char *rx_data_buf, unsigned char rx_data_cnt, unsigned char *tx_data_buf, unsigned char *tx_data_cnt);
unsigned char CMD_SetBaud(unsigned char *rx_data_buf, unsigned char rx_data_cnt, unsigned char *tx_data_buf, unsigned char *tx_data_cnt);
unsigned char CMD_QueryProductType(unsigned char *rx_data_buf, unsigned char rx_data_cnt, unsigned char *tx_data_buf, unsigned char *tx_data_cnt);
unsigned char CMD_SetProductSN(unsigned char *rx_data_buf, unsigned char rx_data_cnt, unsigned char *tx_data_buf, unsigned char *tx_data_cnt);
unsigned char CMD_QueryProtocolVer(unsigned char *rx_data_buf, unsigned char rx_data_cnt, unsigned char *tx_data_buf, unsigned char *tx_data_cnt);
unsigned char CMD_QueryProductVer(unsigned char *rx_data_buf, unsigned char rx_data_cnt, unsigned char *tx_data_buf, unsigned char *tx_data_cnt);
unsigned char CMD_QueryProductSN(unsigned char *rx_data_buf, unsigned char rx_data_cnt, unsigned char *tx_data_buf, unsigned char *tx_data_cnt);
unsigned char CMD_SaveConfig(unsigned char *rx_data_buf, unsigned char rx_data_cnt, unsigned char *tx_data_buf, unsigned char *tx_data_cnt);
unsigned char CMD_Reboot(unsigned char *rx_data_buf, unsigned char rx_data_cnt, unsigned char *tx_data_buf, unsigned char *tx_data_cnt);
unsigned char CMD_LoadFactory(unsigned char *rx_data_buf, unsigned char rx_data_cnt, unsigned char *tx_data_buf, unsigned char *tx_data_cnt);
unsigned char CMD_GetPic(unsigned char *rx_data_buf, unsigned char rx_data_cnt, unsigned char *tx_data_buf, unsigned char *tx_data_cnt);
unsigned char CMD_SetFactoryKey(unsigned char *rx_data_buf, unsigned char rx_data_cnt, unsigned char *tx_data_buf, unsigned char *tx_data_cnt);
unsigned char CMD_QueryConfig(unsigned char *rx_data_buf, unsigned char rx_data_cnt, unsigned char *tx_data_buf, unsigned char *tx_data_cnt);
unsigned char CMD_WriteConfig(unsigned char *rx_data_buf, unsigned char rx_data_cnt, unsigned char *tx_data_buf, unsigned char *tx_data_cnt);
unsigned char CMD_QueryCalibration(unsigned char *rx_data_buf, unsigned char rx_data_cnt, unsigned char *tx_data_buf, unsigned char *tx_data_cnt);
unsigned char CMD_FindPic(unsigned char *rx_data_buf, unsigned char rx_data_cnt, unsigned char *tx_data_buf, unsigned char *tx_data_cnt);
unsigned char CMD_FindPic_num(unsigned char *rx_data_buf, unsigned char rx_data_cnt, unsigned char *tx_data_buf, unsigned char *tx_data_cnt);
unsigned char CMD_UpLoadPic(unsigned char *rx_data_buf, unsigned char rx_data_cnt, unsigned char *tx_data_buf, unsigned char *tx_data_cnt);
unsigned char CMD_Msg_Pic(unsigned char *rx_data_buf, unsigned char rx_data_cnt, unsigned char *tx_data_buf, unsigned char *tx_data_cnt);
unsigned char CMD_WriteCalibration(unsigned char *rx_data_buf, unsigned char rx_data_cnt, unsigned char *tx_data_buf, unsigned char *tx_data_cnt);
unsigned char CMD_Debug(unsigned char *rx_data_buf, unsigned char rx_data_cnt, unsigned char *tx_data_buf, unsigned char *tx_data_cnt);
unsigned char CMD_QueryWorkMode(unsigned char *rx_data_buf, unsigned char rx_data_cnt, unsigned char *tx_data_buf, unsigned char *tx_data_cnt);
unsigned char CMD_Bootloader(unsigned char *rx_data_buf, unsigned char rx_data_cnt, unsigned char *tx_data_buf, unsigned char *tx_data_cnt);

#endif