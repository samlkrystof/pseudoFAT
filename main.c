#include <stdio.h>
#include "iNodeFileSystem.h"
#include "functions.h"

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Name of file system not provided\n");
        return 1;
    }
    iNodeFileSystem *fileSystem = createFileSystem(1024 * 1024, argv[1]);

    int option;
    while(option != 16) {
        char **input = read_input();
        option = chooseOption(input, fileSystem);
    }
    freeFileSystem(&fileSystem);
    return 0;
}