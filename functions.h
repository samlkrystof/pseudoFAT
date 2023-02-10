//
// Created by Kryst on 10.02.2023.
//

#ifndef PSEUDOFAT_FUNCTIONS_H
#define PSEUDOFAT_FUNCTIONS_H

#include "DirCluster.h"
#include "FATfileSystem.h"

//int copy(char *source, char *destination, FATFileSystem *system, DirCluster *sourceCluster, DirCluster *destinationCluster);
int copy(char *source, char *destination, FATFileSystem *system);

//int move(char *source, char *destination, FATFileSystem *system, DirCluster *sourceCluster, DirCluster *destinationCluster);
int move(char *source, char *destination, FATFileSystem *system);

//int removeFile(FATFileSystem *fileSystem, char *name, DirCluster *cluster);
int removeFile(FATFileSystem *fileSystem, char *name);

//int makeDirectory(FATFileSystem *fileSystem, char *name, DirCluster *cluster);
int makeDirectory(FATFileSystem *fileSystem, char *name);
//int removeDirectory(FATFileSystem *fileSystem, char *name, DirCluster *cluster);
int removeDirectory(FATFileSystem *fileSystem, char *name);

//int listDirectory(FATFileSystem *fileSystem, DirCluster *cluster);
int listDirectory(FATFileSystem *fileSystem, char *name);
//int catFile(FATFileSystem *fileSystem, char *name, DirCluster *cluster);
int catFile(FATFileSystem *fileSystem, char *name);
//int changeDirectory(FATFileSystem *fileSystem, char *name, DirCluster *cluster);
int changeDirectory(FATFileSystem *fileSystem, char *name);
int printWorkingDirectory(FATFileSystem *fileSystem);

//int printFileInformation(FATFileSystem *fileSystem, char *name, DirCluster *cluster);
int printFileInformation(FATFileSystem *fileSystem, char *name);
//int inCopy(FATFileSystem *fileSystem, char *sourceName, char *destinationName, DirCluster *cluster);
int inCopy(FATFileSystem *fileSystem, char *sourceName, char *destinationName);
//int outCopy(FATFileSystem *fileSystem, char *sourceName, char *destinationName, DirCluster *cluster);
int outCopy(FATFileSystem *fileSystem, char *sourceName, char *destinationName);
int load(FATFileSystem *fileSystem, char *name);
int format(FATFileSystem *fileSystem, unsigned int totalSize);
char *getFileName(char *path);

#endif //PSEUDOFAT_FUNCTIONS_H
