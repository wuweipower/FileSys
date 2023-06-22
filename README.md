# Design of  the distribution of the virtual disk and base structure

## Disk area design

we separate the 16 MB into the following distribution.

![image-20230603153151359](img/image-20230603153151359.png)

The super block area stores the information about the whole virtual disk.

The I-Node area stores the the I-Nodes.

The data area stores the content of files and directories.

the address we set is 32 bits. Even if it just need 24 bits, it need complex management, and we need to used some structure occupying 3 bytes such as a 3-element char array which is not easy for programming.

## For Super Block

We design a structure to store the information we need. We store the following information:

- the total number of blocks which is 16*1024 since one block occupies 1 KB
- the total number of I-Nodes which we first set to 1024.
- the bitmap of blocks which can help us to determine whether the corresponding block is free.  Also, it can help us to find the free blocks and manage the used blocks.
- the bitmap of I-Nodes which  can help us to determine whether the corresponding I-Node is free. Also, it can help us to find the free I-Nodes and manage the used I-Nodes.
- the block size which is 1024 Byte.
- the I-Node size which is 128 Byte.
- the number of blocks used.
- the number of free blocks.
- the number of I-Nodes used.
- the number of free I-Nodes.
-  the starting address of I-Node area.
- the starting address of data area.
- The index of data area starting which is used for find the free data block the the block bitmap.

The total size of this structure is about 17 KB but for better extension, we use 20 KB to store which also help us to manage the virtual disk.



## For I-Node

We design a simplified I-Node structure which just stores some important information

- id: used for search in the bitmap. And -1 means it has been deleted
- type: directory or file
- file size: the size of the content whose unit is byte
- direct block address array which has 10 elements
- indirect block address which store the address of the block storing the real address of file content
- create time: the time we create the I-Node
- modify time: the time we modify the I-Node



The total size of this structure is 128 byte and which the power of 2, so it is for us to manage the block position since one block can just store 8 I-Nodes.



## For Data area

We just store two kinds of content. One is directory entry. One is file content.

### directory entry

The directory entry design is following:

- name: 24 byte char array. 
- I-Node address
- index:  Actually it is not used often. Just used to padding.

Why the name is only 24 byte long?

Because if we use 24 bytes to store the name, the total size of this structure is 32 Byte which is power of 2 and it can avoid the memory fragments.

But if we need longer name size, it is alright. The only point we need to pay attention on is that we need to make sure the total structure size is the power of 2.

For better managing the directory entry, we use a array to store the entries of a directory.







排查的bug

1. 做了更改忘了写回磁盘
2. 对不存在的地址没有处理 导致不知道读写指针跑到哪里了
3. 

bug:
deleteFile 有问题 添加位置
createFile 
文件大小的修改 好像有问题
发现随机字符写入有的字符具有控制效果，容易让程序甚至cmd终端出现异常。需要控制ASCII的范围
执行cp后如果删除原来的文件，在删除copy的文件发现删除后仍显示，原因size那里导致顺寻不一样，原因是如果先删前面，后面文件的得到的i对应的磁盘中的地址不对如果先删除cp后的，不会有异常,但是使用的block数量出现问题 1.当大小大于10的时候，忘了free间接block 

deleteDir 后ls异常 已修复，递归删除那里，条件写错了  后续发现sum异常，原来是忘了free block