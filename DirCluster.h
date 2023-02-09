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

int createDirCluster(void *cluster, unsigned int clusterNumber, unsigned int parentClusterNumber) {
    if (cluster == NULL) {
        return 1;
    }
    DirCluster *dirCluster = (DirCluster *) cluster;
    dirCluster->entries[0] = (DirEntry) {clusterNumber, 0, ".", 2};
    dirCluster->entries[1] = (DirEntry) {parentClusterNumber, 0, "..", 2};

    return 0;
}

int addDirEntry(void *cluster, char *name, unsigned int clusterNumber, unsigned int size, char type) {
    if (cluster == NULL) {
        return 1;
    }
    DirCluster *dirCluster = (DirCluster *) cluster;
    for (int i = 0; i < CLUSTER_SIZE / sizeof(DirEntry); i++) {
        if (!dirCluster->entries[i].type) {
            dirCluster->entries[i] = (DirEntry) {clusterNumber, size, *name, type};
            return 0;
        }
    }
    return 1;
}

int removeDirEntry(void *cluster, char *name) {
    if (cluster == NULL || name == NULL) {
        return -1;
    }
    DirCluster *dirCluster = (DirCluster *) cluster;
    for (int i = 0; i < CLUSTER_SIZE / sizeof(DirEntry); i++) {
        if (dirCluster->entries[i].type && !strcmp(dirCluster->entries[i].name, name)) {
            dirCluster->entries[i].type = 0;
            return dirCluster->entries[i].cluster;
        }
    }
    return -1;
}


#endif //PSEUDOFAT_DIRCLUSTER_H
