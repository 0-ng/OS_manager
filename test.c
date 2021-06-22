/*************************************************************************
	> File Name: task.c
	> Author: ma6174
	> Mail: ma6174@163.com 
	> Created Time: 2021年06月17日 星期四 17时30分59秒
 ************************************************************************/

#include<sys/types.h>
#include<sys/ipc.h>
#include<sys/shm.h>
#include<stdlib.h>
#include<stdio.h>
#include<fcntl.h>
#include<unistd.h>
#include<string.h>
#include"std_data_struct.h"
#define rep(i,a,b) for(int i=a;i<=b;i++)
const int kB=1024;
const int MB=1024*1024;
void print(struct ext2_dir_entry_2 t){
	printf("\n--------------------\n");
	printf("inode=%d\n",t.inode);
	printf("rec_len=%d\n",t.rec_len);
	printf("name_len=%d\n",t.name_len);
	printf("file_type=%d\n",t.file_type);
	printf("name=%s\n",t.name);
	printf("--------------------\n\n");
}
int main(){

	int fd = -1;		// fd 就是file descriptor，文件描述符
	int ret = -1;
	
	// 第一步：打开文件
	fd = open("memory", O_RDWR);
	if (-1 == fd)		// 有时候也写成： (fd < 0)
	{
		//printf("\n");
		perror("文件打开错误");
		// return -1;
		return -1;
	}else{
		printf("文件打开成功，fd = %d.\n", fd);
	}
	
	//ret = lseek(fd, 0, SEEK_SET);
	struct ext2_dir_entry_2 tmp;
	struct ext2_dir_entry_2 tmp2;
	ret=read(fd,&tmp,32);
	printf("ret=%d\n",ret);
	print(tmp);
	tmp.inode=10;
	tmp.rec_len=12;
	strcpy(tmp.name,"abc");
	print(tmp);
	ret=write(fd,&tmp,32);
	printf("ret=%d\n",ret);
	
	ret=lseek(fd,0,SEEK_SET);
	ret=read(fd,&tmp2,32);
	printf("ret=%d\n",ret);
	print(tmp2);

	ret=lseek(fd,0,SEEK_SET);
	ret=read(fd,&tmp2,32);
	printf("ret=%d\n",ret);
	print(tmp2);
	
	ret=lseek(fd,0,SEEK_SET);
	ret=read(fd,&tmp2,32);
	printf("ret=%d\n",ret);
	print(tmp2);
	return 0;
}
