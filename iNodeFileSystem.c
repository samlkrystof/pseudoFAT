//
// Created by Kryst on 06.02.2024.
//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "iNodeFileSystem.h"
#include "directory_operations.h"

iNodeFileSystem *createFileSystem(unsigned int totalSize) {
    iNodeFileSystem *fileSystem = malloc(sizeof(iNodeFileSystem));
    fileSystem->size = totalSize;
    fileSystem->blockSize = CLUSTER_SIZE;
    fileSystem->blocksCount = totalSize / fileSystem->blockSize;
    fileSystem->iNodesCount = fileSystem->blocksCount / 10;
    fileSystem->freeBlocks = fileSystem->blocksCount;
    fileSystem->freeINodes = fileSystem->iNodesCount;
    fileSystem->iNodes = malloc(fileSystem->iNodesCount * sizeof(iNode));
    createInodes(fileSystem);
    fileSystem->blocks = calloc(fileSystem->blocksCount, fileSystem->blockSize);
    unsigned int bitmapBlocks = fileSystem->blocksCount / 8 + (8 - fileSystem->blocksCount % 8);
    fileSystem->blocksBitmap = calloc(bitmapBlocks, 1);
    unsigned int bitmapINodes = fileSystem->iNodesCount / 8 + (8 - fileSystem->iNodesCount % 8);
    fileSystem->iNodesBitmap = calloc(bitmapINodes, 1);
    createRootDir(fileSystem);
    fileSystem->currentDir = (dirEntry *)&fileSystem->blocks[0];
    fileSystem->rootDir = (dirEntry *)&fileSystem->blocks[0];
    return fileSystem;
}

void createInodes(iNodeFileSystem *system) {
    for (int i = 0; i < system->iNodesCount; i++) {
        system->iNodes[i].isDirectory = 0;
        system->iNodes[i].size = 0;
        for (int j = 0; j < 5; j++) {
            system->iNodes[i].directBlocks[j] = -1;
        }
        system->iNodes[i].indirectBlock = -1;
        system->iNodes[i].doubleIndirectBlock = -1;
    }
}

iNodeFileSystem *loadFileSystem(char *name) {
    FILE *file = fopen(name, "rb");
    if (file == NULL) {
        return NULL;
    }
    iNodeFileSystem *fileSystem = malloc(sizeof(iNodeFileSystem));
    fread(fileSystem, 4, 8, file); //todo
    fileSystem->iNodes = malloc(fileSystem->iNodesCount * sizeof(iNode));
    fread(fileSystem->iNodes, sizeof(iNode), fileSystem->iNodesCount, file);
    fileSystem->blocks = malloc(fileSystem->blocksCount * fileSystem->blockSize);
    fread(fileSystem->blocks, fileSystem->blockSize, fileSystem->blocksCount, file);
    fileSystem->iNodesBitmap = malloc(fileSystem->iNodesCount);
    fread(fileSystem->iNodesBitmap, 1, fileSystem->iNodesCount, file);
    fileSystem->blocksBitmap = malloc(fileSystem->blocksCount);
    fread(fileSystem->blocksBitmap, 1, fileSystem->blocksCount, file);
    fileSystem->currentDir = fileSystem->rootDir;
    fclose(file);
    return fileSystem;
}

void createRootDir(iNodeFileSystem *system) {
    if (system == NULL) {
        return;
    }

    addDirectory(system, "/", 0);
}

int saveNewFileSystem(iNodeFileSystem *fileSystem, char *name) {
    FILE *file = fopen(name, "wb");
    if (file == NULL) {
        return 1;
    }
    fwrite(fileSystem, 4, 8, file); //todo
    fwrite(fileSystem->iNodes, sizeof(iNode), fileSystem->iNodesCount, file);
    fwrite(fileSystem->blocks, fileSystem->blockSize, fileSystem->blocksCount, file);
    fwrite(fileSystem->iNodesBitmap, 1, fileSystem->iNodesCount, file);
    fwrite(fileSystem->blocksBitmap, 1, fileSystem->blocksCount, file);
    fclose(file);
    return 0;
}


void freeFileSystem(iNodeFileSystem **fileSystem) {
    if (fileSystem == NULL || *fileSystem == NULL) {
        return;
    }
    free((*fileSystem)->iNodes);
    free((*fileSystem)->blocks);
    free((*fileSystem)->iNodesBitmap);
    free((*fileSystem)->blocksBitmap);
    free(*fileSystem);
    *fileSystem = NULL;
    fileSystem = NULL;
}


