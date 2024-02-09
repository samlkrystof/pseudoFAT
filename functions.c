#include "iNodeFileSystem.h"
#include "functions.h"
#include "directory_operations.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

char **read_input() {
    char buffer[1024];
    char *tok;
    char **words = malloc(4 * sizeof(char *));
    for (int i = 0; i < 4; i++) {
        words[i] = calloc(1024,1);
    }
    int count = 0;

    // Read input from console
    fgets(buffer, 1024, stdin);

    for (int i = 0; i < 3; i++) {
        // clear memory
        memset(words[i], 0, 1024);
    }


    // Split input into words using strtok
    tok = strtok(buffer, " \n");
    while (tok != NULL && count <= 3) {
        strcpy(words[count], tok);
        tok = strtok(NULL, " \n");
        count++;
    }

    return words;
}

int getOption(char *input) {
    // remove newline character
    if (input[strlen(input) - 1] == '\n') input[strlen(input) - 1] = '\0';
    char *options[] = {"mv", "cp", "rm", "ls", "cd", "pwd", "mkdir", "rmdir", "cat", "info", "incp", "outcp", "format",
                       "load", "xcp", "short", "exit"};

    for (int i = 0; i < 17; i++) {
        if (!strcmp(input, options[i])) return i;
    }

    return -1;
}

int chooseOption(char **input, iNodeFileSystem *fileSystem) {
    if (input == NULL || *input == NULL) return 0;

    int option = getOption(input[0]);
    char *src = input[1];
    char *dst = input[2];
    input[0] = src;
    input[1] = dst;
    input[2] = input[3];


    //array of function pointers
    int (*functions[16])(iNodeFileSystem *, char **) = {move, copy, removeFile, listDirectory, changeDirectory,
                                                        printWorkingDirectory, makeDirectory, removeDirectory, catFile,
                                                        printFileInformation, inCopy, outCopy, format, load, combineFiles, shorten};

    if (option == 16) {
        return 16;
    }
    if (option < 0 || option >= 15) {
        printf("Invalid command\n");
        return 0;
    }
    int output = functions[option](fileSystem, input);
    //todo leak
    for(int i = 0; i < 3; i++) {
        free(input[i]);
    }
    free(input);
    return output;
}

int move(iNodeFileSystem *fileSystem, char **input) {
    if (input == NULL || input[0] == NULL || input[1] == NULL) return 0;

    //check if the file exists
    if (findDirectory(fileSystem, input[0]) == -1) {
        printf("FILE NOT FOUND\n");
        return 0;
    }

    unsigned int srcInode = findParentDir(fileSystem, input[0]);
    unsigned int dstInode = findParentDir(fileSystem, input[1]);

    if (srcInode == -1 || dstInode == -1) {
        printf("PATH NOT FOUND\n");
        return 0;
    }

    dirEntry *entry = getDirEntry(fileSystem,srcInode, input[0]);

    // if src and dst are same, just rename
    if (srcInode == dstInode) {
        memset(entry->name, 0, 12);
        strncpy(entry->name, input[1], strlen(input[1]));
    } else {
        input[1] = getFileName(input[1]);
        addDirEntry(fileSystem, dstInode, entry->iNode, input[1]);
        free(entry);
    }
    printf("OK\n");
    return 1;
}

int copy(iNodeFileSystem *fileSystem, char **input) {
    if (input == NULL || input[0] == NULL || input[1] == NULL) return 0;

    //check if the file exists
    if (findDirectory(fileSystem, input[0]) == -1) {
        printf("FILE NOT FOUND\n");
        return 0;
    }

    int srcInode = findParentDir(fileSystem, input[0]);
    int dstInode = findParentDir(fileSystem, input[1]);
    if (dstInode == -1 && strncmp(input[1], "/", 1) != 0) {
        dstInode = 0;
    }

    if (srcInode == -1 || dstInode == -1) {
        printf("PATH NOT FOUND\n");
        return 0;
    }

    dirEntry *entry = getDirEntry(fileSystem,srcInode, input[0]);
    //todo multiple blocks
    addFile(fileSystem, entry->name, fileSystem->iNodes[entry->iNode].size, fileSystem->blocks + fileSystem->iNodes[entry->iNode].directBlocks[0], dstInode);
    printf("OK\n");
    return 1;
}

int removeFile(iNodeFileSystem *fileSystem, char **input) {
    if (input == NULL || input[0] == NULL) return 0;

    //check if the file exists
    if (findDirectory(fileSystem, input[0]) != -1) {
        printf("FILE NOT FOUND\n");
        return 0;
    }

    deleteFile(fileSystem, input[0]);
    printf("OK\n");
    return 1;
}
int listDirectory(iNodeFileSystem *fileSystem, char **input) {
    // if input is empty, print current directory
    dirEntry *entry;
    if (input == NULL || input[0] == NULL || strncmp(input[0], "", 1) == 0) {
        entry = fileSystem->currentDir;
    } else {
        int inode = findParentDir(fileSystem, input[0]);
        if (inode == -1) {
            printf("PATH NOT FOUND\n");
            return 0;
        }
        entry = getDirEntry(fileSystem, inode, input[0]);
    }
    for (int i = 0; i < fileSystem->iNodes[entry->iNode].size; i++) {
        dirEntry *another = (dirEntry *)&fileSystem->blocks[fileSystem->iNodes[entry->iNode].directBlocks[0] * fileSystem->blockSize] + i;
        char isDirectory = fileSystem->iNodes[another->iNode].isDirectory;
        // print type and name, isDirectory is 0 for file, 1 for directory
        printf("%c%s\n", isDirectory ? '+' : '-', another->name);
    }
    return 1;
}

int changeDirectory(iNodeFileSystem *fileSystem, char **input) {
    if (input == NULL || input[0] == NULL) return 0;
    // check if is directory
    if(findDirectory(fileSystem, input[0]) == -1) {
        printf("PATH NOT FOUND\n");
        return 0;
    }
    int inode = findParentDir(fileSystem, input[0]);
    if (inode == -1) {
        printf("PATH NOT FOUND\n");
        return 0;
    }
    fileSystem->currentDir = getDirEntry(fileSystem, inode, input[0]);
    printf("OK\n");
    return 1;
}

int printWorkingDirectory(iNodeFileSystem *fileSystem, char **input) {
    // get full path
    char *path = malloc(100);
    dirEntry *actual = fileSystem->currentDir;
    dirEntry *entry = fileSystem->currentDir;
    char **names = malloc(100);
    names[0] = "..";
    while (entry != fileSystem->rootDir) {
        strcat(path, entry->name);
        strcat(path, "/");
        changeDirectory(fileSystem, names);
    }
    printf("%s\n", path);
    fileSystem->currentDir = actual;
    return 1;
}
int makeDirectory(iNodeFileSystem *fileSystem, char **input) {
    if (input == NULL || input[0] == NULL) return 0;
    // check if directory already exists
    if (findDirectory(fileSystem, input[0]) != -1) {
        printf("EXIST\n");
        return 0;
    }
    char *dirName = getFileName(input[0]);
    int parent = findParentDir(fileSystem, input[0]);
    if (parent == -1) {
        printf("PATH NOT FOUND\n");
        return 0;
    }
    addDirectory(fileSystem, dirName, parent);
    printf("OK\n");
    return 1;
}

int removeDirectory(iNodeFileSystem *fileSystem, char **input) {
    if (input == NULL || input[0] == NULL) return 0;
    int output = deleteDirectory(fileSystem, input[0]);
    if(output == 1) {
        printf("FILE NOT FOUND\n");
    } else if (output == 2) {
        printf("NOT EMPTY\n");
    } else {
        printf("OK\n");
    }
    return 1;
}

int catFile(iNodeFileSystem *fileSystem, char **input) {
    //todo
    if (input == NULL || input[0] == NULL) return 0;
    int parent = findParentDir(fileSystem, input[0]);
    if (parent == -1) {
        printf("FILE NOT FOUND\n");
        return 0;
    }
    dirEntry *entry = getDirEntry(fileSystem, parent, input[0]);
    if (entry == NULL) {
        printf("FILE NOT FOUND\n");
        return 0;
    }
    for (int i = 0; i < fileSystem->iNodes[entry->iNode].size; i++) {
        printf("%s", fileSystem->blocks[fileSystem->iNodes[entry->iNode].directBlocks[0] * fileSystem->blockSize + i]);
    }
    return 1;
}

int printFileInformation(iNodeFileSystem *fileSystem, char **input) {
    //print name, size, inode, blocks
    if (input == NULL || input[0] == NULL) return 0;
    int parent = findParentDir(fileSystem, input[0]);
    if (parent == -1) {
        printf("FILE NOT FOUND\n");
        return 0;
    }
    dirEntry *entry = getDirEntry(fileSystem, parent, input[0]);
    if (entry == NULL) {
        printf("FILE NOT FOUND\n");
        return 0;
    }
    //todo
    int size = fileSystem->iNodes[entry->iNode].size;
    int inode = entry->iNode;
    int blocks = fileSystem->iNodes[entry->iNode].size / fileSystem->blockSize;
    printf("Name: %s\nSize: %d\nInode: %d\nBlocks: %d\n", entry->name, size, inode, blocks);
}
int inCopy(iNodeFileSystem *fileSystem, char **input) {
    if (input == NULL || input[0] == NULL || input[1] == NULL) return 0;

    FILE *file = fopen(input[0], "rb");
    if (file == NULL) {
        printf("FILE NOT FOUND\n");
        return 0;
    }
    fseek(file, 0, SEEK_END);
    int size = ftell(file);
    fseek(file, 0, SEEK_SET);
    char *content = malloc(size);
    fread(content, 1, size, file);
    fclose(file);
    addFile(fileSystem, input[1], size, content, findParentDir(fileSystem, input[1]));
    // printf path not found
    free(content);
    printf("OK\n");
    return 1;
}

int outCopy(iNodeFileSystem *fileSystem, char **input) {
    if (input == NULL || input[0] == NULL || input[1] == NULL) return 0;
    int parent = findParentDir(fileSystem, input[0]);
    if (parent == -1) {
        printf("FILE NOT FOUND\n");
        return 0;
    }
    dirEntry *entry = getDirEntry(fileSystem,  parent, input[0]);
    if (entry == NULL) {
        return 0;
    }
    FILE *file = fopen(input[1], "wb");
    if (file == NULL) {
        printf("PATH NOT FOUND\n");
        return 0;
    }
    //todo multiple blocks
    fwrite(fileSystem->blocks + fileSystem->iNodes[entry->iNode].directBlocks[0] * fileSystem->blockSize, 1, fileSystem->iNodes[entry->iNode].size, file);
    fclose(file);
    printf("OK\n");
    return 1;
}

int format(iNodeFileSystem *fileSystem, char **input) {
    if (input == NULL || input[0] == NULL) return 0;
    if (fileSystem != NULL) {
        freeFileSystem(&fileSystem);
    }
    int size = strtol(input[0], NULL, 10);
    fileSystem = createFileSystem(size * 1024 * 1024);
    return 1;
}

int load(iNodeFileSystem *fileSystem, char **input) {
    // open file which contains instructions
    if (input == NULL || input[0] == NULL) return 0;
    FILE *file = fopen(input[0], "r");
    if (file == NULL) {
        return 0;
    }
    // read line by line
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    while ((read = getline(&line, &len, file)) != -1) {
        char **args = malloc(100);
        char *token = strtok(line, " ");
        int i = 0;
        while (token != NULL) {
            args[i] = token;
            token = strtok(NULL, " ");
            i++;
        }
        chooseOption(args, fileSystem);
    }
    fclose(file);
    return 1;
}

int combineFiles(iNodeFileSystem *fileSystem, char **input) {
    printf("Not implemented\n");
    return 0;
}

int shorten(iNodeFileSystem *fileSystem, char **input) {
    printf("Not implemented\n");
    return 0;
}
