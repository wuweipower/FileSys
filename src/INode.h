#ifndef INODE
#define INODE

const short D = 0;//DIR
const short F = 1;//FILE
#include<ctime>
#include<string>
#include<cstring>
#include<vector>
using std::string;
class INode
{
public:

    int i_id;                //inode id used to search int bitmap
    int type;                //DIR or FILE
    int filesize;            //unit: Byte
    int directBlocks[10];    //10 direct block addresses
    int indirectBlock;       //one indirect block address 包含1kB/4B=256个pointer
    tm createTime;           //create time
    tm modTime;              //modify time

    //total 4*4+36+36+40=128
    
public:
    // const static int BLOCK_SIZE = 1;//1KB

    INode(){}
    INode(int i_id,int type,int filesize)
    {
        this->i_id = i_id;
        this->type = type;
        this->filesize = filesize;
        for(int i=0;i<10;i++) directBlocks[i]=-1;
        this->indirectBlock=-1;

        time_t t;
        time(&t);
        this->createTime = timeConvert(localtime(&t));
        this->modTime = timeConvert(localtime(&t));
    }
    void showTime(tm t)
    {
        // time_t now;
        // time(&now);
        // tm* t = localtime(&now);
        printf("%04d.%02d.%02d %02d:%02d:%02d\n",
        t.tm_year+1900,t.tm_mon+1,t.tm_mday,t.tm_hour,t.tm_min,t.tm_sec);
    }
    tm timeConvert(tm* t)
    {
        tm res;
        res.tm_hour  =  t->tm_hour;
        res.tm_isdst =  t->tm_isdst;
        res.tm_mday  =  t->tm_mday;
        res.tm_min   =  t->tm_min;
        res.tm_mon   =  t->tm_mon;
        res.tm_sec   =  t->tm_sec;
        res.tm_wday  =  t->tm_wday;
        res.tm_yday  =  t->tm_yday;
        res.tm_year  =  t->tm_year;
        return res;
    }

};
// 32 Byte and one block can have 32 items
class DirItem
{    
public:
    string name;          //name of file or dir
    int i_id;                  //inode id
    bool pad;
    DirItem(string _name,int _id)
    {
        name = _name;
        i_id = _id;
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