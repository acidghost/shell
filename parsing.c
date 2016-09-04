#include "sh.h"
#include "parsing.h"
#include "utils.h"


char whitespace[] = " \t\r\n\v";
char symbols[] = "<|>";



struct cmd*
execcmd(void)
{
  struct execcmd *cmd;

  cmd = malloc(sizeof(*cmd));
  memset(cmd, 0, sizeof(*cmd));
  cmd->type = ' ';
  return (struct cmd*)cmd;
}

struct cmd*
redircmd(struct cmd *subcmd, char *file, int type)
{
  struct redircmd *cmd;

  cmd = malloc(sizeof(*cmd));
  memset(cmd, 0, sizeof(*cmd));
  cmd->type = type;
  cmd->cmd = subcmd;
  cmd->file = file;
  cmd->mode = (type == '<') ?  O_RDONLY : O_WRONLY|O_CREAT|O_TRUNC;
  cmd->fd = (type == '<') ? 0 : 1;
  return (struct cmd*)cmd;
}

struct cmd*
pipecmd(struct cmd *left, struct cmd *right)
{
  struct pipecmd *cmd;

  cmd = malloc(sizeof(*cmd));
  memset(cmd, 0, sizeof(*cmd));
  cmd->type = '|';
  cmd->left = left;
  cmd->right = right;
  return (struct cmd*)cmd;
}

int
gettoken(char **ps, char *es, char **q, char **eq)
{
  char *s;
  int ret;

  s = *ps;
  while(s < es && strchr(whitespace, *s))
    s++;
  if(q)
    *q = s;
  ret = *s;
  switch(*s){
  case 0:
    break;
  case '|':
  case '<':
    s++;
    break;
  case '>':
    s++;
    break;
  default:
    ret = 'a';
    while(s < es && !strchr(whitespace, *s) && !strchr(symbols, *s))
      s++;
    break;
  }
  if(eq)
    *eq = s;

  while(s < es && strchr(whitespace, *s))
    s++;
  *ps = s;
  return ret;
}

int
peek(char **ps, char *es, char *toks)
{
  char *s;

  s = *ps;
  while(s < es && strchr(whitespace, *s))
    s++;
  *ps = s;
  return *s && strchr(toks, *s);
}

struct cmd *parseline(char**, char*);
struct cmd *parsepipe(char**, char*);
struct cmd *parseexec(char**, char*);

// make a copy of the characters in the input buffer, starting from s through es.
// null-terminate the copy to make it a string.
char
*mkcopy(char *s, char *es)
{
  int n = es - s;
  char *c = malloc(n+1);
  assert(c);
  strncpy(c, s, n);
  c[n] = 0;
  return c;
}

struct cmd*
parsecmd(char *s)
{
  char *es;
  struct cmd *cmd;

  es = s + strlen(s);
  cmd = parseline(&s, es);
  peek(&s, es, "");
  if(s != es){
    fprintf(stderr, "leftovers: %s\n", s);
    exit(-1);
  }
  return cmd;
}

struct cmd*
parseline(char **ps, char *es)
{
  struct cmd *cmd;
  cmd = parsepipe(ps, es);
  return cmd;
}

struct cmd*
parsepipe(char **ps, char *es)
{
  struct cmd *cmd;

  cmd = parseexec(ps, es);
  if(peek(ps, es, "|")){
    gettoken(ps, es, 0, 0);
    cmd = pipecmd(cmd, parsepipe(ps, es));
  }
  return cmd;
}

struct cmd*
parseredirs(struct cmd *cmd, char **ps, char *es)
{
  int tok;
  char *q, *eq;

  while(peek(ps, es, "<>")){
    tok = gettoken(ps, es, 0, 0);
    if(gettoken(ps, es, &q, &eq) != 'a') {
      fprintf(stderr, "missing file for redirection\n");
      exit(-1);
    }
    switch(tok){
    case '<':
      cmd = redircmd(cmd, mkcopy(q, eq), '<');
      break;
    case '>':
      cmd = redircmd(cmd, mkcopy(q, eq), '>');
      break;
    }
  }
  return cmd;
}

struct cmd*
parseexec(char **ps, char *es)
{
  char *q, *eq;
  int tok, argc;
  struct execcmd *cmd;
  struct cmd *ret;

  ret = execcmd();
  cmd = (struct execcmd*)ret;

  argc = 0;
  ret = parseredirs(ret, ps, es);
  while(!peek(ps, es, "|")){
    if((tok=gettoken(ps, es, &q, &eq)) == 0)
      break;
    if(tok != 'a') {
      fprintf(stderr, "syntax error\n");
      exit(-1);
    }
    cmd->argv[argc] = mkcopy(q, eq);
    argc++;
    if(argc >= MAXARGS) {
      fprintf(stderr, "too many args\n");
      exit(-1);
    }
    ret = parseredirs(ret, ps, es);
  }
  cmd->argv[argc] = 0;
  return ret;
}


char*
cmdtostr(struct cmd* cmd)
{
  if (!cmd)
    return NULL;

  struct execcmd *ecmd;
  struct redircmd *rcmd;
  struct pipecmd *pcmd;

  size_t stri = 0;
  char *str = malloc(stri);

  switch (cmd->type) {
  case ' ':
    ecmd = (struct execcmd *) cmd;
    for (size_t i = 0; i < MAXARGS; i++) {
      char *arg = ecmd->argv[i];
      if (arg == NULL)
        break;

      char* argend = strchr(arg, '\0');
      if (argend != NULL) {
        str = realloc(str, stri + strlen(arg) + (i != 0 ? 1 : 0));
        if (i != 0)
          str[stri++] = ' ';
        strcpy(str + stri, arg);
        stri += strlen(arg);
      } else {
        sh_error("argv does not terminate with '\\0'", -1);
      }
    }
    break;

  case '<':
  case '>':
    rcmd = (struct redircmd *) cmd;
    char *subcmdstr = cmdtostr(rcmd->cmd);
    size_t subcmdlen = strlen(subcmdstr);
    size_t filelen = strlen(rcmd->file);
    str = realloc(str, subcmdlen + 3 + filelen);
    strcpy(str, subcmdstr);
    stri += subcmdlen;
    sprintf(str + stri, " %c ", rcmd->type);
    stri += 3;
    strcpy(str + stri, rcmd->file);
    stri += filelen;
    break;

  case '|':
    pcmd = (struct pipecmd *) cmd;
    char *leftcmdstr = cmdtostr(pcmd->left);
    char *rightcmdstr = cmdtostr(pcmd->right);
    size_t leftlen = strlen(leftcmdstr);
    size_t rightlen = strlen(rightcmdstr);
    str = realloc(str, leftlen + 3 + rightlen);
    strcpy(str, leftcmdstr);
    stri += leftlen;
    sprintf(str + stri, " %c ", pcmd->type);
    stri += 3;
    strcpy(str + stri, rightcmdstr);
    stri += rightlen;
    break;

  default:
    sh_error("unhandled command type in cmdtostr\n", -1);
  }

  str[++stri] = '\0';
  return str;
}
