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

    int option;
    while(option != 16) {
        char **input = read_input();
        option = chooseOption(input, fileSystem);
    }
    freeFileSystem(&fileSystem);
    return 0;
}