#include"FileSys.h"

/**
 * @author Xin Huang
 * 
*/

void FileSys::test()
{
    // pathResolve("../usr/local/hello/a");
    // pathResolve("./usr/local/hello/a");
    // pathResolve("/usr/local/hello/a");

    //disk.open();
    //init();
    //cout<<sizeof(SuperBlock)<<endl;
    // cout<<sizeof(string)<<endl;//24 bytes
    // cout<<sizeof(DirItem)<<endl;//28bytes
    // cout<<sizeof(Directory)<<endl;
}

static const int CMD_LEN=64;
FileSys::FileSys()
{
    curDir="/";
    open();
}

FileSys::~FileSys()
{
    updateSuperBlock();
    fs.close();
}

/**
 * 1. check the filesize
 * 2. start from the root inode to find the root direcotory infomation
 * 3. and check the dir in the path, if not exit, create.
 * 4. finally set the corrspoing inode and data
*/
bool FileSys::createFile(string filename,string filesize)
{
    int size;
    try
    {
        size = stoi(filesize);
    }
    catch(const std::exception& e)
    {
        cerr<<"The size should contain the right digits"<<endl;
        std::cerr << e.what() << '\n';
        return false;
    }
      
    if(size>MAX_FILE_SIZE)
    {
        cerr<<"The size should not exceed "<<MAX_FILE_SIZE<<endl;
        return false;
    }

    vector<string> paths = pathResolve(filename);
    Directory* dir = getRootDir();

    //OK, We get the root dir
    /**
     * @todo set inode and set block
    */
    /**由于第一个是根目录，所以直接跳过
     * 先处理文件以前的目录
     * 再处理文件
    */
    INode* temp;
    for(int i =1;i<paths.size()-1;i++)
    {
        //
        int addr = getInodeAddrByName(paths[i],dir);
        if(addr!=-1)//存在对应的dir entry
        {
            temp = getINode(addr);
            dir = getDir(temp);
        }
        else//不存在对应的entry就要创建一个
        {
            
        }       

    }
    //处理文件
    string file = paths[paths.size()-1];
    /**
     * @todo 
     * 根据大小处理block 并且填充随机字符
     * 处理inode
    */


    return true;

}
void FileSys::init()
{
    disk.init();
    //create root directory
    int i_start = disk.superBlock.inode_begin;  //inode area start 
    int d_start = disk.superBlock.data_begin;   //data area start

    /**
     * handle the inode process
     * 1. inode bitmap settting
     * 2. inode_used plus 1
     * 3. 
    */
    disk.superBlock.inode_bitmap[0]=1;          //the first inode is root
    disk.superBlock.inode_used+=1;
    disk.superBlock.block_bitmap[i_start/1024]=1;    //
    disk.superBlock.blocks_used+=ceil(disk.superBlock.inode_used*128/1024);             //one inode and a data block

    //set inode information
    INode inode(0,D,1);
    inode.directBlocks[0]=d_start;
    inode.filesize = 32*2;   //两个items

    //set directory information
    Directory dir;
    dir.items.push_back(DirItem("usr",1));
    dir.items.push_back(DirItem("tmp",2));

    //set the usr and tmp
    INode usr(1,D,0);
    INode tmp(2,D,0);
    disk.superBlock.inode_bitmap[1]=1;
    disk.superBlock.inode_used++;
    disk.superBlock.inode_bitmap[2]=1;
    disk.superBlock.inode_used++;

    //write inodes to disk
    fs.open("../VDISK",ios::in|ios::out|ios::binary);
    fs.seekp(i_start);
    fs.write(reinterpret_cast<const char*>(&inode),sizeof(INode));
    fs.write(reinterpret_cast<const char*>(&usr),sizeof(INode));
    fs.write(reinterpret_cast<const char*>(&tmp),sizeof(INode));
    

    //write data to data area
    //first 10 blocks
    int i=0;
    for(i=0;i<dir.getSize();i++)
    {
        if(i==32*10) break;
        if(i%32==0)
        {
            fs.seekp(inode.directBlocks[i/32]);
        }
        fs.write(reinterpret_cast<const char*>(&dir.items[i]),sizeof(DirItem));
    }
    if(i==32*10)
    {
        //the first 10 blocks are not big enough
        vector<int> addrs;
        //how many indirect address
        fs.seekg(inode.indirectBlock);
        for(int j=0;j<32;j++)//one block can store max 32 32-bit addresses
        {
            int addr;
            fs.read(reinterpret_cast<char*>(&addr),sizeof(int));
            if(addr!=-1)
            {
                addrs.push_back(addr);
            }
        }
        //write 
        int cnt = 0;
        for(int j =i;j<dir.getSize();j++)
        {
            if(cnt%32==0)
            {
                fs.seekp(addrs[cnt/32]);
            }
            fs.write(reinterpret_cast<const char*>(&dir.items[j]),sizeof(DirItem));
            cnt++;
        }
    }

}
bool FileSys::deleteFile(string filename)
{
    /**
     * 设置对应的inode和两个bitmap
    */
    return true;
}

bool FileSys::createDir(string dir)
{
    return true;
}

bool FileSys::deleteDir(string dir)
{
    /**
     * 设置对应的inode和两个bitmap
    */
    return true;
}

bool FileSys::cd(string dir)
{
    /**
     * 首先检查是否存在该路径
     * 存在的话，就设置对应的curInode和curDir就行
    */
    return true;
}

bool FileSys::ls()
{
    /**
     * 就是根据当前的inode将目录的item写出来，并且对应目录的item的inode信息展示出来就行
    */
    return true;
}

bool FileSys::cp(string from, string to)
{
    /**
     * 就是createFile的过程
    */
    return true;
}
void FileSys::showStorageInfo()
{
    cout<<"Blocks used: "<<disk.superBlock.blocks_used<<endl;
    cout<<"Blocks unused: "<<disk.superBlock.free_blocks<<endl;
}

void FileSys::cat(string filename)
{
    /**
     * 首先检查是否存在
     * 然后将inode中使用的block中的信息展示出来
    */
}

vector<string> FileSys::pathResolve(string path)
{
    const char* buf= path.c_str();
    char* argv[CMD_LEN];
    argv[0] = strtok((char*)buf,"/");//字符串分割
    int i=0;
    while ((argv[i]!=NULL))
    {
        i++;
        argv[i] = strtok(NULL,"/");//get the args
    }

    vector<string> res;
    if(path[0]=='/')
    {
        res.push_back("/");
    }
    

    for(int j =0;j<i;j++)
    {
        res.push_back(argv[j]);
    }
    if(res[0]==".")
    {
        vector<string> cur = pathResolve(curDir);
        res.erase(res.begin());
        for(int i =0;i<res.size();i++)
        cur.push_back(res[i]);

        return cur;
    }
    else if(res[0]=="..")
    {
        vector<string> cur = pathResolve(curDir);
        cur.pop_back();
        res.erase(res.begin());
        for(int i =0;i<res.size();i++)
        cur.push_back(res[i]);
        return cur;
    }
    else if(res[0]!="/")//默认为当前目录
    {
        vector<string> cur = pathResolve(curDir);
        res.erase(res.begin());
        for(int i =0;i<res.size();i++)
        cur.push_back(res[i]);
        return cur;
    }
    else
    return res;
}
void FileSys::showHelp()
{
    cout
    <<"createFile [filename] [filesize]\n"
    <<"deleteFile [filename]\n"
    <<"createDir [dir]\n"
    <<"deleteDir [dir]\n"
    <<"cd [dir]\n"
    <<"ls \n"
    <<"cp [from] [to]\n"
    <<"sum \n"
    <<"exit \n"
    ;
}
//一切的入口
void FileSys::entry()
{    
    while (1)
    {
        //show the current dir
        cout<<"FileSys@admin:"<<curDir<<"# ";
        
        //handle the cmd
        //get the full command
        char buf[1024]={0};
        fgets(buf,sizeof(buf)-1,stdin);
        buf[strlen(buf)-1]=0;

        //get the arg
        char* argv[CMD_LEN];
        argv[0] = strtok(buf," ");//字符串分割

        int i =0;
        while ((argv[i]!=NULL))
        {
            i++;
            argv[i] = strtok(NULL," ");//get the args
        }

        for(int i=0;i<sizeof(argv);i++)
        {
            string temp;
            if(argv[i]!=NULL)
            {
                temp = argv[i];
            }
            else break;

            if(temp=="cd")
            {
                cd(argv[i+1]);
                break;
            }
            else if(temp=="exit")
            {
                exit();
                return;
            }
            else if(temp=="createFile")
            {
                createFile(argv[i+1],argv[i+2]);
                break;
            }
            else if(temp=="deleteFile")
            {
                delete(argv[i+1]);
                break;
            }
            else if(temp=="createDir")
            {
                createDir(argv[i+1]);
                break;
            }
            else if(temp=="deleteDir")
            {
                deleteDir(argv[i+1]);
                break;
            }
            else if(temp=="ls")
            {
                ls();
                break;
            }
            else if(temp=="cp")
            {
                cp(argv[i+1],argv[i+2]);
                break;
            }
            else if(temp=="sum")
            {
                showStorageInfo();
                break;
            }
            else if(temp=="cat")
            {
                cat(argv[i+1]);
                break;
            }
            else if(temp=="test")
            {
                test();
                break;
            }
            else
            {
                cerr<<"Error input! See the help below."<<endl;
                showHelp();
            }
        }
        
    } 
}

void FileSys::exit()
{
    cout<<"Exit successfully"<<endl;
}

void FileSys::updateSuperBlock()
{
    fs.seekp(0);
    fs.write(reinterpret_cast<const char*>(&disk.superBlock),sizeof(SuperBlock));
}

Directory* FileSys::getRootDir()
{
    //首先找到根目录 第一个inode就是root的inode
    int i_start = disk.superBlock.inode_begin;
    fs.seekg(i_start);//开始读
    INode root;
    fs.read(reinterpret_cast<char*>(&root),sizeof(INode));
    //根据inode获取目录或者文件
    Directory* dir = new Directory();
    for(int i=0;i<10;i++)
    {
        if(root.directBlocks[i]!=-1)
        {
            fs.seekg(root.directBlocks[i]);
            DirItem item;
            fs.read(reinterpret_cast<char*>(&(item)),sizeof(DirItem));
            dir->items.push_back(item);
        }
    }
    if(root.indirectBlock!=-1)
    {
        fs.seekg(root.indirectBlock);
        vector<int> addrs;
        int addr;
        for(int i =0;i<32;i++)
        {
            if(addr!=-1)
            {
                addrs.push_back(addr);
            }
        }
        //according to the addresses, find the remaining items
        /**
         * @todo get the remain items to read the right number of items
        */
        int remain = root.filesize/32-10;
        for(int j =0;j<addrs.size();j++)
        {
            if(remain<=0) break;//没有剩余的了，不用往下读
            if(j%32==0)
            {
                fs.seekg(addrs[j/32]);
            }
            for(int k =0;k<32;k++)//一个block最多32个directory item
            {
                if(remain<=0) break;
                DirItem item;
                fs.read(reinterpret_cast<char*>(&item),sizeof(DirItem));
                dir->items.push_back(item);
                --remain;
            }
        }
    }
    return dir;
}

void FileSys::open()
{
    fs.open("../VDISK",ios::in|ios::out|ios::binary);
}

void FileSys::close()
{
    fs.close();
}

int FileSys::getInodeAddrByName(string filename,Directory* dir)
{
    for(int i=0;i<dir->getSize();i++)
    {
        if(filename==dir->items[i].name)
        {
            int addr = disk.superBlock.inode_begin+
                       dir->items[i].i_id*disk.superBlock.inode_size;
            return addr;
        }
        
    }
    return -1;
}
INode* FileSys::getINode(int addr)
{
    fs.seekg(addr);
    INode * inode = new INode();
    fs.read(reinterpret_cast<char*>(inode),sizeof(INode));
    return inode;
}


vector<int> FileSys::allocateBlocks(int num)//start from the data area to find 
{
    /**
     * idea:
     * according to the bitmap to find the blocks
    */
    if(num>disk.superBlock.free_blocks)
    {
        cerr<<"No free blocks can be allocated"<<endl;
        return {};
    }
    vector<int> addrs;
    int cnt=0;//暂时记录可以申请多少
    for(int i =disk.superBlock.data_begin/disk.superBlock.block_size;
            i<disk.superBlock.BLOCK_NUM;i++)
    {
        if(++cnt<=num)
        if(disk.superBlock.block_bitmap[i]==0)
        {
            int addr = disk.superBlock.data_begin+i*disk.superBlock.block_size;  
            disk.superBlock.block_bitmap[i]=1;          
            addrs.push_back(addr);
        }
    }
    disk.superBlock.free_blocks-=num;
    disk.superBlock.blocks_used +=num;
    return addrs;
}

int FileSys::allocateINode()
{
    /**
     * idea:
     * according to the bitmap to find the inodes
    */
    if(disk.superBlock.free_inode<=0)
    {
        cerr<<"No free inode can be allocated"<<endl;
        return -1;
    }
    for(int i =0;i<disk.superBlock.INODE_NUM;i++)
    {
        if(disk.superBlock.inode_bitmap[i]==0)
        {
            int addr = disk.superBlock.inode_begin+i*disk.superBlock.inode_size;
            disk.superBlock.inode_bitmap[i]=1;
            return addr;
        }
    }
    disk.superBlock.inode_used+=1;
    disk.superBlock.free_inode-=1;
    return -1;
    
}
bool FileSys::appendDir(INode* cur,string name)
{
    /**
     * 先判断是否还有空间
     * 若有，就要看情况申请block
    */
   int size = cur->filesize;
   if(size>=MAX_FILE_SIZE*1024)
   {
        cerr<<"Can Not append the directory in the currrent directory"<<endl;
        return false;
   }
   //先计算当前已经有多少dir_item 一个item32字节
   int item_size = size/32;
   int inodeAddr = allocateINode();
   if(inodeAddr!=-1)
   {
        int i_id = (inodeAddr-disk.superBlock.inode_begin)/disk.superBlock.inode_size;
        DirItem item(name,i_id);
        //看是否在前10个block是否还有空位，不包括第十个block的最后一个item，因为，这个有的话，还是要indirect block
        if(item_size<10*32)
        {
                int block_no = item_size/32;//第几个block
                int offset = (item_size%32)*32;  //在哪个block的字节偏移量
                cur->filesize+=32;
                //write the block to the disk
                int writeAddr = cur->directBlocks[block_no] + offset;
                fs.write(reinterpret_cast<const char*>(&item),writeAddr);
        }
        //不在前十个就要找indirect
        else
        {
            //一个indirect block最多存32个地址
            vector<int> addrs = getAddrsInIndir(cur->indirectBlock);

            //一个address对应的block最多存32个
            int block_no = (item_size-320)/32;
            int offset = ((item_size-320)%32)*32;
            cur->filesize+=32;
            //write the block to the disk
            int writeAddr = cur->directBlocks[block_no] + offset;
            fs.write(reinterpret_cast<const char*>(&item),writeAddr);
        }
   }
   else
   {
        return false;
   }  

}

Directory* FileSys::getDir(INode* inode)
{
    if(inode->type!=D)
    {
        cerr<<"It is not a directory file"<<endl;
        return nullptr;
    } 
    Directory* dir = new Directory();
    int size = inode->filesize;
    

}

vector<int> FileSys::getAddrsInIndir(int addr)
{
    //the first 10 blocks are not big enough
    vector<int> addrs;
    //how many indirect address
    fs.seekg(addr);
    for(int j=0;j<32;j++)//one block can store max 32 32-bit addresses
    {
        int addr;
        fs.read(reinterpret_cast<char*>(&addr),sizeof(int));
        if(addr!=-1)
        {
            addrs.push_back(addr);
        }
    }
    return addrs;
}