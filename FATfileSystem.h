//
// Created by Kryst on 07.02.2023.
//

#include "FATTable.h"

#define CLUSTER_SIZE 1024
#ifndef PSEUDOFAT_FATFILESYSTEM_H
#define PSEUDOFAT_FATFILESYSTEM_H

#endif //PSEUDOFAT_FATFILESYSTEM_
typedef struct{
    unsigned int clusterSize;
    unsigned int clusterCount;
    FATTable *fatTable1;
    FATTable *fatTable2;
    void *clusterArea;
    unsigned int rootDirCluster;
    unsigned int currentDirCluster;
    unsigned int freeSpace;
    unsigned int usedSpace;
    unsigned int totalSpace;

} FATFileSystem;

// H
