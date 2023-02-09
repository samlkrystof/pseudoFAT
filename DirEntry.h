//
// Created by Kryst on 09.02.2023.
//

#ifndef PSEUDOFAT_DIRENTRY_H
#define PSEUDOFAT_DIRENTRY_H

typedef struct {
    unsigned int cluster;
    unsigned int size;
    char name[12];
    char type; // 1 - file, 2 - directory
} DirEntry;

#endif //PSEUDOFAT_DIRENTRY_H
