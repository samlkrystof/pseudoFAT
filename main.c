#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

}



int main() {
    read_input();
//    printf("%s", input[0]);
//print greetings to the user

    return 0;
}
