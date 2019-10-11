#include "consts.h"

int max(int x, int y) {
  if (x > y)
    return x;
  else
    return y;
}

int comparator(const void *p, const void *q) {
  char p_c[32], q_c[32];
  struct stat p_s = {0}, q_s = {0};
  sprintf(p_c, "%s/%s", TOPICS, (char *)p);
  sprintf(q_c, "%s/%s", TOPICS, (char *)q);
  stat(p_c, &p_s);
  stat(q_c, &q_s);

  return (int)(p_s.st_mtim.tv_sec - q_s.st_mtim.tv_sec);
}