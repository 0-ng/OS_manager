/*************************************************************************
	> File Name: data_struct.c
	> Author: ma6174
	> Mail: ma6174@163.com 
	> Created Time: 2021年06月17日 星期四 20时48分15秒
 ************************************************************************/

#include<stdio.h>
typedef int __le32;
typedef short __le16;
typedef char __le8;
typedef unsigned char __u8;


struct Block{

}



/***************************inode索引节点**************************************/
const int EXT2_N_BLOCKS = 15;
struct ext2_inode {
   
	__le16 i_mode; /* File mode  文件类型和访问权限*/
    __le16 i_uid; /* Low 16 bits of Owner Uid  所有者的 UID（低16位）*/
    __le32 i_size; /* Size in bytes  文件长度（字节）*/
    __le16 i_gid; /* Low 16 bits of Group Id  所有者 GID（低 16 位）*/
    __le16 i_links_count; /* Links count  硬链接计数*/
    __le32 i_blocks; /* Blocks count  文件长度（block 计数）*/
    __le32 i_flags; /* File flags  文件标志*/
    union {
        struct {
            __le32 l_i_reserved1;
        } linux1; /*Linux 中特定的信息之 1*/
        struct {
            __le32 h_i_translator;
        } hurd1;
        struct {
            __le32 m_i_reserved1;
        } masix1;
    } osd1; /* OS dependent 1*/
    __le32 i_block[EXT2_N_BLOCKS];/* Pointers to blocks  数据盘块指针数组*/
    __le32 i_generation; /* File version (for NFS)*/
    __le32 i_file_acl; /* File ACL  文件访问控制列表*/
    __le32 i_dir_acl; /* Directory ACL  目录访问控制列表*/
    __le32 i_faddr; /* Fragment address*/
};
/***************************inode索引节点**************************************/

/***************************目录**************************************/
struct ext2_dir_entry_2 {
    __le32 inode; /* Inode number*/
    __le16 rec_len; /* Directory entry length*/
    __u8 name_len=16; /* Name length*/
    __u8 file_type; /*文件类型（见下面  行）*/
    //char name[]; /* File name, up to EXT2_NAME_LEN*/
	char name[16];
};

enum { /*ext2_dir_entry_2 中的 file_type 枚举类型*/
    EXT2_FT_UNKNOWN = 0,
    EXT2_FT_REG_FILE = 1, /*普通文件*/
    EXT2_FT_DIR = 2, /*目录文件*/
    EXT2_FT_CHRDEV = 3, /*字符设备文件*/
    EXT2_FT_BLKDEV = 4, /*块设备文件*/
    EXT2_FT_FIFO = 5, /*命名管道 FIFO*/
    EXT2_FT_SOCK = 6, /*SOCK*/
    EXT2_FT_SYMLINK = 7, /*符号链接*/
    EXT2_FT_MAX = 8
};
/***************************目录**************************************/

struct Block_Group{
	Block super_block;

	Block data_block_bitmap;
	Block inode_bitmap;
	ext2_inode inode_table;
	Block data_blocks[x];
}
