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
    char *sourceName = getFileName(source);
    unsigned int size = findDirEntry(sourceCluster, sourceName)->size;

    char *destinationName = getFileName(destination);
    char *content = malloc(size);
    // read file content
    getFileContent(system, sourceName, content, sourceCluster);

    addFile(system, destinationName, size, content, destinationCluster);
    free(content);

    return 0;
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

int move(char *source, char *destination, FATFileSystem *system) {
    if (source == NULL || system == NULL) {
        return -1;
    }

    DirCluster *sourceCluster = findDirectory(system, source);
    DirCluster *destinationCluster = findDirectory(system, destination);
    if (sourceCluster == NULL || destinationCluster == NULL) {
        return -1;
    }
    char *sourceName = getFileName(source);
    char *destinationName = getFileName(destination);
    DirEntry *sourceEntry = findDirEntry(sourceCluster, sourceName);
    if (destinationCluster == sourceCluster) {
        // fill name with zeros
        memset(sourceEntry->name, 0, 12);
        // copy name
        memcpy(sourceEntry->name, destinationName, strlen(destinationName));
    } else {
        DirEntry *destinationEntry = findDirEntry(destinationCluster, destinationName);
        if (destinationEntry != NULL) {
            return -1;
        }

        addDirEntry(destinationCluster, destinationName, sourceEntry->cluster, sourceEntry->size, sourceEntry->type);
        removeDirEntry(sourceCluster, sourceName);
    }

    return 0;
}

int removeFile(FATFileSystem *fileSystem, char *name) {
    if (fileSystem == NULL || name == NULL) {
        return -1;
    }

    DirCluster *cluster = findDirectory(fileSystem, name);
    if (cluster == NULL) {
        return -1;
    }

    char *fileName = getFileName(name);

    return deleteFile(fileSystem, fileName, cluster);
}

int makeDirectory(FATFileSystem *fileSystem, char *name) {
    if (fileSystem == NULL || name == NULL) {
        return -1;
    }

    DirCluster *cluster = strchr(name, '/') == NULL ? fileSystem->currentDirCluster : findDirectory(fileSystem, name);
    if (cluster == NULL) {
        return -1;
    }

    char *dirName = getFileName(name);

    addDirectory(fileSystem, dirName, cluster);

    return 0;
}

int removeDirectory(FATFileSystem *fileSystem, char *name) {
    if (fileSystem == NULL || name == NULL) {
        return -1;
    }

    DirCluster *cluster = findDirectory(fileSystem, name);
    if (cluster == NULL) {
        return -1;
    }

    char *dirName = getFileName(name);

    return deleteDirectory(fileSystem, dirName, cluster);
}

int listDirectory(FATFileSystem *fileSystem, char *name) {
    if (fileSystem == NULL) {
        return -1;
    }

    DirCluster *cluster = name == NULL ? fileSystem->currentDirCluster : findDirectory(fileSystem, name);
    if (cluster == NULL) {
        return -1;
    }

    printf("Name\tType\tSize\tCluster\n");
    for (int i = 0; i < CLUSTER_SIZE / sizeof(DirEntry); i++) {
        DirEntry *entry = &cluster->entries[i];
        if (entry->type == 0) {
            break;
        }
        //todo edit print
        char *type = entry->type == 1 ? "DIR" : "FILE";
        printf("%s\t%s\t%d\t%d\n", entry->name, type, entry->size, entry->cluster);

        //format string to 12 chars

    }

    return 0;
}

int catFile(FATFileSystem *fileSystem, char *name) {
    if (fileSystem == NULL || name == NULL) {
        return -1;
    }

    DirCluster *cluster = findDirectory(fileSystem, name);
    if (cluster == NULL) {
        return -1;
    }

    char *fileName = getFileName(name);
    DirEntry *entry = findDirEntry(cluster, fileName);
    if (entry == NULL) {
        return -1;
    }

    char *content = malloc(entry->size);
    getFileContent(fileSystem, fileName, content, cluster);
    printf("%s", content);
    free(content);

    return 0;
}


int changeDirectory(FATFileSystem *fileSystem, char *name) {
    if (fileSystem == NULL || name == NULL) {
        return -1;
    }


    DirCluster *cluster = findDirectory(fileSystem, name);
    if (cluster == NULL) {
        return -1;
    }

    fileSystem->currentDirCluster = cluster;

    return 0;
}

int printWorkingDirectory(FATFileSystem *fileSystem) {
    if (fileSystem == NULL) {
        return -1;
    }
    char **path = malloc(1024 * sizeof(char *));
    int i = 0;
    DirCluster *cluster = fileSystem->currentDirCluster;
    while (cluster != fileSystem->rootDirCluster) {
        path[i] = malloc(12);

        memcpy(path[i], cluster->entries[0].name, 12);
        DirCluster *parent = (DirCluster *) &fileSystem->clusterArea[cluster->entries[1].cluster];
        // find actual cluster in parent
        for (int j = 0; j < CLUSTER_SIZE / sizeof(DirEntry); j++) {
            if (parent->entries[j].cluster == cluster->entries[0].cluster) {
                memcpy(path[i], parent->entries[j].name, 12);
            }
        }
        i++;
    }

    printf("/");
    for (int j = i - 1; j >= 0; j--) {
        printf("%s/", path[j]);
        free(path[j]);
    }
    free(path);
    return 0;
}

int printFileInformation(FATFileSystem *fileSystem, char *name) {
    if (fileSystem == NULL || name == NULL) {
        return -1;
    }

    DirCluster *cluster = findDirectory(fileSystem, name);
    if (cluster == NULL) {
        return -1;
    }

    char *fileName = getFileName(name);
    DirEntry *entry = findDirEntry(cluster, fileName);
    if (entry == NULL) {
        return -1;
    }

    printf("Name: %s\n", entry->name);
    printf("Size: %d\n", entry->size);
    printf("Type: %d\n", entry->type);
    printf("Cluster: %d\n", entry->cluster);

    return 0;
}

int inCopy(FATFileSystem *fileSystem, char *sourceName, char *destinationName) {
    if (fileSystem == NULL || sourceName == NULL || destinationName == NULL) {
        return -1;
    }

    FILE *source = fopen(sourceName, "r");
    if (source == NULL) {
        return -1;
    }


    DirCluster *destinationCluster = findDirectory(fileSystem, destinationName);
    if (destinationCluster == NULL) {
        return -1;
    }
    char *fileName = getFileName(destinationName);

    //get size of file
    fseek(source, 0, SEEK_END);
    int size = ftell(source);
    fseek(source, 0, SEEK_SET);

    char *content = malloc(size);
    fread(content, 1, size, source);
    fclose(source);

    int result = addFile(fileSystem, fileName, size, content, destinationCluster);
    free(content);
    return result;
}

int outCopy(FATFileSystem *fileSystem, char *sourceName, char *destinationName) {
    if (fileSystem == NULL || sourceName == NULL || destinationName == NULL) {
        return -1;
    }

    DirCluster *sourceCluster = findDirectory(fileSystem, sourceName);
    if (sourceCluster == NULL) {
        return -1;
    }
    char *fileName = getFileName(sourceName);

    DirEntry *sourceEntry = findDirEntry(sourceCluster, fileName);
    if (sourceEntry == NULL) {
        return -1;
    }

    char *content = malloc(sourceEntry->size);
    getFileContent(fileSystem, fileName, content, sourceCluster);

    FILE *destination = fopen(destinationName, "w");
    if (destination == NULL) {
        return -1;
    }

    fwrite(content, 1, sourceEntry->size, destination);
    fclose(destination);
    free(content);

    return 0;
}







