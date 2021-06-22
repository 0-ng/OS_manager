/*************************************************************************
	> File Name: task.c
	> Author: ma6174
	> Mail: ma6174@163.com
	> Created Time: 2021年06月17日 星期四 17时30分59秒
 ************************************************************************/


#include<semaphore.h>
#include<sys/types.h>
#include<sys/ipc.h>
#include<sys/shm.h>
#include<stdlib.h>
#include<stdio.h>
#include<unistd.h>
#include<string.h>
#include<algorithm>
#include<fcntl.h>
#include"std_data_struct.h"

#define rep(i, a, b) for(int i=a;i<=b;i++)
const int kB = 1024;
const int MB = 1024 * 1024;
bool show= false;
void dir_print(struct ext2_dir_entry_2 t) {
    printf("\n--------------------\n");
    printf("inode=%d\n", t.inode);
    printf("rec_len=%d\n", t.rec_len);
    printf("name_len=%d\n", t.name_len);
    printf("file_type=%d\n", t.file_type);
    printf("name=%s\n", t.name);
    printf("--------------------\n\n");
}

void inode_print(struct ext2_inode t) {
    printf("\n--------------------\n");
    printf("i_size=%d\n", t.i_size);
//    printf("rec_len=%d\n",t.rec_len);
//    printf("name_len=%d\n",t.name_len);
//    printf("file_type=%d\n",t.file_type);
//    printf("name=%s\n",t.name);
    printf("--------------------\n\n");
}

int now_dir_inode = 0;
char file_name[256];
char dir_name[256];
char old_file_name[256];
char new_file_name[256];

int fd = -1;        // fd 就是file descriptor，文件描述符
int get_group_number(int inode) {
    return (inode - 1) / inodes_per_group;
}

int get_inode_offset(int inode) {
    return (inode - 1) % inodes_per_group;
}

int get_group_number_data_block(int block_number) {
    return (block_number - 1) / blocks_per_group;
}

int get_data_block_offset(int block_number) {
    return (block_number - 1) % blocks_per_group;
}

int get_group_descripe_begin(int block_group_number) {
    return block_size + (block_group_number) * group_descripe_size;
}

int group_number2data_bitmap(int group_number) {
    return group_size * group_number + 2 * block_size;
}

int group_number2inode_bitmap(int group_number) {
    return group_size * group_number + 3 * block_size;
}

int group_number2inode_table(int group_number) {
    return group_size * group_number + 4 * block_size;
}

int get_inode_location(int inode_number) {
    int ret;
    int block_group_number = get_group_number(inode_number);
    int inode_offset = get_inode_offset(inode_number);
    int group_descripe_location = get_group_descripe_begin(block_group_number);
    struct ext2_group_desc desc;
    ret = lseek(fd, group_descripe_location, SEEK_SET);
    ret = read(fd, &desc, group_descripe_size);
    int inode_table_location = desc.bg_inode_table;
    return inode_table_location + inode_size * inode_offset;
}

int inode_number2inode_bitmap(int inode_number) {
    int ret;
    int block_group_number = get_group_number(inode_number);
    int inode_offset = get_inode_offset(inode_number);
    int group_descripe_location = get_group_descripe_begin(block_group_number);
    struct ext2_group_desc desc;
    ret = lseek(fd, group_descripe_location, SEEK_SET);
    ret = read(fd, &desc, group_descripe_size);
    return desc.bg_inode_bitmap;
}

int block_number2block_bitmap(int block_number){
    int ret;
    int block_group_number = get_group_number_data_block(block_number);
    int block_offset = get_data_block_offset(block_number);
    int group_descripe_location = get_group_descripe_begin(block_group_number);
    struct ext2_group_desc desc;
    ret = lseek(fd, group_descripe_location, SEEK_SET);
    ret = read(fd, &desc, sizeof(desc));
    return desc.bg_block_bitmap;
}
int location2block_bitmap(int location){
    int ret;
    int group_num=location/group_size;
    int group_descripe_location=get_group_descripe_begin(group_num);
    struct ext2_group_desc desc;
    ret = lseek(fd, group_descripe_location, SEEK_SET);
    ret = read(fd, &desc, sizeof(desc));
    return desc.bg_block_bitmap;
}
int data_block_location2block_number(int data_block_location){
    int data_block_begin=258*1024;
    return (data_block_location/group_size)*blocks_per_group+
            ((data_block_location%group_size)-data_block_begin)/block_size+1;
}
int get_data_block_location(int block_number) {
    int ret;
    int block_group_number = get_group_number_data_block(block_number);
    int block_offset = get_data_block_offset(block_number);
    int group_descripe_location = get_group_descripe_begin(block_group_number);
    struct ext2_group_desc desc;
    ret = lseek(fd, group_descripe_location, SEEK_SET);
    ret = read(fd, &desc, group_descripe_size);
    int data_table_location = desc.bg_inode_table + inodes_per_group * inode_size;
    return data_table_location + block_size * block_offset;
}

int get_free_block_number(char *block_bitmap) {
    rep(i, 0, block_size - 1) {
        rep(j, 0, 7) {
            if (!(block_bitmap[i] & (1 << j))) {
                block_bitmap[i] |= (1 << j);
                return i * 8 + j + 1;
            }
        }
    }
    return -1;
}

int get_free_inode_number(char *inode_bitmap) {
    rep(i, 0, block_size - 1) {
        rep(j, 0, 7) {
            if (!(inode_bitmap[i] & (1 << j))) {
                inode_bitmap[i] |= (1 << j);
                return i * 8 + j + 1;
            }
        }
    }
    return -1;
}

bool check_str_same(char *s1, char *s2, int len) {
    if (strlen(s2) != len)return false;
    rep(i, 0, len - 1) {
        if (s1[i] != s2[i])
            return false;
    }
    return true;
}

int alloc_data_block() {
    int ret;
    struct ext2_group_desc desc;
    char block_bitmap[block_size];
    rep(i, 0, 11) {
        int block_bitmap_location = group_number2data_bitmap(i);
        ret = lseek(fd, block_bitmap_location, SEEK_SET);
        ret = read(fd, block_bitmap, sizeof(block_bitmap));
        int free_block_number = get_free_block_number(block_bitmap);
        if (free_block_number == -1)continue;
        ret = lseek(fd, block_bitmap_location, SEEK_SET);
        ret = write(fd, block_bitmap, sizeof(block_bitmap));
        return free_block_number;
    }
    return -1;
}

int alloc_inode_block() {
    //获取空闲inode
    int ret;
    struct ext2_group_desc desc;
    char inode_bitmap[block_size];
    rep(i, 0, 11) {
        int inode_bitmap_location = group_number2inode_bitmap(i);
        ret = lseek(fd, inode_bitmap_location, SEEK_SET);
        ret = read(fd, inode_bitmap, sizeof(inode_bitmap));
        int free_inode_number = get_free_inode_number(inode_bitmap);
        if (free_inode_number == -1)continue;
        ret = lseek(fd, inode_bitmap_location, SEEK_SET);
        ret = write(fd, inode_bitmap, sizeof(inode_bitmap));
        return free_inode_number;
    }
    return -1;
}

void clear_inode(int inode_number) {
    int ret;
    int inode_location = get_inode_location(inode_number);
    struct ext2_inode inode;
    ret = lseek(fd, inode_location, SEEK_SET);
    ret = read(fd, &inode, sizeof(inode));
    int data_blocks = inode.i_blocks;
    int data_block_location;
    rep(i,1,data_blocks) {
        if(i<=12){
            data_block_location=inode.i_block[i-1];
        }else if(i<=12+256){

        }else if(i<=12+256+256*256){

        }else if(i<=12+256+256*256+256*256*256){

        }
        int block_number=data_block_location2block_number(data_block_location);
        int block_bitmap_location=location2block_bitmap(data_block_location);
        char block_bitmap[block_size];
        ret = lseek(fd, block_bitmap_location, SEEK_SET);
        ret = read(fd, block_bitmap, sizeof(inode_bitmap));
        int id=(block_number-1)%blocks_per_group;
        block_bitmap[id / 8] ^= 1 << (id % 8);
        ret = lseek(fd, block_bitmap_location, SEEK_SET);
        ret = write(fd, block_bitmap, sizeof(block_bitmap));

        char clear_block[block_size]={0};
        ret = lseek(fd, data_block_location, SEEK_SET);
        ret = write(fd, clear_block, sizeof(clear_block));
    }
    memset(&inode, 0, sizeof(inode));
    ret = lseek(fd, inode_location, SEEK_SET);
    ret = write(fd, &inode, sizeof(inode));
    int inode_bitmap_location = inode_number2inode_bitmap(inode_number);
    char inode_bitmap[block_size];
    ret = lseek(fd, inode_bitmap_location, SEEK_SET);
    ret = read(fd, inode_bitmap, sizeof(inode_bitmap));
    inode_bitmap[(inode_number - 1) / 8] ^= 1 << ((inode_number - 1) % 8);
    ret = lseek(fd, inode_bitmap_location, SEEK_SET);
    ret = write(fd, inode_bitmap, sizeof(inode_bitmap));

}

void _ls() {
    if(show)printf("__ls\n");
    int ret;
    struct ext2_dir_entry_2 root;
    root.inode = 1;
    root.rec_len = 12;
    root.name_len = 1;
    root.file_type = EXT2_FT_DIR;
    root.name[0] = '~';
    // 根目录的目录
    int block_group_number = get_group_number(root.inode);
    int inode_offset = get_inode_offset(root.inode);
    int group_descripe_location = get_group_descripe_begin(block_group_number);
    struct ext2_group_desc desc;
    ret = lseek(fd, group_descripe_location, SEEK_SET);
    ret = read(fd, &desc, group_descripe_size);

    int inode_table_location = desc.bg_inode_table;
    struct ext2_inode inode;
    ret = lseek(fd, inode_table_location + inode_size * inode_offset, SEEK_SET);
    ret = read(fd, &inode, inode_size);

    struct ext2_dir_entry_2 dir[32];
    rep(i, 1, inode.i_blocks) {
        int data_block_pointer;
        if (i >= 1 && i <= 12) {
            data_block_pointer = inode.i_block[i - 1];
        } else if (i <= 12 + 256) {
            int one_inode[256];
            ret = lseek(fd, inode.i_block[12], SEEK_SET);
            ret = read(fd, one_inode, block_size);
            data_block_pointer = one_inode[inode.i_blocks - 12 - 1];

        } else if (i <= 12 + 256 + 256 * 256) {
            data_block_pointer = inode.i_block[13];

        } else if (i <= 12 + 256 + 256 * 256 + 256 * 256 * 256) {

        }
        ret = lseek(fd, data_block_pointer, SEEK_SET);
        ret = read(fd, dir, sizeof(dir));
        int dirnum = 0;
        rep(j, 0, 31) {
            if (dir[j].inode) {
                if (dir[j].file_type == 1) {
                    rep(k, 0, dir[j].name_len - 1) {
                        printf("%c", dir[j].name[k]);
                    }
                } else if (dir[j].file_type == 2) {
                    rep(k, 0, dir[j].name_len - 1) {
                        printf("\033[34m%c\033[0m", dir[j].name[k]);
                    }
                }
                dirnum++;
                if (dirnum % 5 == 0)printf("\n");
                else printf(" ");
            }
        }
        if (dirnum % 5) printf("\n");
    }
}

void _mkdir() {

    scanf("%s", dir_name);
    if(show)printf("__mkdir %s\n", dir_name);


    int ret;
    struct ext2_dir_entry_2 root;
    root.inode = 1;
    root.rec_len = 12;
    root.name_len = 1;
    root.file_type = EXT2_FT_DIR;
    root.name[0] = '~';
    // 根目录的目录
    int block_group_number = get_group_number(root.inode);
    int inode_offset = get_inode_offset(root.inode);
    int group_descripe_location = get_group_descripe_begin(block_group_number);
    struct ext2_group_desc desc;
    ret = lseek(fd, group_descripe_location, SEEK_SET);
    ret = read(fd, &desc, group_descripe_size);


    int inode_table_location = desc.bg_inode_table;
    struct ext2_inode inode;
    ret = lseek(fd, inode_table_location + inode_size * inode_offset, SEEK_SET);
    ret = read(fd, &inode, inode_size);

    int data_pointer;
    struct ext2_dir_entry_2 dir[32];
    int insert_id = -1;
    rep(insert_block, 1, inode.i_blocks) {//获取第一块有空闲的
        if (insert_block <= 12) {
            data_pointer = inode.i_block[insert_block - 1];
        } else if (insert_block <= 12 + 256) {
            int one_inode[256];
            ret = lseek(fd, inode.i_block[12], SEEK_SET);
            ret = read(fd, one_inode, block_size);
            data_pointer = one_inode[insert_block - 12 - 1];
        } else if (insert_block <= 12 + 256 + 256 * 256) {
            int one_inode[256];
            ret = lseek(fd, inode.i_block[13], SEEK_SET);
            ret = read(fd, one_inode, block_size);
            int id = (insert_block - 12 - 256 - 1) / 256;
            int two_inode[256];
            ret = lseek(fd, one_inode[id], SEEK_SET);
            ret = read(fd, two_inode, block_size);
            data_pointer = two_inode[(insert_block - 12 - 256 - 1) % 256];
        } else if (insert_block <= 12 + 256 + 256 * 256 + 256 * 256 * 256) {
            int one_inode[256];
            ret = lseek(fd, inode.i_block[14], SEEK_SET);
            ret = read(fd, one_inode, block_size);
            int id = (insert_block - 12 - 256 - 256 * 256 - 1) / 256 / 256;
            int two_inode[256];
            ret = lseek(fd, one_inode[id], SEEK_SET);
            ret = read(fd, two_inode, block_size);
            int three_inode[256];
            id = (insert_block - 12 - 256 - 256 * 256 - 1) / 256;
            ret = lseek(fd, two_inode[id], SEEK_SET);
            ret = read(fd, three_inode, block_size);
            data_pointer = three_inode[(insert_block - 12 - 256 - 256 * 256 - 1) % 256];
        }
        ret = lseek(fd, data_pointer, SEEK_SET);
        ret = read(fd, dir, sizeof(dir));
        rep(i, 0, 31) {
            if (!dir[i].inode) {
                insert_id = i;
                break;
            }
        }
        if (insert_id == -1)continue;
        break;
    }
    int free_inode_number = alloc_inode_block();

    if (insert_id == -1) {//所有块都满了，加一块
        int free_block_number = alloc_data_block();
        int data_block_location = get_data_block_location(free_block_number);
        inode.i_blocks++;
        if (inode.i_blocks >= 1 && inode.i_blocks <= 12) {
            inode.i_block[inode.i_blocks - 1] = data_block_location;
        } else if (inode.i_blocks <= 12 + 256) {
            if (inode.i_blocks == 12 + 1) {
                int free_block_number2 = alloc_data_block();
                int data_block_location2 = get_data_block_location(free_block_number2);
                inode.i_block[12] = data_block_location2;
            }
            int one_inode[256];
            ret = lseek(fd, inode.i_block[12], SEEK_SET);
            ret = read(fd, one_inode, sizeof(one_inode));
            one_inode[(inode.i_blocks - 12 - 1)] = data_block_location;
            ret = lseek(fd, inode.i_block[12], SEEK_SET);
            ret = write(fd, one_inode, sizeof(one_inode));
        } else if (inode.i_blocks <= 12 + 256 + 256 * 256) {
            if (inode.i_blocks == 12 + 256 + 1) {
                int free_block_number2 = alloc_data_block();
                int data_block_location2 = get_data_block_location(free_block_number2);
                inode.i_block[13] = data_block_location2;
            }
            int one_inode[256];
            ret = lseek(fd, inode.i_block[13], SEEK_SET);
            ret = read(fd, one_inode, sizeof(one_inode));
            int id = (inode.i_blocks - 12 - 256 - 1) / 256;
            if ((inode.i_blocks - 12 - 256 - 1) % 256 == 0) {
                int free_block_number3 = alloc_data_block();
                int data_block_location3 = get_data_block_location(free_block_number3);
                one_inode[id] = data_block_location3;
                ret = lseek(fd, inode.i_block[13], SEEK_SET);
                ret = write(fd, one_inode, sizeof(one_inode));
            }
            int two_inode[256];
            ret = lseek(fd, one_inode[id], SEEK_SET);
            ret = read(fd, two_inode, sizeof(two_inode));
            two_inode[(inode.i_blocks - 12 - 256 - 1) % 256] = data_block_location;
            ret = lseek(fd, two_inode[id], SEEK_SET);
            ret = write(fd, two_inode, sizeof(two_inode));
        } else if (inode.i_blocks <= 12 + 256 + 256 * 256 + 256 * 256 * 256) {

        }
        ret = lseek(fd, inode_table_location + inode_size * inode_offset, SEEK_SET);
        ret = write(fd, &inode, sizeof(inode));

        ret = lseek(fd, data_block_location, SEEK_SET);
        ret = read(fd, dir, sizeof(dir));
        data_pointer = data_block_location;
        insert_id = 0;
    }

    struct ext2_inode write_inode;
    int free_inode_location = get_inode_location(free_inode_number);
    ret = lseek(fd, free_inode_location, SEEK_SET);
    ret = read(fd, &write_inode, sizeof(write_inode));
    write_inode.i_flags = 2;
    ret = lseek(fd, free_inode_location, SEEK_SET);
    ret = write(fd, &write_inode, sizeof(write_inode));

    int len = strlen(dir_name);
    dir[insert_id].inode = free_inode_number;
    dir[insert_id].rec_len = 32;
    dir[insert_id].name_len = std::min(24, len);
    dir[insert_id].file_type = EXT2_FT_DIR;
    rep(i, 0, dir[insert_id].name_len - 1) {
        dir[insert_id].name[i] = dir_name[i];
    }
    ret = lseek(fd, data_pointer, SEEK_SET);
    ret = write(fd, dir, sizeof(dir));
//    printf("insert id=%d\n",insert_id);
}
int file_name2inode_number(char *file_name){
    int ret;
    struct ext2_dir_entry_2 root;
    root.inode = 1;
    root.rec_len = 12;
    root.name_len = 1;
    root.file_type = EXT2_FT_DIR;
    root.name[0] = '~';
    // 根目录的目录
    int block_group_number = get_group_number(root.inode);
    int inode_offset = get_inode_offset(root.inode);
    int group_descripe_location = get_group_descripe_begin(block_group_number);
    struct ext2_group_desc desc;
    ret = lseek(fd, group_descripe_location, SEEK_SET);
    ret = read(fd, &desc, group_descripe_size);


    int inode_table_location = desc.bg_inode_table;
    struct ext2_inode inode;
    ret = lseek(fd, inode_table_location + inode_size * inode_offset, SEEK_SET);
    ret = read(fd, &inode, inode_size);

    struct ext2_dir_entry_2 dir[32];
    rep(i, 1, inode.i_blocks) {
        int data_block_pointer;
        if (i >= 1 && i <= 12) {
            data_block_pointer = inode.i_block[i - 1];
        } else if (i <= 12 + 256) {
            int one_inode[256];
            ret = lseek(fd, inode.i_block[12], SEEK_SET);
            ret = read(fd, one_inode, block_size);
            data_block_pointer = one_inode[inode.i_blocks - 12 - 1];

        } else if (i <= 12 + 256 + 256 * 256) {
            data_block_pointer = inode.i_block[13];

        } else if (i <= 12 + 256 + 256 * 256 + 256 * 256 * 256) {

        }
        ret = lseek(fd, data_block_pointer, SEEK_SET);
        ret = read(fd, dir, sizeof(dir));
        int delete_id = -1;
        rep(j, 0, 31) {
            if (check_str_same(dir[j].name, file_name, dir[j].name_len)) {
                delete_id = j;
                break;
            }
        }
        if (delete_id != -1) {
            return dir[delete_id].inode;
        }
    }
    return -1;
}
void _rmdir() {

    scanf("%s", dir_name);
    if(show)printf("__rmdir %s\n", dir_name);

    int ret;
    struct ext2_dir_entry_2 root;
    root.inode = 1;
    root.rec_len = 12;
    root.name_len = 1;
    root.file_type = EXT2_FT_DIR;
    root.name[0] = '~';
    // 根目录的目录
    int block_group_number = get_group_number(root.inode);
    int inode_offset = get_inode_offset(root.inode);
    int group_descripe_location = get_group_descripe_begin(block_group_number);
    struct ext2_group_desc desc;
    ret = lseek(fd, group_descripe_location, SEEK_SET);
    ret = read(fd, &desc, group_descripe_size);


    int inode_table_location = desc.bg_inode_table;
    struct ext2_inode inode;
    ret = lseek(fd, inode_table_location + inode_size * inode_offset, SEEK_SET);
    ret = read(fd, &inode, inode_size);

    struct ext2_dir_entry_2 dir[32];
    rep(i, 1, inode.i_blocks) {
        int data_block_pointer;
        if (i >= 1 && i <= 12) {
            data_block_pointer = inode.i_block[i - 1];
        } else if (i <= 12 + 256) {
            int one_inode[256];
            ret = lseek(fd, inode.i_block[12], SEEK_SET);
            ret = read(fd, one_inode, block_size);
            data_block_pointer = one_inode[inode.i_blocks - 12 - 1];

        } else if (i <= 12 + 256 + 256 * 256) {
            data_block_pointer = inode.i_block[13];

        } else if (i <= 12 + 256 + 256 * 256 + 256 * 256 * 256) {

        }
        ret = lseek(fd, data_block_pointer, SEEK_SET);
        ret = read(fd, dir, sizeof(dir));
        int delete_id = -1;
        rep(j, 0, 31) {
            if (check_str_same(dir[j].name, dir_name, dir[j].name_len)) {
                delete_id = j;
                break;
            }
        }
        if (delete_id != -1) {
            int inode_number = dir[delete_id].inode;
            clear_inode(inode_number);
            memset(&dir[delete_id], 0, sizeof(dir[delete_id]));
            ret = lseek(fd, data_block_pointer, SEEK_SET);
            ret = write(fd, dir, sizeof(dir));
            break;
        }
    }

}

void _mv() {

    scanf("%s", old_file_name);
    scanf("%s", new_file_name);
    if(show)printf("__mv %s %s\n", old_file_name, new_file_name);

    int ret;
    struct ext2_dir_entry_2 root;
    root.inode = 1;
    root.rec_len = 12;
    root.name_len = 1;
    root.file_type = EXT2_FT_DIR;
    root.name[0] = '~';
    // 根目录的目录
    int block_group_number = get_group_number(root.inode);
    int inode_offset = get_inode_offset(root.inode);
    int group_descripe_location = get_group_descripe_begin(block_group_number);
    struct ext2_group_desc desc;
    ret = lseek(fd, group_descripe_location, SEEK_SET);
    ret = read(fd, &desc, group_descripe_size);

    int inode_table_location = desc.bg_inode_table;
    struct ext2_inode inode;
    ret = lseek(fd, inode_table_location + inode_size * inode_offset, SEEK_SET);
    ret = read(fd, &inode, inode_size);

    struct ext2_dir_entry_2 dir[32];
    rep(i, 1, inode.i_blocks) {
        int data_block_pointer;
        if (i >= 1 && i <= 12) {
            data_block_pointer = inode.i_block[i - 1];
        } else if (i <= 12 + 256) {
            int one_inode[256];
            ret = lseek(fd, inode.i_block[12], SEEK_SET);
            ret = read(fd, one_inode, block_size);
            data_block_pointer = one_inode[inode.i_blocks - 12 - 1];

        } else if (i <= 12 + 256 + 256 * 256) {
            data_block_pointer = inode.i_block[13];

        } else if (i <= 12 + 256 + 256 * 256 + 256 * 256 * 256) {

        }
        ret = lseek(fd, data_block_pointer, SEEK_SET);
        ret = read(fd, dir, sizeof(dir));
        int change_id = -1;
        rep(j, 0, 31) {
            if (check_str_same(dir[j].name, old_file_name, dir[j].name_len)) {
                change_id = j;
                break;
            }
        }
        if (change_id != -1) {
            dir[change_id].name_len = std::min(24ul, strlen(new_file_name));
            rep(j, 0, dir[change_id].name_len - 1) {
                dir[change_id].name[j] = new_file_name[j];
            }
            ret = lseek(fd, data_block_pointer, SEEK_SET);
            ret = write(fd, dir, sizeof(dir));
            break;
        }
    }

}

void _open() {

    scanf("%s", file_name);
    if(show)printf("__open %s\n", file_name);
    int ret;
    struct ext2_dir_entry_2 root;
    root.inode = 1;
    root.rec_len = 12;
    root.name_len = 1;
    root.file_type = EXT2_FT_DIR;
    root.name[0] = '~';
    // 根目录的目录
    int block_group_number = get_group_number(root.inode);
    int inode_offset = get_inode_offset(root.inode);
    int group_descripe_location = get_group_descripe_begin(block_group_number);
    struct ext2_group_desc desc;
    ret = lseek(fd, group_descripe_location, SEEK_SET);
    ret = read(fd, &desc, group_descripe_size);


    int inode_table_location = desc.bg_inode_table;
    struct ext2_inode inode;
    ret = lseek(fd, inode_table_location + inode_size * inode_offset, SEEK_SET);
    ret = read(fd, &inode, inode_size);

    int data_pointer;
    struct ext2_dir_entry_2 dir[32];
    int insert_id = -1;
    rep(insert_block, 1, inode.i_blocks) {//获取第一块有空闲的
        if (insert_block <= 12) {
            data_pointer = inode.i_block[insert_block - 1];
        } else if (insert_block <= 12 + 256) {
            int one_inode[256];
            ret = lseek(fd, inode.i_block[12], SEEK_SET);
            ret = read(fd, one_inode, block_size);
            data_pointer = one_inode[insert_block - 12 - 1];
        } else if (insert_block <= 12 + 256 + 256 * 256) {
            int one_inode[256];
            ret = lseek(fd, inode.i_block[13], SEEK_SET);
            ret = read(fd, one_inode, block_size);
            int id = (insert_block - 12 - 256 - 1) / 256;
            int two_inode[256];
            ret = lseek(fd, one_inode[id], SEEK_SET);
            ret = read(fd, two_inode, block_size);
            data_pointer = two_inode[(insert_block - 12 - 256 - 1) % 256];
        } else if (insert_block <= 12 + 256 + 256 * 256 + 256 * 256 * 256) {
            int one_inode[256];
            ret = lseek(fd, inode.i_block[14], SEEK_SET);
            ret = read(fd, one_inode, block_size);
            int id = (insert_block - 12 - 256 - 256 * 256 - 1) / 256 / 256;
            int two_inode[256];
            ret = lseek(fd, one_inode[id], SEEK_SET);
            ret = read(fd, two_inode, block_size);
            int three_inode[256];
            id = (insert_block - 12 - 256 - 256 * 256 - 1) / 256;
            ret = lseek(fd, two_inode[id], SEEK_SET);
            ret = read(fd, three_inode, block_size);
            data_pointer = three_inode[(insert_block - 12 - 256 - 256 * 256 - 1) % 256];
        }
        ret = lseek(fd, data_pointer, SEEK_SET);
        ret = read(fd, dir, sizeof(dir));
        rep(i, 0, 31) {
            if (!dir[i].inode) {
                insert_id = i;
                break;
            }
        }
        if (insert_id == -1)continue;
        break;
    }
    int free_inode_number = alloc_inode_block();

    if (insert_id == -1) {//所有块都满了，加一块
        int free_block_number = alloc_data_block();
        int data_block_location = get_data_block_location(free_block_number);
        inode.i_blocks++;
        if (inode.i_blocks >= 1 && inode.i_blocks <= 12) {
            inode.i_block[inode.i_blocks - 1] = data_block_location;
        } else if (inode.i_blocks <= 12 + 256) {
            if (inode.i_blocks == 12 + 1) {
                int free_block_number2 = alloc_data_block();
                int data_block_location2 = get_data_block_location(free_block_number2);
                inode.i_block[12] = data_block_location2;
            }
            int one_inode[256];
            ret = lseek(fd, inode.i_block[12], SEEK_SET);
            ret = read(fd, one_inode, sizeof(one_inode));
            one_inode[(inode.i_blocks - 12 - 1)] = data_block_location;
            ret = lseek(fd, inode.i_block[12], SEEK_SET);
            ret = write(fd, one_inode, sizeof(one_inode));
        } else if (inode.i_blocks <= 12 + 256 + 256 * 256) {
            if (inode.i_blocks == 12 + 256 + 1) {
                int free_block_number2 = alloc_data_block();
                int data_block_location2 = get_data_block_location(free_block_number2);
                inode.i_block[13] = data_block_location2;
            }
            int one_inode[256];
            ret = lseek(fd, inode.i_block[13], SEEK_SET);
            ret = read(fd, one_inode, sizeof(one_inode));
            int id = (inode.i_blocks - 12 - 256 - 1) / 256;
            if ((inode.i_blocks - 12 - 256 - 1) % 256 == 0) {
                int free_block_number3 = alloc_data_block();
                int data_block_location3 = get_data_block_location(free_block_number3);
                one_inode[id] = data_block_location3;
                ret = lseek(fd, inode.i_block[13], SEEK_SET);
                ret = write(fd, one_inode, sizeof(one_inode));
            }
            int two_inode[256];
            ret = lseek(fd, one_inode[id], SEEK_SET);
            ret = read(fd, two_inode, sizeof(two_inode));
            two_inode[(inode.i_blocks - 12 - 256 - 1) % 256] = data_block_location;
            ret = lseek(fd, two_inode[id], SEEK_SET);
            ret = write(fd, two_inode, sizeof(two_inode));
        } else if (inode.i_blocks <= 12 + 256 + 256 * 256 + 256 * 256 * 256) {

        }
        ret = lseek(fd, inode_table_location + inode_size * inode_offset, SEEK_SET);
        ret = write(fd, &inode, sizeof(inode));

        ret = lseek(fd, data_block_location, SEEK_SET);
        ret = read(fd, dir, sizeof(dir));
        data_pointer = data_block_location;
        insert_id = 0;
    }

    struct ext2_inode write_inode;
    int free_inode_location = get_inode_location(free_inode_number);
    ret = lseek(fd, free_inode_location, SEEK_SET);
    ret = read(fd, &write_inode, sizeof(write_inode));
    write_inode.i_flags = 1;
    ret = lseek(fd, free_inode_location, SEEK_SET);
    ret = write(fd, &write_inode, sizeof(write_inode));

    int len = strlen(file_name);
    dir[insert_id].inode = free_inode_number;
    dir[insert_id].rec_len = 32;
    dir[insert_id].name_len = std::min(24, len);
    dir[insert_id].file_type = EXT2_FT_REG_FILE;
    rep(i, 0, dir[insert_id].name_len - 1) {
        dir[insert_id].name[i] = file_name[i];
    }
    ret = lseek(fd, data_pointer, SEEK_SET);
    ret = write(fd, dir, sizeof(dir));
//    printf("insert id=%d\n",insert_id);

    char sem_str[32]={0};
    rep(i,0,dir[insert_id].name_len - 1){
        sem_str[i]=dir[insert_id].name[i];
    }
    //read_queue
    sem_str[dir[insert_id].name_len]='r';
    sem_str[dir[insert_id].name_len+1]='q';
    sem_unlink(sem_str);
    sem_open(sem_str,O_CREAT,0644,1);

    //write_queue
    sem_str[dir[insert_id].name_len]='w';
    sem_str[dir[insert_id].name_len+1]='q';
    sem_unlink(sem_str);
    sem_open(sem_str,O_CREAT,0644,1);

    //read_number
    sem_str[dir[insert_id].name_len]='r';
    sem_str[dir[insert_id].name_len+1]='n';
    sem_unlink(sem_str);
    sem_open(sem_str,O_CREAT,0644,0);
}

void _edit() {

    scanf("%s", file_name);
    getchar();
    if(show)printf("__edit %s\n",file_name);
    int len=strlen(file_name);
    char file_write_queue[32]={0};
    strcpy(file_write_queue,file_name);
    file_write_queue[len]='w';
    file_write_queue[len+1]='q';
    sem_t *sem_write_queue=sem_open(file_write_queue,0);

//    printf("waiting write_queue %s\n",file_write_queue);
    int val;
    sem_wait(sem_write_queue);
//    sem_getvalue(sem_write_queue,&val);
//    printf("write val=%d\n",val);
//    printf("waiting write_queue %s ac\n",file_write_queue);

    char write_buffer[block_size/8];
    char write_block[block_size/8];

    int ret;
    int inode_number=file_name2inode_number(file_name);
    int inode_location=get_inode_location(inode_number);
    struct ext2_inode inode;
    ret = lseek(fd, inode_location, SEEK_SET);
    ret = read(fd, &inode, sizeof(inode));
    inode.i_size=0;
    inode.i_blocks=0;
    memset(write_block,0,sizeof(write_block));
    while(1){
        fgets(write_buffer,sizeof(write_buffer),stdin);
        if(show) printf("%s",write_buffer);
        if(write_buffer[0]=='q'&&write_buffer[1]=='u'&&write_buffer[2]=='i'&&write_buffer[3]=='t'&&write_buffer[4]==10)break;
        int len=strlen(write_buffer);
        if(strlen(write_block)+len>block_size/8) {
            int more = strlen(write_block) + len - block_size / 8;
            int last = len - more;
            for (int i = block_size / 8 - last, j = 0; i <= block_size / 8 - 1; i++, j++) {
                write_block[i] = write_buffer[j];
            }
            for (int i = 0, j = last; i <more; i++, j++) {
                write_buffer[i] = write_buffer[j];
            }
            for(int i=more;i<block_size/8;i++){
                write_buffer[i]=0;
            }
            int free_block_number=alloc_data_block();
            int data_block_location = get_data_block_location(free_block_number);
            inode.i_block[inode.i_blocks]=data_block_location;
            ret = lseek(fd, data_block_location, SEEK_SET);
            ret = write(fd, write_block, sizeof(write_block));
            inode.i_blocks++;
            inode.i_size+=block_size/8;
            memset(write_block,0,sizeof(write_block));
        }
        strcat(write_block,write_buffer);
    }
    if(strlen(write_block)){
        int free_block_number=alloc_data_block();
        int data_block_location = get_data_block_location(free_block_number);
        inode.i_block[inode.i_blocks]=data_block_location;
        rep(i, strlen(write_block),block_size / 8-1){
            write_block[i]=0;
        }
        ret = lseek(fd, data_block_location, SEEK_SET);
        ret = write(fd, write_block, sizeof(write_block));
        inode.i_blocks++;
        inode.i_size+=strlen(write_block);
    }
    ret = lseek(fd, inode_location, SEEK_SET);
    ret = write(fd, &inode, sizeof(inode));

    sem_post(sem_write_queue);
}

void _read(){
    scanf("%s", file_name);
    if(show)printf("__read %s\n",file_name);
    int len= strlen(file_name);
    char file_read_queue[32]={0};
    char file_write_queue[32]={0};
    char file_read_number[32]={0};
    strcpy(file_read_queue,file_name);
    strcpy(file_write_queue,file_name);
    strcpy(file_read_number,file_name);
    file_read_queue[len]='r';
    file_read_queue[len+1]='q';
    file_write_queue[len]='w';
    file_write_queue[len+1]='q';
    file_read_number[len]='r';
    file_read_number[len+1]='n';
    sem_t *sem_read_queue=sem_open(file_read_queue,0);
    sem_t *sem_write_queue=sem_open(file_write_queue,0);
    sem_t *sem_read_number=sem_open(file_read_number,0);
    int val;
    sem_wait(sem_read_queue);
    sem_getvalue(sem_read_number,&val);
    if(val==0){
        sem_wait(sem_write_queue);
    }
    sem_post(sem_read_number);
    sem_post(sem_read_queue);

    char read_buffer[block_size/8];
    int ret;
    int inode_number=file_name2inode_number(file_name);
    int inode_location=get_inode_location(inode_number);
    struct ext2_inode inode;
    ret = lseek(fd, inode_location, SEEK_SET);
    ret = read(fd, &inode, sizeof(inode));
    rep(i,1,inode.i_blocks){
        int len;
        if(i<=12){
            if(i==inode.i_blocks){
                len=inode.i_size-(i-1)*sizeof(read_buffer);
            }else{
                len=sizeof(read_buffer);
            }
            ret = lseek(fd, inode.i_block[i-1], SEEK_SET);
            ret = read(fd, read_buffer, len);
        }else{

        }
        rep(j,0,len-1){
            printf("%c",read_buffer[j]);
        }
    }

    sem_wait(sem_read_queue);
    sem_wait(sem_read_number);
    sem_getvalue(sem_read_number,&val);
    if(val==0)sem_post(sem_write_queue);
    sem_post(sem_read_queue);
}

void _rm() {

    scanf("%s", file_name);
    if(show)printf("__rm %s\n", file_name);

    int ret;
    struct ext2_dir_entry_2 root;
    root.inode = 1;
    root.rec_len = 12;
    root.name_len = 1;
    root.file_type = EXT2_FT_DIR;
    root.name[0] = '~';
    // 根目录的目录
    int block_group_number = get_group_number(root.inode);
    int inode_offset = get_inode_offset(root.inode);
    int group_descripe_location = get_group_descripe_begin(block_group_number);
    struct ext2_group_desc desc;
    ret = lseek(fd, group_descripe_location, SEEK_SET);
    ret = read(fd, &desc, group_descripe_size);


    int inode_table_location = desc.bg_inode_table;
    struct ext2_inode inode;
    ret = lseek(fd, inode_table_location + inode_size * inode_offset, SEEK_SET);
    ret = read(fd, &inode, inode_size);

    struct ext2_dir_entry_2 dir[32];
    rep(i, 1, inode.i_blocks) {
        int data_block_pointer;
        if (i >= 1 && i <= 12) {
            data_block_pointer = inode.i_block[i - 1];
        } else if (i <= 12 + 256) {
            int one_inode[256];
            ret = lseek(fd, inode.i_block[12], SEEK_SET);
            ret = read(fd, one_inode, block_size);
            data_block_pointer = one_inode[inode.i_blocks - 12 - 1];

        } else if (i <= 12 + 256 + 256 * 256) {
            data_block_pointer = inode.i_block[13];

        } else if (i <= 12 + 256 + 256 * 256 + 256 * 256 * 256) {

        }
        ret = lseek(fd, data_block_pointer, SEEK_SET);
        ret = read(fd, dir, sizeof(dir));
        int delete_id = -1;
        rep(j, 0, 31) {
            if (check_str_same(dir[j].name, file_name, dir[j].name_len)) {
                delete_id = j;
                break;
            }
        }
        if (delete_id != -1) {
            int inode_number = dir[delete_id].inode;
            clear_inode(inode_number);
            memset(&dir[delete_id], 0, sizeof(dir[delete_id]));
            ret = lseek(fd, data_block_pointer, SEEK_SET);
            ret = write(fd, dir, sizeof(dir));
            break;
        }
    }
    char sem_str[32]={0};
    int len=strlen(file_name);
    rep(i,0,len-1){
        sem_str[i]= file_name[i];
    }
    //read_queue
    sem_str[len]='r';
    sem_str[len+1]='q';
    sem_unlink(sem_str);

    //write_queue
    sem_str[len]='w';
    sem_str[len+1]='q';
    sem_unlink(sem_str);

    //read_number
    sem_str[len]='r';
    sem_str[len+1]='n';
    sem_unlink(sem_str);
}

char arr[100 * MB] = {0};

int main(int argc,char **argv) {
    if(argc==2){
        show=true;
        freopen("data", "r", stdin);
    }
    int ret = -1;
    char op[256];
    fd = open("memory", O_RDWR);

    if (-1 == fd) {
        perror("文件打开错误");
        return -1;
    }
    while (1) {
        printf("\033[32;1mleung-0ng@0ng\033[0m");
        printf("\033[37;1m:\033[0m");
        printf("\033[34;1m~\033[0m");
        printf("\033[37m$ \033[0m");
        scanf("%s", op);
        if (strcmp(op, "quit") == 0) {
            if(show)printf("quit\n");
            break;
        } else if (strcmp(op, "__ls") == 0) {
            _ls();
        } else if (strcmp(op, "__mkdir") == 0) {
            _mkdir();
        } else if (strcmp(op, "__rmdir") == 0) {
            _rmdir();
        } else if (strcmp(op, "__mv") == 0) {
            _mv();
        } else if (strcmp(op, "__open") == 0) {
            _open();
        } else if (strcmp(op, "__edit") == 0) {
            _edit();
        } else if (strcmp(op, "__rm") == 0) {
            _rm();
        } else if(strcmp(op,"__read")==0){
            _read();
        }else if (strcmp(op, "init_memory") == 0) {
            if(show)printf("init_memory\n");
            ret = lseek(fd, 0, SEEK_SET);
            ret = write(fd, arr, sizeof(arr));
            int ret;
            struct ext2_dir_entry_2 root;
            root.inode = 1;
            root.rec_len = 12;
            root.name_len = 1;
            root.file_type = EXT2_FT_DIR;
            root.name[0] = '~';
            // 根目录的目录
            int block_group_number = get_group_number(root.inode);
            int inode_offset = get_inode_offset(root.inode);
            int group_descripe_location = get_group_descripe_begin(block_group_number);
            struct ext2_group_desc desc;
            ret = lseek(fd, group_descripe_location, SEEK_SET);
            ret = read(fd, &desc, sizeof(desc));
            desc.bg_block_bitmap = 2 * block_size;
            desc.bg_inode_bitmap = 3 * block_size;
            desc.bg_inode_table = 4 * block_size;
            ret = lseek(fd, group_descripe_location, SEEK_SET);
            ret = write(fd, &desc, sizeof(desc));
            char inode_bitmap[block_size] = {0};
            inode_bitmap[0] = 1;
            ret = lseek(fd, desc.bg_inode_bitmap, SEEK_SET);
            ret = write(fd, inode_bitmap, sizeof(inode_bitmap));
        }
    }
    return 0;
}
