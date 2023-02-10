//
// Created by Kryst on 08.02.2023.
//


#ifndef PSEUDOFAT_DIRCLUSTER_H
#define PSEUDOFAT_DIRCLUSTER_H

#include <string.h>
#include "sizes.h"
#include "DirEntry.h"

typedef struct {
    DirEntry entries[CLUSTER_SIZE / sizeof(DirEntry)];
} DirCluster;

int createDirCluster(void *cluster, unsigned int clusterNumber, unsigned int parentClusterNumber);
int addDirEntry(void *cluster, char *name, unsigned int clusterNumber, unsigned int size, char type);
int removeDirEntry(void *cluster, char *name);
int getFreeDirEntry(void *cluster);


#endif //PSEUDOFAT_DIRCLUSTER_H
