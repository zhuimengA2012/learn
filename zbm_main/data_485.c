#include "485.h"
#include "head.h"
#include "set_485.h"

extern int  fd_ser;
unsigned int cnt = 0, retval_old = 0, retval = 0;
FRAME tx_frame;
FRAME rx_frame;
extern struct ProcesData PD;
extern struct VedioData VD;
FILE* fd_485_note;
struct timeval time_start_485, time_end_485;

/***************************485相关初始化************************************/
void COMM_Init(void)
{
	memset(&rx_frame, 0, sizeof(rx_frame));
	memset(&tx_frame, 0, sizeof(tx_frame));
	fd_485_note = fopen("/opt/ZBM/proc/485cmd_note", "a+");
	if (NULL == fd_485_note){
		
		perror("fd_485_note");
		close_v4l2();
		exit(1);
	}
	CMD_Init();

	return;
}

/***************************485通信函数**********************************/
void signal_handler_IO()
{
	
	int   status = 0;
	unsigned char  recvbuf[256] = {0}, buff[256] = {0};
	while (1){
		
		memset(recvbuf, 0x00, sizeof(recvbuf));
		status = read_port(recvbuf, 256);
	
		if (status > 0) {
		
			memcpy(buff + cnt, recvbuf, status);
			cnt = cnt + status;
			if (cnt > 250)
				cnt = 0;
		}
		if (retval == 0 && retval_old > 0) {    //一帧结束后拷贝数据
	
			memcpy(rx_frame.buf, buff, cnt);
			rx_frame.cnt = cnt;
			retval_old = retval;
			cnt = 0;
			//gettimeofday(&time_start_485, NULL);
			//FramePase();
			FramePase_old();
			return;
		}
		//usleep(100);
	}
	
	return;
}

/************************读取485的数据**************************/
int read_port(unsigned char *data, int datalength)
{
	fd_set   fs_read;
	struct timeval tv_timeout = {0, 0};
	
	tv_timeout.tv_sec = 0;
	tv_timeout.tv_usec = 3000;
	
	FD_ZERO(&fs_read);
	FD_SET(fd_ser, &fs_read);

	retval = select(fd_ser + 1, &fs_read, NULL, NULL, &tv_timeout);    // 多路复用  Linux下用select查询串口数据 
	if (retval > 0) {
		
		retval_old = retval;
		if (FD_ISSET(fd_ser, &fs_read)) { 
			return read(fd_ser, data, datalength);
			tcflush(fd_ser, TCIOFLUSH);   //tcflush : 清空终端未完成的输入/输出请求及数据。  放在return后面相当于没有
		}
	}
	else if (retval < 0) {
		 
		perror("select");
		return -1;
	}
	else if (retval == 0) {
		
		return 0;
	}
	return retval;
}

/*********************************对485发过来的信息进行判断和处理***************************************/
void FramePase()
{
	FRAME now_frame;
	unsigned char* now_head = NULL;
	int now_cnt = 0;
	
	now_head = (unsigned char*)(&rx_frame);
	//PrintBuf(rx_frame.buf, rx_frame.cnt);
	while (1)
	{
		while (0xA5A5 != *((unsigned short*)now_head)) {

			now_head++;
			rx_frame.cnt--;
			if (rx_frame.cnt < 9) {//基本长度判断
	
				return;
			}
		}
		if (rx_frame.cnt < 9) {//基本长度判断
	
			return;
		}
		memcpy(&now_frame, now_head, now_head[6] + 9);
		
		if (now_frame.destination != reg[REG_ADDR] && now_frame.destination != 0xFF) {
	
			now_head += 2;
			rx_frame.cnt -= 2;
			continue;
		}
	
		now_frame.pCRC = (unsigned short*)(now_frame.data_buf + now_frame.dlen);
	
		if (*now_frame.pCRC != CRCCheck(now_frame.buf, rx_frame.dlen + 7)) { //CRC校验
		
			now_head += 2;
			rx_frame.cnt -= 2;
			continue;
		}
		if (now_frame.destination == 0xFF && now_frame.cmd != 0x05) { //查询设备地址命令除外
		
			now_head += 2;
			rx_frame.cnt -= 2;
			continue;
		}
		if (now_frame.cmd == 0x0E)
			return;
		printf("=====recv===== \r\n");
		PrintBuf(now_frame.buf, now_frame.dlen + 9);
		now_cnt = now_frame.dlen + 9;
		tx_frame.dlen = 0;
		tx_frame.data_buf[0] = CMD(now_frame.cmd, now_frame.data_buf, now_frame.dlen, tx_frame.data_buf + 1, &tx_frame.dlen);
		printf("--CMD-- --after--\n");
		tx_frame.dlen += 1;
		memcpy(&tx_frame, &now_frame, 3);
		tx_frame.destination = now_frame.source;
		if (now_frame.destination == 0xFF && now_frame.cmd == 0x05) {
		
			tx_frame.source = reg[REG_ADDR];
		}else {
		
			tx_frame.source = now_frame.destination;
		}
		tx_frame.cmd = now_frame.cmd;
		tx_frame.pCRC = (unsigned short*)(tx_frame.data_buf + tx_frame.dlen);
		tx_frame.cnt = tx_frame.dlen + 9;
		*(tx_frame.pCRC) = CRCCheck(tx_frame.buf, tx_frame.cnt - 2);
		printf("=====send===== \r\n");
		PrintBuf(tx_frame.buf, tx_frame.cnt);
		SendData();
		now_head += now_cnt;
		rx_frame.cnt -= now_cnt;
	}
	return;
}
void FramePase_old()
{
	if (0xA5A5 != rx_frame.header) { //头判断
		
		printf("0xA5A5 != rx_frame.header error\n");
		PrintBuf(rx_frame.buf, rx_frame.cnt);
		printf("++++++++++++++++++++++++++++++++++up++++++++++++++++++++++++++++++++++++++++\n");
		return;
	}
	if (rx_frame.cnt < 9) {//基本长度判断
	
		printf("rx_frame.cnt < 9 error\n");
		PrintBuf(rx_frame.buf, rx_frame.cnt);
		printf("++++++++++++++++++++++++++++++++++up++++++++++++++++++++++++++++++++++++++++\n");
		return;
	}
	if (rx_frame.destination != reg[REG_ADDR] && rx_frame.destination != 0xFF) { //ID判断    //用的是与不是或？
	
		//printf("ID check error\n");
		//PrintBuf(rx_frame.buf, rx_frame.cnt);
		//printf("++++++++++++++++++++++++++++++++++up++++++++++++++++++++++++++++++++++++++++\n");
		return;
	}
	if (rx_frame.dlen + 9 != rx_frame.cnt) { //数据长度判断
		
		printf("rx_frame.dlen + 9 != rx_frame.cnt error\n");
		PrintBuf(rx_frame.buf, rx_frame.cnt);
		printf("++++++++++++++++++++++++++++++++++++up++++++++++++++++++++++++++++++++++++++\n");
		return;
	}
	rx_frame.pCRC = (unsigned short*)(rx_frame.data_buf + rx_frame.dlen);
	
	if (*rx_frame.pCRC != CRCCheck(rx_frame.buf, rx_frame.cnt - 2)) { //CRC校验
		
		printf("crc check error\n");
		PrintBuf(rx_frame.buf, rx_frame.cnt);
		printf("+++++++++++++++++++++++++++++++++++up+++++++++++++++++++++++++++++++++++++++\n");
		return;
	}
	//命令解析
	if (rx_frame.destination == 0xFF && rx_frame.cmd != 0x05) { //查询设备地址命令除外
		
		printf("rx_frame.destination == 0xFF && rx_frame.cmd != 0x05\n");
		PrintBuf(rx_frame.buf, rx_frame.cnt);
		printf("++++++++++++++++++++++++++++++++++up++++++++++++++++++++++++++++++++++++++++\n");
		return;
	}
	printf("=====recv===== \r\n");
	PrintBuf(rx_frame.buf, rx_frame.cnt);
	tx_frame.dlen = 0;
	tx_frame.data_buf[0] = CMD(rx_frame.cmd, rx_frame.data_buf, rx_frame.dlen, tx_frame.data_buf + 1, &tx_frame.dlen);
	printf("--CMD-- --after--\n");
	if (rx_frame.cmd == 0x0E)
		return;
	tx_frame.dlen += 1;
	memcpy(&tx_frame, &rx_frame, 3);
	tx_frame.destination = rx_frame.source;
	if (rx_frame.destination == 0xFF && rx_frame.cmd == 0x05) {
		
		tx_frame.source = reg[REG_ADDR];
	}
	else {
		
		tx_frame.source = rx_frame.destination;
	}
	tx_frame.cmd = rx_frame.cmd;
	tx_frame.pCRC = (unsigned short*)(tx_frame.data_buf + tx_frame.dlen);
	tx_frame.cnt = tx_frame.dlen + 9;
	*(tx_frame.pCRC) = CRCCheck(tx_frame.buf, tx_frame.cnt - 2);
	printf("=====send===== \r\n");
	PrintBuf(tx_frame.buf, tx_frame.cnt);
	SendData();
	rx_frame.cnt = 0;
	
	return;
}
/************************CRC校验函数*****************************/
unsigned short CRCCheck(unsigned char *str_, unsigned short crc_cd)
{
	unsigned short crc_gs;
	unsigned char crc_w;
	unsigned short   crc_sj = 0xffff;	
	for (crc_gs = 0; crc_gs < crc_cd; crc_gs++) {
		
		crc_sj = ((crc_sj ^ str_[crc_gs]) & 0x00ff) + (crc_sj & 0xff00);		
		for (crc_w = 0; crc_w < 8; crc_w++) {	
			
			if ((crc_sj & 0x0001) == 0x0000) {
				
				crc_sj >>= 1;
			}
			else {
				crc_sj >>= 1;
				crc_sj ^= 0xa001;
			}
		}
	}        
	return (crc_sj >> 8) + (crc_sj << 8);
}

/**********************通信判断成功，进行回复******************************/
void SendData()
{
	if (tx_frame.cnt > 0) {     
		
		uart_send_date(tx_frame.buf, tx_frame.cnt);
		tx_frame.cnt = 0;
	}
	return;
}
void uart_send_date(unsigned char *p_send, unsigned short send_num)
{
	//long int my_s, my_us;
	//gettimeofday(&time_end_485, NULL);
	Rs485_Send; 
	usleep(100);
	write(fd_ser, p_send, send_num);	
	usleep(send_num * 1200);	
	Rs485_Receive;
	
	//my_s = time_end_485.tv_sec - time_start_485.tv_sec;
	//my_us = time_end_485.tv_usec - time_start_485.tv_usec;
	//printf("s = %ldus = %ld\n", my_us > 0 ? my_s : my_s - 1, my_us > 0 ? my_us : 1000000 + my_us);
	return;
}

/******************************数据打印函数**********************************/
void PrintBuf(unsigned char *buf, int cnt)
{
	int i;
	printf("RX%d:", cnt);
	for (i = 0;i < cnt;i++) {
		
		printf("%02x", buf[i]);
	}
	printf("\n");
	return;
}

/***************************从ini中读出有效数据******************************/
void read_data(void)
{
	double x = 0.0, y = 0.0, ave = 0.0;
	
	read_val_data(INI, "ZBM_DATA", "val_x", &x);
	read_val_data(INI, "ZBM_DATA", "val_y", &y);
	read_val_data(INI, "ZBM_DATA", "value", &ave);
	cache_data_x = (float)x;
	cache_data_y = (float)y;
	cache_data = (float)ave;
	return;
}
/***************************每隔50个数就存储一个有效数据******************************/
void set_data(void)
{

	write_val_data(INI, "ZBM_DATA", "val_x", PD.val_x);
	write_val_data(INI, "ZBM_DATA", "val_y", PD.val_y);
	write_val_data(INI, "ZBM_DATA", "value", VD.value);
	sync();
	return;
}