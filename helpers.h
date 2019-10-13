#ifndef __HELPERS_H__
#define __HELPERS_H__

int max(int x, int y);
int comparator(const void *p, const void *q);
int parseCommand(char *buffer, char *commandArgs[]);
int isValidTopic(char *topic);
int isValidId(char *userId);
int isnumber(char *number);
bool fileExists(char *filename);

#endif