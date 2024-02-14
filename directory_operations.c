#include <string.h>
#include <stdlib.h>
#include "directory_operations.h"
#include "iNodeFileSystem.h"
#include "bitmap_operations.h"

/**
 * @brief Gets the name of a file from a path.
 * @param name
 * @return
 */
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

    char *nameCopy = malloc(strlen(name) + 1);

    if (i == -1) {
        strncpy(nameCopy, name, 12);
    } else {
        strncpy(nameCopy, name + i + 1, 12);
    }

    return nameCopy;
}

/**
 * @brief Gets the directory name from a path.
 * @param name
 * @return
 */
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

/**
 * @brief Finds the parent directory of a file.
 * @param fileSystem
 * @param name
 * @return
 */
unsigned int findParentDir(iNodeFileSystem *fileSystem, char *name) {
    if (fileSystem == NULL || name == NULL) {
        return -1;
    }

    char *parentName = getDirName(name);
    unsigned int parentINode;
    if (parentName == NULL) {
        return -1;
    } else if (!strncmp(parentName, ".", 2)) {

        if (!strncmp(name, fileSystem->currentDir->name, 12)) {
            parentINode = 0;
        } else {
            parentINode = fileSystem->currentDir->iNode;
        }

        // if parentName is /, file is in root directory
    } else if (!strncmp(parentName, "/", 2)) {
        parentINode = 0;
    } else {
        parentINode = findDirectory(fileSystem, parentName);
    }

    return parentINode;

}

/**
 * @brief Finds a directory in the file system.
 * @param fileSystem
 * @param name
 * @return
 */
int findDirectory(iNodeFileSystem *fileSystem, char *name) {
    if (fileSystem == NULL || name == NULL) {
        return -1;
    }

    char *nameCopy = malloc(strlen(name) + 1);
    strcpy(nameCopy, name);
    unsigned int currentINode = fileSystem->currentDir->iNode;

    if (nameCopy[0] == '/') {
        currentINode = 0;
        nameCopy++;
    }

    char *token = strtok(nameCopy, "/");
    while (token != NULL) {
        int found = 0;
        iNode *iNodePtr = loadINode(fileSystem, currentINode);
        int elements = iNodePtr->size;

        for (int i = 0; i < elements; i++) {

            unsigned int blockIndex = i * sizeof(dirEntry) / fileSystem->blockSize;
            unsigned int dirEntryIndex = i % (fileSystem->blockSize / sizeof(dirEntry));
            int numUsedBlocks = 0;

            int *blocks = getAllBlocks(fileSystem, currentINode, &numUsedBlocks);
            unsigned int block = blocks[blockIndex];
            free(blocks);
            char *blockPtr = loadBlock(fileSystem, block);
            dirEntry *entry = (dirEntry *) blockPtr;
            entry += dirEntryIndex;

            if (entry->name[0] == '\0') {
                free(blockPtr);
                elements++;
                continue;
            }

            if (!strncmp(entry->name, token, 12)) {
                currentINode = entry->iNode;
                found = 1;
                break;
            }

            saveBlock(fileSystem, blockPtr, block);
        }

        if (!found) {
            return -1;
        }

        token = strtok(NULL, "/");
    }

    return currentINode;
}

/**
 * @brief Adds a file to the file system.
 * @param fileSystem
 * @param name
 * @param size
 * @param content
 * @param parent
 * @return
 */
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
    iNode *inodePtr = loadINode(fileSystem, inode);
    inodePtr->isDirectory = 0;
    inodePtr->size = size;

    for (int i = 0; i < neededBlocks; i++) {
        addBlockToINode(fileSystem, inodePtr, blocks[i]);
    }

    saveName(fileSystem, name, inode);
    addToParent(fileSystem, parent, inode, name);
    for (int i = 0; i < neededBlocks; i++) {

        unsigned int toCopy = size > fileSystem->blockSize ? fileSystem->blockSize : size;
        char *block = loadBlock(fileSystem, blocks[i]);
        memcpy(block, content + i * fileSystem->blockSize, toCopy);
        saveBlock(fileSystem, block, blocks[i]);
        size -= toCopy;
    }

    saveINode(fileSystem, inodePtr, inode);
    return 0;
}

/**
 * @brief Adds a directory to the file system.
 * @param fileSystem
 * @param name
 * @param parent
 * @return
 */
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

    char *blockPtr = loadBlock(fileSystem, block);
    memset(blockPtr, 0, fileSystem->blockSize);
    saveBlock(fileSystem, blockPtr, block);

    markINodeUsed(fileSystem, inode);
    markBlockUsed(fileSystem, block);
    iNode *inodePtr = loadINode(fileSystem, inode);
    inodePtr->isDirectory = 1;
    inodePtr->size = 2;
    inodePtr->directBlocks[0] = block;

    saveINode(fileSystem, inodePtr, inode);
    saveName(fileSystem, name, inode);
    addBaseEntries(fileSystem, inode, parent, block);

    // if name is not root, add entry to parent directory
    if (strncmp(name, "/", 2)) {
        addToParent(fileSystem, parent, inode, name);
    }

    return 0;
}

/**
 * @brief Adds a file to a directory.
 * @param fileSystem
 * @param parent
 * @param inode
 * @param name
 * @return
 */
int addToParent(iNodeFileSystem *fileSystem, unsigned int parent, unsigned int inode, char *name) {
    if (fileSystem == NULL || name == NULL) {
        return 1;
    }

    iNode *parentINode = loadINode(fileSystem, parent);
    unsigned int blockIndex = parentINode->size * sizeof(dirEntry) / fileSystem->blockSize;
    unsigned int dirEntryIndex = parentINode->size % (fileSystem->blockSize / sizeof(dirEntry));
    int numUsedBlocks = 0;
    int *blocks = getAllBlocks(fileSystem, parent, &numUsedBlocks);
    int block;

    if (blockIndex >= numUsedBlocks) {
        block = getFreeBlock(fileSystem);
        if (block == -1) {
            return 1;
        }

        char *blockPtr = loadBlock(fileSystem, block);
        memset(blockPtr, 0, fileSystem->blockSize);
        markBlockUsed(fileSystem, block);
        addBlockToINode(fileSystem, parentINode, block);
        free(blockPtr);
    } else {
        block = blocks[blockIndex];
    }
    free(blocks);

    char *blockPtr = loadBlock(fileSystem, block);

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
    memcpy((dirEntry *) blockPtr + dirEntryIndex, entry, sizeof(dirEntry));
    saveINode(fileSystem, parentINode, parent);
    saveBlock(fileSystem, blockPtr, block);

    return 0;
}

/**
 * @brief Adds base entries to a directory.
 * @param fileSystem
 * @param inode
 * @param parent
 * @param block
 * @return
 */
int addBaseEntries(iNodeFileSystem *fileSystem, unsigned int inode, unsigned int parent, unsigned int block) {
    if (fileSystem == NULL) {
        return 1;
    }

    char *blockPtr = loadBlock(fileSystem, block);
    memset(blockPtr, 0, fileSystem->blockSize);

    dirEntry *entry = (dirEntry *) blockPtr;
    entry->iNode = parent;
    strncpy(entry->name, "..", 12);
    entry++;
    entry->iNode = inode;
    strncpy(entry->name, ".", 12);
    saveBlock(fileSystem, blockPtr, block);
    return 0;
}

/**
 * @brief Deletes a directory from the file system.
 * @param fileSystem
 * @param name
 * @return
 */
int deleteDirectory(iNodeFileSystem *fileSystem, char *name) {
    if (fileSystem == NULL || name == NULL) {
        return 1;
    }

    int inode = findDirectory(fileSystem, name);
    if (inode == -1) {
        return 1;
    }

    iNode *iNodePtr = loadINode(fileSystem, inode);
    if (iNodePtr->size > 2) {
        return 2;
    }

    int numUsedBlocks = 0;
    int *blocks = getAllBlocks(fileSystem, inode, &numUsedBlocks);
    dirEntry *parent = (dirEntry *) loadBlock(fileSystem, blocks[0]);
    free(blocks);

    unsigned int parentINode = parent->iNode;
    iNode *parentINodePtr = loadINode(fileSystem, parentINode);

    for (int i = 0; i < parentINodePtr->size; i++) {
        unsigned int blockIndex = i * sizeof(dirEntry) / fileSystem->blockSize;
        unsigned int dirEntryIndex = i % (fileSystem->blockSize / sizeof(dirEntry));

        numUsedBlocks = 0;
        blocks = getAllBlocks(fileSystem, parentINode, &numUsedBlocks);
        unsigned int block = blocks[blockIndex];
        free(blocks);
        char *blockPtr = loadBlock(fileSystem, block);

        dirEntry *entry = (dirEntry *) blockPtr + dirEntryIndex;
        if (entry->iNode == inode) {
            memset(entry, 0, sizeof(dirEntry));
            parentINodePtr->size--;
            saveINode(fileSystem, parentINodePtr, parentINode);
            saveBlock(fileSystem, blockPtr, block);
            break;
        }

        free(blockPtr);
    }
    free(parent);

    iNodePtr = loadINode(fileSystem, inode);
    numUsedBlocks = 0;
    blocks = getAllBlocks(fileSystem, inode, &numUsedBlocks);

    for (int i = 0; i < iNodePtr->size; i++) {
        unsigned int block = blocks[i];
        markBlockFree(fileSystem, block);
    }

    free(blocks);
    free(iNodePtr);
    markINodeFree(fileSystem, inode);
    return 0;
}

/**
 * @brief Deletes a file from the file system.
 * @param fileSystem
 * @param name
 * @return
 */
int deleteFile(iNodeFileSystem *fileSystem, char *name) {
    if (fileSystem == NULL || name == NULL) {
        return 1;
    }

    char *fileName = getFileName(name);
    unsigned int parentDir = findParentDir(fileSystem, name);
    if (parentDir == -1) {
        return 1;
    }

    iNode *parentINode = loadINode(fileSystem, parentDir);

    for (int i = 0; i < parentINode->size; i++) {
        unsigned int blockIndex = i * sizeof(dirEntry) / fileSystem->blockSize;
        unsigned int dirEntryIndex = i % (fileSystem->blockSize / sizeof(dirEntry));

        int numUsedBlocks = 0;
        int *blocks = getAllBlocks(fileSystem, parentDir, &numUsedBlocks);
        unsigned int parentBlock = blocks[blockIndex];
        free(blocks);
        char *blockPtr = loadBlock(fileSystem, parentBlock);
        dirEntry *entry = ((dirEntry *) blockPtr) + dirEntryIndex;

        if (!strncmp(entry->name, fileName, 12)) {
            iNode *inodePtr = loadINode(fileSystem, entry->iNode);
            unsigned int sizeBlocks = inodePtr->size / fileSystem->blockSize;

            if (inodePtr->size % fileSystem->blockSize != 0) {
                sizeBlocks++;
            }

            for (int j = 0; j < sizeBlocks; j++) {
                numUsedBlocks = 0;
                blocks = getAllBlocks(fileSystem, entry->iNode, &numUsedBlocks);
                unsigned int block = blocks[j];
                free(blocks);
                markBlockFree(fileSystem, block);
            }

            markINodeFree(fileSystem, entry->iNode);
            entry->iNode = 0;
            entry->name[0] = '\0';
            parentINode->size--;

            saveINode(fileSystem, parentINode, parentDir);
            saveBlock(fileSystem, blockPtr, parentBlock);
            free(inodePtr);

            return 0;
        }
        free(blockPtr);
    }
    free(parentINode);

    return 1;
}

/**
 * @brief Gets a directory entry from the file system.
 * @param fileSystem
 * @param inode
 * @param name
 * @return
 */
dirEntry *getDirEntry(iNodeFileSystem *fileSystem, unsigned int inode, char *name) {
    if (fileSystem == NULL || name == NULL) {
        return NULL;
    }

    name = getFileName(name);
    iNode *iNodePtr = loadINode(fileSystem, inode);
    int numEntries = iNodePtr->size;

    for (int i = 0; i < numEntries; i++) {
        unsigned int blockIndex = i * sizeof(dirEntry) / fileSystem->blockSize;
        unsigned int dirEntryIndex = i % (fileSystem->blockSize / sizeof(dirEntry));
        int numUsedBlocks = 0;
        int *blocks = getAllBlocks(fileSystem, inode, &numUsedBlocks);
        unsigned int block = blocks[blockIndex];

        char *blockPtr = loadBlock(fileSystem, block);
        dirEntry *entry = (dirEntry *) blockPtr + dirEntryIndex;
        if (entry->name[0] == '\0') {
            free(blockPtr);
            free(blocks);
            numEntries++;
            continue;
        }

        if (!strncmp(entry->name, name, 12)) {
            dirEntry *copy = malloc(sizeof(dirEntry));
            memcpy(copy, entry, sizeof(dirEntry));
            free(iNodePtr);
            free(blockPtr);
            free(blocks);
            return copy;
        }

        free(blockPtr);
        free(blocks);
    }
    free(iNodePtr);
    return NULL;
}

/**
 * @brief Adds a directory entry to the file system.
 * @param fileSystem
 * @param parent
 * @param inode
 * @param name
 * @return
 */
int addDirEntry(iNodeFileSystem *fileSystem, unsigned int parent, unsigned int inode, char *name) {
    if (fileSystem == NULL || name == NULL) {
        return 1;
    }

    iNode *parentINode = loadINode(fileSystem, parent);
    unsigned int blockIndex = parentINode->size * sizeof(dirEntry) / fileSystem->blockSize;
    unsigned int dirEntryIndex = parentINode->size % (fileSystem->blockSize / sizeof(dirEntry));
    int numUsedBlocks = 0;
    int *blocks = getAllBlocks(fileSystem, parent, &numUsedBlocks);
    int block;

    if (blockIndex >= numUsedBlocks) {
        block = getFreeBlock(fileSystem);
        if (block == -1) {
            return 1;
        }

        char *blockPtr = loadBlock(fileSystem, block);
        memset(blockPtr, 0, fileSystem->blockSize);
        markBlockUsed(fileSystem, block);
        addBlockToINode(fileSystem, parentINode, block);
        free(blockPtr);
    } else {
        block = blocks[blockIndex];
    }

    free(blocks);
    char *blockPtr = loadBlock(fileSystem, block);

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

    saveINode(fileSystem, parentINode, parent);
    saveBlock(fileSystem, blockPtr, block);

    return 0;
}

/**
 * @brief Deletes a directory entry from the file system.
 * @param fileSystem
 * @param parent
 * @param name
 * @return
 */
int *getAllBlocks(iNodeFileSystem *fileSystem, unsigned int index, int *numBlocks) {
    if (fileSystem == NULL || numBlocks == NULL) {
        return NULL;
    }

    iNode *iNodePtr = loadINode(fileSystem, index);
    int sizeBlocks = 5;

    if (iNodePtr->indirectBlock != -1) {
        sizeBlocks += fileSystem->blockSize / sizeof(int);
    }

    if (iNodePtr->doubleIndirectBlock != -1) {
        sizeBlocks += (fileSystem->blockSize / sizeof(int)) * (fileSystem->blockSize / sizeof(int));
    }

    int *blocks = calloc(sizeBlocks, sizeof(int));
    for (int i = 0; i < 5; i++) {

        if (iNodePtr->directBlocks[i] != -1) {
            blocks[(*numBlocks)++] = iNodePtr->directBlocks[i];
        }
    }

    if (iNodePtr->indirectBlock != -1) {
        int *indirectBlock = (int *) loadBlock(fileSystem, iNodePtr->indirectBlock);

        for (int i = 0; i < fileSystem->blockSize / sizeof(int); i++) {

            if (indirectBlock[i] != -1) {
                blocks[(*numBlocks)++] = indirectBlock[i];
            }
        }
        free(indirectBlock);
    }

    if (iNodePtr->doubleIndirectBlock != -1) {
        int *doubleIndirectBlock = (int *) loadBlock(fileSystem, iNodePtr->doubleIndirectBlock);

        for (int i = 0; i < fileSystem->blockSize / sizeof(int); i++) {

            if (doubleIndirectBlock[i] != -1) {
                int *indirectBlock = (int *) loadBlock(fileSystem, doubleIndirectBlock[i]);

                for (int j = 0; j < fileSystem->blockSize / sizeof(int); j++) {

                    if (indirectBlock[j] != -1) {
                        blocks[(*numBlocks)++] = indirectBlock[j];
                    }
                }
                free(indirectBlock);
            }
        }
        free(doubleIndirectBlock);
    }

    free(iNodePtr);
    return blocks;
}

/**
 * @brief Gets the content of a file from the file system.
 * @param fileSystem
 * @param inode
 * @return
 */
char *getFileContent(iNodeFileSystem *fileSystem, int inode) {
    if (fileSystem == NULL) {
        return NULL;
    }

    int numBlocks = 0;
    iNode *iNodePtr = loadINode(fileSystem, inode);
    unsigned int fileSize = iNodePtr->size;

    int *blocks = getAllBlocks(fileSystem, inode, &numBlocks);
    char *content = calloc(1, fileSize);
    free(iNodePtr);

    for (int i = 0; i < numBlocks; i++) {
        unsigned int toCopy = fileSize > fileSystem->blockSize ? fileSystem->blockSize : fileSize;
        char *block = loadBlock(fileSystem, blocks[i]);
        memcpy(content + i * fileSystem->blockSize, block, toCopy);
        fileSize -= toCopy;
        free(block);
    }

    free(blocks);
    return content;

}

/**
 * @brief Saves a directory entry to the file system.
 * @param fileSystem
 * @param inode
 * @param entry
 * @return
 */
int saveDirEntry(iNodeFileSystem *fileSystem, unsigned int inode, dirEntry *entry) {
    if (fileSystem == NULL || entry == NULL) {
        return 1;
    }

    iNode *iNodePtr = loadINode(fileSystem, inode);
    int numUsedBlocks = 0;
    int *blocks = getAllBlocks(fileSystem, inode, &numUsedBlocks);

    for (int i = 0; i < iNodePtr->size; i++) {
        unsigned int blockIndex = i * sizeof(dirEntry) / fileSystem->blockSize;
        unsigned int dirEntryIndex = i % (fileSystem->blockSize / sizeof(dirEntry));
        unsigned int block = blocks[blockIndex];

        char *blockPtr = loadBlock(fileSystem, block);
        dirEntry *entryPtr = (dirEntry *) blockPtr + dirEntryIndex;

        if (!strncmp(entryPtr->name, entry->name, 12)) {
            memcpy(entryPtr, entry, sizeof(dirEntry));
            saveBlock(fileSystem, blockPtr, block);
            free(blockPtr);
            free(blocks);
            free(iNodePtr);
            return 0;
        }
        free(blockPtr);
    }

    free(blocks);
    free(iNodePtr);
    return 1;
}

/**
 * @brief Adds a block to an inode.
 * @param fileSystem
 * @param parent
 * @param block
 * @return
 */
int addBlockToINode(iNodeFileSystem *fileSystem, iNode *parent, int block) {
    if (block >= fileSystem->blocksCount) {
        return 1;
    }

    int numPtrInBlock = fileSystem->blockSize / 4;
    for (int i = 0; i < 5; i++) {

        if (parent->directBlocks[i] == -1) {
            parent->directBlocks[i] = block;
            return 0;
        }
    }

    if (parent->indirectBlock == -1) {
        int indirectBlock = getFreeBlock(fileSystem);
        if (indirectBlock == -1) {
            return 1;
        }

        parent->indirectBlock = indirectBlock;
        markBlockUsed(fileSystem, indirectBlock);
        char *blockPtr = loadBlock(fileSystem, parent->indirectBlock);
        int *ptr = (int *) blockPtr;
        for (int i = 0; i < numPtrInBlock; i++) {
            ptr[i] = -1;
        }

        saveBlock(fileSystem, blockPtr, parent->indirectBlock);
    }
    char *indirectBlock = loadBlock(fileSystem, parent->indirectBlock);
    int *ptr = (int *) indirectBlock;

    for (int i = 0; i < numPtrInBlock; i++) {

        if (ptr[i] == -1) {
            ptr[i] = block;
            saveBlock(fileSystem, indirectBlock, parent->indirectBlock);
            return 0;
        }
    }
    free(indirectBlock);

    if (parent->doubleIndirectBlock == -1) {
        int doubleIndirectBlock = getFreeBlock(fileSystem);
        if (doubleIndirectBlock == -1) {
            return 1;
        }

        parent->doubleIndirectBlock = doubleIndirectBlock;
        markBlockUsed(fileSystem, doubleIndirectBlock);
        char *blockPtr = loadBlock(fileSystem, parent->doubleIndirectBlock);
        int *ptr = (int *) blockPtr;

        for (int i = 0; i < numPtrInBlock; i++) {
            ptr[i] = -1;
        }
        saveBlock(fileSystem, blockPtr, parent->doubleIndirectBlock);

    }
    char *doubleIndirectBlock = loadBlock(fileSystem, parent->doubleIndirectBlock);
    ptr = (int *) doubleIndirectBlock;

    for (int i = 0; i < numPtrInBlock; i++) {

        if (ptr[i] == -1) {
            int indirectBlock = getFreeBlock(fileSystem);

            if (indirectBlock == -1) {
                return 1;
            }
            ptr[i] = indirectBlock;
            markBlockUsed(fileSystem, indirectBlock);
            char *blockPtr = loadBlock(fileSystem, ptr[i]);
            int *ptr2 = (int *) blockPtr;

            for (int j = 0; j < numPtrInBlock; j++) {
                ptr2[j] = -1;
            }
            saveBlock(fileSystem, blockPtr, ptr[i]);
        }

        char *indirectBlock = loadBlock(fileSystem, ptr[i]);
        int *ptr2 = (int *) indirectBlock;

        for (int j = 0; j < numPtrInBlock; j++) {
            if (ptr2[j] == -1) {
                ptr2[j] = block;
                saveBlock(fileSystem, indirectBlock, ptr[i]);
                saveBlock(fileSystem, doubleIndirectBlock, parent->doubleIndirectBlock);
                return 0;
            }
        }
        free(indirectBlock);
    }
    free(doubleIndirectBlock);
    return 0;
}

