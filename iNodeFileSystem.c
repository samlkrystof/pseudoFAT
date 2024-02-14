#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "iNodeFileSystem.h"
#include "directory_operations.h"

/**
 * @brief Creates a new file system.
 *
 * @param totalSize The total size of the file system.
 * @param name The name of the file system.
 * @return The created file system.
 */
iNodeFileSystem *createFileSystem(unsigned int totalSize, char *name) {
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
    fileSystem->fileNames = malloc(fileSystem->iNodesCount * sizeof(char *));

    for (int i = 0; i < fileSystem->iNodesCount; i++) {
        fileSystem->fileNames[i] = malloc(12);
    }

    saveNewFileSystem(fileSystem, name);
    freeFileSystem(&fileSystem);
    fileSystem = loadFileSystem(name);
    createRootDir(fileSystem);

    fileSystem->currentDir = (dirEntry *)loadBlock(fileSystem, 0);
    fileSystem->rootDir = (dirEntry *)loadBlock(fileSystem, 0);
    fileSystem->name = name;
    return fileSystem;
}

/**
 * @brief Creates inodes for the file system.
 *
 * @param system The file system to create inodes for.
 */
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
    FILE *file = fopen(name, "rb+");
    if (file == NULL) {
        return NULL;
    }

    iNodeFileSystem *fileSystem = malloc(sizeof(iNodeFileSystem));
    fread(fileSystem, 4, 6, file);
    fileSystem->file = file;
    iNode *root = loadINode(fileSystem, 0);
    fileSystem->rootDir = (dirEntry *)loadBlock(fileSystem, root->directBlocks[0]);
    fileSystem->currentDir = fileSystem->rootDir;
    free(root);
    return fileSystem;
}

/**
 * @brief Creates the root directory.
 *
 * @param system The file system to create the root directory for.
 */
void createRootDir(iNodeFileSystem *system) {
    if (system == NULL) {
        return;
    }

    addDirectory(system, "/", 0);
}

/**
 * @brief Saves a new file system to a file.
 *
 * @param fileSystem The file system to save.
 * @param name The name of the file to save to.
 * @return 0 if the file system was saved successfully, 1 otherwise.
 */
int saveNewFileSystem(iNodeFileSystem *fileSystem, char *name) {
    FILE *file = fopen(name, "wb");
    if (file == NULL) {
        return 1;
    }

    fwrite(fileSystem, 4, 6, file);
    fwrite(fileSystem->iNodes, sizeof(iNode), fileSystem->iNodesCount, file);
    fwrite(fileSystem->blocks, fileSystem->blockSize, fileSystem->blocksCount, file);
    fwrite(fileSystem->iNodesBitmap, 1, fileSystem->iNodesCount, file);
    fwrite(fileSystem->blocksBitmap, 1, fileSystem->blocksCount, file);
    fwrite(fileSystem->fileNames, 12, fileSystem->iNodesCount, file);

    fclose(file);
    return 0;
}

/**
 * @brief Frees a file system.
 *
 * @param fileSystem The file system to free.
 */
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

/**
 * @brief Loads an inode from a file system.
 *
 * @param fileSystem The file system to load the inode from.
 * @param index The index of the inode to load.
 * @return The loaded inode.
 */
iNode *loadINode(iNodeFileSystem *fileSystem, unsigned int index) {
    if (fileSystem == NULL || index >= fileSystem->iNodesCount) {
        return NULL;
    }

    unsigned int iNodeOffset = 24 + index * sizeof(iNode);
    iNode *inode = malloc(sizeof(iNode));

    fseek(fileSystem->file, iNodeOffset, SEEK_SET);
    fread(inode, sizeof(iNode), 1, fileSystem->file);
    return inode;
}

/**
 * @brief Saves an inode to a file system.
 *
 * @param fileSystem The file system to save the inode to.
 * @param inode The inode to save.
 * @param index The index of the inode to save.
 * @return 0 if the inode was saved successfully, 1 otherwise.
 */
int saveINode(iNodeFileSystem *fileSystem, iNode *inode, unsigned int index) {
    if (fileSystem == NULL || inode == NULL || index >= fileSystem->iNodesCount) {
        return 1;
    }

    unsigned int iNodeOffset = 24 + index * sizeof(iNode);
    fseek(fileSystem->file, iNodeOffset, SEEK_SET);
    fwrite(inode, sizeof(iNode), 1, fileSystem->file);
    free(inode);
    return 0;
}

/**
 * @brief Loads a block from a file system.
 *
 * @param fileSystem The file system to load the block from.
 * @param index The index of the block to load.
 * @return The loaded block.
 */
char *loadBlock(iNodeFileSystem *fileSystem, unsigned int index) {
    if (fileSystem == NULL || index >= fileSystem->blocksCount) {
        return NULL;
    }

    unsigned int blockOffset = 24 + fileSystem->iNodesCount * sizeof(iNode) + index * fileSystem->blockSize;
    char *block = malloc(fileSystem->blockSize);
    fseek(fileSystem->file, blockOffset, SEEK_SET);
    fread(block, fileSystem->blockSize, 1, fileSystem->file);
    return block;
}

/**
 * @brief Saves a block to a file system.
 *
 * @param fileSystem The file system to save the block to.
 * @param block The block to save.
 * @param index The index of the block to save.
 * @return 0 if the block was saved successfully, 1 otherwise.
 */
int saveBlock(iNodeFileSystem *fileSystem, char *block, unsigned int index) {
    if (fileSystem == NULL || block == NULL || index >= fileSystem->blocksCount) {
        return 1;
    }

    unsigned int blockOffset = 24 + fileSystem->iNodesCount * sizeof(iNode) + index * fileSystem->blockSize;
    fseek(fileSystem->file, blockOffset, SEEK_SET);
    fwrite(block, fileSystem->blockSize, 1, fileSystem->file);
    free(block);
    return 0;
}

/**
 * @brief Gets a bit from a bitmap.
 *
 * @param fileSystem The file system to get the bit from.
 * @param index The index of the bit to get.
 * @param type The type of the bitmap to get the bit from.
 * @return The value of the bit.
 */
int getBitmapInt(iNodeFileSystem *fileSystem, unsigned int index, int type) {
    //type 0 - iNodes, 1 - blocks
    if (fileSystem == NULL || index >= fileSystem->blocksCount) {
        return -1;
    }

    unsigned int byte = index / 8;
    unsigned int bit = index % 8;
    int result;
    int offset = 24 + fileSystem->iNodesCount * sizeof(iNode) + fileSystem->blocksCount * fileSystem->blockSize;

    if (type == 0) {
        offset += byte;

    } else {
        offset += fileSystem->blocksCount / 8 + (8 - fileSystem->blocksCount % 8) + byte;
    }

    fseek(fileSystem->file, offset, SEEK_SET);
    fread(&result, 1, 1, fileSystem->file);
return (result >> bit) & 1;
}

/**
 * @brief Sets a bit in a bitmap.
 *
 * @param fileSystem The file system to set the bit in.
 * @param index The index of the bit to set.
 * @param type The type of the bitmap to set the bit in.
 * @param value The value to set the bit to.
 * @return 0 if the bit was set successfully, 1 otherwise.
 */
int setBitmapInt(iNodeFileSystem *fileSystem, unsigned int index, int type, int value) {
    //type 0 - iNodes, 1 - blocks
    if (fileSystem == NULL || index >= fileSystem->blocksCount) {
        return 1;
    }

    unsigned int byte = index / 8;
    unsigned int bit = index % 8;

    int offset = 24 + fileSystem->iNodesCount * sizeof(iNode) + fileSystem->blocksCount * fileSystem->blockSize;
    if (type == 0) {
        offset += byte;
    } else {
        offset += fileSystem->blocksCount / 8 + (8 - fileSystem->blocksCount % 8) + byte;
    }
    fseek(fileSystem->file, offset, SEEK_SET);
    int result;
    fread(&result, 1, 1, fileSystem->file);
    if (value == 1) {
        result |= 1 << bit;
    } else {
        result &= ~(1 << bit);
    }

    fseek(fileSystem->file, offset, SEEK_SET);
    fwrite(&result, 1, 1, fileSystem->file);
    return 0;
}

/**
 * @brief Loads a name from a file system.
 *
 * @param fileSystem The file system to load the name from.
 * @param index The index of the name to load.
 * @return The loaded name.
 */
char *loadName(iNodeFileSystem *fileSystem, unsigned int index) {
    if (fileSystem == NULL || index >= fileSystem->iNodesCount) {
        return NULL;
    }

    char *name = malloc(12);
    int offset = 24 + fileSystem->iNodesCount * sizeof(iNode) + fileSystem->blocksCount * fileSystem->blockSize + fileSystem->iNodesCount / 8 + (8 - fileSystem->iNodesCount % 8) + fileSystem->blocksCount / 8 + (8 - fileSystem->blocksCount % 8) + index * 12;
    fseek(fileSystem->file, offset, SEEK_SET);
    fread(name, 12, 1, fileSystem->file);

    return name;
}

/**
 * @brief Saves a name to a file system.
 *
 * @param fileSystem The file system to save the name to.
 * @param name The name to save.
 * @param index The index of the name to save.
 * @return 0 if the name was saved successfully, 1 otherwise.
 */
int saveName(iNodeFileSystem *fileSystem, char *name, unsigned int index) {
    if (fileSystem == NULL || name == NULL || index >= fileSystem->iNodesCount) {
        return 1;
    }

    int offset = 24 + fileSystem->iNodesCount * sizeof(iNode) + fileSystem->blocksCount * fileSystem->blockSize + fileSystem->iNodesCount / 8 + (8 - fileSystem->iNodesCount % 8) + fileSystem->blocksCount / 8 + (8 - fileSystem->blocksCount % 8) + index * 12;
    fseek(fileSystem->file, offset, SEEK_SET);
    fwrite(name, 12, 1, fileSystem->file);
    return 0;
}



