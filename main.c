#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "functions.h"

void read_input(char **words) {
    char buffer[1024];
    char *tok;

    int count = 0;

    // Read input from console
    fgets(buffer, 1024, stdin);

    for (int i = 0; i < 3; i++) {
        // clear memory
        memset(words[i], 0, 1024);
    }


    // Split input into words using strtok
    tok = strtok(buffer, " ");
    while (tok != NULL && count < 3) {
        // Allocate memory for each word
        strcpy(words[count], tok);
        count++;

        tok = strtok(NULL, " \n");
    }
}

int getOption(char *input) {
    // remove newline character
    if (input[strlen(input) - 1] == '\n') input[strlen(input) - 1] = '\0';
    char *options[] = {"mv", "cp", "rm", "ls", "cd", "pwd", "mkdir", "rmdir", "cat", "info", "incp", "outcp", "format", "load", "exit"};

    for (int i = 0; i < 15; i++) {
        if (!strcmp(input, options[i])) return i;
    }

    return -1;
}

int chooseOption(char **input, FATFileSystem *fileSystem) {
    if (input == NULL || *input == NULL) return 0;

    int option = getOption(input[0]);
    char *src = input[1];
    char *dst = input[2];

    switch (option) {
        case 0:
            if (src == NULL || dst == NULL) return 0;
            move(src, dst, fileSystem);
            break;
        case 1:
            if (src == NULL || dst == NULL) return 0;
            copy(src, dst, fileSystem);
            printf("cp\n");
            break;
        case 2:
            if (src == NULL) return 0;
            removeFile(fileSystem, src);
            printf("rm\n");
            break;
        case 3:
            listDirectory(fileSystem, src);
            break;
        case 4:
            if (src == NULL) return 0;
            changeDirectory(fileSystem, src);
            break;
        case 5:
            printWorkingDirectory(fileSystem);
            printf("pwd\n");
            break;
        case 6:
            if (src == NULL) return 0;
            makeDirectory(fileSystem, src);
            break;
        case 7:
            if (src == NULL) return 0;
            removeDirectory(fileSystem, src);
            printf("rmdir\n");
            break;
        case 8:
            if (src == NULL) return 0;
            catFile(fileSystem, src);
            printf("cat\n");
            break;
        case 9:
            if (src == NULL) return 0;
            printFileInformation(fileSystem, src);
            printf("info\n");
            break;
        case 10:
            if (src == NULL || dst == NULL) return 0;
            inCopy(fileSystem, src, dst);
            printf("incp\n");
            break;
        case 11:
            if (src == NULL || dst == NULL) return 0;
            outCopy(fileSystem, src, dst);
            printf("outcp\n");
            break;
        case 12:
            if (src == NULL) return 0;
            printf("format\n");
            char *ptr;
            int size = strtol(src, &ptr, 10);
//            format(fileSystem, size);
            break;
        case 13:
            if (src == NULL) return 0;
//            load(fileSystem, src);
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
//    FILE *fp = fopen("option.txt", "r");
//    //get size of file
//    fseek(fp, 0, SEEK_END);
//    long fsize = ftell(fp);
//    rewind(fp);
//    fseek(fp, 0, SEEK_SET);
    //print size of file
//    printf("Size of file: %ld\n", fsize);
//    FILE *fp2 = fopen("output.txt", "w");
//    char *buffer = calloc(1024,1);
//    while (fread(buffer, 1, 1024, fp) > 0) {
//        fwrite(buffer, 1, fsize, fp2);
//    }

//    fclose(fp);
//    fclose(fp2);
//    free(buffer);

    int size = 1024 * 1024 * 600;
    char **inputBuffer = calloc(3, sizeof(char *));
    for (int i = 0; i < 3; i++) {
        inputBuffer[i] = calloc(1024, sizeof(char));
    }

    FATFileSystem *fileSystem = createFileSystem(size);
    int option = 0;
    while (option != 14) {
        printf(">");
        read_input(inputBuffer);
        option = chooseOption(inputBuffer, fileSystem);
    }

//    printf("%d\n", sizeof(DirCluster));
//    printf("%d\n", sizeof(FileCluster));
//    printf("%d\n", sizeof(DirectoryEntry));


    return 0;
}
