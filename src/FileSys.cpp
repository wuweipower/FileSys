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
    //disk.init();
    cout<<sizeof(SuperBlock)<<endl;
}

static const int CMD_LEN=64;
FileSys::FileSys()
{
    curDir="/";
}

FileSys::~FileSys()
{
    fs.close();
}


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
    fs.seekp(inode.directBlocks[0]);
    fs.write(reinterpret_cast<const char*>(&dir),sizeof(Directory));


    fs.close();

}
bool FileSys::deleteFile(string filename)
{

}

bool FileSys::createDir(string dir)
{

}

bool FileSys::deleteDir(string dir)
{

}

bool FileSys::cd(string dir)
{

}

bool FileSys::ls()
{

}

bool FileSys::cp(string from, string to)
{

}
void FileSys::showStorageInfo()
{
    cout<<"Blocks used: "<<disk.superBlock.blocks_used<<endl;
    cout<<"Blocks unused: "<<disk.superBlock.free_blocks<<endl;
}

void FileSys::cat(string filename)
{

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
