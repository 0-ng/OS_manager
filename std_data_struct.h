/*************************************************************************
	> File Name: std_data_struct.c
	> Author: ma6174
	> Mail: ma6174@163.com 
	> Created Time: 2021年06月18日 星期五 14时25分55秒
 ************************************************************************/

#include<stdio.h>
const int inode_size=128;
const int inodes_per_group=2032;
const int blocks_per_group=7932;
const int block_size=1024;
const int group_descripe_size=32;
const int group_size=8*1024*1024;
typedef int __le32;
typedef short __le16;
typedef unsigned int __u32;
typedef unsigned short __u16;
typedef unsigned char __u8;
struct ext2_super_block {
    __le32 s_inodes_count; /* Inodes count  索引节点总数*/
    __le32 s_blocks_count; /* Blocks count  盘块的总数*/
    __le32 s_r_blocks_count; /* Reserved blocks count  保留的盘块数*/
    __le32 s_free_blocks_count; /* Free blocks count  空闲盘块数*/
    __le32 s_free_inodes_count; /* Free inodes count  空闲索引节点数*/
    __le32 s_first_data_block; /* First Data Block  第一个数据盘块号*/
    __le32 s_log_block_size; /* Block size  盘块大小（以2的幂次表示）*/
    __le32 s_log_frag_size; /* Fragment size  片的大小*/
    __le32 s_blocks_per_group; /* # Blocks per group  块组内部的盘块数量*/
    __le32 s_frags_per_group; /* # Fragments per group 块组中的片数*/
    __le32 s_inodes_per_group; /* # Inodes per group  块组内的索引节点数*/
    __le32 s_mtime; /* Mount time  最后一次安装挂载时间*/
    __le32 s_wtime; /* Write time  最后一次写操作的时间*/
    __le16 s_mnt_count; /* Mount count  安装挂载次数（检查后清零）*/
    __le16 s_max_mnt_count; /* Maximal mount count 最大挂载次数，超过则触发文件系统检查*/

    __le16 s_magic; /* Magic signature  文件系统魔数（标志）*/
    __le16 s_state; /* File system state  文件系统状态*/
    __le16 s_errors; /* Behaviour when detecting errors 出错时的行为*/

    __le16 s_minor_rev_level; /* minor revision level  文件系统的次版本号*/
    __le32 s_lastcheck; /* time of last check 上一次文件系统检查时间*/

    __le32 s_checkinterval; /* max. time between checks  两次检查之间的间隔*/

    __le32 s_creator_os; /* OS  创建该文件系统的操作系统*/
    __le32 s_rev_level; /* Revision level  版本号*/
    __le16 s_def_resuid; /* Default uid for reserved blocks 保留块的缺省UID*/

    __le16 s_def_resgid; /* Default gid for reserved blocks 保留块的缺省用户组 ID*/

/*
* These fields are for EXT2_DYNAMIC_REV superblocks only.
*
* Note: the difference between the compatible feature set and
* the incompatible feature set is that if there is a bit set
* in the incompatible feature set that the kernel doesn't
* know about, it should refuse to mount the filesystem.
*
* e2fsck's requirements are more strict; if it doesn't know
* about a feature in either the compatible or incompatible
* feature set, it must abort and not try to meddle with
* things it doesn't understand...
*/
    __le32 s_first_ino; /* First non-reserved inode 第一个非保留的索引节点号*/

    __le16 s_inode_size; /* size of inode structure  索引节点大小*/
    __le16 s_block_group_nr; /* block group # of this superblock  本超级块的块组号*/

    __le32 s_feature_compat; /* compatible feature set 具有兼容特点的位图*/

    __le32 s_feature_incompat; /* incompatible feature set 具有非兼容特点的位图*/

    __le32 s_feature_ro_compat; /* readonly-compatible feature set 只读特点的位图*/

    __u8 s_uuid[16]; /* 128-bit uuid for volume 位的文件系统标识符*/

    char s_volume_name[16]; /* volume name  卷名*/
    char s_last_mounted[64]; /* directory where last mounted  最 后一个安装点路径名*/

    __le32 s_algorithm_usage_bitmap; /* For compression  用于压缩*/
/*
* Performance hints. Directory preallocation should only
* happen if the EXT2_COMPAT_PREALLOC flag is on.
*/
    __u8 s_prealloc_blocks; /* Nr of blocks to try to preallocate预分配的盘块数*/

    __u8 s_prealloc_dir_blocks; /* Nr to preallocate for dirs  目录的预分配盘块数*/

    __u16 s_padding1; /*（填充位，无具体功能）*/
/*
* Journaling support valid if EXT3_FEATURE_COMPAT_HAS_JOURNAL set
 以下是 EXT3 日志相关的*/
    __u8 s_journal_uuid[16]; /* uuid of journal superblock */
    __u32 s_journal_inum; /* inode number of journal file */
    __u32 s_journal_dev; /* device number of journal file */
    __u32 s_last_orphan; /* start of list of inodes to delete */
    __u32 s_hash_seed[4]; /* HTREE hash seed */
    __u8 s_def_hash_version; /* Default hash version to use */
    __u8 s_reserved_char_pad;
    __u16 s_reserved_word_pad;
    __le32 s_default_mount_opts;
    __le32 s_first_meta_bg; /* First metablock block group */
    __u32 s_reserved[190]; /* Padding to the end of the block */
};

struct ext2_group_desc {
    __le32 bg_block_bitmap; /* Blocks bitmap block 数据盘块位图所在的盘块号*/

    __le32 bg_inode_bitmap; /* Inodes bitmap block 索引节点位图所在的盘块号*/

    __le32 bg_inode_table; /* Inodes table block 索引节点表的起点所在的盘块号*/
    __le16 bg_free_blocks_count; /* Free blocks count  空闲盘块数*/
    __le16 bg_free_inodes_count; /* Free inodes count  空闲索引节点数*/
    __le16 bg_used_dirs_count; /* Directories count  在用目录个数*/
    __le16 bg_pad; /*字节对齐的填充*/
    __le32 bg_reserved[3]; /*保留*/
};


struct ext2_inode {
    __le16 i_mode; /* File mode  文件类型和访问权限*/
    __le16 i_uid; /* Low 16 bits of Owner Uid  所有者的 UID（低16位）*/
    __le32 i_size; /* Size in bytes  文件长度（字节）*/
    __le32 i_atime; /* Access time  访问时间戳*/
    __le32 i_ctime; /* Creation time  创建时间戳*/
    __le32 i_mtime; /* Modification time  修改时间戳*/
    __le32 i_dtime; /* Deletion Time  删除时间戳*/
    __le16 i_gid; /* Low 16 bits of Group Id  所有者 GID（低 16 位）*/
    __le16 i_links_count; /* Links count  硬链接计数*/
    __le32 i_blocks; /* Blocks count  文件长度（block 计数）*/
    __le32 i_flags; /* File flags  文件标志*/
    union {
        struct {
            __le32 l_i_reserved1;
        } linux1; //Linux 中特定的信息之 1
        struct {
            __le32 h_i_translator;
        } hurd1;
        struct {
            __le32 m_i_reserved1;
        } masix1;
    } osd1; /* OS dependent 1 */
    __le32 i_block[15];/* Pointers to blocks  数据盘块指针数组*/
    __le32 i_generation; /* File version (for NFS) */
    __le32 i_file_acl; /* File ACL  文件访问控制列表*/
    __le32 i_dir_acl; /* Directory ACL  目录访问控制列表*/
    __le32 i_faddr; /* Fragment address */
    union {
        struct {
            __u8 l_i_frag; /* Fragment number */
            __u8 l_i_fsize; /* Fragment size */
            __u16 i_pad1;
            __le16 l_i_uid_high; /* these 2 fields */
            __le16 l_i_gid_high; /* were reserved2[0] */
            __u32 l_i_reserved2;
        } linux2; //Linux 中特定的信息之 2
        struct {
            __u8 h_i_frag; /* Fragment number */
            __u8 h_i_fsize; /* Fragment size */
            __le16 h_i_mode_high;
            __le16 h_i_uid_high;
            __le16 h_i_gid_high;
            __le32 h_i_author;
        } hurd2;
        struct {
            __u8 m_i_frag; /* Fragment number */
            __u8 m_i_fsize; /* Fragment size */
            __u16 m_pad1;
            __u32 m_i_reserved2[2];
        } masix2;
    } osd2; /* OS dependent 2 */
};


struct ext2_dir_entry_2 {
    __le32 inode; /* Inode number */
    __le16 rec_len; /* Directory entry length */
    __u8 name_len; /* Name length */
    __u8 file_type; //文件类型（见下面 行）
    char name[24]; /* File name, up to EXT2_NAME_LEN */
};
enum { //ext2_dir_entry_2 中的 file_type 枚举类型
    EXT2_FT_UNKNOWN = 0,
    EXT2_FT_REG_FILE = 1, //普通文件
    EXT2_FT_DIR = 2, //目录文件
    EXT2_FT_CHRDEV = 3, //字符设备文件
    EXT2_FT_BLKDEV = 4, //块设备文件
    EXT2_FT_FIFO = 5, //命名管道 FIFO
    EXT2_FT_SOCK = 6, //SOCK
    EXT2_FT_SYMLINK = 7, //符号链接
    EXT2_FT_MAX89
};

struct inode_bitmap{
    bool mp[block_size];
};

