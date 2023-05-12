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

    /**
     * @return return the sperated terms, simple
     * @example pathResolve("../usr/cpp/local") .. usr cpp local
     * @example pathResolve("./usr/cpp/local"); . usr cpp local
     * @example pathResolve("usr/cpp/local"); usr cpp local
    */
    vector<string> pathResolve(string path);

    void open();
    void close();

    /**
     * write superblock back to disk in order to keep consistency
    */
    void updateSuperBlock();

    //get the root directory and it is start of everything
    void getRootDir(Directory* dir);

    //according to the address to get the inode content
    void getINode(int addr,INode* inode);

    //write the inode data to disk
    void writeINode(INode*,int addr);
    
    //according to the inode to get the directory items
    bool getDir(INode* inode,Directory* dir);

    //write the directory values back to disk
    void writeDir(Directory *dir,int addr);

    /**
     * according to the filename to find the inode address
     * and if not exis return -1
    */
    int getInodeAddrByName(string filename,Directory* dir);

    /**
     * append the directory item on the specified inode
    */
    bool appendDir(INode* cur,string name,int inodeAddr,int currAddr);

    /**
     * delete the directory item on the specified inode
    */
    bool deleteDirItem(INode* cur,int currAddr,Directory* dir, string filename);

    /**
     * @param id the parameter will be changed to the allocated inode id
     * @return return the address of the corrsponding block addres
     * @attention it starts from the inode area
    */
    int allocateINode(int& id);                 

    /**
     * @param num the number of allocating blocks
     * @return return the addresses
     * @attention it starts from the data area
    */
    vector<int> allocateBlocks(int num);    

    /**
     * @param id the id of the freed inode
    */     
    void freeInode(int& id);

    /**
     * @param id the id of the freed block
     * it just change the indexes
    */
    void freeBlock(int& id);

    /**
     * @param addr the indirect block address
     * @return the address in the indirect block
    */
    vector<int> getAddrsInIndir(int addr);

    /**
     * generate and write 
    */
    void generateRandomChs(int addr);

    /**
     * @param addr the address
     * @return the corrsponding id
    */
    int blockAddrToId(int addr);

    /**
     * @param id the block id
     * @return the address
    */
    int blockIdToAddr(int id);

    /**
     * @param v the container to store
     * @param addr the address
     * @param capacity the capacity in the address block
     * @attention the vector container will be only exactly the content
    */
    template<typename T>
    void getContent(vector<T>&v,int addr,int capacity);

    void freeDIrHelper(INode* inode);

    string getAbsPath(vector<string>&paths);

    void getFirstInodeDir(vector<string>& paths,INode* inode,Directory* dir);

    /**
     * @return if there is a dead one, return the address, else -1
    */
    int getDeadInodeAddr(INode* inode);

public:

    static const int MAX_FILE_SIZE = 266;//10+1024/4 
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