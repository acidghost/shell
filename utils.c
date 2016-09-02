#include "sh.h"
#include "utils.h"


inline int
fork1(void)
{
  int pid;

  pid = fork();
  if(pid == -1)
    perror("fork");
  return pid;
}

inline void
close1(int fd)
{
  if (close(fd) == -1)
    sh_error();
}

inline int
dup21(int fd1, int fd2)
{
  int r;
  if ((r = dup2(fd1, fd2)) == -1)
    sh_error();
  return r;
}
