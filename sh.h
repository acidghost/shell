#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>


// Simplifed xv6 shell.

#define MAXARGS 10


extern int errno;

#define GET_MACRO(_0, _1, _2, NAME, ...) NAME
#define sh_print_error_2(errorstr, errno) { \
  fprintf(stderr, "xv6 error: %s\n", errorstr);  \
  exit(errno); \
}
#define sh_print_error_1(errorstr) sh_print_error_2(errorstr, errno)
#define sh_print_error_0() sh_print_error_1(strerror(errno))
#define sh_print_error(...) GET_MACRO(__VA_ARGS__, sh_print_error_2, sh_print_error_1, sh_print_error_0)(__VA_ARGS__)


// All commands have at least a type. Have looked at the type, the code
// typically casts the *cmd to some specific cmd type.
struct cmd {
  int type;          //  ' ' (exec), | (pipe), '<' or '>' for redirection
};

struct execcmd {
  int type;              // ' '
  char *argv[MAXARGS];   // arguments to the command to be exec-ed
};

struct redircmd {
  int type;          // < or >
  struct cmd *cmd;   // the command to be run (e.g., an execcmd)
  char *file;        // the input/output file
  int mode;          // the mode to open the file with
  int fd;            // the file descriptor number to use for the file
};

struct pipecmd {
  int type;          // |
  struct cmd *left;  // left side of pipe
  struct cmd *right; // right side of pipe
};
