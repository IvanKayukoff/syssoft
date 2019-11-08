#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CMD_BUFFER_SIZE 256
#define RES_BUFFER_SIZE 512

int main(int argc, char *argv[]) {

  char *cmd = (char *) calloc(CMD_BUFFER_SIZE, sizeof(char));
  char *arg = (char *) calloc(CMD_BUFFER_SIZE, sizeof(char));

  for (int i = 1; i < argc; ++i) {
    strcat(cmd, argv[i]);
    strcat(cmd, " ");
  }

  char *result = (char *) malloc(RES_BUFFER_SIZE);
  while (scanf("%s", arg) != EOF) {
    memset(result, 0, RES_BUFFER_SIZE);
    strcpy(result, cmd);
    strcat(result, " ");
    strcat(result, arg);
    system(result);
  }

  free(result);
  free(arg);
  free(cmd);
  return 0;
}
