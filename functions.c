#include "iNodeFileSystem.h"
#include "functions.h"
#include "directory_operations.h"
#include "bitmap_operations.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/**
 * @brief Reads input from the user.
 *
 * @return The input from the user.
 */
char **read_input() {
    char buffer[1024];
    char *tok;
    char **words = malloc(4 * sizeof(char *));

    for (int i = 0; i < 4; i++) {
        words[i] = calloc(1024, 1);
    }

    int count = 0;
    fgets(buffer, 1024, stdin);

    for (int i = 0; i < 3; i++) {
        memset(words[i], 0, 1024);
    }

    tok = strtok(buffer, " \n");

    while (tok != NULL && count <= 3) {
        strcpy(words[count], tok);
        tok = strtok(NULL, " \n");
        count++;
    }

    return words;
}

/**
 * @brief Gets the option from the input.
 *
 * @param input The input to get the option from.
 * @return The option.
 */
int getOption(char *input) {
    if (input[strlen(input) - 1] == '\n') input[strlen(input) - 1] = '\0';
    char *options[] = {"mv", "cp", "rm", "ls", "cd", "pwd", "mkdir", "rmdir", "cat", "info", "incp", "outcp", "format",
                       "load", "xcp", "short", "exit"};

    for (int i = 0; i < 17; i++) {
        if (!strcmp(input, options[i])) return i;
    }

    return -1;
}

/**
 * @brief Chooses an option based on the input.
 *
 * @param input The input to choose the option from.
 * @param fileSystem The file system to operate on.
 * @return 1 on success, 0 on failure.
 */
int chooseOption(char **input, iNodeFileSystem *fileSystem) {
    if (input == NULL || *input == NULL) return 0;

    int option = getOption(input[0]);
    char *src = input[1];
    char *dst = input[2];
    input[0] = src;
    input[1] = dst;
    input[2] = input[3];

    int (*functions[16])(iNodeFileSystem *, char **) = {move, copy, removeFile, listDirectory, changeDirectory,
                                                        printWorkingDirectory, makeDirectory, removeDirectory, catFile,
                                                        printFileInformation, inCopy, outCopy, format, load,
                                                        combineFiles, shorten};
    if (option == 16) {
        return 16;
    }

    if (option < 0 || option > 15) {
        printf("Invalid command\n");
        return 0;
    }

    int output = functions[option](fileSystem, input);

//    if (input == NULL) return output;
//    for (int i = 0; i < 3; i++) {
//        if(input[i] != NULL) {
//            free(input[i]);
//        }
//    }

    return output;
}

/**
 * @brief Moves a file from one directory to another.
 *
 * @param fileSystem The file system to operate on.
 * @param input The input to operate on.
 * @return 1 on success, 0 on failure.
 */
int move(iNodeFileSystem *fileSystem, char **input) {
    if (input == NULL || input[0] == NULL || input[1] == NULL) return 0;

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

    dirEntry *entry = getDirEntry(fileSystem, srcInode, input[0]);

    if (srcInode == dstInode) {
        memset(entry->name, 0, 12);
        strncpy(entry->name, input[1], strlen(input[1]));
        saveDirEntry(fileSystem, srcInode, entry);
        saveName(fileSystem, input[1], entry->iNode);

    } else {
        input[1] = getFileName(input[1]);
        addDirEntry(fileSystem, dstInode, entry->iNode, input[1]);
        char *block = loadBlock(fileSystem, srcInode);
        dirEntry *dir = (dirEntry *) block;
        for (int i = 0; i < fileSystem->blockSize / sizeof(dirEntry); i++) {
            if (dir[i].iNode == entry->iNode) {
                memset(dir[i].name, 0, 12);
                break;
            }
        }
        saveBlock(fileSystem, block, srcInode);
    }

    free(entry);
    printf("OK\n");
    return 1;
}

/**
 * @brief Copies a file from one directory to another.
 *
 * @param fileSystem The file system to operate on.
 * @param input The input to operate on.
 * @return 1 on success, 0 on failure.
 */
int copy(iNodeFileSystem *fileSystem, char **input) {
    if (input == NULL || input[0] == NULL || input[1] == NULL) return 0;

    if (findDirectory(fileSystem, input[0]) == -1) {
        printf("FILE NOT FOUND\n");
        return 0;
    }

    unsigned int srcInode = findParentDir(fileSystem, input[0]);
    unsigned int dstInode = findParentDir(fileSystem, input[1]);
    if (dstInode == -1 && strncmp(input[1], "/", 1) != 0) {
        dstInode = 0;
    }

    if (srcInode == -1 || dstInode == -1) {
        printf("PATH NOT FOUND\n");
        return 0;
    }

    dirEntry *entry = getDirEntry(fileSystem, srcInode, input[0]);
    char *content = getFileContent(fileSystem, entry->iNode);
    iNode *inode = loadINode(fileSystem, entry->iNode);
    addFile(fileSystem, input[1], inode->size, content, dstInode);

    free(content);
    free(inode);
    free(entry);

    printf("OK\n");
    return 1;
}

/**
 * @brief Removes a file from the file system.
 *
 * @param fileSystem The file system to operate on.
 * @param input The input to operate on.
 * @return 1 on success, 0 on failure.
 */
int removeFile(iNodeFileSystem *fileSystem, char **input) {
    if (input == NULL || input[0] == NULL) return 0;

    if (findDirectory(fileSystem, input[0]) == -1) {
        printf("FILE NOT FOUND\n");
        return 0;
    }

    deleteFile(fileSystem, input[0]);
    printf("OK\n");
    return 1;
}
/**
 * @brief Lists the contents of a directory.
 *
 * @param fileSystem The file system to operate on.
 * @param input The input to operate on.
 * @return 1 on success, 0 on failure.
 */
int listDirectory(iNodeFileSystem *fileSystem, char **input) {
    dirEntry *entry;
    if (input == NULL || input[0] == NULL || strncmp(input[0], "", 1) == 0) {
        entry = fileSystem->currentDir;

    } else {
        unsigned int inode = findParentDir(fileSystem, input[0]);

        if (inode == -1) {
            printf("PATH NOT FOUND\n");
            return 0;
        }

        entry = getDirEntry(fileSystem, inode, input[0]);
    }

    iNode *iNodePtr = loadINode(fileSystem, entry->iNode);
    int numBlocks = 0;
    int *blocks = getAllBlocks(fileSystem, entry->iNode, &numBlocks);
    int visited = 0;

    for (int i = 0; i < numBlocks; i++) {
        char *block = loadBlock(fileSystem, blocks[i]);

        for (int j = 0; j < fileSystem->blockSize / sizeof(dirEntry); j++) {
            if (visited >= iNodePtr->size) {
                break;
            }

            dirEntry *another = (dirEntry *) (block + j * sizeof(dirEntry));
            if (another->name[0] == '\0') {
                continue;
            }

            iNode *anotherInode = loadINode(fileSystem, another->iNode);

            printf("%c%s\n", anotherInode->isDirectory ? '+' : '-', another->name);
            visited++;
            free(anotherInode);
        }
        free(block);
    }
    free(iNodePtr);
    free(blocks);

    return 1;
}

/**
 * @brief Changes the current directory.
 *
 * @param fileSystem The file system to operate on.
 * @param input The input to operate on.
 * @return 1 on success, 0 on failure.
 */
int changeDirectory(iNodeFileSystem *fileSystem, char **input) {
    if (input == NULL || input[0] == NULL) return 0;
    if (findDirectory(fileSystem, input[0]) == -1) {
        printf("PATH NOT FOUND\n");
        return 0;
    }

    unsigned int inode = findParentDir(fileSystem, input[0]);
    if (inode == -1) {
        printf("PATH NOT FOUND\n");
        return 0;
    }

    fileSystem->currentDir = getDirEntry(fileSystem, inode, input[0]);
    printf("OK\n");
    return 1;
}

/**
 * @brief Prints the current working directory.
 *
 * @param fileSystem The file system to operate on.
 * @param input The input to operate on.
 * @return 1 on success, 0 on failure.
 */
int printWorkingDirectory(iNodeFileSystem *fileSystem, char **input) {
    if (fileSystem == NULL) return 0;
    char **path = malloc(100 * sizeof(char *));

    for (int i = 0; i < 100; i++) {
        path[i] = malloc(12);
    }

    int filled = 0;
    dirEntry *current = fileSystem->currentDir;
    while (current->iNode != 0) {

        iNode *iNodePtr = loadINode(fileSystem, current->iNode);
        char *block = loadBlock(fileSystem, iNodePtr->directBlocks[0]);
        dirEntry *parent = malloc(sizeof(dirEntry));
        memcpy(parent, block, sizeof(dirEntry));

        char *name = loadName(fileSystem, current->iNode);
        strncpy(path[filled], name, 12);
        free(name);
        current = parent;
        filled++;
        free(iNodePtr);
    }

    for (int i = filled - 1; i >= 0; i--) {
        printf("/%s", path[i]);
    }

    printf("\n");
    for (int i = 0; i < 100; i++) {
        free(path[i]);
    }

    free(path);
    return 1;
}

/**
 * @brief Creates a directory in the file system.
 *
 * @param fileSystem The file system to operate on.
 * @param input The input to operate on.
 * @return 1 on success, 0 on failure.
 */
int makeDirectory(iNodeFileSystem *fileSystem, char **input) {
    if (input == NULL || input[0] == NULL) return 0;
    if (findDirectory(fileSystem, input[0]) != -1) {
        printf("EXIST\n");
        return 0;
    }

    char *dirName = getFileName(input[0]);
    unsigned int parent = findParentDir(fileSystem, input[0]);
    if (parent == -1) {
        printf("PATH NOT FOUND\n");
        return 0;
    }

    addDirectory(fileSystem, dirName, parent);
    printf("OK\n");
    return 1;
}

/**
 * @brief Removes a directory from the file system.
 *
 * @param fileSystem The file system to operate on.
 * @param input The input to operate on.
 * @return 1 on success, 0 on failure.
 */
int removeDirectory(iNodeFileSystem *fileSystem, char **input) {
    if (input == NULL || input[0] == NULL) return 0;
    int output = deleteDirectory(fileSystem, input[0]);
    if (output == 1) {
        printf("FILE NOT FOUND\n");

    } else if (output == 2) {
        printf("NOT EMPTY\n");

    } else {
        printf("OK\n");

    }
    return 1;
}

/**
 * @brief Prints the content of a file.
 *
 * @param fileSystem The file system to operate on.
 * @param input The input to operate on.
 * @return 1 on success, 0 on failure.
 */
int catFile(iNodeFileSystem *fileSystem, char **input) {
    if (input == NULL || input[0] == NULL) return 0;
    unsigned int parent = findParentDir(fileSystem, input[0]);

    if (parent == -1) {
        printf("FILE NOT FOUND\n");
        return 0;
    }

    dirEntry *entry = getDirEntry(fileSystem, parent, input[0]);
    if (entry == NULL) {
        printf("FILE NOT FOUND\n");
        return 0;
    }

    iNode *iNodePtr = loadINode(fileSystem, entry->iNode);
    char *content = getFileContent(fileSystem, entry->iNode);
    char *contentTerminator = malloc(iNodePtr->size + 1);
    strncpy(contentTerminator, content, iNodePtr->size);
    contentTerminator[iNodePtr->size] = '\0';

    printf("%s\n", contentTerminator);
    free(content);
    free(contentTerminator);
    free(iNodePtr);
    free(entry);

    return 1;
}

/**
 * @brief Prints information about a file.
 *
 * @param fileSystem The file system to operate on.
 * @param input The input to operate on.
 * @return 1 on success, 0 on failure.
 */
int printFileInformation(iNodeFileSystem *fileSystem, char **input) {
    if (input == NULL || input[0] == NULL) return 0;
    unsigned int parent = findParentDir(fileSystem, input[0]);

    if (parent == -1) {
        printf("FILE NOT FOUND\n");
        return 0;
    }

    dirEntry *entry = getDirEntry(fileSystem, parent, input[0]);
    if (entry == NULL) {
        printf("FILE NOT FOUND\n");
        return 0;
    }

    iNode *iNodePtr = loadINode(fileSystem, entry->iNode);
    int numBlocks = 0;
    int *blocks = getAllBlocks(fileSystem, entry->iNode, &numBlocks);

    printf("Name: %s - Size: %d - Inode: %d - Blocks: %d\n", entry->name, iNodePtr->size, entry->iNode, numBlocks);

    free(iNodePtr);
    free(blocks);
    free(entry);
    return 1;
}

/**
 * @brief Copies a file from the host file system to the pseudo file system.
 *
 * @param fileSystem The file system to operate on.
 * @param input The input to operate on.
 * @return 1 on success, 0 on failure.
 */
int inCopy(iNodeFileSystem *fileSystem, char **input) {
    if (input == NULL || input[0] == NULL || input[1] == NULL) return 0;

    FILE *file = fopen(input[0], "rb");
    if (file == NULL) {
        printf("FILE NOT FOUND\n");
        return 0;
    }

    unsigned int parent = findParentDir(fileSystem, input[1]);
    if (parent == -1) {
        printf("PATH NOT FOUND\n");
        return 0;
    }

    fseek(file, 0, SEEK_END);
    int size = ftell(file);
    fseek(file, 0, SEEK_SET);
    char *content = malloc(size);
    fread(content, 1, size, file);
    fclose(file);

    addFile(fileSystem, input[0], size, content, findDirectory(fileSystem, input[1]));
    free(content);
    printf("OK\n");
    return 1;
}

/**
 * @brief Copies a file from the pseudo file system to the host file system.
 *
 * @param fileSystem The file system to operate on.
 * @param input The input to operate on.
 * @return 1 on success, 0 on failure.
 */
int outCopy(iNodeFileSystem *fileSystem, char **input) {
    if (input == NULL || input[0] == NULL || input[1] == NULL) return 0;
    unsigned int parent = findParentDir(fileSystem, input[0]);
    if (parent == -1) {
        printf("FILE NOT FOUND\n");
        return 0;
    }

    dirEntry *entry = getDirEntry(fileSystem, parent, input[0]);
    if (entry == NULL) {
        return 0;
    }

    FILE *file = fopen(input[1], "wb");
    if (file == NULL) {
        printf("PATH NOT FOUND\n");
        return 0;
    }

    char *content = getFileContent(fileSystem, entry->iNode);
    iNode *iNodePtr = loadINode(fileSystem, entry->iNode);
    fwrite(content, 1, iNodePtr->size, file);

    free(iNodePtr);
    free(content);
    fclose(file);
    printf("OK\n");
    return 1;
}

/**
 * @brief Formats the file system.
 *
 * @param fileSystem The file system to operate on.
 * @param input The input to operate on.
 * @return 1 on success, 0 on failure.
 */
int format(iNodeFileSystem *fileSystem, char **input) {
    if (input == NULL || input[0] == NULL) return 0;
    char *name = fileSystem->name;
    if (fileSystem != NULL) {
        free(fileSystem);
    }

    unsigned int size = strtol(input[0], NULL, 10);
    fileSystem = createFileSystem(size * 1024 * 1024, name);
    saveNewFileSystem(fileSystem, input[1]);
    fileSystem = loadFileSystem(input[1]);
    printf("OK\n");
    return 1;
}

/**
 * @brief Loads a file system from a file.
 *
 * @param fileSystem The file system to operate on.
 * @param input The input to operate on.
 * @return 1 on success, 0 on failure.
 */
int load(iNodeFileSystem *fileSystem, char **input) {
    if (input == NULL || input[0] == NULL) return 0;
    FILE *file = fopen(input[0], "r");
    if (file == NULL) {
        return 0;
    }

    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    while ((read = getline(&line, &len, file)) != -1) {

        char **args = malloc(100);
        char *token = strtok(line, " ");
        int i = 0;

        while (token != NULL) {
            //get rid of newline if it's the last character
            if (token[strlen(token) - 1] == '\n') {
                token[strlen(token) - 1] = '\0';
            }
            args[i] = token;
            token = strtok(NULL, " ");
            i++;
        }
        chooseOption(args, fileSystem);
    }
    fclose(file);
    return 1;
}

/**
 * @brief Combines two files into one.
 *
 * @param fileSystem The file system to operate on.
 * @param input The input to operate on.
 * @return 1 on success, 0 on failure.
 */
int combineFiles(iNodeFileSystem *fileSystem, char **input) {
    if (input == NULL || input[0] == NULL || input[1] == NULL || input[2] == NULL) return 0;
    int file1 = findDirectory(fileSystem, input[0]);
    int file2 = findDirectory(fileSystem, input[1]);

    if (file1 == -1 || file2 == -1) {
        printf("FILE NOT FOUND\n");
        return 0;
    }

    if (findDirectory(fileSystem, input[2]) != -1) {
        printf("EXIST\n");
        return 0;
    }

    unsigned int parent = findParentDir(fileSystem, input[2]);
    if (parent == -1) {
        printf("PATH NOT FOUND\n");
        return 0;
    }

    dirEntry *entry1 = getDirEntry(fileSystem, parent, input[0]);
    dirEntry *entry2 = getDirEntry(fileSystem, parent, input[1]);
    iNode *iNode1 = loadINode(fileSystem, entry1->iNode);
    iNode *iNode2 = loadINode(fileSystem, entry2->iNode);

    char *content1 = getFileContent(fileSystem, entry1->iNode);
    char *content2 = getFileContent(fileSystem, entry2->iNode);
    char *content = malloc(iNode1->size + iNode2->size);

    memcpy(content, content1, iNode1->size);
    memcpy(content + iNode1->size, content2, iNode2->size);
    addFile(fileSystem, input[2], iNode1->size + iNode2->size, content, parent);

    free(content);
    free(content1);
    free(content2);
    free(iNode1);
    free(iNode2);

    deleteFile(fileSystem, input[0]);
    deleteFile(fileSystem, input[1]);
    printf("OK\n");
    return 0;
}
/**
 * @brief Shortens a file to 3000 bytes.
 *
 * @param fileSystem The file system to operate on.
 * @param input The input to operate on.
 * @return 1 on success, 0 on failure.
 */
int shorten(iNodeFileSystem *fileSystem, char **input) {
    if (input == NULL || input[0] == NULL || input[1] == NULL) return 0;
    int file = findDirectory(fileSystem, input[0]);
    if (file == -1) {
        printf("FILE NOT FOUND\n");
        return 0;
    }

    int maxLen = 3000;
    iNode *iNodePtr = loadINode(fileSystem, file);
    int toClean = iNodePtr->size / fileSystem->blockSize;

    if (iNodePtr->size % fileSystem->blockSize != 0) {
        toClean++;
    }
    toClean -= 3;
    if (iNodePtr->size > maxLen) {
        printf("FILE TOO BIG\n");

        char *block = loadBlock(fileSystem, iNodePtr->directBlocks[2]);
        memset(block + maxLen - 2 * fileSystem->blockSize, 0, 3 * fileSystem->blockSize - maxLen);
        saveBlock(fileSystem, block, iNodePtr->directBlocks[2]);

        for (int i = 3; i < 5; i++) {
            iNodePtr->directBlocks[i] = -1;
            markBlockFree(fileSystem, iNodePtr->directBlocks[i]);
            toClean--;
        }

        int *indirectBlock = (int *) loadBlock(fileSystem, iNodePtr->indirectBlock);

        for (int i = 0; i < fileSystem->blockSize / sizeof(int); i++) {
            if (toClean == 0) {
                break;
            }

            indirectBlock[i] = -1;
            markBlockFree(fileSystem, indirectBlock[i]);
            toClean--;
        }

        saveBlock(fileSystem, (char *) indirectBlock, iNodePtr->indirectBlock);
        int *doubleIndirectBlock = (int *) loadBlock(fileSystem, iNodePtr->doubleIndirectBlock);

        for (int i = 0; i < fileSystem->blockSize / sizeof(int); i++) {
            if (toClean == 0) {
                break;
            }

            int *indirectBlock = (int *) loadBlock(fileSystem, doubleIndirectBlock[i]);
            for (int j = 0; j < fileSystem->blockSize / sizeof(int); j++) {
                if (toClean == 0) {
                    break;
                }

                indirectBlock[j] = -1;
                markBlockFree(fileSystem, indirectBlock[j]);
                toClean--;
            }

            saveBlock(fileSystem, (char *) indirectBlock, doubleIndirectBlock[i]);
        }
        saveBlock(fileSystem, (char *) doubleIndirectBlock, iNodePtr->doubleIndirectBlock);
        iNodePtr->indirectBlock = -1;
        iNodePtr->doubleIndirectBlock = -1;
        iNodePtr->size = maxLen;
        saveINode(fileSystem, iNodePtr, file);
        return 0;
    }
    return 0;
}
