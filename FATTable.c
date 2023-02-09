//
// Created by Kryst on 07.02.2023.
//

#include "FATTable.h"


FATTable *createFATTable(unsigned int entriesCount) {
    FATTable *table = malloc(sizeof(FATTable));
    table->entriesCount = entriesCount;
    table->freeEntriesCount = entriesCount;
    table->entries = malloc(sizeof(DirectoryEntry) * entriesCount);
    return table;
}

void freeFATTable(FATTable **table) {
    if (table == NULL || *table == NULL) {
        return;
    }
    //todo: free entries
    free((* table)->entries);
    free((*table));
    *table = NULL;
    table = NULL;
}

int addEntry(FATTable *table1, FATTable *table2, char *name, unsigned int size, unsigned int directory, char type) {
    if (table1 == NULL || table2 == NULL || name == NULL) {
        return 1;
    }

    if (table1->freeEntriesCount == 0 || table2->freeEntriesCount == 0) {
        return 1;
    }

    DirectoryEntry *entry = createDirectoryEntry(name, size, 0, directory, type);
    table1->entries[table1->entriesCount - table1->freeEntriesCount] = *entry;
    table2->entries[table2->entriesCount - table2->freeEntriesCount] = *entry;
    table1->freeEntriesCount--;
    table2->freeEntriesCount--;
    freeDirectoryEntry(&entry);
    return 0;
}

