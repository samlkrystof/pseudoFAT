//
// Created by Kryst on 06.02.2024.
//

#ifndef PSEUDOFAT_INODEFILESYSTEM_H
#define PSEUDOFAT_INODEFILESYSTEM_H

#define CLUSTER_SIZE 1024;

typedef struct iNode {
    char isDirectory;
    unsigned int size; // size of file in bytes, in case of directory it's number of entries
    int directBlocks[5];
    int indirectBlock;
    int doubleIndirectBlock;
} iNode;

typedef struct {
    char name[12];
    unsigned int iNode;
} dirEntry;

typedef struct {
    unsigned int size;
    unsigned int blockSize;
    unsigned int freeBlocks;
    unsigned int freeINodes;
    unsigned int iNodesCount;
    unsigned int blocksCount;
    dirEntry *rootDir;
    dirEntry *currentDir;
    iNode *iNodes;
    char *blocks;
    char *iNodesBitmap;
    char *blocksBitmap;
} iNodeFileSystem;

iNodeFileSystem *createFileSystem(unsigned int totalSize);
void createInodes(iNodeFileSystem *system);
void createRootDir(iNodeFileSystem *system);
iNodeFileSystem *loadFileSystem(char *name);
int saveNewFileSystem(iNodeFileSystem *fileSystem, char *name);
void freeFileSystem(iNodeFileSystem **fileSystem);

#endif //PSEUDOFAT_INODEFILESYSTEM_H
