//
// Created by Kryst on 09.02.2023.
//

#include "DirCluster.h"

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
    // Check if file with this name already exists
    for (int i = 0; i < CLUSTER_SIZE / sizeof(DirEntry); i++) {
        if (dirCluster->entries[i].type && !strcmp(dirCluster->entries[i].name, name)) {
            return 1;
        }
    }

    int index = getFreeDirEntry(cluster);
    if (index == -1) {
        return 1;
    }
    dirCluster->entries[index] = (DirEntry) {clusterNumber, size, *name, type};
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

int getFreeDirEntry(void *cluster) {
    if (cluster == NULL) {
        return -1;
    }
    DirCluster *dirCluster = (DirCluster *) cluster;
    for (int i = 0; i < CLUSTER_SIZE / sizeof(DirEntry); i++) {
        if (!dirCluster->entries[i].type) {
            return i;
        }
    }
    return -1;
}

DirEntry *findDirEntry(void *cluster, char *name) {
    if (cluster == NULL || name == NULL) {
        return NULL;
    }
    DirCluster *dirCluster = (DirCluster *) cluster;
    for (int i = 0; i < CLUSTER_SIZE / sizeof(DirEntry); i++) {
        if (dirCluster->entries[i].type && !strncmp(dirCluster->entries[i].name, name, 12)) {
            return &dirCluster->entries[i];
        }
    }
    return NULL;
}

