//
// Created by Kryst on 07.02.2023.
//

#include <malloc.h>
#include <string.h>
#include "DirectoryEntry.h"

DirectoryEntry *
createDirectoryEntry(char *name, unsigned int size, unsigned int firstCluster, unsigned int directory, char type) {
    DirectoryEntry *entry = malloc(sizeof(DirectoryEntry));
    strncpy(entry->name, name, 12);
    entry->size = size;
    entry->firstCluster = firstCluster;
    entry->directory = directory;
    entry->type = type;
    return entry;
}

void freeDirectoryEntry(DirectoryEntry **entry) {
    if (entry == NULL || *entry == NULL) {
        return;
    }
    free(*entry);
    *entry = NULL;
    entry = NULL;

}
