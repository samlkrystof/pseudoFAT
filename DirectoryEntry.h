//
// Created by Kryst on 07.02.2023.
//

#ifndef PSEUDOFAT_DIRECTORYENTRY_H
#define PSEUDOFAT_DIRECTORYENTRY_H

#endif //PSEUDOFAT_DIRECTORYENTRY_H

typedef struct {
    char name[12];
    unsigned int size;
    unsigned int firstCluster;
    unsigned int directory; // index of directory in FATTable
    char type; // 0 - file, 1 - directory
} DirectoryEntry;

DirectoryEntry *createDirectoryEntry(char *name, unsigned int size, unsigned int firstCluster, unsigned int directory, char type);
void freeDirectoryEntry(DirectoryEntry **entry);
