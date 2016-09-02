#include "sh.h"
#include "parsing.h"
#include "utils.h"



// Execute cmd.  Never returns.
void
runcmd(struct cmd *cmd)
{
  struct execcmd *ecmd;
  struct pipecmd *pcmd;
  struct redircmd *rcmd;
  int fd;

  if(cmd == 0)
    exit(0);

  switch(cmd->type){
  default:
    sh_error("unknown runcmd", -1);

  case ' ':
    ecmd = (struct execcmd*)cmd;
    if(ecmd->argv[0] == 0)
      exit(0);
    if (execvp(ecmd->argv[0], ecmd->argv) == -1)
      sh_error();
    break;

  case '>':
  case '<':
    rcmd = (struct redircmd*)cmd;
    fd = (rcmd->mode & O_CREAT) ?
                open(rcmd->file, rcmd->mode, 0644) :
                open(rcmd->file, rcmd->mode);
    if (fd == -1 || dup2(fd, rcmd->fd) == -1 || close(fd) == -1)
      sh_error();
    runcmd(rcmd->cmd);
    break;

  case '|':
    pcmd = (struct pipecmd*)cmd;
    int p[2];
    // create pipe
    if (pipe(p) == -1)
      sh_error();
    // fork process
    switch (fork()) {
    case -1:
      sh_error("error forking while piping", -1);
    // child process
    case 0:
      fd = dup21(p[0], 0);
      close1(p[1]);
      runcmd(pcmd->right);
      break;
    // parent process
    default:
      fd = dup21(p[1], 1);
      close1(p[0]);
      runcmd(pcmd->left);
    }
    close1(fd);
    break;
  }
  exit(0);
}

int
getcmd(char *buf, int nbuf)
{

  if (isatty(fileno(stdin)))
    fprintf(stdout, "$ ");
  memset(buf, 0, nbuf);
  fgets(buf, nbuf, stdin);
  if(buf[0] == 0) // EOF
    return -1;
  return 0;
}

int
main(void)
{
  static char buf[100];
  int r;

  // Read and run input commands.
  while(getcmd(buf, sizeof(buf)) >= 0){
    if(buf[0] == 'c' && buf[1] == 'd' && buf[2] == ' '){
      // Clumsy but will have to do for now.
      // Chdir has no effect on the parent if run in the child.
      buf[strlen(buf)-1] = 0;  // chop \n
      if(chdir(buf+3) < 0)
        fprintf(stderr, "cannot cd %s\n", buf+3);
      continue;
    }
    if(fork1() == 0)
      runcmd(parsecmd(buf));
    wait(&r);
  }
  exit(0);
}
