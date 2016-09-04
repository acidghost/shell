#include "sh.h"
#include "history.h"
#include "parsing.h"
#include "utils.h"



struct termios newt;
struct hist history;


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
    fprintf(stdout, PROMPT);
  memset(buf, 0, nbuf);

  int c;
  size_t i = 0;
  size_t histi = 0;
  while (i < nbuf && (c = getchar()) != '\n' && c != EOF) {
    buf[i] = (char) c;
    if (i > 0 && buf[i] == newt.c_cc[VERASE]) {
      // erase char
      buf[i] = 0; buf[i--] = 0;
      fprintf(stdout, "\b \b");
    } else if (buf[i] == '\033') {
      // esaped sequence (eg. arrows)
      int cs[2];
      cs[0] = getchar();
      cs[1] = getchar();
      if (cs[0] == '[' && cs[1] == 'A') {
        // arrow up
        struct cmd* cmd = history_get(&history, histi++);
        if (cmd) {
          const char *cmdstr = cmdtostr(cmd);
          fprintf(stdout, "\r"PROMPT"%s", cmdstr);
          strcpy(buf, cmdstr);
          i += strlen(cmdstr);
        } else {
          fprintf(stdout, "\r"PROMPT);
        }
      }
    } else {
      fprintf(stdout, "%c", c);
      i++;
    }
  }
  fprintf(stdout, "\n");

  if(buf[0] == 0) // EOF
    return -1;
  return 0;
}


int
main(void)
{
  struct termios oldt;
  // retrieve old tty configs
  tcgetattr(STDIN_FILENO, &oldt);
  newt = oldt;
  // change flags
  newt.c_lflag &= ~ICANON;
  newt.c_lflag &= ~ECHO;
  // set new configs now
  tcsetattr(STDIN_FILENO, TCSANOW, &newt);

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
    struct cmd* cmd = parsecmd(buf);
    if(fork1() == 0)
      runcmd(cmd);
    if (cmd != 0)
      history_push(&history, cmd);
    wait(&r);
  }

  // restore old configs
  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);

  exit(EXIT_SUCCESS);
}
