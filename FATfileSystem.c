//
// Created by Kryst on 07.02.2023.
//

#include <malloc.h>
#include "FATfileSystem.h"
#include "DirCluster.h"


FATFileSystem *createFileSystem(unsigned int totalSize) {
    FATFileSystem *fileSystem = malloc(sizeof(FATFileSystem));
    fileSystem->clusterCount = totalSize / CLUSTER_SIZE;
    fileSystem->fatTable1 = createFATTable(fileSystem->clusterCount);
    fileSystem->fatTable2 = createFATTable(fileSystem->clusterCount);
    fileSystem->clusterArea = malloc(fileSystem->clusterCount * CLUSTER_SIZE);
    fileSystem->rootDirCluster = fileSystem->clusterArea;
    fileSystem->currentDirCluster = fileSystem->clusterArea;
    fileSystem->freeSpace = fileSystem->clusterCount * CLUSTER_SIZE;
    fileSystem->totalSpace = fileSystem->clusterCount * CLUSTER_SIZE;

    createRootDir(fileSystem);
    return fileSystem;
}



void freeFileSystem(FATFileSystem **fileSystem) {
    if (fileSystem == NULL || *fileSystem == NULL) {
        return;
    }

    freeFATTable((&(* fileSystem)->fatTable1));
    freeFATTable((&(* fileSystem)->fatTable2));
    free((*fileSystem)->clusterArea);
    free(*fileSystem);
}

int saveNewFileSystem(FATFileSystem *fileSystem, char *name) {
    if (fileSystem == NULL || name == NULL) {
        return 1;
    }

    FILE *file = fopen(name, "wb");
    if (file == NULL) {
        return 1;
    }

    fwrite(fileSystem, 4, 2, file);
    fwrite(fileSystem->fatTable1, sizeof(unsigned int), fileSystem->fatTable1->clustersCount, file);
    fwrite(fileSystem->fatTable2, sizeof(unsigned int), fileSystem->fatTable2->clustersCount, file);
    fwrite(fileSystem->clusterArea, 1, fileSystem->clusterCount * CLUSTER_SIZE, file);

    fclose(file);
    return 0;
}

void createRootDir(FATFileSystem *system) {
    if (system == NULL) {
        return;
    }

    createDirCluster(system->clusterArea, 0, 0);
    system->fatTable1->clusters[0] = 0xFFFFFFFF;
    system->fatTable2->clusters[0] = 0xFFFFFFFF;
    system->fatTable1->freeClustersCount--;
    system->fatTable2->freeClustersCount--;
    system->freeSpace -= CLUSTER_SIZE;

}

FATFileSystem *loadFileSystem(char *name) {
    if (name == NULL) {
        return NULL;
    }

    FILE *file = fopen(name, "rb");
    if (file == NULL) {
        return NULL;
    }

    FATFileSystem *fileSystem = malloc(sizeof(FATFileSystem));
    fread(fileSystem, 4, 2, file);
    fileSystem->fatTable1 = createFATTable(fileSystem->clusterCount);
    fileSystem->fatTable2 = createFATTable(fileSystem->clusterCount);
    fileSystem->clusterArea = malloc(fileSystem->clusterCount * CLUSTER_SIZE);
    fread(fileSystem->fatTable1, sizeof(unsigned int), fileSystem->fatTable1->clustersCount, file);
    fread(fileSystem->fatTable2, sizeof(unsigned int), fileSystem->fatTable2->clustersCount, file);
    fread(fileSystem->clusterArea, 1, fileSystem->clusterCount * CLUSTER_SIZE, file);

    fclose(file);
    return fileSystem;
}

int addDirectory(FATFileSystem *fileSystem, char *name, DirCluster *cluster) {
    if (fileSystem == NULL || name == NULL) {
        return 1;
    }


    int freeCluster = getFreeCluster(fileSystem->fatTable1, fileSystem->fatTable2);
    if (freeCluster == -1) {
        return 1;
    }
    if (cluster == NULL) cluster = fileSystem->currentDirCluster;
    if(addDirEntry(cluster, name, freeCluster, 0, 2) == 1) return 1;

    createDirCluster(fileSystem->clusterArea + freeCluster * CLUSTER_SIZE, freeCluster, fileSystem->currentDirCluster->entries[1].cluster);
    fileSystem->fatTable1->clusters[freeCluster] = 0xFFFFFFFF;
    fileSystem->fatTable2->clusters[freeCluster] = 0xFFFFFFFF;
    fileSystem->fatTable1->freeClustersCount--;
    fileSystem->fatTable2->freeClustersCount--;
    fileSystem->freeSpace -= CLUSTER_SIZE;
    return 0;
}

int addFile(FATFileSystem *fileSystem, char *name, unsigned int size, char *content, DirCluster *cluster) {
    if (fileSystem == NULL || name == NULL || content == NULL || size > fileSystem->freeSpace) {
        return 1;
    }

    int freeCluster = getFreeCluster(fileSystem->fatTable1, fileSystem->fatTable2);
    if (freeCluster == -1) {
        return 1;
    }

    if (cluster == NULL) cluster = fileSystem->currentDirCluster;

    if (addDirEntry(cluster, name, freeCluster, size, 1) == 1) return 1;

    int clustersCount = size / CLUSTER_SIZE;
    if (size % CLUSTER_SIZE != 0) clustersCount++;
    for (int i = 0; i < clustersCount; i++) {
        int nextFreeCluster = getNextFreeCluster(fileSystem->fatTable1, fileSystem->fatTable2, freeCluster);
        memccpy(fileSystem->clusterArea + freeCluster * CLUSTER_SIZE, content + i * CLUSTER_SIZE, 0, CLUSTER_SIZE);
        fileSystem->fatTable1->clusters[freeCluster] = nextFreeCluster;
        fileSystem->fatTable2->clusters[freeCluster] = nextFreeCluster;
        fileSystem->fatTable1->freeClustersCount--;
        fileSystem->fatTable2->freeClustersCount--;
        freeCluster = nextFreeCluster;
    }
    fileSystem->fatTable1->clusters[freeCluster] = 0xFFFFFFFF;
    fileSystem->fatTable2->clusters[freeCluster] = 0xFFFFFFFF;
    fileSystem->freeSpace -= size;
    return 0;
}

int deleteDirectory(FATFileSystem *fileSystem, char *name, DirCluster *cluster) {
    if (fileSystem == NULL || name == NULL || cluster == NULL) {
        return 1;
    }

    // check if directory is empty
    for (int i = 2; i < CLUSTER_SIZE / sizeof(DirEntry); i++) {
        if (cluster->entries[i].type != 0) {
            return 1;
        }
    }

    // remove directory from parent directory
    for (int i = 2; i < CLUSTER_SIZE / sizeof(DirEntry); i++) {
        if (strncmp(fileSystem->currentDirCluster->entries[i].name, name, 12) == 0) {
            fileSystem->currentDirCluster->entries[i].type = 0;
            break;
        }
    }

    // remove directory from data area
    memset(cluster, 0, CLUSTER_SIZE);

    // remove directory from FAT tables
    int clusterNumber = cluster->entries[1].cluster;
    fileSystem->fatTable1->clusters[clusterNumber] = 0;
    fileSystem->fatTable2->clusters[clusterNumber] = 0;
    fileSystem->fatTable1->freeClustersCount++;
    fileSystem->fatTable2->freeClustersCount++;
    fileSystem->freeSpace += CLUSTER_SIZE;

    return 0;
}

int deleteFile(FATFileSystem *fileSystem, char *name, DirCluster *cluster) {
    if (fileSystem == NULL || name == NULL || cluster == NULL) {
        return 1;
    }
    int clusterNumber = 0;
    int size = 0;
    // remove file from parent directory
    for (int i = 2; i < CLUSTER_SIZE / sizeof(DirEntry); i++) {
        if (strncmp(cluster->entries[i].name, name, 12) == 0) {
            cluster->entries[i].type = 0;
            clusterNumber = cluster->entries[i].cluster;
            size = cluster->entries[i].size;
            break;
        }
    }

    // remove file from data area
    int clustersCount = size / CLUSTER_SIZE;
    if (size % CLUSTER_SIZE != 0) clustersCount++;
    int nextCluster = fileSystem->fatTable1->clusters[clusterNumber];

    for (int i = 0; i < clustersCount; i++) {
        memset(fileSystem->clusterArea + clusterNumber * CLUSTER_SIZE, 0, CLUSTER_SIZE);
        fileSystem->fatTable1->clusters[clusterNumber] = 0;
        fileSystem->fatTable2->clusters[clusterNumber] = 0;
        clusterNumber = nextCluster;
        nextCluster = fileSystem->fatTable1->clusters[clusterNumber];
    }

    fileSystem->fatTable1->freeClustersCount += clustersCount;
    fileSystem->fatTable2->freeClustersCount += clustersCount;
    fileSystem->freeSpace += size;

    return 0;

}

DirCluster *findDirectory(FATFileSystem *fileSystem, char *name) {
    if (fileSystem == NULL || name == NULL) {
        return NULL;
    }

    DirCluster *cluster = name[0] == '/' ? fileSystem->currentDirCluster : fileSystem->rootDirCluster;
    char *token = strtok(name, "/");
    while (token != NULL) {
        int found = 0;
        for (int i = 0; i < CLUSTER_SIZE / sizeof(DirEntry); i++) {
            if (strncmp(cluster->entries[i].name, token, 12) == 0) {
                if (cluster->entries[i].type == 2) {
                    cluster = fileSystem->clusterArea + cluster->entries[i].cluster * CLUSTER_SIZE;
                    found = 1;
                    break;
                } else {
                    return cluster;
                }

            }
        }
        if (!found) {
            return NULL;
        }
        token = strtok(NULL, "/");
    }

    return cluster;

}

int getFileContent(FATFileSystem *fileSystem, char *name, char *content, DirCluster *cluster) {
    if (fileSystem == NULL || name == NULL || content == NULL) {
        return 1;
    }

    if (cluster == NULL) cluster = fileSystem->currentDirCluster;

    // find file in parent directory
    DirEntry *dirEntry = findDirEntry(cluster, name);
    int clusterNumber = dirEntry->cluster;
    int size = dirEntry->size;

    // get file content from data area
    int clustersCount = size / CLUSTER_SIZE;
    int nextCluster = fileSystem->fatTable1->clusters[clusterNumber];

    for (int i = 0; i < clustersCount; i++) {
        memccpy(content + i * CLUSTER_SIZE, fileSystem->clusterArea + clusterNumber * CLUSTER_SIZE, 0, CLUSTER_SIZE);
        clusterNumber = nextCluster;
        nextCluster = fileSystem->fatTable1->clusters[clusterNumber];
    }
    // copy last cluster
    memccpy(content + clustersCount * CLUSTER_SIZE, fileSystem->clusterArea + clusterNumber * CLUSTER_SIZE, 0, size % CLUSTER_SIZE);

    return 0;
}









