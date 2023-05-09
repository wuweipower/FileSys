#include"Disk.h"

Disk::Disk()
{
    open();
    fs.seekg(0);
    fs.read(reinterpret_cast<char*>(&superBlock),sizeof(SuperBlock));
    close();
}

void Disk::init()
{
    // memset(superBlock.block_bitmap,0,sizeof(superBlock.block_bitmap));
    // //for superblock
    // memset(superBlock.block_bitmap,1,20);
    // memset(superBlock.inode_bitmap,0,sizeof(superBlock.inode_bitmap));

    for(int i=0;i<superBlock.BLOCK_NUM;i++)
    {
        superBlock.block_bitmap[i]=false;
    }
    for(int i=0;i<superBlock.data_area_start_index;i++)
    {
        superBlock.block_bitmap[i]=true;
    }
    for(int i=0;i<superBlock.INODE_NUM;i++)
    {
        superBlock.inode_bitmap[i]=false;
    }
    
    superBlock.blocks_used=superBlock.data_area_start_index;
    superBlock.free_blocks=SuperBlock::BLOCK_NUM-superBlock.data_area_start_index;
    superBlock.inode_used=0;
    superBlock.free_inode=SuperBlock::INODE_NUM;

    open();
    //format the disk with all zero
    for(int i =0;i<16*1024;i++)
    {
        int buf = 0;
        for(int j=0;j<32;j++)
        {
            fs.write(reinterpret_cast<const char*>(&buf),sizeof(int));
        }
    }
    //write the initial information to the first few blocks
    //position to the first the byte
    fs.seekp(0);
    fs.write(reinterpret_cast<const char*>(&superBlock),sizeof(SuperBlock)); 
    // fs.seekg(0);
    // fs.read(reinterpret_cast<char *>(&superBlock),sizeof(SuperBlock));
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

int SuperBlock::convertBlockIdToAddr(int id)
{
    return id * block_size;
}

int SuperBlock::convertInodeIdToAddr(int id)
{
    int addr = inode_begin + id * inode_size;
    // printf("id:%d, inode_begin:%d inodesize:%d",id,inode_begin,inode_size);
    // printf("converted addr %d",addr);
    return inode_begin + id * inode_size;
}