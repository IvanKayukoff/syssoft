#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>

static void usage();
static void error(char const *reason);

static char const *program_name = "cmp";
static char const *program_version = "0.1";
static char const *file_names[2];

static enum {
  type_first_diff,
  type_status_only
} comparison_type = type_first_diff;

static struct option const long_options[] = {
    {"silent",  0, NULL, 's'},
    {"quiet",   0, NULL, 's'},
    {"version", 0, NULL, 'v'},
    {"help",    0, NULL, 129},
    {NULL,      0, NULL, 0}
};


int main(int argc, char *argv[]) {
  int key = 0;
  while ((key = getopt_long (argc, argv, "sv", long_options, 0)) != EOF) {
    switch (key) {
      case 's':
        comparison_type = type_status_only;
        break;

      case 'v':
        printf("%s - version %s\n", program_name, program_version);
        exit(EXIT_SUCCESS);

      case 129:
        usage();
        exit(EXIT_SUCCESS);

      default:
        error(NULL);
    }
  }

  if (optind == argc) {
    error("missing operand");
  }

  file_names[0] = argv[optind++];
  file_names[1] = optind < argc ? argv[optind++] : "-";

  if (optind < argc) {
    error("extra operands");
  }

  return 0;
}

static void usage() {
  printf("Usage: %s [OPTION]... FILE1 [FILE2 [SKIP1 [SKIP2]]]\n", program_name);
  printf("Compare two files byte by byte.\n\n");

  printf("The optional SKIP1 and SKIP2 specify the number of bytes to skip\n");
  printf("at the beginning of each file (zero by default).\n\n");

  printf("%s", "\
  -s  --quiet  --silent  Output nothing; yield exit status only.\n\
  -v  --version  Output version info.\n\
  --help  Output this help.\n\n");

  printf("If a FILE is `-' or missing, read standard input.\n");
  printf("Exit status is 0 if inputs are the same, 1 if different, 2 if trouble.\n");
}

static void error(char const *reason) {
  if (reason != NULL) {
    fprintf(stderr, "%s: %s\n", program_name, reason);
  } else {
    fprintf(stderr, "Try '%s --help' for more information.\n", program_name);
  }
  exit(EXIT_FAILURE);
}
