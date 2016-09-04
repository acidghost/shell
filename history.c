#include "sh.h"
#include "history.h"



void
history_push(struct hist* history, struct cmd* cmd)
{
  // shift all by one right
  for (int i = history->size; i > 0; i--) {
    history->cmds[i] = history->cmds[i - 1];
  }
  history->cmds[0] = cmd;
  if (history->size < MAX_HISTORY)
    history->size++;
}


struct cmd*
history_get(struct hist* history, size_t index)
{
  if (index >= history->size)
    return NULL;
  return history->cmds[index];
}

void
history_print(struct hist* history)
{
  for (size_t i = 0; i < history->size; i++) {
    fprintf(stdout, "%zd: %c\n", i, history->cmds[i]->type);
  }
}
