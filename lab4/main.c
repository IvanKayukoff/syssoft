#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[]) {

    printf("Arguments: \n");
    for (int i = 1; i < argc; ++i) {
        printf("\t%s\n", argv[i]);
    }
    if (argc < 2) {
        printf("cmp: missing operand after 'cmp'\n");
        exit(EXIT_FAILURE);
    }


    return 0;
}

