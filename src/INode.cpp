#include"INode.h"

INode::INode(int i_id,int type,int filesize)
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

tm INode::timeConvert(tm* t)
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

void INode::showTime(const tm& t)
{       
    // time_t now;
    // time(&now);
    // tm* t = localtime(&now);
    printf("%04d.%02d.%02d %02d:%02d:%02d\n",
    t.tm_year+1900,t.tm_mon+1,t.tm_mday,t.tm_hour,t.tm_min,t.tm_sec);   
}

void INode::print()
{
    printf("%d %d ",type,filesize);
    showTime(createTime);
    showTime(modTime);
}

void INode::setId(int id)
{
    change();
    this->i_id = id;
}

int INode::getId()
{
    return i_id;
}

void INode::setType(int type)
{
    if(type!=D && type !=F)
    {
        cerr<<"Type error"<<std::endl;
        return;
    }
    change();
    this->type = type;
}

int INode::getType()
{
    return type;
}

void INode::setDirectBlock(int index,int addr)
{
    if(index<0 || index>=10)
    {
        cerr<<"Index error"<<std::endl;
        return;
    }
    change();
    this->directBlocks[index] = addr;
}
int  INode::getDirectBlock(int index)
{
    return directBlocks[index];
}
void INode::setIndirectBlock(int addr)
{
    change();
    this->indirectBlock = addr;
}
int  INode::getIndirecrBlock()
{
    return indirectBlock;
}

void INode::change()
{
    time_t t;
    time(&t);
    this->modTime = timeConvert(localtime(&t));
}

int INode::convertIdToAddr()
{
    if(i_id<0)
    {
        return -1;
    }
    Disk disk;
    return disk.superBlock.inode_begin + i_id*disk.superBlock.inode_size;
}

