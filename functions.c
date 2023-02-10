//
// Created by Kryst on 10.02.2023.
//

#include "functions.h"


int copy(char *source, char *destination, FATFileSystem *system) {
    if (source == NULL || destination == NULL || system == NULL) {
        return -1;
    }

    DirCluster *sourceCluster = findDirectory(system, source);
    DirCluster *destinationCluster = findDirectory(system, destination);

    if (sourceCluster == NULL || destinationCluster == NULL) {
        return -1;
    }


}

char *getFileName(char *path) {
    if (path == NULL) {
        return NULL;
    }

    // trim all slashes and leave only file name
    char *fileName = strrchr(path, '/');
    if (fileName == NULL) {
        fileName = path;
    } else {
        fileName++;
    }

    return fileName;
}
