//
// Created by Kryst on 07.02.2024.
//

#ifndef PSEUDOFAT_BITMAP_OPERATIONS_H
#define PSEUDOFAT_BITMAP_OPERATIONS_H

#include "iNodeFileSystem.h"

int markBlockUsed(iNodeFileSystem *fileSystem, unsigned int block);
int markBlockFree(iNodeFileSystem *fileSystem, unsigned int block);
int markINodeUsed(iNodeFileSystem *fileSystem, unsigned int iNode);
int markINodeFree(iNodeFileSystem *fileSystem, unsigned int iNode);
int getFreeBlock(iNodeFileSystem *fileSystem);
int getFreeINode(iNodeFileSystem *fileSystem);
char isBlockUsed(iNodeFileSystem *fileSystem, unsigned int block);

#endif //PSEUDOFAT_BITMAP_OPERATIONS_H
