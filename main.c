#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "FATfileSystem.h"
#include "functions.h"

char **read_input(void) {
    char buffer[1024];
    char *tok;
    char **words;
    int count = 0;

    // Read input from console
    fgets(buffer, 1024, stdin);

    // Allocate memory for array of pointers to strings
    words = malloc(sizeof(char*) * 1024);

    // Split input into words using strtok
    tok = strtok(buffer, " ");
    while (tok != NULL) {
        // Allocate memory for each word
        words[count] = malloc(strlen(tok) + 1);
        strcpy(words[count], tok);
        count++;

        tok = strtok(NULL, " \n");
    }

    // Add a null terminator to the array
    words[count] = NULL;

    return words;
}

int getOption(char *input) {
    char *options[] = {"mv", "cp", "rm", "ls", "cd", "pwd", "mkdir", "rmdir", "cat", "info", "incp", "outcp", "format", "load", "exit"};
    for (int i = 0; i < 15; i++) {
        if (!strcmp(input, options[i])) return i;
    }

    return -1;
}

int chooseOption(char **input, FATFileSystem *fileSystem) {
    if (input == NULL || *input == NULL || !input[1]) return 0;

    int option = getOption(input[0]);
    char *src = input[1];
    char *dst = input[2];

    switch (option) {
        case 0:
            move(src, dst, fileSystem);
            printf("mv\n");
            break;
        case 1:
            copy(src, dst, fileSystem);
            printf("cp\n");
            break;
        case 2:
            removeFile(fileSystem, src);
            printf("rm\n");
            break;
        case 3:
            listDirectory(fileSystem);
            printf("ls\n");
            break;
        case 4:
            changeDirectory(fileSystem, src);
            printf("cd\n");
            break;
        case 5:
            printWorkingDirectory(fileSystem);
            printf("pwd\n");
            break;
        case 6:
            makeDirectory(fileSystem, src);
            printf("mkdir\n");
            break;
        case 7:
            removeDirectory(fileSystem, src);
            printf("rmdir\n");
            break;
        case 8:
            catFile(fileSystem, src);
            printf("cat\n");
            break;
        case 9:
            printFileInformation(fileSystem, src);
            printf("info\n");
            break;
        case 10:
            inCopy(fileSystem, src, dst);
            printf("incp\n");
            break;
        case 11:
            outCopy(fileSystem, src, dst);
            printf("outcp\n");
            break;
        case 12:
            printf("format\n");
            char *ptr;
            int size = strtol(src, &ptr, 10);
            format(fileSystem, size);
            break;
        case 13:
            load(fileSystem, src);
            printf("load\n");
            break;
        case 14:
            printf("exit\n");
            break;
        default:
            printf("Invalid command\n");
            break;
    }
    return option;
}






int main() {
    FILE *fp = fopen("input.txt", "r");
    //get size of file
    fseek(fp, 0, SEEK_END);
    long fsize = ftell(fp);
    rewind(fp);
//    fseek(fp, 0, SEEK_SET);
    //print size of file
    printf("Size of file: %ld\n", fsize);
    FILE *fp2 = fopen("output.txt", "w");
    char *buffer = calloc(1024,1);
    while (fread(buffer, 1, 1024, fp) > 0) {
        fwrite(buffer, 1, fsize, fp2);
    }

    fclose(fp);
    fclose(fp2);
    free(buffer);

    int size = 1024 * 1024 * 600;
    FATFileSystem *fileSystem = createFileSystem(size);
    saveNewFileSystem(fileSystem, "test");
    printf("Total space: %d\n", fileSystem->totalSpace);
    freeFileSystem(&fileSystem);

//    printf("%d\n", sizeof(DirCluster));
//    printf("%d\n", sizeof(FileCluster));
//    printf("%d\n", sizeof(DirectoryEntry));


    return 0;
}
