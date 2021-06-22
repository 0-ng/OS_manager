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
#define rep(i,a,b) for(int i=a;i<=b;i++)
const int kB=1024;
const int MB=1024*1024;
int main(){

	int shm_id;
	shm_id=shmget((key_t)1234, 100*MB, 0666|IPC_CREAT);
	if(shm_id<0){
		perror("shmget fail!\n");
		exit(1);
	}
	printf("%d\n",shm_id);
	int *shared_memory=shmat(shm_id, (void *)0, 0);
	rep(i,1,101){
		printf("%d ",shared_memory[i]);
		if(i%10==0)printf("\n");
	}
	system("ipcs -m");
	return 0;
}
