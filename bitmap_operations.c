#include "bitmap_operations.h"
#include "iNodeFileSystem.h"
#include <string.h>

void operateOnBitmap(char *bitmap, unsigned int index, int operation) {
    if (bitmap == NULL) {
        return;
    }
    unsigned int byte = index / 8;
    unsigned int bit = index % 8;
    if (operation == 1) {
        bitmap[byte] |= 1 << bit;
    } else {
        bitmap[byte] &= ~(1 << bit);
    }
}

int markBlockUsed(iNodeFileSystem *fileSystem, unsigned int block) {
    if (fileSystem == NULL || block >= fileSystem->blocksCount) {
        return 1;
    }

    operateOnBitmap(fileSystem->blocksBitmap, block, 1);
    fileSystem->freeBlocks--;

    return 0;
}

int markBlockFree(iNodeFileSystem *fileSystem, unsigned int block) {
    if (fileSystem == NULL || block >= fileSystem->blocksCount) {
        return 1;
    }

    operateOnBitmap(fileSystem->blocksBitmap, block, 0);
    fileSystem->freeBlocks++;

    return 0;
}

int markINodeUsed(iNodeFileSystem *fileSystem, unsigned int inode) {
    if (fileSystem == NULL || inode >= fileSystem->iNodesCount) {
        return 1;
    }

    operateOnBitmap(fileSystem->iNodesBitmap, inode, 1);
    fileSystem->freeINodes--;

    return 0;
}

int markINodeFree(iNodeFileSystem *fileSystem, unsigned int inode) {
    if (fileSystem == NULL || inode >= fileSystem->iNodesCount) {
        return 1;
    }

    operateOnBitmap(fileSystem->iNodesBitmap, inode, 0);
    fileSystem->freeINodes++;

    return 0;
}

int getFreeBlock(iNodeFileSystem *fileSystem) {
    if (fileSystem == NULL) {
        return -1;
    }
    for (int i = 0; i < fileSystem->blocksCount; i++) {
        if (!(fileSystem->blocksBitmap[i / 8] & 1 << i % 8)) {
            return i;
        }
    }
    return -1;
}

int getFreeINode(iNodeFileSystem *fileSystem) {
    if (fileSystem == NULL) {
        return -1;
    }
    for (int i = 0; i < fileSystem->iNodesCount; i++) {
        if (!(fileSystem->iNodesBitmap[i / 8] & 1 << i % 8)) {
            return i;
        }
    }
    return -1;
}

char isBlockUsed(iNodeFileSystem *fileSystem, unsigned int block) {
    if (fileSystem == NULL || block >= fileSystem->blocksCount) {
        return 1;
    }

    return fileSystem->blocksBitmap[block / 8] & 1 << block % 8;
}

