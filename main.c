//
// Created by Kryst on 09.02.2024.
//

#include <stdio.h>
#include <stdlib.h>
#include "iNodeFileSystem.h"
#include "functions.h"

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Name of file system not provided\n");
        return 1;
    }
    iNodeFileSystem *fileSystem = createFileSystem(1024 * 1024);
    char **input = malloc(4 * sizeof(char *));
    for (int i = 0; i < 4; i++) {
        input[i] = malloc(100);
    }
    int option;
    while(option != 16) {
        read_input(input);
        option = chooseOption(input, fileSystem);
    }
    freeFileSystem(&fileSystem);
    for (int i = 0; i < 4; i++) {
        free(input[i]);
    }
    free(input);
    return 0;
}