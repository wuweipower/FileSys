#ifndef DISK
#define DISK
/**
 * @author Xin Huang
 * @brief used to read the stored infomation about the filesystem
 * and save the information when exit
*/
#include<iostream>
#include<fstream>
#include<cmath>
#include<string.h>
using std::ios;
using std::fstream;
using std::ifstream;
using std::ofstream;
class SuperBlock //use the first few blocks to store
{
public:
    static const int BLOCK_NUM=16*1024;           // total blocks
    static const int INODE_NUM=128*(1024/128);    // use the first 128 blocks to store inode

    bool block_bitmap[BLOCK_NUM];                 // 16*1024 B
    bool inode_bitmap[INODE_NUM];                 // 1024 B

    const static unsigned int block_size=1024;    // 1024B for a block
    const static unsigned int inode_size=128;     // the size of one inode structure 128byte
    unsigned int blocks_used;                     // 32B
    unsigned int free_blocks;                     // 32B
    unsigned int inode_used;
    unsigned int free_inode;

    // the first 20 blocks are used for superblock Byte
    // and the first inode is the root
    // it is an address
    const static int inode_begin=20*1024;
                     
    const static int data_begin=inode_begin+INODE_NUM*inode_size;   // superblock + inode area  Byte

    const static int data_area_start_index = 148;

    int convertBlockIdToAddr(int id);
    int convertInodeIdToAddr(int id);
    // const int inode_area_start_index = 20;
};

class Disk
{
private:
    fstream fs;
public:
    SuperBlock superBlock;
    Disk();
    ~Disk();
    void init();                                  //最初始的操作
    void open();
    void close();

};


#endif