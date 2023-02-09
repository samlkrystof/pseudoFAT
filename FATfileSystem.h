//
// Created by Kryst on 07.02.2023.
//
#include "sizes.h"
#include "FATTable.h"
#include "DirCluster.h"

#ifndef PSEUDOFAT_FATFILESYSTEM_H
#define PSEUDOFAT_FATFILESYSTEM_H

#endif //PSEUDOFAT_FATFILESYSTEM_
typedef struct {
    unsigned int clusterCount;
    unsigned int totalSpace;
    unsigned int freeSpace;
    unsigned int rootDirCluster;
    unsigned int currentDirCluster;
    FATTable *fatTable1;
    FATTable *fatTable2;
    void *clusterArea;
} FATFileSystem;

FATFileSystem *createFileSystem(unsigned int totalSize);
int saveNewFileSystem(FATFileSystem *fileSystem, char *name);
void freeFileSystem(FATFileSystem **fileSystem);
FATFileSystem *loadFileSystem(char *name);
int addDirectory(FATFileSystem *fileSystem, char *name);



// H
