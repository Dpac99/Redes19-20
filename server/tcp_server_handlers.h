#ifndef __TCP_SERVER_HANDLERS_H__
#define __TCP_SERVER_HANDLERS_H__

int parseGetQuestion(char *info, char *topic, char *question);
int handleGetQuestion(int fd);
int parseSubmitQuestion(char *info, int *id, char *topic, char *question,
                        int *size, char *data, int *qIMG, char *ext, int *iSize,
                        char *iData);
int handleSubmitQuestion(int fd);
int parseSubmitAnswer(char *info, int *id, char *topic, char *question,
                      int *size, char *data, int *aIMG, char *ext, int *iSize,
                      char *iData);
int handleSubmitAnswer(int fd);

#endif