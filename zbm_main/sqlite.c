#include "head.h"

/*******************回调函数，显示结果********************/
int callback(void *p, int c, char **val, char **name)
{
	int i;

	if (*(int *)p == 0) {
		for (i = 0;i < c;i++) {
			printf("%-25s", name[i]);
		}
		printf("\n");
		*(int *)p = 1;
	}
	for (i = 0;i < c;i++) {
		printf("%-25s", val[i]);
	}
	printf("\n");
	return 0;
}

/***********************在已有数据库中创建一个表格*********************************/
int zbm_sqlite_creat_table(sqlite3 *sqlp, char *table_name)
{
	char *errmsg;
	char buf_table[128];

	bzero(buf_table, sizeof(buf_table));
	sprintf(buf_table, "create table %s (id integer primary key autoincrement,name text,time1 ReturnDate,time2 ReturnDate);", table_name);
	if (SQLITE_OK != sqlite3_exec(sqlp, buf_table, NULL, NULL, &errmsg)) {
		
		puts(errmsg);
		return -1;
	}
	
	printf("sqlite table creat success \n");
	return 0;
}

/***********************在表格中插入数据*****************************/
int zbm_sqlite_insert(sqlite3 *sqlp, char *table_name, char *image_name)
{
	char *errmsg;
	char buf_image[256];
	char image_time1[32];     //image_time为获取的系统时间
	char image_time2[64]; 
	struct tm *my_time;
	time_t t;
	
	t = time(NULL);
	my_time = localtime(&t);
	sprintf(image_time1, "%d-%02d-%02d", 1900 + my_time->tm_year, 1 + my_time->tm_mon, my_time->tm_mday);
	sprintf(image_time2, "%d-%02d-%02d %02d:%02d:%02d", 1900 + my_time->tm_year, 1 + my_time->tm_mon, my_time->tm_mday, my_time->tm_hour, my_time->tm_min, my_time->tm_sec);
	
	bzero(buf_image, sizeof(buf_image));
	sprintf(buf_image, "insert into %s (name,time1,time2) values ('%s', '%s' ,'%s');", table_name, image_name, image_time1, image_time2);
	if (SQLITE_OK != sqlite3_exec(sqlp, buf_image, NULL, NULL, &errmsg)) {
		
		puts(errmsg);
		return -1;
	}

	printf("sqlite table insert success \n");
	return 0;
}

/***********************在表格中删除数据*****************************/
int zbm_sqlite_delete(sqlite3 *sqlp, char *table_name, char *image_name)
{
	char *errmsg;
	char buf_table[128] = {0};

	sprintf(buf_table, "delete from '%s' where name = '%s';", table_name, image_name);
	if (SQLITE_OK != sqlite3_exec(sqlp, buf_table, NULL, NULL, &errmsg)) {
		puts(errmsg);
		return -1;
	}
	
	printf("sqlite table's  '%s' delete success \n", table_name);
	return 0;
}

/***********************删除表格*****************************/
int zbm_sqlite_delete_table(sqlite3 *sqlp, char *table_name)
{
	char *errmsg;
	char buf_del_table[32];

	bzero(buf_del_table, sizeof(buf_del_table));
	sprintf(buf_del_table, "drop table '%s';", table_name);
	if (SQLITE_OK != sqlite3_exec(sqlp, buf_del_table, NULL, NULL, &errmsg)) {
		puts(errmsg);
	}
	
	printf("sqlite table %s delete success \n", table_name);
	return 0;
}

/***********************在表格中查找数据*****************************/
int zbm_sqlite_find(sqlite3 *sqlp, char *table_name, char *time2_start, char *time2_end)
{
	char *errmsg;
	int i, j;
	char buf_sentence[256] = {0};
	char image_time2_start[64] = {0};
	char image_time2_end[64] = {0};
	
	sprintf(image_time2_start, "%s 00:00:00", time2_start);
	sprintf(image_time2_end, "%s 24:00:00", time2_end);
	sprintf(buf_sentence, "select * from '%s' where datetime(time2)>=datetime('%s') and datetime(time2)<datetime('%s');", table_name, image_time2_start, image_time2_end);
	if (SQLITE_OK != sqlite3_get_table(sqlp, buf_sentence, &image_result, &image_row, &image_colunm, &errmsg)) {
		
		puts(errmsg);
		return -1;
	}
	/*
	for (i = 0;i <= row;i++) {
		for (j = 0;j < colunm;j++) {
			printf("%s\t", result[j + i * colunm]);
		}
		printf("\n");
	}
	sqlite3_free_table(result);
*/
	return 0;
}

/***********************在表格中查找数据*****************************/
int zbm_sqlite_time_get_picture(sqlite3 *sqlp, char *table_name, char *time2)
{
	char *errmsg;
	int i, j;
	char ** result;
	int row, colunm;
	char buf_sentence[128] = {0};
	
	sprintf(buf_sentence, "select * from '%s' where time2 = '%s';", table_name, time2);
	if (SQLITE_OK != sqlite3_get_table(sqlp, buf_sentence, &result, &row, &colunm, &errmsg)) {
		
		puts(errmsg);
		return -1;
	}
	strcpy(image_trans, result[5]);
	sqlite3_free_table(result);

	return 0;
}

int zbm_sqlite_del_picture(sqlite3 *sqlp, char *table_name)
{
	char buf[128] = {0};
	int i = 0;
	char *errmsg;
	char ** result;
	int row, colunm;
	sprintf(buf, "select * from '%s'", table_name);
	if (SQLITE_OK != sqlite3_get_table(sqlp, buf, &result, &row, &colunm, &errmsg)) {
		
		puts(errmsg);
		return -1;
	}
	if (row > PIC_NUM){
		for (i = 0; i < row - PIC_NUM;i++){
			bzero(buf,sizeof(buf));
			sprintf(buf, "rm -rf /opt/ZBM/pic/pic_save/%s", result[colunm * (i + 1) + 1]);
			system(buf);
			zbm_sqlite_delete(sqlp, table_name, result[colunm * (i + 1) + 1]);
		}
	}
	
	return 0;
}