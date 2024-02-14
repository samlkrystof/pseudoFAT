#include "bitmap_operations.h"
#include "iNodeFileSystem.h"
#include <string.h>

/**
 * @brief Operates on a bitmap.
 *
 * @param fileSystem The file system to operate on.
 * @param index The index of the block or inode.
 * @param type The type of operation (0 for inode, 1 for block).
 * @param value The value to set (0 for free, 1 for used).
 */
void operateOnBitmap(iNodeFileSystem *fileSystem, unsigned int index, int type, int value) {
    if (fileSystem == NULL || index >= fileSystem->blocksCount) {
        return;
    }
    setBitmapInt(fileSystem, index, type, value);
}

/**
 * @brief Marks a block as used.
 *
 * @param fileSystem The file system to operate on.
 * @param block The block to mark as used.
 * @return 0 on success, 1 on failure.
 */
int markBlockUsed(iNodeFileSystem *fileSystem, unsigned int block) {
    if (fileSystem == NULL || block >= fileSystem->blocksCount) {
        return 1;
    }

    setBitmapInt(fileSystem, block, 1, 1);
    fileSystem->freeBlocks--;

    return 0;
}

/**
 * @brief Marks a block as free.
 *
 * @param fileSystem The file system to operate on.
 * @param block The block to mark as free.
 * @return 0 on success, 1 on failure.
 */
int markBlockFree(iNodeFileSystem *fileSystem, unsigned int block) {
    if (fileSystem == NULL || block >= fileSystem->blocksCount) {
        return 1;
    }

    setBitmapInt(fileSystem, block, 1, 0);
    fileSystem->freeBlocks++;

    return 0;
}

/**
 * @brief Marks an inode as used.
 *
 * @param fileSystem The file system to operate on.
 * @param inode The inode to mark as used.
 * @return 0 on success, 1 on failure.
 */
int markINodeUsed(iNodeFileSystem *fileSystem, unsigned int inode) {
    if (fileSystem == NULL || inode >= fileSystem->iNodesCount) {
        return 1;
    }

    setBitmapInt(fileSystem, inode, 0, 1);
    fileSystem->freeINodes--;

    return 0;
}

/**
 * @brief Marks an inode as free.
 *
 * @param fileSystem The file system to operate on.
 * @param inode The inode to mark as free.
 * @return 0 on success, 1 on failure.
 */
int markINodeFree(iNodeFileSystem *fileSystem, unsigned int inode) {
    if (fileSystem == NULL || inode >= fileSystem->iNodesCount) {
        return 1;
    }

    setBitmapInt(fileSystem, inode, 0, 0);
    fileSystem->freeINodes++;

    return 0;
}

/**
 * @brief Gets the index of a free block.
 *
 * @param fileSystem The file system to operate on.
 * @return The index of a free block, or -1 if no free block is found.
 */
int getFreeBlock(iNodeFileSystem *fileSystem) {
    if (fileSystem == NULL) {
        return -1;
    }
    for (int i = 0; i < fileSystem->blocksCount; i++) {
        if (!getBitmapInt(fileSystem, i, 1)) {
            return i;
        }
    }
    return -1;
}

/**
 * @brief Gets the index of a free inode.
 *
 * @param fileSystem The file system to operate on.
 * @return The index of a free inode, or -1 if no free inode is found.
 */
int getFreeINode(iNodeFileSystem *fileSystem) {
    if (fileSystem == NULL) {
        return -1;
    }
    for (int i = 0; i < fileSystem->iNodesCount; i++) {
        if (!getBitmapInt(fileSystem, i, 0)) {
            return i;
        }
    }
    return -1;
}

/**
 * @brief Checks if a block is used.
 *
 * @param fileSystem The file system to operate on.
 * @param block The block to check.
 * @return 1 if the block is used, 0 otherwise.
 */
char isBlockUsed(iNodeFileSystem *fileSystem, unsigned int block) {
    if (fileSystem == NULL || block >= fileSystem->blocksCount) {
        return 1;
    }

    return getBitmapInt(fileSystem, block, 1);
}