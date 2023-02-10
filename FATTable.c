//
// Created by Kryst on 07.02.2023.
//

#include "FATTable.h"


FATTable *createFATTable(unsigned int entriesCount) {
    FATTable *table = malloc(sizeof(FATTable));
    table->clustersCount = entriesCount;
    table->freeClustersCount = entriesCount;
    table->clusters = calloc(entriesCount, sizeof(unsigned int));
    return table;
}

void freeFATTable(FATTable **table) {
    if (table == NULL || *table == NULL) {
        return;
    }
    free((* table)->clusters);
    free((*table));
    *table = NULL;
    table = NULL;
}

int getFreeCluster(FATTable *table1, FATTable *table2) {
    if (table1 == NULL || table2 == NULL) {
        return -1;
    }

    for (int i = 0; i < table1->clustersCount; i++) {
        if (table1->clusters[i] == 0 && table2->clusters[i] == 0) {
            return i;
        }
    }
    return -1;
}

int getNextFreeCluster(FATTable *table1, FATTable *table2, int firstCluster) {
    if (table1 == NULL || table2 == NULL) {
        return -1;
    }

    for (int i = firstCluster; i < table1->clustersCount; i++) {
        if (table1->clusters[i] == 0 && table2->clusters[i] == 0) {
            return i;
        }
    }
    return -1;
}


