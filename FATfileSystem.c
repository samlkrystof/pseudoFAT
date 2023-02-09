//
// Created by Kryst on 07.02.2023.
//

#include <malloc.h>
#include "FATfileSystem.h"

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
    fwrite(fileSystem->fatTable1, sizeof(unsigned int), fileSystem->fatTable1->entriesCount, file);
    fwrite(fileSystem->fatTable2, sizeof(unsigned int), fileSystem->fatTable2->entriesCount, file);
    fwrite(fileSystem->clusterArea, 1, fileSystem->clusterCount * CLUSTER_SIZE, file);


    fclose(file);
    return 0;
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
    fread(fileSystem->fatTable1, sizeof(unsigned int), fileSystem->fatTable1->entriesCount, file);
    fread(fileSystem->fatTable2, sizeof(unsigned int), fileSystem->fatTable2->entriesCount, file);
    fread(fileSystem->clusterArea, 1, fileSystem->clusterCount * CLUSTER_SIZE, file);

    fclose(file);
    return fileSystem;
}






