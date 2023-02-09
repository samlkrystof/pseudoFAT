//
// Created by Kryst on 07.02.2023.
//

#include "FATTable.h"


FATTable *createFATTable(unsigned int entriesCount) {
    FATTable *table = malloc(sizeof(FATTable));
    table->entriesCount = entriesCount;
    table->freeEntriesCount = entriesCount;
    table->entries = calloc(entriesCount,sizeof(unsigned int));
    return table;
}

void freeFATTable(FATTable **table) {
    if (table == NULL || *table == NULL) {
        return;
    }
    free((* table)->entries);
    free((*table));
    *table = NULL;
    table = NULL;
}


