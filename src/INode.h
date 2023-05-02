#ifndef INODE
#define INODE

const short D = 0;//DIR
const short F = 1;//FILE
#include<ctime>
#include<string>
#include<cstring>
#include<vector>
#include<iostream>
#include"Disk.h"
using std::string;
using std::cerr;
using std::vector;
class INode
{
public:

    //inode id used to search int bitmap,-1 means been deleted
    int i_id;

    //DIR or FILE         
    int type;     

    //unit: Byte           
    int filesize;            

    /**
     * 10 direct block addresses
     * -1 means not used
     * addtional 256kB
    */
    int directBlocks[10];    

    /**
     * one indirect block address 1kB/4B=256 pointer
     * if the address is -1, it means it is not used and could not be occupied by others
     * 
    */
    int indirectBlock;       

    //create time
    tm createTime;           

    //modify time
    tm modTime;              


    //total 4*4+36+36+40=128
    //when changed, change the modified time
    void change();
public:
    INode(){}
    INode(int i_id,int type,int filesize);
    void showTime(const tm& t);
    tm timeConvert(tm* t);
    void print();

    //setters and getters
    void setId(int id);
    int  getId();
    void setType(int type);
    int  getType();
    void setDirectBlock(int index,int addr);
    int  getDirectBlock(int index);
    void setIndirectBlock(int addr);
    int  getIndirecrBlock();

    int  convertIdToAddr();

};
// 32 Byte and one block can have 32 items
class DirItem
{    
public:
    string name;                  //name of file or dir
    int    i_addr;                //@attention 这里是地址                  
    bool   live;                  //false means deleted
    DirItem(string _name,int _id)
    {
        name = _name;
        i_addr = _id;
        live = true;
    }
    DirItem(){}
};

class Directory
{
public:
    std::vector<DirItem> items;
    int getSize()
    {
        return items.size();
    }  
};


#endif