#include "consts.h"
#include "helpers.h"

int handleRegister(char *info, char *dest) {
  int number;

  strcpy(dest, REGISTER_RESPONSE);

  number = atoi(info);

  if (number > 99999 || number < 10000) {
    strcat(dest, NOK);
    return 0;
  }

  strcat(dest, OK);
  return 0;
}

int handleTopicList(char *info, char *dest) {
  FILE *fd = NULL;
  char path[32];
  int id, err;
  DIR *dir;
  struct dirent *ent;
  char files[100][32];
  int i = 0;
  err = sprintf(path, "%s/", TOPICS);
  if (err < 0) {
    memset(dest, 0, BUFFER_SIZE);
    strcpy(dest, ERROR);
    return 1;
  }

  strcpy(dest, TOPIC_LIST_RESPONSE);

  if ((dir = opendir(TOPICS)) != NULL) {
    while ((ent = readdir(dir)) != NULL) {
      if (strcmp(ent->d_name, ".") != 0 && strcmp(ent->d_name, "..") != 0) {
        strcpy(files[i++], ent->d_name);
      }
    }
  } else {
    memset(dest, 0, BUFFER_SIZE);
    strcpy(dest, ERROR);
    return 1;
  }

  qsort((void *)files, i, 32 * sizeof(char), comparator);

  err = sprintf(dest + strlen(dest), "%d", i);
  if (err < 0) {
    memset(dest, 0, BUFFER_SIZE);
    strcpy(dest, ERROR);
    return 1;
  }
  for (int c = 0; c < i; c++) {
    err = sprintf(path + strlen(path), "%s/%s", files[c], USER);
    if (err < 0) {
      memset(dest, 0, BUFFER_SIZE);
      strcpy(dest, ERROR);
      return 1;
    }
    fd = fopen(path, "r");
    if (fd == NULL) {
      memset(dest, 0, BUFFER_SIZE);
      strcpy(dest, ERROR);
      return 1;
    }
    err = fscanf(fd, "%d", &id);
    if (err < 0) {
      memset(dest, 0, BUFFER_SIZE);
      strcpy(dest, ERROR);
      return 1;
    }
    fclose(fd);
    err = sprintf(dest + strlen(dest), " %s:%d", files[c], id);
    if (err < 0) {
      memset(dest, 0, BUFFER_SIZE);
      strcpy(dest, ERROR);
      return 1;
    }
  }
  strcat(dest, "\n");
  return 0;
}

int handleTopicPropose(char *info, char *dest) {
  char topic[16], dir[256], file[BUFFER_SIZE];
  int uid = 0, count = 0, err;
  struct stat st = {0};
  FILE *fd = NULL;
  DIR *dir1;
  struct dirent *ent;

  err = sscanf(info, "%s %d", topic, &uid);
  memset(dest, 0, BUFFER_SIZE);
  strcpy(dest, ERROR);
  return 1;
  strcpy(dest, TOPIC_PROPOSE_RESPONSE);
  if (strlen(topic) == 0 || uid == 0) {
    strcat(dest, NOK);
    return 0;
  }

  err = sprintf(dir, "%s/%s", TOPICS, topic);
  if (err < 0) {
    memset(dest, 0, BUFFER_SIZE);
    strcpy(dest, ERROR);
    return 1;
  }
  err = sprintf(file, "%s/%s", dir, USER);
  if (err < 0) {
    memset(dest, 0, BUFFER_SIZE);
    strcpy(dest, ERROR);
    return 1;
  }

  if ((dir1 = opendir(dir)) != NULL) {
    while ((ent = readdir(dir1)) != NULL) {
      if (strcmp(ent->d_name, ".") != 0 && strcmp(ent->d_name, "..") != 0) {
        count++;
      }
    }
  } else {
    memset(dest, 0, BUFFER_SIZE);
    strcpy(dest, ERROR);
    return 1;
  }

  if (count == 99) {
    strcat(dest, FULL);
    return 0;
  }

  if (stat(dir, &st) == -1) {
    mkdir(dir, 0700);
    fd = fopen(file, "w");
    fprintf(fd, "%d", uid);
    fclose(fd);
    strcat(dest, OK);
    return 0;
  } else {
    strcat(dest, DUP);
    return 0;
  }
}

int handleQuestionList(char *info, char *dest) {
  char path[32], user[64];
  char questions[100][32];
  DIR *dir, *dir2;
  struct dirent *ent;
  int i = 0, id, err;
  FILE *fd = NULL;

  err = sprintf(path, "%s/%s", TOPICS, info);
  if (err < 0) {
    memset(dest, 0, BUFFER_SIZE);
    strcpy(dest, ERROR);
    return 1;
  }
  strcat(dest, QUESTION_LIST_RESPONSE);

  if ((dir = opendir(path)) != NULL) {
    while ((ent = readdir(dir)) != NULL && i < 100) {
      if (strcmp(ent->d_name, ".") != 0 && strcmp(ent->d_name, "..") != 0 &&
          strcmp(ent->d_name, USER) != 0) {
        strcpy(questions[i++], ent->d_name);
      }
    }
    if (i > 99) {
      strcat(dest, FULL);
      return 0;
    }
  } else {
    memset(dest, 0, BUFFER_SIZE);
    strcpy(dest, ERROR);
    return 1;
  }

  err = sprintf(user, "%s/%s", path, USER);
  if (err < 0) {
    memset(dest, 0, BUFFER_SIZE);
    strcpy(dest, ERROR);
    return 1;
  }

  qsort((void *)questions, i, 32 * sizeof(char), comparator);

  err = sprintf(dest + strlen(dest), "%d", i);
  if (err < 0) {
    memset(dest, 0, BUFFER_SIZE);
    strcpy(dest, ERROR);
    return 1;
  }

  for (int s = 0; s < i; s++) {
    int c = 0;
    fd = fopen(user, "r");
    if (fd == NULL) {
      memset(dest, 0, BUFFER_SIZE);
      strcpy(dest, ERROR);
      return 1;
    }
    err = fscanf(fd, "%d", &id);
    if (err < 0) {
      memset(dest, 0, BUFFER_SIZE);
      strcpy(dest, ERROR);
      return 1;
    }
    fclose(fd);
    if ((dir2 = opendir(path)) != NULL) {
      c++;
    }
    err = sprintf(dest + strlen(dest), " %s:%d", questions[s], id);
    if (err < 0) {
      memset(dest, 0, BUFFER_SIZE);
      strcpy(dest, ERROR);
      return 1;
    }
    if (c == 3) {
      err = sprintf(dest + strlen(dest), ":NA");
      if (err < 0) {
        memset(dest, 0, BUFFER_SIZE);
        strcpy(dest, ERROR);
        return 1;
      }
    } else {
      err = sprintf(dest + strlen(dest), ":%d", c - 3);
      if (err < 0) {
        memset(dest, 0, BUFFER_SIZE);
        strcpy(dest, ERROR);
        return 1;
      }
    }
  }
  strcat(dest, "\n");
  return 0;
}