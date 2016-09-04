#ifndef _H_HISTORY
#define _H_HISTORY_

#define MAX_HISTORY 100

struct hist {
  size_t size;
  struct cmd* cmds[MAX_HISTORY];
};

void history_push(struct hist*, struct cmd*);
struct cmd* history_get(struct hist*, size_t);
void history_print(struct hist*);

#endif
