#ifndef __HELPERS_H__
#define __HELPERS_H__

int max(int x, int y);
int comparator(const void *p, const void *q);
int sizeOfNumber(int a);
int isnumber(char *number);
int isValidId(char *userId);
int isValidTopic(char *topic);
int parseCommand(char *buffer, char *commandArgs[]);

#endif