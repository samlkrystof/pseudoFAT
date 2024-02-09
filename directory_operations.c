//
// Created by Kryst on 07.02.2024.
//

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "directory_operations.h"
#include "iNodeFileSystem.h"
#include "bitmap_operations.h"

char *getFileName(char *name) {
    if (name == NULL) {
        return NULL;
    }
    int i = strlen(name) - 1;
    for (; i >= 0; i--) {
        if (name[i] == '/') {
            break;
        }
    }
    if (i == -1) {
        return name;
    } else {
        return name + i + 1;
    }
}

char *getDirName(char *name) {
    if (name == NULL) {
        return NULL;
    }
    int i = strlen(name) - 1;
    for (; i >= 0; i--) {
        if (name[i] == '/') {
            break;
        }
    }
    if (i == 0) {
        return "/";
    } else if (i == -1) {
        return ".";
    } else {
        name[i] = '\0';
        return name;
    }
}

unsigned int findParentDir(iNodeFileSystem *fileSystem, char *name) {
    // return parent inode of file
    if (fileSystem == NULL || name == NULL) {
        return -1;
    }

    // use findDirectory to find parent directory, remove last token from name
    char *parentName = getDirName(name);
    unsigned int parentINode;
    if (parentName == NULL) {
        return -1;
    } else if (!strncmp(parentName, ".", 2)) {
        parentINode = fileSystem->currentDir->iNode;
        // if parentName is /, file is in root directory
    } else if (!strncmp(parentName, "/", 2)) {
        parentINode = 0;
    } else {
        parentINode = findDirectory(fileSystem, parentName);
    }

    return parentINode;
}

int findDirectory(iNodeFileSystem *fileSystem, char *name) {
    if (fileSystem == NULL || name == NULL) {
        return -1;
    }
    // check if name is absolute or relative
    unsigned int currentINode = fileSystem->currentDir->iNode;
    if (name[0] == '/') {
        currentINode = 0;
        name++;
    }
    char *token = strtok(name, "/");
    //todo indirect block
    while (token != NULL) {
        int found = 0;
        //todo fread
        for (int i = 0; i < fileSystem->iNodes[currentINode].size; i++) {
            unsigned int blockIndex = i * sizeof(dirEntry) / fileSystem->blockSize;
            unsigned int dirEntryIndex = i % (fileSystem->blockSize / sizeof(dirEntry));
            //todo fread
            unsigned int block = fileSystem->iNodes[currentINode].directBlocks[blockIndex];
            //todo fread
            char *blockPtr = &fileSystem->blocks[block * fileSystem->blockSize];
            dirEntry *entry = (dirEntry *) blockPtr;
            entry += dirEntryIndex;
            if (!strncmp(entry->name, token, 12)) {
                currentINode = entry->iNode;
                found = 1;
                break;
            }
        }
        if (!found) {
            return -1;
        }
        token = strtok(NULL, "/");
    }

    return currentINode;
}

int addFile(iNodeFileSystem *fileSystem, char *name, unsigned int size, char *content, unsigned int parent) {
    if (fileSystem == NULL || name == NULL || content == NULL) {
        return 1;
    }
    int inode = getFreeINode(fileSystem);
    if (inode == -1) {
        return 1;
    }
    unsigned int neededBlocks = size / fileSystem->blockSize;
    if (size % fileSystem->blockSize != 0) {
        neededBlocks++;
    }

    int blocks[neededBlocks];
    for (int i = 0; i < neededBlocks; i++) {
        blocks[i] = getFreeBlock(fileSystem);
        if (blocks[i] == -1) {
            return 1;
        }
        markBlockUsed(fileSystem, blocks[i]);
    }
    markINodeUsed(fileSystem, inode);
    //todo fread
    fileSystem->iNodes[inode].isDirectory = 0;
    fileSystem->iNodes[inode].size = size;
    for (int i = 0; i < neededBlocks; i++) {
        //todo fread
        fileSystem->iNodes[inode].directBlocks[i] = blocks[i];
    }
    addToParent(fileSystem, parent, inode, name);
    for (int i = 0; i < neededBlocks; i++) {
        //todo fread
        memcpy(&fileSystem->blocks[blocks[i] * fileSystem->blockSize], content + i * fileSystem->blockSize,
               fileSystem->blockSize);
    }
    //todo indirect block
    return 0;
}

int addDirectory(iNodeFileSystem *fileSystem, char *name, unsigned int parent) {
    if (fileSystem == NULL || name == NULL) {
        return 1;
    }
    int inode = getFreeINode(fileSystem);
    if (inode == -1) {
        return 1;
    }
    int block = getFreeBlock(fileSystem);

    if (block == -1) {
        return 1;
    }
    //erase block with memset
    //todo fread
    memset(&fileSystem->blocks[block * fileSystem->blockSize], 0, fileSystem->blockSize);

    markINodeUsed(fileSystem, inode);
    markBlockUsed(fileSystem, block);
    //todo fread
    fileSystem->iNodes[inode].isDirectory = 1;
    fileSystem->iNodes[inode].size = 2; // . and ..
    fileSystem->iNodes[inode].directBlocks[0] = block;
    addBaseEntries(fileSystem, inode, parent, block);

    // if name is not root, add entry to parent directory
    if (strncmp(name, "/", 2)) {
        addToParent(fileSystem, parent, inode, name);
    }
    return 0;
}

int addToParent(iNodeFileSystem *fileSystem, unsigned int parent, unsigned int inode, char *name) {
    if (fileSystem == NULL || name == NULL) {
        return 1;
    }
    //todo fread
    iNode *parentINode = &fileSystem->iNodes[parent];
    unsigned int blockIndex = parentINode->size * sizeof(dirEntry) / fileSystem->blockSize;
    unsigned int dirEntryIndex = parentINode->size % (fileSystem->blockSize / sizeof(dirEntry));
    // todo indirect block
    if (blockIndex >= 5) {
        return 1;
    }

    unsigned int block = parentINode->directBlocks[blockIndex];
    if (!isBlockUsed(fileSystem, block)) {
        block = getFreeBlock(fileSystem);
        if (block == -1) {
            return 1;
        }
        //todo fread
        memset(&fileSystem->blocks[block * fileSystem->blockSize], 0, fileSystem->blockSize);
        parentINode->directBlocks[blockIndex] = block;
        markBlockUsed(fileSystem, block);
    }
    //todo fread
    char *blockPtr = &fileSystem->blocks[block * fileSystem->blockSize];
    for (int i = 0; i <= dirEntryIndex; i++) {
        dirEntry *entry = ((dirEntry *) blockPtr) + i;
        if (entry->name[0] == '\0') {
            dirEntryIndex = i;
            break;
        }
    }

    dirEntry *entry = ((dirEntry *) blockPtr) + dirEntryIndex;
    memset(entry, 0, sizeof(dirEntry));
    entry->iNode = inode;
    strncpy(entry->name, name, 12);
    parentINode->size++;
    return 0;
}

int addBaseEntries(iNodeFileSystem *fileSystem, unsigned int inode, unsigned int parent, unsigned int block) {
    if (fileSystem == NULL) {
        return 1;
    }
    //todo fread
    char *blockPtr = &fileSystem->blocks[block * fileSystem->blockSize];
    memset(blockPtr, 0, fileSystem->blockSize);

    dirEntry *entry = (dirEntry *) blockPtr;
    entry->iNode = parent;
    strncpy(entry->name, "..", 12);
    entry++;
    entry->iNode = inode;
    strncpy(entry->name, ".", 12);
    return 0;
}

//int deleteEntry(iNodeFileSystem *fileSystem, char *name, int isDirectory) {
//    if (fileSystem == NULL || name == NULL) {
//        return 1;
//    }
//    char *fileName = getFileName(name);
//    int parentDir = findParentDir(fileSystem, name);
//    if (parentDir == -1) {
//        return 1;
//    }
//    int inode = findDirectory(fileSystem, name);
//    if (inode == -1) {
//        return 1;
//    }
//    if (isDirectory) {
//        if (fileSystem->iNodes[inode].size > 2) {
//            return 1;
//        }
//    }
//    operateOnDirEntry(fileSystem, parentDir, inode, fileName, 0);
//    if (isDirectory) {
//        for (int i = 0; i < fileSystem->iNodes[inode].size; i++) {
//            unsigned int block = fileSystem->iNodes[inode].directBlocks[i];
//            markBlockFree(fileSystem, block);
//        }
//        markINodeFree(fileSystem, inode);
//    }
//    return 0;
//}

int deleteDirectory(iNodeFileSystem *fileSystem, char *name) {
    if (fileSystem == NULL || name == NULL) {
        return 1;
    }
    // find directory
    int inode = findDirectory(fileSystem, name);
    if (inode == -1) {
        printf("FILE NOT FOUND\n");
        return 1;
    }
    // check if directory is empty
    //todo fread
    if (fileSystem->iNodes[inode].size > 2) {
        printf("NOT EMPTY\n");
        return 1;
    }
    // todo fread
    // get parent directory, the first entry in directory is always parent
    dirEntry *parent = (dirEntry *) &fileSystem->blocks[fileSystem->iNodes[inode].directBlocks[0] *
                                                        fileSystem->blockSize];
    unsigned int parentINode = parent->iNode;
    // remove current directory entry from parent
    //todo fread
    iNode *parentINodePtr = &fileSystem->iNodes[parentINode];
    for (int i = 0; i < parentINodePtr->size; i++) {
        unsigned int blockIndex = i * sizeof(dirEntry) / fileSystem->blockSize;
        unsigned int dirEntryIndex = i % (fileSystem->blockSize / sizeof(dirEntry));
        unsigned int block = parentINodePtr->directBlocks[blockIndex];
        //todo fread
        char *blockPtr = &fileSystem->blocks[block * fileSystem->blockSize];
        dirEntry *entry = (dirEntry *) blockPtr + dirEntryIndex;
        if (entry->iNode == inode) {
            // erase entry
            memset(entry, 0, sizeof(dirEntry));
            parentINodePtr->size--;
            break;
        }

    }

    // remove directory from data area
    //todo indirect block
    //todo fread
    for (int i = 0; i < fileSystem->iNodes[inode].size; i++) {
        unsigned int block = fileSystem->iNodes[inode].directBlocks[i];
        markBlockFree(fileSystem, block);
    }
    markINodeFree(fileSystem, inode);
    return 0;
}

int deleteFile(iNodeFileSystem *fileSystem, char *name) {
    if (fileSystem == NULL || name == NULL) {
        return 1;
    }

    char *fileName = getFileName(name);
    int parentDir = findParentDir(fileSystem, name);
    if (parentDir == -1) {
        return 1;
    }
    // todo indirect block
    //todo fread
    iNode *parentINode = &fileSystem->iNodes[parentDir];
    for (int i = 0; i < parentINode->size; i++) {
        unsigned int blockIndex = i * sizeof(dirEntry) / fileSystem->blockSize;
        unsigned int dirEntryIndex = i % (fileSystem->blockSize / sizeof(dirEntry));
        unsigned int parentBlock = parentINode->directBlocks[blockIndex];
        //todo fread
        char *blockPtr = &fileSystem->blocks[parentBlock * fileSystem->blockSize];
        dirEntry *entry = ((dirEntry *) blockPtr) + dirEntryIndex;
        if (!strncmp(entry->name, fileName, 12)) {
            unsigned int inode = entry->iNode;
            //todo fread
            unsigned int sizeBlocks = fileSystem->iNodes[inode].size / fileSystem->blockSize;
            if (fileSystem->iNodes[inode].size % fileSystem->blockSize != 0) {
                sizeBlocks++;
            }
            for (int j = 0; j < sizeBlocks; j++) {
                //todo fread
                unsigned int block = fileSystem->iNodes[inode].directBlocks[j];
                markBlockFree(fileSystem, block);
            }
            markINodeFree(fileSystem, inode);
            entry->iNode = 0;
            entry->name[0] = '\0';
            parentINode->size--;
            return 0;
        }
    }
    return 1;
}

dirEntry *getDirEntry(iNodeFileSystem * fileSystem, int inode, char *name) {
    if (fileSystem == NULL || name == NULL) {
        return NULL;
    }
    name = getFileName(name);
    //todo fread
    iNode *iNodePtr = &fileSystem->iNodes[inode];
    for (int i = 0; i < iNodePtr->size; i++) {
        unsigned int blockIndex = i * sizeof(dirEntry) / fileSystem->blockSize;
        unsigned int dirEntryIndex = i % (fileSystem->blockSize / sizeof(dirEntry));
        unsigned int block = iNodePtr->directBlocks[blockIndex];
        //todo fread
        char *blockPtr = &fileSystem->blocks[block * fileSystem->blockSize];
        dirEntry *entry = (dirEntry *) blockPtr + dirEntryIndex;
        if (!strncmp(entry->name, name, 12)) {
            return entry;
        }
    }
    return NULL;
}

int addDirEntry(iNodeFileSystem *fileSystem, int parent, int inode, char *name) {
    if (fileSystem == NULL || name == NULL) {
        return 1;
    }
    //todo fread
    iNode *parentINode = &fileSystem->iNodes[parent];
    unsigned int blockIndex = parentINode->size * sizeof(dirEntry) / fileSystem->blockSize;
    unsigned int dirEntryIndex = parentINode->size % (fileSystem->blockSize / sizeof(dirEntry));
    //todo indirect block
    if (blockIndex >= 5) {
        return 1;
    }

    unsigned int block = parentINode->directBlocks[blockIndex];
    if (!isBlockUsed(fileSystem, block)) {
        block = getFreeBlock(fileSystem);
        if (block == -1) {
            return 1;
        }
        //todo fread
        memset(&fileSystem->blocks[block * fileSystem->blockSize], 0, fileSystem->blockSize);
        parentINode->directBlocks[blockIndex] = block;
        markBlockUsed(fileSystem, block);
    }
    //todo fread
    char *blockPtr = &fileSystem->blocks[block * fileSystem->blockSize];
    for (int i = 0; i <= dirEntryIndex; i++) {
        dirEntry *entry = ((dirEntry *) blockPtr) + i;
        if (entry->name[0] == '\0') {
            dirEntryIndex = i;
            break;
        }
    }

    dirEntry *entry = ((dirEntry *) blockPtr) + dirEntryIndex;
    memset(entry, 0, sizeof(dirEntry));
    entry->iNode = inode;
    strncpy(entry->name, name, 12);
    parentINode->size++;
    return 0;
}

int *getAllBlocks(iNodeFileSystem *fileSystem, unsigned int inode, int *numBlocks) {
    if (fileSystem == NULL || numBlocks == NULL) {
        return NULL;
    }

    // compute worst case scenatio - all blocks are used
    int sizeBlocks = 5 * sizeof(int);
    if (fileSystem->iNodes[inode].indirectBlock != -1) {
        sizeBlocks += fileSystem->blockSize / sizeof(int);
    }
    if (fileSystem->iNodes[inode].doubleIndirectBlock != -1) {
        sizeBlocks += (fileSystem->blockSize / sizeof(int)) * (fileSystem->blockSize / sizeof(int));
    }

    int *blocks = calloc(sizeBlocks, sizeof(int));
    for (int i = 0; i < 5; i++) {
        if (fileSystem->iNodes[inode].directBlocks[i] != -1) {
            blocks[*numBlocks++] = fileSystem->iNodes[inode].directBlocks[i];
        }
    }
    if (fileSystem->iNodes[inode].indirectBlock != -1) {
        //todo fread
        int *indirectBlock = (int *) &fileSystem->blocks[fileSystem->iNodes[inode].indirectBlock * fileSystem->blockSize];
        for (int i = 0; i < fileSystem->blockSize / sizeof(int); i++) {
            if (indirectBlock[i] != -1) {
                blocks[*numBlocks++] = indirectBlock[i];
            }
        }
    }

    if (fileSystem->iNodes[inode].doubleIndirectBlock != -1) {
        //todo fread
        int *doubleIndirectBlock = (int *) &fileSystem->blocks[fileSystem->iNodes[inode].doubleIndirectBlock * fileSystem->blockSize];
        for (int i = 0; i < fileSystem->blockSize / sizeof(int); i++) {
            if (doubleIndirectBlock[i] != -1) {
                //todo fread
                int *indirectBlock = (int *) &fileSystem->blocks[doubleIndirectBlock[i] * fileSystem->blockSize];
                for (int j = 0; j < fileSystem->blockSize / sizeof(int); j++) {
                    if (indirectBlock[j] != -1) {
                        blocks[*numBlocks++] = indirectBlock[j];
                    }
                }
            }
        }
    }

    return blocks;
}
char *getFileContent(iNodeFileSystem *fileSystem, int inode) {
if (fileSystem == NULL) {
        return NULL;
    }
    int numBlocks = 0;
    int *blocks = getAllBlocks(fileSystem, inode, &numBlocks);
    char *content = calloc(numBlocks, fileSystem->blockSize);
    for (int i = 0; i < numBlocks; i++) {
        //todo fread
        memcpy(content + i * fileSystem->blockSize, &fileSystem->blocks[blocks[i] * fileSystem->blockSize], fileSystem->blockSize);
    }
    free(blocks);
    return content;

}