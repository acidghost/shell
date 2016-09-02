#ifndef _UTILS_H_
#define _UTILS_H_

#define GET_MACRO(_0, _1, _2, NAME, ...) NAME
#define sh_error_2(errorstr, errno) { \
  fprintf(stderr, "xv6 error: %s\n", errorstr);  \
  exit(errno); \
}
#define sh_error_1(errorstr) sh_error_2(errorstr, errno)
#define sh_error_0(...) sh_error_1(strerror(errno))
#define sh_error(...) GET_MACRO(_0, ##__VA_ARGS__, sh_error_2, sh_error_1, sh_error_0)(__VA_ARGS__)


int fork1(void);        // Fork but exits on failure.
void close1(int);       // Close file and exits on failure
int dup21(int, int);    // like dup2 but exits on failure

#endif
