//
// Created by Kryst on 07.02.2023.
//

#include <malloc.h>
#include "DirectoryEntry.h"

#ifndef PSEUDOFAT_FATTABLE_H
#define PSEUDOFAT_FATTABLE_H

#endif //PSEUDOFAT_FATTABLE_H

typedef struct {
    unsigned int entriesCount;
    unsigned int freeEntriesCount;
    DirectoryEntry *entries;
} FATTable;

FATTable *createFATTable(unsigned int entriesCount);
void freeFATTable(FATTable **table);
int addEntry(FATTable *table1, FATTable *table2, char *name, unsigned int size, unsigned int directory, char type);