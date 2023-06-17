#include"FileSys.h"
#define DEBUG 1

#if DEBUG
#include"util.cpp"
#endif

/**
 * @author Xin Huang
 * 
*/

void FileSys::test()
{
    // auto a = pathResolve("../usr/local/hello/a");
    // auto b = pathResolve("./usr/local/hello/a");
    // auto c =  pathResolve("/usr/local/hello/a");
    // for(int i=0;i<a.size();i++)
    // {
    //     cout<<a[i]<<" ";
    // }
    // cout<<endl;
    // for(int i=0;i<b.size();i++)
    // {
    //     cout<<b[i]<<" ";
    // }
    // cout<<endl;
    // for(int i=0;i<c.size();i++)
    // {
    //     cout<<c[i]<<" ";
    // }
    // cout<<endl;
    //disk.open();
    //init();
    //cout<<sizeof(SuperBlock)<<endl;
    // cout<<sizeof(string)<<endl;//24 bytes
    // cout<<sizeof(DirItem)<<endl;//28bytes
    // cout<<sizeof(Directory)<<endl;
    //cout<<sizeof(DirItem)<<endl;

    // auto a = pathResolve("../usr/cpp/local");
    // print_vector(a);
    // a = pathResolve("./usr/cpp/local");
    // print_vector(a);
    // a = pathResolve("usr/cpp/local");
    // print_vector(a);
    //init();
    // for(int i=0;i<disk.superBlock.BLOCK_NUM;i++)
    // {
    //     cout<<disk.superBlock.block_bitmap[i];
    //     if(i%148==0)
    //     cout<<endl;
    // }
    string dir = "../usr/local";
    auto p = pathResolve(dir);
    getAbsPath(p);

    dir = "./ll/aa";
    p = pathResolve(dir);
    getAbsPath(p);

    dir = "/usr/l/a";
    p = pathResolve(dir);
    getAbsPath(p);
}

static const int CMD_LEN=64;
FileSys::FileSys()
{
    curDir="/";
    srand(unsigned(time(0)));
    //init();
    open();
    //superblock
    fs.seekg(0);
    fs.read(reinterpret_cast<char*>(&disk.superBlock),sizeof(SuperBlock));
    //root inode
    currInode = new INode();//没有这个就会异常 卡死
    getINode(disk.superBlock.inode_begin,currInode);    
}

FileSys::~FileSys()
{
    updateSuperBlock();
    delete currInode;
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
    if(filesize.empty())
    {
        cerr<<"No filesize"<<endl;
        return false;
    }
    try
    {
        size = stoi(filesize);
    }
    catch(const std::exception& e)
    {
        cerr<<"The size should contain the right digits"<<endl;
        //std::cerr << e.what() << '\n';
        return false;
    }
      
    if(size>MAX_FILE_SIZE)
    {
        cerr<<"The size should not exceed "<<MAX_FILE_SIZE<<endl;
        return false;
    }

    vector<string> paths = pathResolve(filename);
    Directory dir;
    INode temp;
    int preAddr;
    if(paths[0]=="/")
    {
        getRootDir(&dir);
        paths.erase(paths.begin());
        getINode(disk.superBlock.inode_begin,&temp);
    }
    else
    {
        getDir(currInode,&dir);
        int addr = getInodeAddrByName(".",&dir);
        getINode(addr,&temp);
    }
    preAddr = getInodeAddrByName(".",&dir);
    //print_vector(dir->items);
    
    for(int i =0;i<paths.size()-1;i++)
    {
        int addr = getInodeAddrByName(paths[i],&dir);
        if(addr!=-1)//存在对应的dir entry
        {
            getINode(addr,&temp);
            getDir(&temp,&dir);
        }
        else//不存在对应的entry就要创建一个
        {
            int id;
            int new_iaddr = allocateINode(id);
            int new_baddr = allocateBlocks(1)[0];

            INode new_inode(id,D,32*2);
            new_inode.directBlocks[0] = new_baddr;
            writeINode(&new_inode,new_iaddr); 

            Directory new_dir;
            new_dir.items.push_back(DirItem(".",new_iaddr));
            new_dir.items.push_back(DirItem("..",preAddr));
            
            writeDir(&new_dir,new_baddr);
                   
            appendDir(&temp,paths[i],new_iaddr,preAddr);
            temp = new_inode;
            dir= new_dir;
        }    
        preAddr = getInodeAddrByName(".",&dir);
    }
    //处理文件
    string file = paths[paths.size()-1];
    int flag=getInodeAddrByName(file,&dir);
    if(flag!=-1)
    {
        cerr<<"The file exits! error!"<<endl;
        return false;
    }
    /**
     * @todo 
     * 根据大小处理block 并且填充随机字符
     * 处理inode
    */
    int id;
    int iaddr = allocateINode(id);
    INode inode(id,F,size*1024);
    if(size<=10)
    {
        vector<int> addrs = allocateBlocks(size);
        for(int i=0;i<size;i++)
        {
            inode.directBlocks[i] = addrs[i];
        }
        //随机生成字符
        for(int i =0;i<addrs.size();i++)
        {
            generateRandomChs(addrs[i]);
        }
    }
    else
    {
        vector<int> addrs = allocateBlocks(10);
        for(int i=0;i<10;i++)
        {
            inode.directBlocks[i] = addrs[i];
        }
        for(int i =0;i<10;i++)
        {
            generateRandomChs(addrs[i]);
        }

        int remain = size-10;
        int indir = allocateBlocks(1)[0];//间接的block
        vector<int> remains = allocateBlocks(remain);
        inode.indirectBlock = indir;

        fs.seekp(indir);
        for(int i=0;i<remains.size();i++)
        {
            fs.write(reinterpret_cast<const char*>(&remains[i]),sizeof(int));
        }
        int unused = 0;
        for(int i=remains.size();i<32;i++)
        {
            fs.write(reinterpret_cast<const char*>(&unused),sizeof(int));
        }

        for(int i=0;i<remains.size();i++)
        {
            generateRandomChs(remains[i]);
        }
    }
    //要在最后的目录中加入这个文件的entry
    //还要更新目前的inode
    int curAddr = getInodeAddrByName(".",&dir);
    appendDir(&temp,file,iaddr,curAddr);
    //将文件的inode写到对应的inode中
    writeINode(&inode,iaddr);
    return true;

}

bool FileSys::deleteFile(string filename)
{
    /**
     * find the inode
     * 设置对应的inode和两个bitmap
    */
    vector<string> paths = pathResolve(filename);
    Directory dir;
    INode inode;
    if(paths[0]=="/")
    {
        getRootDir(&dir);
        paths.erase(paths.begin());
        getINode(disk.superBlock.inode_begin,&inode);
    }
    else
    {
        getDir(currInode,&dir);
        int addr = getInodeAddrByName(".",&dir);
        getINode(addr,&inode);
    }
    for(int i=0;i<paths.size()-1;i++)
    {
        int iaddr = getInodeAddrByName(paths[i],&dir);
        if(iaddr==-1)
        {
            cerr<<"Wrong path"<<endl;
            return false;
        }
        getINode(iaddr,&inode);
        bool flag = getDir(&inode,&dir);
        if(!flag)
        {
            cerr<<"Dir not exit"<<endl;
            return false;
        }
    }

    //处理文件
    string file = paths[paths.size()-1];
    //处理文件目录项
    int currAddr = getInodeAddrByName(".",&dir);
    deleteDirItem(&inode,currAddr,&dir,file);

    int fileInodeAddr=getInodeAddrByName(file,&dir);

    if(fileInodeAddr==-1)
    {
        cerr<<"Wrong path"<<endl;
        return false;
    }
    getINode(fileInodeAddr,&inode);
    for(int i=0;i<10;i++)
    {
        if(inode.directBlocks[i]!=-1)
        {
            int id = blockAddrToId(inode.directBlocks[i]);
            freeBlock(id);
        }
    }
    if(inode.indirectBlock!=-1)
    {
        int id = blockAddrToId(inode.indirectBlock);
        freeBlock(id);
        vector<int> addrs = getAddrsInIndir(inode.indirectBlock);
        for(int i=0;i<addrs.size();i++)
        {
            int id = blockAddrToId(addrs[i]);
            freeBlock(id);
        }
    }
    freeInode(inode.i_id);
    writeINode(&inode,fileInodeAddr);//一定要写回去，不然只是在内存中改变
    //inode.i_id=-1;
    return true;
}

bool FileSys::createDir(string dir)
{
    vector<string> paths = pathResolve(dir);
    /**
     * 类似createFile 只不过加的是entry 设置对应的inode，写回去
    */
    Directory directory;
    INode inode;
    int preAddr;
    if(paths[0]=="/")
    {
        getRootDir(&directory);
        paths.erase(paths.begin());
        getINode(disk.superBlock.inode_begin,&inode);
    }
    else
    {
        getDir(currInode,&directory);
        int addr = getInodeAddrByName(".",&directory);
        getINode(addr,&inode);
    }
    preAddr = getInodeAddrByName(".",&directory);
    for(int i=0;i<paths.size()-1;i++)
    {
        int addr = getInodeAddrByName(paths[i],&directory);
        if(addr!=-1)//存在对应的dir entry
        {
            getINode(addr,&inode);
            getDir(&inode,&directory);
        }
        else//不存在对应的entry就要创建一个
        {
            int id;
            int new_iaddr = allocateINode(id);
            int new_baddr = allocateBlocks(1)[0];

            INode new_inode(id,D,32*2);
            new_inode.directBlocks[0] = new_baddr;
            writeINode(&new_inode,new_iaddr); 

            Directory new_dir;
            new_dir.items.push_back(DirItem(".",new_iaddr));
            new_dir.items.push_back(DirItem("..",preAddr));
            
            writeDir(&new_dir,new_baddr);
                   
            appendDir(&inode,paths[i],new_iaddr,preAddr);
            inode = new_inode;
            directory = new_dir;
        }    
        preAddr = getInodeAddrByName(".",&directory);
    }
    /**
     * @todo 处理重名
     * 
    */
    string lastDir = paths[paths.size()-1];
    int flag=getInodeAddrByName(lastDir,&directory);
    if(flag!=-1)
    {
        cerr<<"The dir exits! error!"<<endl;
        return false;
    }
    int id;
    int new_iaddr = allocateINode(id);
    int new_baddr = allocateBlocks(1)[0];

    INode new_inode(id,D,32*2);
    new_inode.directBlocks[0] = new_baddr;
    writeINode(&new_inode,new_iaddr); 

    Directory new_dir;
    new_dir.items.push_back(DirItem(".",new_iaddr));
    new_dir.items.push_back(DirItem("..",preAddr));
    
    writeDir(&new_dir,new_baddr);
            
    appendDir(&inode,lastDir,new_iaddr,preAddr);
    inode = new_inode;
    directory = new_dir;
    return true;
}

bool FileSys::deleteDir(string dir)
{
    /**
     * 设置对应的inode和两个bitmap
    */
    vector<string>paths = pathResolve(dir);
    string absPath = getAbsPath(paths);
    
    if(absPath==curDir)
    {
        cerr<<"The current dir should not be deleted!"<<endl;
        return false;
    }

    //得到最后一个dir的inode
    Directory directory;
    INode inode;
    if(paths[0]=="/")
    {
        getRootDir(&directory);
        paths.erase(paths.begin());
        getINode(disk.superBlock.inode_begin,&inode);
    }
    else
    {
        getDir(currInode,&directory);
        int addr = getInodeAddrByName(".",&directory);
        getINode(addr,&inode);
    }
    for(int i=0;i<paths.size()-1;i++)
    {
        int iaddr = getInodeAddrByName(paths[i],&directory);
        if(iaddr==-1)
        {
            cerr<<"Wrong directory name"<<endl;
            return false;
        }
        getINode(iaddr,&inode);
        bool flag = getDir(&inode,&directory);
        if(!flag)
        {
            cerr<<"Dir not exit！"<<endl;
            return false;
        }
    }
    
    //处理目录文件
    string file = paths[paths.size()-1];
    int currAddr = getInodeAddrByName(".",&directory);
    deleteDirItem(&inode,currAddr,&directory,file);
    //处理这个dir以及下面的所有文件
    int fileInodeAddr=getInodeAddrByName(file,&directory);
    getINode(fileInodeAddr,&inode);
    freeDIrHelper(&inode);    
    //freeInode(id);
    return true;
}

bool FileSys::cd(string dir)
{
    /**
     * 首先检查是否存在该路径
     * 存在的话，就设置对应的curInode和curDir就行
    */
    vector<string> paths = pathResolve(dir);
    Directory directory;
    INode inode;
    // print_info("cd","cd path\n");
    // print_vector(paths);
    bool flag = false;//是否是绝对路径
    if(paths[0]=="/")
    {
        getRootDir(&directory);
        paths.erase(paths.begin());
        flag=true;
        int addr = getInodeAddrByName(".",&directory);
        getINode(addr,&inode);
    }
    else
    {
        getDir(currInode,&directory);
        int addr = getInodeAddrByName(".",&directory);
        getINode(addr,&inode);
    }

    for(int i=0;i<paths.size();i++)
    {
        int iaddr = getInodeAddrByName(paths[i],&directory);
        if(iaddr==-1)
        {
            cerr<<"Wrong directory"<<endl;
            return false;
        }
        getINode(iaddr,&inode);
        bool flag = getDir(&inode,&directory);
        if(!flag)
        {
            cerr<<"Dir not exit！"<<endl;
            return false;
        }
    }
    //解决一下显示的问题
    if(paths[0]==".")
    {

    }
    else if(paths[0]=="..")
    {
        if(curDir=="/")
        {
        }
        else
        {
            auto p = pathResolve(curDir);
            curDir = curDir.substr(0,curDir.size()-p[p.size()-1].size()-1);
        }
        
    }
    else
    {       
        if(flag)
        {
            curDir="";
        }
        else
        curDir = curDir.substr(0,curDir.size()-1);

        for(int i =0;i<paths.size();i++)
        {
            curDir+="/"+paths[i];
        }
        curDir+="/";
    }

    delete currInode;
    currInode = new INode(inode);
    return true;
}

bool FileSys::ls()
{
    /**
     * 就是根据当前的inode将目录的item写出来，并且对应目录的item的inode信息展示出来就行
    */

    Directory dir;

    getDir(currInode,&dir);
    
    //cout<<"dir"<<dir.getSize()<<endl;
    for(int i=0;i<dir.getSize();i++)
    {
        INode temp;
        int iaddr = dir.items[i].i_addr;
        getINode(iaddr,&temp);
        cout<<dir.items[i].name<<" ";
        temp.print();
    }
    
    return true;
}

bool FileSys::cp(string from, string to)
{
    /**
     * 检查from是否合法
     * 创建to
     * 就是createFile的过程
    */
    vector<string> fp = pathResolve(from);
    vector<string> tp = pathResolve(to);
    Directory fdir;
    INode finode;
    getFirstInodeDir(fp,&finode,&fdir);
    for(int i=0;i<fp.size()-1;i++)
    {
        int iaddr = getInodeAddrByName(fp[i],&fdir);
        if(iaddr == -1)
        {
            cerr<<"File or directory not found"<<endl;
            return false;
        }
        getINode(iaddr,&finode);
        bool flag = getDir(&finode,&fdir);
        if(!flag)
        {
            cerr<<"cp error"<<endl;
            return false;
        }
    }
    string ffile = fp[fp.size()-1]; //last name is file name.
    int faddr = getInodeAddrByName(ffile,&fdir);
    if(faddr == -1) //not a file.
    {
        cerr<<"the original file does not exit!"<<endl;
        return false;
    }
    getINode(faddr,&finode); //get the inode of the file.

    Directory tdir;
    INode tinode;
    getFirstInodeDir(tp,&tinode,&tdir); //get the first inode of the target dir.
    int preAddr =  getInodeAddrByName(".",&tdir);
    for(int i=0;i<tp.size()-1;i++)
    {
        int iaddr = getInodeAddrByName(tp[i],&tdir);
        if(iaddr!=-1)
        {
            getINode(iaddr,&tinode);
            getDir(&tinode,&tdir);
        }
        else
        {
            int id;
            int new_iaddr = allocateINode(id);
            int new_baddr = allocateBlocks(1)[0];

            INode new_inode(id,D,32*2);
            new_inode.directBlocks[0] = new_baddr;
            writeINode(&new_inode,new_iaddr);

            Directory new_dir;
            new_dir.items.push_back(DirItem(".",new_iaddr));
            new_dir.items.push_back(DirItem("..",preAddr));

            writeDir(&new_dir,new_baddr);

            appendDir(&tinode,tp[i],new_iaddr,preAddr);
            tinode = new_inode;
            tdir = new_dir;
        }
        preAddr = getInodeAddrByName(".",&tdir);
    }

    string tfile = tp[tp.size()-1];
    int flag=getInodeAddrByName(tfile,&tdir);
    if(flag!=-1)
    {
        cerr<<"The file exits! error!"<<endl;
        return false;
    }
    //copy
    //申请inode
    int id;
    int iaddr = allocateINode(id);
    //根据原来的inode的信息复制过来
    INode new_inode = INode(id,F,finode.filesize);
    //遍历原来的inode中的block，读取block中的信息，然后写到新的block中
    for(int i =0;i<10;i++)
    {
        if(finode.directBlocks[i]!=-1)
        {
            char buf[1024];
            fs.seekg(finode.directBlocks[i]);
            fs.read(buf,1024);

            int addr = allocateBlocks(1)[0];
            new_inode.directBlocks[i] = addr;
            fs.seekp(addr);
            fs.write(reinterpret_cast<const char*>(buf),1024);
        }
    }
    
    if(finode.indirectBlock!=-1)
    {
        vector<int> new_indirAddrs;
        vector<int> addrs = getAddrsInIndir(finode.indirectBlock);
        for(int i =0;i<addrs.size();i++)
        {
            char buf[1024];
            fs.seekg(addrs[i]);
            fs.read(buf,1024);

            int addr = allocateBlocks(1)[0];
            new_indirAddrs.push_back(addr);
            fs.seekp(addr);
            fs.write(reinterpret_cast<const char*>(buf),1024);
        }
        new_inode.indirectBlock = allocateBlocks(1)[0];
        fs.seekp(new_inode.indirectBlock);
        for(int j=0;j<new_indirAddrs.size();j++)
        {
            fs.write(reinterpret_cast<const char*>(&new_indirAddrs[j]),sizeof(int));
        }
    }
    int curAddr = getInodeAddrByName(".",&tdir);
    if(appendDir(&tinode,tfile,iaddr,curAddr))
    {
        writeINode(&new_inode,iaddr);
        return true;
    }
    else
    return false;
   
}


void FileSys::cat(string filename)
{

    /**
     * 首先检查是否存在
     * 然后将inode中使用的block中的信息展示出来
    */
    vector<string> paths = pathResolve(filename);
    Directory dir;
    INode inode;
    if(paths[0]=="/")
    {
        getRootDir(&dir);
        paths.erase(paths.begin());
        getINode(disk.superBlock.inode_begin,&inode);
    }
    else
    {
        getDir(currInode,&dir);
        int addr = getInodeAddrByName(".",&dir);
        getINode(addr,&inode);
    }
    //get the nearest directory
    for(int i=0;i<paths.size()-1;i++)
    {
        int iaddr = getInodeAddrByName(paths[i],&dir);
        if(iaddr==-1)
        {
            cerr<<"Could not find the file"<<endl;
            return;
        }
        getINode(iaddr,&inode);
        bool flag = getDir(&inode,&dir);
        if(!flag)
        {
            cerr<<"Dir not exit!"<<endl;
            return;
        }
    }
    //get the file
    int iaddr = getInodeAddrByName(paths[paths.size()-1],&dir);
    //cout<<iaddr<<endl;
    if(iaddr==-1)
    {
        cerr<<"Could not find the file"<<endl;
        return;
    }
    getINode(iaddr,&inode);
    // int size = inode.filesize;如果是一般情况是需要文件大小来控制读的范围的
    for(int i =0;i<10;i++)
    {
        char buf[1024];
        if(inode.directBlocks[i]!=-1)
        {
            fs.seekg(inode.directBlocks[i]);
            fs.read(reinterpret_cast<char*>(&buf),1024);
            cout<<buf;
            cout<<endl;
        } 
    }
    if(inode.indirectBlock!=-1)
    {
        vector<int> addrs = getAddrsInIndir(inode.indirectBlock);
        for(int i =0;i<addrs.size();i++)
        {
            fs.seekg(addrs[i]);
            char buf[1024];
            fs.read(buf,1024);
            cout<<buf<<endl;
        }
    }

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
        cout<<"FileSys@admin:"<<curDir<<" # ";    
        
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
                if(argv[i+1]!=NULL)
                {
                    cd(argv[i+1]);
                }
                else
                {
                    cerr<<"Error: Missing argument for command 'cd'\n";
                }              
                break;
            }
            else if(temp=="exit")
            {
                exit();
                return;
            }
            else if(temp=="createFile")
            {
                if(argv[i+1]!=NULL&&argv[i+2]!=NULL) 					//arg1 = name of the file
                {
                    createFile(argv[i+1],argv[i+2]);
                }
                else
                {
                    cerr<<"Error: Missing argument for command 'createFile'\n";
                }
                break;
            }
            else if(temp=="deleteFile")
            {
                if (argv[i+1]!=NULL)
                {
                    deleteFile(argv[i+1]); 							//arg1 = name of the file
                }
                else
                {
                    cerr<<"Error: Missing argument for command 'deleteFile'\n";
                }               
                break;
            }
            else if(temp=="createDir")
            {
                if(argv[i+1]!=NULL)
                {
                    createDir(argv[i+1]);
                }  
                else
                {
                    cerr<<"Error: Missing argument for command 'createDir'\n";
                }
                break;
            }
            else if(temp=="deleteDir")
            {
                if(argv[i+1]!=NULL)
                {
                    deleteDir(argv[i+1]);
                }
                else
                {
                    cerr<<"Error: Missing argument for command 'deleteDir'\n";
                }
                break;
            }
            else if(temp=="ls")
            {
                ls();
                break;
            }
            else if(temp=="cp")
            {
                if(argv[i+1]==NULL || argv[i+2]==NULL) 		//checking for null arguments.
                {
                    cerr<<"Missing arguments"<<endl; 		//if null arguments are entered.
                    break;
                }
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
                if(argv[i+1]==NULL)
                {
                    cerr<<"No filename"<<endl;
                    break;
                }
                cat(argv[i+1]);
                break;
            }
            else if(temp=="test")
            {
                test();
                break;
            }
            else if(temp=="init")
            {
                init();
                break;
            }
            else if(temp=="cls")
            {
                system( "cls");
            }
            else
            {
                cerr<<"Error input! See the help below."<<endl;
                showHelp();
                break;
            }
        }
        
    } 
}

void FileSys::exit()
{
    updateSuperBlock();
    fs.close();
    cout<<"Exit successfully"<<endl;
}

void FileSys::updateSuperBlock()
{
    fs.seekp(0);
    fs.write(reinterpret_cast<const char*>(&disk.superBlock),sizeof(SuperBlock));
}

void FileSys::getRootDir(Directory* dir)
{
    //首先找到根目录 第一个inode就是root的inode
    // int i_start = disk.superBlock.inode_begin;
    // fs.seekg(i_start);//开始读
    INode root;
    getINode(disk.superBlock.inode_begin,&root);
    getDir(&root,dir);
}

void FileSys::open()
{
    try
    {
        fs.open("../VDISK",ios::in|ios::out|ios::binary);
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
     
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
            return dir->items[i].i_addr;
        }       
    }
    return -1;
}
void FileSys::getINode(int addr,INode* inode)
{
    fs.seekg(addr);
    //cout<<addr<<endl;
    fs.read(reinterpret_cast<char*>(inode),sizeof(INode));
    //cout<<inode->filesize<<endl;
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
    for(int i=disk.superBlock.data_area_start_index;
            i<disk.superBlock.BLOCK_NUM;i++)
    {
        if(cnt<num && disk.superBlock.block_bitmap[i]==false)
        {
            int addr = disk.superBlock.convertBlockIdToAddr(i);  
            disk.superBlock.block_bitmap[i]=1;          
            addrs.push_back(addr);
            ++cnt;
        }
        if(cnt==num)
        {
            break;
        }
    }
    disk.superBlock.free_blocks -= num;
    disk.superBlock.blocks_used += num;
    return addrs;
}

int FileSys::allocateINode(int& id)
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
            int addr = disk.superBlock.convertInodeIdToAddr(i);
            disk.superBlock.inode_bitmap[i]=1;
            id = i;
            disk.superBlock.inode_used+=1;
            disk.superBlock.free_inode-=1;
            return addr;
        }
    }
    return -1;
    
}
bool FileSys::appendDir(INode* cur,string name,int inodeAddr, int currAddr)
{
    //cout<<"appended addr: "<<inodeAddr<<endl;
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

   if(inodeAddr!=-1)
   {
        // int i_id = (inodeAddr-disk.superBlock.inode_begin)/disk.superBlock.inode_size;
        DirItem item(name,inodeAddr);
        //看是否在前10个block是否还有空位，不包括第十个block的最后一个item，因为，这个有的话，还是要indirect block
        int deadAddr = getDeadInodeAddr(cur);
        if(deadAddr!=-1)
        {
            fs.seekp(deadAddr);
            fs.write(reinterpret_cast<const char*>(&item),sizeof(DirItem));
            cur->filesize+=32;
            writeINode(cur,currAddr);
        }
        else if(item_size<10*32)//这里要重新写，先找到一个live是false的在这里重写
        {
                int block_no = item_size/32;//第几个block
                if(cur->directBlocks[block_no]==-1)
                {
                    cur->directBlocks[block_no]=allocateBlocks(1)[0];
                }
                int offset = (item_size%32)*32;  //在哪个block的字节偏移量
                cur->filesize+=32;
                //write the block to the disk
                int writeAddr = cur->directBlocks[block_no] + offset;
                fs.seekp(writeAddr);
                fs.write(reinterpret_cast<const char*>(&item),sizeof(DirItem));
                writeINode(cur,currAddr);             
        }
        //不在前十个就要找indirect
        else
        {
            //一个indirect block最多存32个地址
            if(cur->indirectBlock==-1)
            {
                cur->indirectBlock = allocateBlocks(1)[0];
                int address = allocateBlocks(1)[0];
                fs.seekp(cur->indirectBlock);
                fs.write(reinterpret_cast<const char*>(&address),sizeof(int));
            }

            vector<int> addrs = getAddrsInIndir(cur->indirectBlock);

            //一个address对应的block最多存32个
            int block_no = (item_size-320)/32;
            int offset = ((item_size-320)%32)*32;
            cur->filesize+=32;
            //write the block to the disk
            if(block_no>=addrs.size())
            {
                int address = allocateBlocks(1)[0];
                addrs.push_back(address);
                fs.seekp(cur->indirectBlock+32*block_no);
                fs.write(reinterpret_cast<const char*>(&address),sizeof(int));
            }
            int writeAddr = addrs[block_no] + offset;
            fs.seekp(writeAddr);
            fs.write(reinterpret_cast<const char*>(&item),sizeof(DirItem));
            writeINode(cur,currAddr); 
        }
   }
   else
   {
        return false;
   }  
   return 1;

}
bool FileSys::deleteDirItem(INode* cur,int currAddr,Directory* dir, string filename)
{
    int i;
    for( i=0;i<dir->getSize();i++)
    {
        if(dir->items[i].name==filename)
        {
            dir->items[i].live = false;
            //dir->items[i].i_addr = -1;  //indirect block will be set later when needed.  It will be set to -1
            cur->filesize-=32;
            writeINode(cur,currAddr);
            break;
        }
    }
    //写回目录 这个
    int block_no = i/32;
    int block_offset = i%32;
    if(block_no<10)
    {
        fs.seekp(cur->directBlocks[block_no]+block_offset * sizeof(DirItem));
        fs.write(reinterpret_cast<const char*>(&dir->items[i]),sizeof(DirItem));
    }
    else
    {
        auto addrs = getAddrsInIndir(cur->indirectBlock);
        fs.seekp(addrs[block_no-10]+block_offset * sizeof(DirItem));
        fs.write(reinterpret_cast<const char*>(&dir->items[i]),sizeof(DirItem));
    }
    return true;
    
}
bool FileSys::getDir(INode* inode,Directory* dir)
{
    if(inode==nullptr)
    {
        cerr<<"nullprt error"<<endl;
        return false;
    }
    if(inode->type!=D)
    {
        cerr<<"filesize: "<<inode->filesize<<"id: "<<inode->i_id<<endl;
        cerr<<"It is not a directory file"<<endl;
        return false;
    }
    dir->items.clear();
    int size = inode->filesize;
    
    int item_num = size/32;
    int item_cnt = 0;
    //先读前面十个
    for(int i =0;i<10;i++)
    {
        //cout<<inode->directBlocks[i]<<endl;
        if(inode->directBlocks[i]!=-1)
        {
            fs.seekg(inode->directBlocks[i]);
            //cout<<inode->directBlocks[i]<<endl;
            for(int j=0;j<32;j++)
            {
                try
                {
                    /* code */
                    DirItem item;
                    fs.read(reinterpret_cast<char*>(&item),sizeof(DirItem));                   
                    if(item.i_addr!=-1 && item.i_addr!=0 && item.live)//0表示没有用过 -1表示被删除了
                    {
                        //cout<<"item info"<<item.i_addr<<" "<<item.name<<" "<<item.live<<endl;
                        dir->items.push_back(item);
                    }                   
                }
                catch(const std::exception& e)
                {
                    std::cerr << e.what() << '\n';
                }
            }
        }
    }
    if(inode->indirectBlock!=-1)
    {
        vector<int> addrs = getAddrsInIndir(inode->indirectBlock);
        for(int i=0;i<addrs.size();i++)
        {
            fs.seekg(addrs[i]);
            for(int j=0;j<32;j++)
            {
                DirItem item;
                fs.read(reinterpret_cast<char*>(&item),sizeof(DirItem));
                if(item.i_addr!=0 && item.i_addr!=-1 && item.live)
                {
                    //cout<<item.name<<endl;
                    dir->items.push_back(item);
                }
            }
        }
    }
    return true;
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
        if(addr!=0)
        {
            addrs.push_back(addr);
        }
    }
    return addrs;
}

void FileSys::generateRandomChs(int addr)
{
    fs.seekp(addr);
    char ch;
    for(int i =0;i<1023;i++)
    {
        try
        {
            ch = char((rand()%90) +31 );
        }
        catch(const std::exception& e)
        {
            cerr<<"Error in generate Random characters"<<endl;
            std::cerr << e.what() << '\n';
        }
        fs.write(reinterpret_cast<const char*>(&ch),sizeof(char));            
    }
    ch = '\0';
    fs.write(reinterpret_cast<const char*>(&ch),sizeof(char));            
}

void FileSys::freeBlock(int& id)
{
    if(id>=disk.superBlock.BLOCK_NUM || id<0)
    {
        cerr<<"block id is wrong, check your inode id and the id is "<<id<<endl;
    }
    disk.superBlock.free_blocks+=1;
    disk.superBlock.blocks_used-=1;
    disk.superBlock.block_bitmap[id]=0;
    id=-1;//avoid free twice
    //set content to be zero 
    //防止重利用的时候，读的坏数据
    //如果在inode上添加额外信息，控制读取范围其实这一步可以不要
    int addr = blockIdToAddr(id);
    fs.seekp(addr);
    int w = 0;
    for(int i=0;i<disk.superBlock.block_size/4;i++)
    {
        fs.write(reinterpret_cast<const char*>(&w),sizeof(int));
    }
}
/**
 * @todo 针对文件和目录对应下面的block的free
*/
void FileSys::freeInode(int& id)
{
    if(id>=disk.superBlock.INODE_NUM || id<0)
    {
        cerr<<"inode id is wrong, check your inode id and the id is "<<id<<endl;
        return;
    }
    disk.superBlock.inode_bitmap[id]=0;
    disk.superBlock.free_inode+=1;
    disk.superBlock.inode_used-=1;
    id=-1;//avoid free twice
}

int FileSys::blockAddrToId(int addr)
{
    if(addr<0 || addr>=16*1024*1024)
    {
        cerr<<"Error block address and the addr is "<<addr<<endl;
        return -1;
    }
    int id = (addr-disk.superBlock.data_begin)/disk.superBlock.block_size;
    return id;
}

//when we update the inode, we need write back
void FileSys::writeINode(INode* inode,int addr)
{
    fs.seekp(addr);
    fs.write(reinterpret_cast<const char*>(inode),sizeof(INode));
}

//when a directory updates, wirte back
//@attention 只有在添加少于32个的目录项的时候可以用
void FileSys::writeDir(Directory* dir, int addr)
{
    fs.seekp(addr);
    for(int i=0;i<dir->getSize();i++)
    {
        fs.write(reinterpret_cast<const char*>(&dir->items[i]),sizeof(DirItem));
    }
}

template<typename T>
void FileSys::getContent(vector<T>&v, int addr, int capacity)
{
    fs.seekg(addr);
    T t;
    for(int i=0;i<capacity;i++)
    {
        fs.read(reinterpret_cast<char*>(&t),sizeof(T));
        v.push_back(t);
    }
}

void FileSys::showStorageInfo()
{
    cout<<"Blocks used: "<<disk.superBlock.blocks_used<<endl;
    cout<<"Blocks unused: "<<disk.superBlock.free_blocks<<endl;
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
    disk.superBlock.block_bitmap[disk.superBlock.data_area_start_index]=1;    //
    disk.superBlock.blocks_used+=1;             //one inode and a data block

    //set inode information
    INode root(0,D,0);
    root.directBlocks[0]=d_start;
    root.filesize = 32*4;   //两个items

    //set directory information
    Directory dir;
    int usr_id;
    int usr_addr = allocateINode(usr_id);
    //cout<<"id " <<usr_id<<"addr "<<usr_addr<<endl;

    int tmp_id;
    int tmp_addr = allocateINode(tmp_id);
    //cout<<"id " <<tmp_id<<"addr "<<tmp_addr<<endl;

    //set the usr and tmp
    INode usr(usr_id,D,32*2);
    INode tmp(tmp_id,D,32*2);
    //store . and ..
    auto u_addr = allocateBlocks(1);
    auto t_addr = allocateBlocks(1);
 
    usr.directBlocks[0] = u_addr[0];
    tmp.directBlocks[0] = t_addr[0];
    Directory u_dir;
    Directory t_dir;
    
    u_dir.items.push_back(DirItem(".",usr_addr));
    u_dir.items.push_back(DirItem("..",i_start));
    t_dir.items.push_back(DirItem(".",tmp_addr));
    t_dir.items.push_back(DirItem("..",i_start));

    //root dir
    dir.items.push_back(DirItem(".",i_start));
    dir.items.push_back(DirItem("..",i_start));
    dir.items.push_back(DirItem("usr",usr_addr));
    dir.items.push_back(DirItem("tmp",tmp_addr));
    
    //write inodes to disk
    writeINode(&root,i_start);
    writeINode(&usr,usr_addr);
    writeINode(&tmp,tmp_addr);

    //write data to data area
    //first 10 blocks
    writeDir(&dir,root.directBlocks[0]);
    writeDir(&u_dir,usr.directBlocks[0]);
    writeDir(&t_dir,tmp.directBlocks[0]);
    
    updateSuperBlock();
    cout<<"format success"<<endl;
}

void FileSys::freeDIrHelper(INode* inode)
{
    //记得要写回去

    if(inode==nullptr)
    {
        return;
    }
    freeInode(inode->i_id);
    if(inode->type==F)//删除文件
    {
        for(int i =0;i<10;i++)
        {
            if(inode->directBlocks[i]!=-1)
            {
                int id = blockAddrToId(inode->directBlocks[i]);
                inode->directBlocks[i]=-1;
                freeBlock(id);
            }
        }
        if(inode->indirectBlock!=-1)
        {
            int id = blockAddrToId(inode->indirectBlock);
            freeBlock(id);
        }
    }
    else
    {
        Directory dir;
        getDir(inode,&dir);
        for(int i =0;i<10;i++)
        {
            if(inode->directBlocks[i]!=-1)
            {
                int id = blockAddrToId(inode->directBlocks[i]);
                inode->directBlocks[i]=-1;
                freeBlock(id);
            }
        }
        if(inode->indirectBlock!=-1)
        {
            int id = blockAddrToId(inode->indirectBlock);
            freeBlock(id);
        }
        for(int i=2;i<dir.getSize();i++)//. 和..会无限循环
        {
            INode next;
            getINode(dir.items[i].i_addr,&next);
            freeDIrHelper(&next);
        }  
    }  
}

int FileSys::blockIdToAddr(int id)
{
    int addr = disk.superBlock.data_begin+id*disk.superBlock.block_size;
    return addr;
}

string FileSys::getAbsPath(vector<string>&paths)
{
    string absPath = "";
    //先判断是否与当前的路径相同
    if(paths[0]==".")
    {
        absPath = curDir;
        for(int i =1;i<paths.size();i++)
        {
            absPath+=paths[i]+"/";
        }
        
    }
    else if(paths[0]=="..")
    {
        if(curDir=="/")
        {
            for(int i =1;i<paths.size();i++)
            {
                absPath+=paths[i]+"/";
            }
            if(paths.size()==1)
            {
                absPath="/";
            }
        }
        else
        {
            auto p = pathResolve(curDir);
            absPath = curDir.substr(0,curDir.size()-p[p.size()-1].size()-1);
            for(int i =1;i<paths.size();i++)
            {
                absPath+=paths[i]+"/";
            }

        }
    }
    else
    {
        if(paths[0]=="/")
        {
            absPath="/";
            paths.erase(paths.begin());
        }
        for(int i =0;i<paths.size();i++)
        {
            absPath+=paths[i]+"/";
        }
    }
    
    //cout<<absPath<<endl;//test
    return absPath;
}
void FileSys::getFirstInodeDir(vector<string>& paths,INode* inode,Directory* dir)
{
    if(paths[0]=="/")
    {
        getRootDir(dir);
        paths.erase(paths.begin());
        getINode(disk.superBlock.inode_begin,inode);
    }
    else
    {
        getDir(currInode,dir);
        int addr = getInodeAddrByName(".",dir);
        getINode(addr,inode);
    }
}

int FileSys::getDeadInodeAddr(INode* inode)
{
    for(int i=0;i<10;i++)
    {
        DirItem item;
        if(inode->directBlocks[i]!=-1)
        {
            fs.seekg(inode->directBlocks[i]);
            for(int j=0;j<32;j++)
            {
                int addr = fs.tellg();
                fs.read(reinterpret_cast<char*>(&item),sizeof(DirItem));
                if(item.live==false)
                {
                    return addr;//return addr of dead inode addr
                }
            }
        }
    }
    if(inode->indirectBlock!=-1)
    {
        vector<int> addrs = getAddrsInIndir(inode->indirectBlock);
        for(int i=0;i<addrs.size();i++)
        {
            fs.seekg(addrs[i]);
            for(int j=0;j<32;j++)
            {
                DirItem item;
                int addr = fs.tellg();
                fs.read(reinterpret_cast<char*>(&item),sizeof(DirItem));
                if(item.live==false)
                {
                    return addr;
                }
            }
        }
    }
    return -1;
}