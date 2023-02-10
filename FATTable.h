//
// Created by Kryst on 07.02.2023.
//

#include <malloc.h>

#ifndef PSEUDOFAT_FATTABLE_H
#define PSEUDOFAT_FATTABLE_H

#endif //PSEUDOFAT_FATTABLE_H

typedef struct {
    unsigned int clustersCount;
    unsigned int freeClustersCount;
    unsigned int *clusters; // 0 - free, 0xFFFFFFFF - last cluster, 0xFFFFFFFE - corrupted
} FATTable;

FATTable *createFATTable(unsigned int entriesCount);
void freeFATTable(FATTable **table);
int getFreeCluster(FATTable *table1, FATTable *table2);
int getNextFreeCluster(FATTable *table1, FATTable *table2, int firstCluster);