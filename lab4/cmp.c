#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>

static void usage();
static void error(char const *reason);
static int cmp(size_t *line_number, size_t *byte_number);
static int cmp_blocks(size_t block0, size_t block1, size_t *line_number, size_t *byte_number);
static size_t count_set_bits(size_t n);
static void print_result(int status, size_t const *line_number, size_t const *byte_number);
static int cmp_blocks_with_eof(size_t block0, size_t block1, size_t const block_size[],
    size_t *line_number, size_t *byte_number);

static char const *program_name = "cmp";
static char const *program_version = "0.1";
static char const *filename[2];
static int file_desc[2];
static size_t file_offset[2];

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

  filename[0] = argv[optind++];
  filename[1] = optind < argc ? argv[optind++] : "-";

  // TODO use endptr instead of NULL to parse the suffix and check irregular input
  file_offset[0] = optind < argc ? strtoul(argv[optind++], NULL, 10) : 0;
  file_offset[1] = optind < argc ? strtoul(argv[optind++], NULL, 10) : 0;

  if (optind < argc) {
    error("extra operands");
  }

  struct stat stat_buf[2];
  for (int i = 0; i < 2; ++i) {
    if (i && strcmp(filename[0], filename[1]) == 0 && file_offset[0] == file_offset[1]) {
      exit(0);
    }

    file_desc[i] = (strcmp(filename[i], "-") != 0 ? open(filename[i], O_RDONLY) : STDIN_FILENO);

    if (file_desc[i] < 0 || fstat(file_desc[i], &stat_buf[i]) != 0) {
      if (file_desc[i] < 0 && comparison_type != type_status_only) {
        fprintf(stderr, "%s: inaccessible file '%s'", program_name, filename[i]);
      }
      exit(2);
    }
  }

  if (stat_buf[0].st_ino == stat_buf[1].st_ino && file_offset[0] == file_offset[1]) {
    exit(0);
  }

  size_t line_number = 1;
  size_t byte_number = file_offset[0] + 1;
  int status = cmp(&line_number, &byte_number);

  print_result(status, &line_number, &byte_number);

  for (int i = 0; i < 2; ++i) {
    if (close(file_desc[i]) != 0) {
      if (status != 0 && comparison_type != type_status_only) {
        fprintf(stderr, "%s: could not close file '%s'", program_name, filename[i]);
      }
    }
  }

  return status;
}

static void print_result(int status, size_t const *line_number, size_t const *byte_number) {
  if (comparison_type != type_status_only && status != 0) {
    printf("%s %s differ: byte %lu, line %lu\n", filename[0], filename[1], *byte_number, *line_number);
  }
}

/**
 * Compares files content, with FILE_DECS descriptors, byte by byte.
 * @param line_number a number of lines which are identical.
 * @param byte_number a number of bytes which are identical.
 * @return 0 if they are identical, otherwise - 1.
 */
static int cmp(size_t *line_number, size_t *byte_number) {
  size_t buffer[2];

  for (int i = 0; i < 2; ++i) {
    if (lseek(file_desc[i], file_offset[i], SEEK_SET) == -1L) {
      fprintf(stderr, "%s: error while reading file '%s'", program_name, filename[i]);
      exit(2);
    }
  }

  while (1) {
    long bytes_read[2];
    int eof_reached = 0;
    for (int i = 0; i < 2; ++i) {
      bytes_read[i] = read(file_desc[i], buffer + i, sizeof(long));
      if (bytes_read[i] == -1) {
        fprintf(stderr, "%s: error while reading file '%s'", program_name, filename[i]);
        exit(2);
      }
      if (bytes_read[i] == 0) {
        return 0; // the files are identical
      }
      if (bytes_read[i] < (long) sizeof(long)) {
        ((char *)(buffer + i))[bytes_read[i]] = '\0';
        eof_reached = 1;
      }
    }

    int cmp_res = eof_reached
        ? cmp_blocks_with_eof(buffer[0], buffer[1], (size_t *) bytes_read, line_number, byte_number)
        : cmp_blocks(buffer[0], buffer[1], line_number, byte_number);
    if (cmp_res != -1) {
      return 1;
    } else if (eof_reached) {
      return 0;
    }
  }
}

/**
 * Compares two strings and finds the first different byte.
 * Make sure the blocks do not contain EOF before calling this function.
 * NOTE: the function does not care if the blocks contain '\0' symbols,
 * the function treats them as regular symbols.
 * @return index of the first difference or -1 if the blocks are identical.
 */
// TODO use page of memory instead of single block
static int cmp_blocks(size_t block0, size_t block1, size_t *line_number, size_t *byte_number) {
  size_t magic_bits = -1;
  magic_bits = magic_bits / 0xFF * 0xFE << 1 >> 1 | 1;

  unsigned char newline_char = '\n';
  size_t newline_mask = newline_char | (newline_char << 8);
  newline_mask |= newline_mask << 16;
  if (sizeof (size_t) > 4) {
    newline_mask |= (newline_mask << 16) << 16;
  }

  /* The blocks identity test  */
  size_t identity_test_res = (((block0 ^ block1) + magic_bits) ^ ~(block0 ^ block1)) & magic_bits;
  if (identity_test_res == 0) {
    *byte_number += sizeof(size_t);

    /* Test block0 for '\n' chars  */
    size_t newline_applied = (((block0 ^ newline_mask) + magic_bits) ^ ~(block0 ^ newline_mask)) & ~magic_bits;
    if (newline_applied != 0) {
      *line_number += count_set_bits(newline_applied);
    }
  } else {
    unsigned char *cp0 = (unsigned char *) &block0;
    unsigned char *cp1 = (unsigned char *) &block1;
    for (size_t i = 0; i < sizeof(size_t); ++i) {
      if (cp0[i] != cp1[i]) {
        return i;
      } else if (cp0[0] == '\n') {
        *line_number += 1;
      }
      *byte_number += 1;
    }
  }
  return -1;
}

/**
 * Compares two strings and finds the first different byte.
 * Assumes that one of the blocks contains EOF symbol at 'amount' position.
 * NOTE: the function does not care if the blocks contain '\0' symbols,
 * the function treats them as regular symbols.
 * @param block_size array with number of meaningful bytes of each block.
 * @return index of the first difference or -1 if the blocks are identical.
 */
static int cmp_blocks_with_eof(size_t block0, size_t block1, size_t const block_size[],
    size_t *line_number, size_t *byte_number) {

  size_t amount = block_size[0] > block_size[1] ? block_size[1] : block_size[0];
  unsigned char *s0 = (unsigned char *) &block0;
  unsigned char *s1 = (unsigned char *) &block1;

  for (size_t i = 0; i < amount; ++i) {
    if (s0[i] == s1[i]) {
      *byte_number += 1;
      *line_number += s0[i] == '\n' ? 1 : 0;
    } else {
      return i;
    }
  }

  return block_size[0] == block_size[1] ? -1 : (int) amount;
}

static size_t count_set_bits(size_t n) {
  size_t count = 0;
  while (n) {
    count += n & 1UL;
    n >>= 1UL;
  }
  return count;
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

  printf("If a FILE is '-' or missing, read standard input.\n");
  printf("Exit status is 0 if inputs are the same, 1 if different, 2 if trouble.\n");
}

static void error(char const *reason) {
  if (reason != NULL) {
    fprintf(stderr, "%s: %s\n", program_name, reason);
  } else {
    fprintf(stderr, "Try '%s --help' for more information.\n", program_name);
  }
  exit(2);
}
