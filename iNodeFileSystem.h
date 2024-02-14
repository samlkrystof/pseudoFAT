//
// Created by Kryst on 06.02.2024.
//

#include <stdio.h>

#ifndef PSEUDOFAT_INODEFILESYSTEM_H
#define PSEUDOFAT_INODEFILESYSTEM_H

#define CLUSTER_SIZE 1024

typedef struct {
    char name[12];
    unsigned int iNode;
} dirEntry;

typedef struct iNode {
    char isDirectory;
    unsigned int size; // size of file in bytes, in case of directory it's number of entries
    int directBlocks[5];
    int indirectBlock;
    int doubleIndirectBlock;
} iNode;

typedef struct {
    unsigned int size;
    unsigned int blockSize;
    unsigned int freeBlocks;
    unsigned int freeINodes;
    unsigned int iNodesCount;
    unsigned int blocksCount;
    iNode *iNodes;
    char *blocks;
    char *iNodesBitmap;
    char *blocksBitmap;
    char **fileNames;
    dirEntry *rootDir;
    dirEntry *currentDir;
    FILE *file;
    char *name;
} iNodeFileSystem;

iNodeFileSystem *createFileSystem(unsigned int totalSize, char *name);
void createInodes(iNodeFileSystem *system);
void createRootDir(iNodeFileSystem *system);
iNodeFileSystem *loadFileSystem(char *name);
int saveNewFileSystem(iNodeFileSystem *fileSystem, char *name);
void freeFileSystem(iNodeFileSystem **fileSystem);
iNode *loadINode(iNodeFileSystem *fileSystem, unsigned int index);
int saveINode(iNodeFileSystem *fileSystem, iNode *iNode, unsigned int index);
char *loadBlock(iNodeFileSystem *fileSystem, unsigned int index);
int saveBlock(iNodeFileSystem *fileSystem, char *block, unsigned int index);
int getBitmapInt(iNodeFileSystem *fileSystem, unsigned int index, int type);
int setBitmapInt(iNodeFileSystem *fileSystem, unsigned int index, int type, int value);
char *loadName(iNodeFileSystem *fileSystem, unsigned int index);
int saveName(iNodeFileSystem *fileSystem, char *name, unsigned int index);

#endif //PSEUDOFAT_INODEFILESYSTEM_H
