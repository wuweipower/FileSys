#include"Disk.h"

Disk::Disk()
{
    open();
    fs.read(reinterpret_cast<char*>(&superBlock),sizeof(SuperBlock));
    close();
}

void Disk::init()
{
    memset(superBlock.block_bitmap,0,sizeof(superBlock.block_bitmap));
    //for superblock
    memset(superBlock.block_bitmap,1,20);
    memset(superBlock.inode_bitmap,0,sizeof(superBlock.inode_bitmap));
    
    superBlock.blocks_used=20;
    superBlock.free_blocks=SuperBlock::BLOCK_NUM-20;
    superBlock.inode_used=0;
    superBlock.free_inode=SuperBlock::INODE_NUM;

    open();
    //format the disk with all zero
    for(int i =0;i<16*1024;i++)
    {
        char buf[1024]={0};
        fs.write(buf,sizeof(buf));
    }
    //write the initial information to the first few blocks
    //position to the first the byte
    fs.seekp(0);
    fs.write(reinterpret_cast<const char*>(&superBlock),sizeof(SuperBlock)); 
    fs.seekg(0);
    fs.read(reinterpret_cast<char *>(&superBlock),sizeof(SuperBlock));
    close();
}

void Disk::open()
{
    try
    {
        fs.open("../VDISK",ios::in|ios::out|ios::binary);
        //fs.seekg(0);
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }  
}

void Disk::close()
{
    fs.close();
}
Disk::~Disk()
{
    close();
}

