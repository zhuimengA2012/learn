#include "head.h"

/*********************************ȫ�ֱ���*******************************************/

int      fd_v4l;
static   struct   v4l2_capability   cap;  
struct v4l2_fmtdesc fmtdesc;  
struct v4l2_format fmt, fmtack;  
struct v4l2_streamparm setfps;    
struct v4l2_requestbuffers req;  
struct v4l2_buffer buf;  
enum v4l2_buf_type type;  
unsigned char *frame_buffer;  
//unsigned char frame_buffer[IMAGEWIDTH * IMAGEHEIGHT * 3];  
struct buffer *buffers;

extern struct VedioData VD;
extern int wdog_fd;
extern int driver_fd;

/******************************v4l2�ĳ�ʼ��,typeΪ1��MJPG,typeΪ0��YUYV****************************************/
int init_v4l2(int type)  
{  
	unsigned int n_buffers;
	
	USB_CMOS_OPEN;      //open led and cmos power
	sleep(5);
	fd_v4l = open(FILE_VIDEO, O_RDWR);
	if (-1 == fd_v4l) {  
		perror("fd_v4l open");      //��ʱ�����ִ�ʧ��
		return -1;  
	}  
	frame_buffer = (unsigned char *)malloc(IMAGEWIDTH * IMAGEHEIGHT * 3 * sizeof(unsigned char));
	    //query cap  
	if (ioctl(fd_v4l, VIDIOC_QUERYCAP, &cap) == -1){  
		printf("Error opening device %s: unable to query device.\n", FILE_VIDEO);  
		return -1;  
	}else {  
		printf("driver:\t\t%s\n", cap.driver);  
		printf("card:\t\t%s\n", cap.card);  
		printf("bus_info:\t%s\n", cap.bus_info);  
		printf("version:\t%d\n", cap.version);  
		printf("capabilities:\t%x\n", cap.capabilities);         
		if ((cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) == V4L2_CAP_VIDEO_CAPTURE) {  
			printf("Device %s: supports capture.\n", FILE_VIDEO);  
		}
		if ((cap.capabilities & V4L2_CAP_STREAMING) == V4L2_CAP_STREAMING) {  
			printf("Device %s: supports streaming.\n", FILE_VIDEO);  
		}  
	}   
      
	fmtdesc.index = 0;  
	fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;  
	printf("Support format:\n");  
	while (ioctl(fd_v4l, VIDIOC_ENUM_FMT, &fmtdesc) != -1) {  
		
		printf("\t%d.%s\n", fmtdesc.index + 1, fmtdesc.description);  
		fmtdesc.index++;  
	}  
    
	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;  
	if (1 == type) {               //��ʼ������ͷ�����ʽ
		fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;  //V4L2_PIX_FMT_JPEG   V4L2_PIX_FMT_YUYV
	}else {
		fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;  //V4L2_PIX_FMT_JPEG   V4L2_PIX_FMT_YUYV
	}
		
	fmt.fmt.pix.height = IMAGEHEIGHT;  
	fmt.fmt.pix.width = IMAGEWIDTH;   
	fmt.fmt.pix.field = V4L2_FIELD_INTERLACED;  
      
	if (ioctl(fd_v4l, VIDIOC_S_FMT, &fmt) == -1) {  
		
		printf("Unable to set format\n");  
		return -1;  
	}       
	if (ioctl(fd_v4l, VIDIOC_G_FMT, &fmt) == -1) {  
		
		printf("Unable to get format\n");  
		return -1;  
	}else {  
		
		printf("fmt.type:\t\t%d\n", fmt.type);  
		printf("pix.pixelformat:\t%c%c%c%c\n", fmt.fmt.pix.pixelformat & 0xFF, (fmt.fmt.pix.pixelformat >> 8) & 0xFF, (fmt.fmt.pix.pixelformat >> 16) & 0xFF, (fmt.fmt.pix.pixelformat >> 24) & 0xFF);  
		printf("pix.height:\t\t%d\n", fmt.fmt.pix.height);  
		printf("pix.width:\t\t%d\n", fmt.fmt.pix.width);  
		printf("pix.field:\t\t%d\n", fmt.fmt.pix.field);  
	}
	
	setfps.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;  
	setfps.parm.capture.timeperframe.numerator = 10;  
	setfps.parm.capture.timeperframe.denominator = 10;  
	printf("init %s \t[OK]\n", FILE_VIDEO);

	req.count = 1;     //Ϊ����ͷ����������
	req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;  
	req.memory = V4L2_MEMORY_MMAP;  
	if (ioctl(fd_v4l, VIDIOC_REQBUFS, &req) == -1) {  
		
		printf("request for buffers error\n");  
		return -1; 
	}
		//mmap for buffers  
	buffers = malloc(req.count*sizeof(*buffers));  
	if (!buffers) {  
		
		printf("Out of memory\n");  
		return -1;  
	}  

	memset(&buf, 0, sizeof(buf));  //������

	for (n_buffers = 0; n_buffers < req.count; n_buffers++){  
		
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;  
		buf.memory = V4L2_MEMORY_MMAP;  
		buf.index = n_buffers;  

		if (ioctl(fd_v4l, VIDIOC_QUERYBUF, &buf) == -1) {  
			
			printf("query buffer error\n");  
			return -1;  
		}  

		buffers[n_buffers].length = buf.length;  
		buffers[n_buffers].start = mmap(NULL, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd_v4l, buf.m.offset);  
		if (buffers[n_buffers].start == MAP_FAILED) {  
			
			printf("buffer map error\n");  
			return -1;  
		}
		
		ioctl(fd_v4l, VIDIOC_QBUF, &buf);  
	} 
	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;  		
	ioctl(fd_v4l, VIDIOC_STREAMON, &type);  
	printf("v4l init ok\n");     
    
	return 0;  
}

/******************************��ͼƬ��Ϣ��������****************************************/
int v4l2_grab(void)  
{  
	int n_buffers;
	fd_set fd_read;
	int ret;
	struct timeval timeout;

	printf("picture start! \n");

	for (n_buffers = 0; n_buffers < req.count; n_buffers++) {  
		
		buf.index = n_buffers;  
		ioctl(fd_v4l, VIDIOC_QBUF, &buf);  //VIDIOC_QBUF : Ͷ��һ���յ���Ƶ����������Ƶ��������������� 
	}									   //ִ�гɹ�ʱ����������ֵΪ 0������ִ�гɹ���ָ��(ָ��)����Ƶ������������Ƶ������У���������Ƶ�豸����ͼ��ʱ����Ӧ����Ƶ���ݱ����浽��Ƶ���������Ӧ����Ƶ�������С�
	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;    //���������ͣ�������Զ��V4L2_BUF_TYPE_VIDEO_CAPTURE
	ioctl(fd_v4l, VIDIOC_STREAMON, &type); //������Ƶ�ɼ����Ӧ�ó������VIDIOC_STREAMON������Ƶ�ɼ������,��Ƶ�豸��������ʼ�ɼ���Ƶ���ݣ����Ѳɼ�������Ƶ���ݱ��浽��Ƶ��������Ƶ�������С�
										   //ִ�гɹ�ʱ����������ֵΪ 0������ִ�гɹ�����Ƶ�豸��������ʼ�ɼ���Ƶ���ݣ���ʱӦ�ó���һ��ͨ������select�������ж�һ֡��Ƶ�����Ƿ�ɼ���ɣ�����Ƶ�豸�������һ֡��Ƶ���ݲɼ������浽��Ƶ��������ʱ��select�������أ�Ӧ�ó�����ſ��Զ�ȡ��Ƶ���ݣ�����select��������ֱ����Ƶ���ݲɼ���ɡ�
	timeout.tv_sec = 15;
	timeout.tv_usec = 0;
	FD_ZERO(&fd_read);
	FD_SET(fd_v4l, &fd_read);
	ret = select(fd_v4l + 1, &fd_read, NULL, NULL, &timeout);
	if (-1 == ret){
		
		printf("grab select error\n");
		return -1;
	}else if (0 == ret){
		
		printf("grab timeout and exit\n");
		return -1;
	}else{
		if (FD_ISSET(fd_v4l, &fd_read)) {
		
			ioctl(fd_v4l, VIDIOC_DQBUF, &buf);      //���Ѿ��������Ƶ���ڴ������Ѳ�����Ƶ�Ķ���
			return 0;
		}
	}
	
	return 0;     
}  

/******************************����ͼƬ��/dev/shm�ļ���****************************************/
int save_shm_picture(void)
{	
	FILE *fptr_jpg;
	void *p;
	
	fptr_jpg = fopen("/dev/shm/image_jpeg.jpg", "w+");
	if (fptr_jpg == NULL) {
		
		perror("save picture fopen");
		return -1;
	}

	fwrite(buffers[0].start, 1, buffers[0].length, fptr_jpg);

	fclose(fptr_jpg);
	printf("picture ok! \n");
	return 0;
}

/******************yuyv to rgb**********************/
void yuyv_2_rgb888(int num)
{      
	int           i, j;
	unsigned char y1, y2, u, v;
	int r1, g1, b1, r2, g2, b2;
	char * pointer;
	pointer = buffers[num].start;
	int width = 0;
	
	width = IMAGEWIDTH / 2;
	for (i = 0;i < IMAGEHEIGHT;i++) {     
		
		for (j = 0;j < width;j++)//ÿ��ȡ4���ֽڣ�Ҳ�����������ص㣬ת��rgb��6���ֽڣ������������ص�
		{
			y1 = *(pointer + (i*width + j) * 4);
			u  = *(pointer + (i*width + j) * 4 + 1);
			y2 = *(pointer + (i*width + j) * 4 + 2);
			v  = *(pointer + (i*width + j) * 4 + 3);
			
			r1 = y1 + 1.042*(v - 128);
			g1 = y1 - 0.34414*(u - 128) - 0.71414*(v - 128);
			b1 = y1 + 1.772*(u - 128);
			
			r2 = y2 + 1.042*(v - 128);
			g2 = y2 - 0.34414*(u - 128) - 0.71414*(v - 128);
			b2 = y2 + 1.772*(u - 128);
			
			if (r1 > 255)
				r1 = 255;      
			else if (r1 < 0)
				r1 = 0;

			if (b1 > 255)
				b1 = 255;
			else if (b1 < 0)
				b1 = 0;
			
			if (g1 > 255) 
				g1 = 255; 
			else if (g1 < 0) 
				g1 = 0; 

			if (r2 > 255)
				r2 = 255;
			else if (r2 < 0)
				r2 = 0; 
			
			if (b2 > 255) 
				b2 = 255; 
			else if (b2 < 0) 
				b2 = 0; 
			
			if (g2 > 255)   
				g2 = 255; 
			else if (g2 < 0)  
				g2 = 0;    
			if (0 == num) {
				*(frame_buffer + (i*IMAGEWIDTH / 2 + j) * 6) = (unsigned char)b1;
				*(frame_buffer + (i*IMAGEWIDTH / 2 + j) * 6 + 1) = (unsigned char)g1;
				*(frame_buffer + (i*IMAGEWIDTH / 2 + j) * 6 + 2) = (unsigned char)r1;
				*(frame_buffer + (i*IMAGEWIDTH / 2 + j) * 6 + 3) = (unsigned char)b2;
				*(frame_buffer + (i*IMAGEWIDTH / 2 + j) * 6 + 4) = (unsigned char)g2;
				*(frame_buffer + (i*IMAGEWIDTH / 2 + j) * 6 + 5) = (unsigned char)r2;
			}else if (1 == num) {
					/*
					*(frame_buffer2 + (i*IMAGEWIDTH / 2 + j) * 6) = (unsigned char)b1;
					*(frame_buffer2 + (i*IMAGEWIDTH / 2 + j) * 6 + 1) = (unsigned char)g1;
					*(frame_buffer2 + (i*IMAGEWIDTH / 2 + j) * 6 + 2) = (unsigned char)r1;
					*(frame_buffer2 + (i*IMAGEWIDTH / 2 + j) * 6 + 3) = (unsigned char)b2;
					*(frame_buffer2 + (i*IMAGEWIDTH / 2 + j) * 6 + 4) = (unsigned char)g2;
					*(frame_buffer2 + (i*IMAGEWIDTH / 2 + j) * 6 + 5) = (unsigned char)r2;*/
			}			
		}
	}
	printf("change to RGB OK \n"); 
	
	return;
} 


	/***********************ѹ��ͼƬ***************************/
int encode_jpeg(char image_name [], unsigned char *lpbuf, int width, int height)  
{  
	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;
	JSAMPROW  row_pointer[1];
	int row_stride;
	unsigned char *buf = NULL;
	int x;

	FILE *fptr_jpg = fopen(image_name, "wb");
	if (fptr_jpg == NULL) {
		
		printf("Encoder:open file failed!\n");
		return -1;
	}
	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_compress(&cinfo);
	jpeg_stdio_dest(&cinfo, fptr_jpg);
	cinfo.image_width = width;
	cinfo.image_height = height;
	cinfo.input_components = 3;
	cinfo.in_color_space = JCS_RGB;
	jpeg_set_defaults(&cinfo);
	jpeg_set_quality(&cinfo, 80, TRUE);
	jpeg_start_compress(&cinfo, TRUE);
	row_stride = width * 3;
	buf = malloc(row_stride);
	row_pointer[0] = buf;
	while (cinfo.next_scanline < height) {
		
		for (x = 0; x < row_stride; x += 3) {        
			
			buf[x]   = lpbuf[x];
			buf[x + 1] = lpbuf[x + 1];
			buf[x + 2] = lpbuf[x + 2];

		}
		jpeg_write_scanlines(&cinfo, row_pointer, 1);//critical
		lpbuf += row_stride;
	}
	jpeg_finish_compress(&cinfo);
	fclose(fptr_jpg);
	jpeg_destroy_compress(&cinfo);
	free(buf);

	printf("RGB24 to Jpeg OK\n");
	return 0 ; 
}  

/****************************���ɲ�����һ��ͼƬ***********************************/
void save_yuyv_jpeg(void)
{
	int ret;
	char image_name[32];
    /********************����YUYVͼƬ��������**********************/
	v4l2_grab();   //�ӵײ㽻������Ƶ��Ϣ��������
	sprintf(image_name, "/dev/shm/image_jpeg.jpg");
	yuyv_2_rgb888(0);  //��ʽת��
	ret = encode_jpeg(image_name, frame_buffer, IMAGEWIDTH, IMAGEHEIGHT); //RGB24 to Jpeg   //ѹ��
	if (-1 == ret)
		return ;

	printf("save image_jpeg.jpg OK!\n");
	
	return;
}

/****************************�ر�����ͷ���*******************************/
int close_v4l2(void)  
{  
	printf("********close v4l********\n");
	close(wdog_fd);
	USB_CMOS_CLOSE;		//close led and cmos power
	close(driver_fd);
	fclose(fd_time);
	
	if (fd_v4l != -1) {  
		close(fd_v4l);  
		return -1;
	}
	free(frame_buffer);
	
	return 0;  
}

void yuyv_2_gray(unsigned char *y_data, unsigned char *pointer)  
{  
	int           i, j;  
	unsigned char y1, y2, u, v;  
	int width = 0;
	width = IMAGEWIDTH / 2;	
	

	for (i = 0;i < IMAGEHEIGHT;i++)     //IMAGEHEIGHT  : 720 
	{  
		for (j = 0;j < width;j++)   //   IMAGEWIDTH   :1280  
		{  
			/*
			*
			y1 = *(pointer + (i*width + j) * 4);       
			u  = *(pointer + (i*width + j) * 4 + 1);  
			y2 = *(pointer + (i*width + j) * 4 + 2);  
			v  = *(pointer + (i*width + j) * 4 + 3);  

			y_data[i * IMAGEWIDTH + j * 2] = y1;
			y_data[i * IMAGEWIDTH + j * 2 + 1] = y2;
			*
			*/
			
			y_data[i * IMAGEWIDTH + j * 2] = *(pointer + (i*width + j) * 4);
			y_data[i * IMAGEWIDTH + j * 2 + 1] = *(pointer + (i*width + j) * 4 + 2);
		
		}  
	} 			

	printf("gra_jpg ok! \r\n");
	usleep(1000);

	return;
}