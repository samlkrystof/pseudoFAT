//
// Created by Kryst on 07.02.2024.
//

#ifndef PSEUDOFAT_DIRECTORY_OPERATIONS_H
#define PSEUDOFAT_DIRECTORY_OPERATIONS_H

#include "iNodeFileSystem.h"

int addDirectory(iNodeFileSystem *fileSystem, char *name, unsigned int parent);
int addFile(iNodeFileSystem *fileSystem, char *name, unsigned int size, char *content, unsigned int parent);
int deleteDirectory(iNodeFileSystem *fileSystem, char *name);
int deleteFile(iNodeFileSystem *fileSystem, char *name);
int addBaseEntries(iNodeFileSystem *fileSystem, unsigned int inode, unsigned int parent, unsigned int block);
int addToParent(iNodeFileSystem *fileSystem, unsigned int parent, unsigned int inode, char *name);
int findDirectory(iNodeFileSystem *fileSystem, char *name);
unsigned int findParentDir(iNodeFileSystem *fileSystem, char *name);
dirEntry *getDirEntry(iNodeFileSystem * fileSystem, int inode, char *name);
int addDirEntry(iNodeFileSystem *fileSystem, int parent, int inode, char *name);
char *getFileName(char *name);
int *getAllBlocks(iNodeFileSystem *fileSystem, unsigned int inode, int *numBlocks);



#endif //PSEUDOFAT_DIRECTORY_OPERATIONS_H
