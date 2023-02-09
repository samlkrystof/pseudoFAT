//
// Created by Kryst on 07.02.2023.
//

#include <malloc.h>

#ifndef PSEUDOFAT_FATTABLE_H
#define PSEUDOFAT_FATTABLE_H

#endif //PSEUDOFAT_FATTABLE_H

typedef struct {
    unsigned int entriesCount;
    unsigned int freeEntriesCount;
    unsigned int *entries; // 0 - free, 0xFFFFFFFF - last cluster, between 2 and 0xFFFFFFFE - next cluster
} FATTable;

FATTable *createFATTable(unsigned int entriesCount);
void freeFATTable(FATTable **table);
int addEntry(FATTable *table1, FATTable *table2, char *name, unsigned int size, unsigned int directory, char type);