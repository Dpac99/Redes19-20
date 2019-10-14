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

int sizeOfNumber(int n) {
  if (n < 10) {
    return 1;
  } else {
    return 1 + sizeOfNumber(n / 10);
  }
}

int parseCommand(char *buffer, char *commandArgs[]) {
  char *token;
  int i = 0;
  token = strtok(buffer, " ");

  i=0;
  while ((token != NULL) && (i < COMMANDS)) {
    if (strlen(token) <= ARG_SIZE) {
      strcpy(commandArgs[i], token);
      i++;
      token = strtok(NULL, " ");
    } else {
      return INVALID;
    }
  }
  if ((i >= COMMANDS) && (token != NULL)) {
    return INVALID;
  } else {
    return VALID;
  }
}

int isValidTopic(char *topic) {
  if (strlen(topic) <= TOPIC_SIZE) {
    return VALID;
  } else {
    return INVALID;
  }
}

int isValidId(char *userId) {
  if (strlen(userId) == USER_ID_SIZE) {
    return VALID;
  } else {
    return INVALID;
  }
}

int isnumber(char *number) {
  int i, size;
  size = strlen(number);
  for (i = 0; i < size; i++) {
    if (!isdigit(number[i]) && (number[i] != '\0')) {
      return INVALID;
    }
  }
  return atoi(number);
}

bool fileExists(char *filename) {
  struct stat buff;
  return (stat(filename, &buff) == 0);
}