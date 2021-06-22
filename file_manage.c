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

int now_dir_inode=0;
char file_name[256];
char dir_name[256];
char old_file_name[256];
char new_file_name[256];

int fd = -1;		// fd 就是file descriptor，文件描述符
void _ls(){
	int ret;
	int block_group_number=0;
	int inode_offset=0;
	int inode_table_location=;
	
	struct ext2_group_desc desc[1];
	ret=lseek(fd,0,SEEK_SET);
	ret=read(fd,desc,256);
	
}
void _mkdir(){

	scanf("%s",dir_name);
}
void _rmdir(){

	scanf("%s",dir_name);

}
void _mv(){

	scanf("%s",old_file_name);
	scanf("%s",new_file_name);

}
void _open(){

	scanf("%s",file_name);

}
void _vim(){

	scanf("%s",file_name);

}
void _rm(){

	scanf("%s",file_name);

}
int main(){

	int ret = -1;
	char op[256];
	fd = open("memory", O_RDWR);
	if (-1 == fd){
		perror("文件打开错误");
		return -1;
	}
	while(1){
		printf("\033[32;1mleung-0ng@0ng\033[0m");
		printf("\033[37;1m:\033[0m");
		printf("\033[34;1m~\033[0m");
		printf("\033[37m$ \033[0m");
		scanf("%s",op);
		if(strcmp(op,"quit")==0){
			break;
		}else if(strcmp(op,"ls")==0){
			_ls();			
		}else if(strcmp(op,"mkdir")==0){
			_mkdir();
		}else if(strcmp(op,"rmdir")==0){
			_rmdir();
		}else if(strcmp(op,"mv")==0){
			_mv();
		}else if(strcmp(op,"open")==0){
			_open();
		}else if(strcmp(op,"vim")==0){
			_vim();
		}else if(strcmp(op,"rm")==0){
			_rm();
		}
	}
	return 0;
	//ret = lseek(fd, 0, SEEK_SET);
	struct ext2_dir_entry_2 tmp[1];
	struct ext2_dir_entry_2 tmp2[1];
	ret=read(fd,tmp,32);
	printf("ret=%d\n",ret);
	print(tmp[0]);
	tmp[0].inode=10;
	tmp[0].rec_len=12;
	strcpy(tmp[0].name,"abc");
	print(tmp[0]);
	ret=write(fd,tmp,32);
	printf("ret=%d\n",ret);
	
	ret=lseek(fd,0,SEEK_SET);
	ret=read(fd,tmp2,32);
	printf("ret=%d\n",ret);
	print(tmp2[0]);

	ret=lseek(fd,0,SEEK_SET);
	ret=read(fd,tmp2,32);
	printf("ret=%d\n",ret);
	print(tmp2[0]);
	
	ret=lseek(fd,0,SEEK_SET);
	ret=read(fd,tmp2,32);
	printf("ret=%d\n",ret);
	print(tmp2[0]);
	return 0;
}
