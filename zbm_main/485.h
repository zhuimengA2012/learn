#ifndef __COMM_485__
#define __COMM_485__

#define ERR_OK			0x00	//正常
#define ERR_MAC			0x21	//数据正常，但设备工作异常，列入供电不稳，温度过高等
#define ERR_DATA		0x22	//数据错误，算法或图像输出错误，回传的为缓存数据
#define ERR_NO_DATA		0x24	//设备故障，缓存也没有数据
#define ERR_UNKNOW_CMD	0x23	//未知命令
#define ERR_PARAM		0x25	//参数错误
#define ERR_WOKR_MODE   0x26	//工作模式错误

unsigned char err;

//#define ERR_PARAM 0x02 //参数错误
//#define ERR_UNKNOW_CMD 0x01 //未知命令
#define ERR_NOT_READY  0x09
#define ERR_NO_MORE_DATA    0x0a
 

#define REG_PRODUCT_VER    0  //固件版本号
#define REG_PRODUCT_SN     2  //序列号
#define REG_PROTOCOL_VER   6  //通讯协议版本号
#define REG_ADDR           8  //设备地址
#define REG_BAUD           9  //设备通讯波特率
#define REG_RELATIVE_FLAG  10 //相对零点标志位
#define REG_RELATIVE_ZERO  11 //相对零点偏移
//传感器数据区
#define REG_DATA_DISTANCE  15 //传感器位移,即标定后的实测值
#define REG_DATA_TEMP      19 //传感器温度

float cache_data_x;				//倾斜x
float cache_data_y;				//倾斜y
float cache_data;					//裂缝，沉降

typedef union
{
	struct
	{ 
		unsigned short header;          //帧起始符：表明一帧数据的开始，固定为0xA5A5。
		unsigned char  packet_id;       //帧ID：用于判断上行帧和下行帧是否匹配，每次通讯后自动加一。
		unsigned char  source;		    //源地址：通讯数据帧发起方的地址,主机地址固定为0x00。
		unsigned char  destination;	    //目标地址：通讯数据帧接收方的地址。
		unsigned char  cmd;			    //指令符：表明该数据欲帧执行的操作。
		unsigned char  dlen;			//数据区长度：数据区长度，其值随指令符变化。
		unsigned char  data_buf[255];	//数据区：根据指令符，内容和长度都相应的变化，最大长度为255个字节，返回数据区的第一个字节为错误代码
		
		//运行时临时变量
		unsigned short  cnt;   //帧长度计数
		unsigned short *pCRC; //CRC指针
		unsigned char  data_bnt;//数据区计数
	};									//数据区格式约定：先发送高位数据（MSB），再发送低位数据（LSB），如果是小数类型全部采用标准float类型。
	unsigned char buf[280];
}FRAME;

extern FRAME tx_frame, rx_frame;
extern unsigned char reg[128];

/********************************data_485相关函数**************************************/
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

/******************************comm_485相关函数****************************************/
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