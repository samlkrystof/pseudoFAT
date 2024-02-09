//
// Created by Kryst on 10.02.2023.
//

#ifndef PSEUDOFAT_FUNCTIONS_H
#define PSEUDOFAT_FUNCTIONS_H

#include "iNodeFileSystem.h"


void read_input(char **words);
int getOption(char *input);
int chooseOption(char **input, iNodeFileSystem *fileSystem);

int move(iNodeFileSystem *fileSystem, char **input);
int copy(iNodeFileSystem *fileSystem, char **input);
int removeFile(iNodeFileSystem *fileSystem, char **input);
int listDirectory(iNodeFileSystem *fileSystem, char **input);
int changeDirectory(iNodeFileSystem *fileSystem, char **input);
int printWorkingDirectory(iNodeFileSystem *fileSystem, char **input);
int makeDirectory(iNodeFileSystem *fileSystem, char **input);
int removeDirectory(iNodeFileSystem *fileSystem, char **input);
int catFile(iNodeFileSystem *fileSystem, char **input);
int printFileInformation(iNodeFileSystem *fileSystem, char **input);
int inCopy(iNodeFileSystem *fileSystem, char **input);
int outCopy(iNodeFileSystem *fileSystem, char **input);
int format(iNodeFileSystem *fileSystem, char **input);
int load(iNodeFileSystem *fileSystem, char **input);
int combineFiles(iNodeFileSystem *fileSystem, char **input);
int shorten(iNodeFileSystem *fileSystem, char **input);

#endif //PSEUDOFAT_FUNCTIONS_H
