#ifndef __HELPERS_H__
#define __HELPERS_H__

#include "consts.h"

int max(int x, int y);
int comparator(const void *p, const void *q);
int sizeOfNumber(int a);
int isnumber(char *number);
int isValidId(char *userId);
int isValidTopic(char *topic);
int parseCommand(char *buffer, char *commandArgs[]);
bool fileExists(char *filename);
char *copyFile(char *filename);
void shiftLeft(char *string, int shiftLength);
void deleteDir(const char path[]);
void checkFileContent(struct Submission *submission);

#endif