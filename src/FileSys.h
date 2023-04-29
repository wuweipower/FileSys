#ifndef FILESYS
#define FILESYS

#include<iostream>
#include<string>
#include"INode.h"
#include"Disk.h"
#include<vector>
#include<random>
using namespace std;
using std::string;
//16MB space
//space is divided into blocks with block size 1KB 
//address length is 24-bit
//The i-node should support 10 direct block addresses, 
//and one indirect block address
/**
 * 一个char是1个字节，一个block是一个2^10，有2^14个block
 * 所以24bit的结构为block_number + offset_in_a_block
 * 使用int来存，浪费1个字节，但方便操作。
 * 
*/
/**
 * the first a few blocks can be used for storing the i-nodes, 
 * and the first i-node can be used for the root directory (/). 
*/

/**
 * using random strings to fill the files you created. 
 * It means you just need to specify the file size (in KB) and path+name.
*/

/**
 * Designed idea
 * super_block + ionde + data_area
*/

class FileSys
{
private:
    string curDir;//current directory
    Disk disk; 
    fstream fs;
    INode* currInode;

    void init();
    void showHelp();

    vector<string> pathResolve(string path);

    void open();
    void close();

    void updateSuperBlock();//write back when exit

    void getRootDir(Directory* dir);
    void getINode(int addr,INode* inode);
    void writeINode(INode*,int addr);
    
    void getDir(INode* inode,Directory* dir);
    void writeDir(INode*,int addr);

    int getInodeAddrByName(string filename,Directory* dir);//不存在就返回-1,在当前目录下找对应的文件名的inode
    bool appendDir(INode* cur,string name);//append dir on the specified inode

    int allocateINode(int&);                 //return the address
    vector<int> allocateBlocks(int);         //return the addresses
    void freeInode(int id);
    void freeBlock(int id);

    vector<int> getAddrsInIndir(int addr);

    void generateRandomChs(int addr);

    int blockAddrToId(int addr);


public:

    static const int MAX_FILE_SIZE = 42;//10+1024/32 KB
    //A welcome message with the group info (names and IDs) when the system is launched. 
    //It is also the claim of your ‘copyright’
    void welcome()
    {
        cout<<"   Designed by Xin Huang  "<<endl;
        cout<<"Copyright reserved by Xin Huang"<<endl;
    }

    //constructor
    FileSys();
    ~FileSys();

    //createFile /dir1/myFile 10 (in KB)
    //if fileSiz > max file size, print out an error message.
    bool createFile(string filename, string filesize);

    //deleteFile filename 
    //deleteFile /dir1/myFile
    bool deleteFile(string filename);

    //create a directory：createDir
    //createDir /dir1/sub1 (should support nested-directory)
    bool createDir(string dir); 

    //Delete a directory：deleteDir
    //deleteDir /dir1/sub1 （The current working directory is not allowed to be deleted）
    bool deleteDir(string dir);

    //Change current working direcotry：changeDir 
    //changeDir /dir2
    bool cd(string dir);

    //List all the files and sub-directories under current working directory：dir 
    //You also need to list at least two file attributes. (i.e. file size, time created, etc.)
    bool ls();

    //copy a file
    //cp file1 file2
    bool cp(string from,string to);

    //Display the usage of storage space：sum
    //Display the usage of the 16MB space. 
    //You need to list how many blocks are used and how many blocks are unused. 
    void showStorageInfo();

    //Print out the file contents: cat
    //cat /dir1/file1
    void cat(string filename);

    //command resolve
    void entry();

    //show exit info
    void exit();

    //In the dev, test some functions
    void test();
    
};

#endif