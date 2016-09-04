#ifndef _H_PARSING_
#define _H_PARSING_

#define MAX_CMD_STR 8192

struct cmd *parsecmd(char*);
char *cmdtostr(struct cmd*);

#endif
