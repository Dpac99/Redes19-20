#include "../others/consts.h"
#include "../others/helpers.h"

int handleRegister(char *info, char *dest) {
  int number;

  number = atoi(info);

  if (number > 99999 || number < 10000) {
    sprintf(dest, "%s %s\n", REGISTER_RESPONSE, NOK);
    return 0;
  }

  sprintf(dest, "%s %s\n", REGISTER_RESPONSE, OK);
  return 0;
}

int handleTopicList(char *info, char *dest) {
  FILE *fd = NULL;
  char path[128];
  int id, err;
  DIR *dir;
  struct dirent *ent;
  char files[100][32];
  int i = 0;

  sprintf(dest, "%s ", TOPIC_LIST_RESPONSE);

  if ((dir = opendir(TOPICS)) != NULL) {
    while ((ent = readdir(dir)) != NULL) {
      if (strcmp(ent->d_name, ".") != 0 && strcmp(ent->d_name, "..") != 0) {
        strcpy(files[i++], ent->d_name);
      }
    }
  } else {
    memset(dest, 0, BUFFER_SIZE);
    sprintf(dest, "%s\n", ERROR);
    return 1;
  }

  // qsort((void *)files, i, 32 * sizeof(char), comparator);

  err = sprintf(dest + strlen(dest), "%d", i);
  if (err < 0) {
    memset(dest, 0, BUFFER_SIZE);
    sprintf(dest, "%s\n", ERROR);
    return 1;
  }
  for (int c = 0; c < i; c++) {
    memset(path, 0, 128);
    err = sprintf(path, "%s/%s/%s", TOPICS, files[c], USER);
    if (err < 0) {
      memset(dest, 0, BUFFER_SIZE);
      sprintf(dest, "%s\n", ERROR);
      return 1;
    }

    fd = fopen(path, "r");
    if (fd == NULL) {
      memset(dest, 0, BUFFER_SIZE);
      sprintf(dest, "%s\n", ERROR);
      return 1;
    }
    err = fscanf(fd, "%d", &id);
    if (err < 0) {
      memset(dest, 0, BUFFER_SIZE);
      sprintf(dest, "%s\n", ERROR);
      return 1;
    }
    fclose(fd);
    err = sprintf(dest + strlen(dest), " %s:%d", files[c], id);
    if (err < 0) {
      memset(dest, 0, BUFFER_SIZE);
      sprintf(dest, "%s\n", ERROR);
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

  err = sprintf(dest, "%s ", TOPIC_PROPOSE_RESPONSE);
  if (err < 0) {
    memset(dest, 0, BUFFER_SIZE);
    sprintf(dest, "%s\n", ERROR);
    return 1;
  }

  // Read id of user and name of topic
  err = sscanf(info, "%d %s", &uid, topic);
  if (err < 0) {
    memset(dest, 0, BUFFER_SIZE);
    sprintf(dest, "%s\n", ERROR);
    return 1;
  }

  if (strlen(topic) == 0 || uid == 0) {
    sprintf(dest + strlen(dest), "%s\n", NOK);
    return 0;
  }

  err = sprintf(dir, "%s/%s", TOPICS, topic);
  if (err < 0) {
    memset(dest, 0, BUFFER_SIZE);
    sprintf(dest, "%s\n", ERROR);
    return 1;
  }
  err = sprintf(file, "%s/%s", dir, USER);
  if (err < 0) {
    memset(dest, 0, BUFFER_SIZE);
    sprintf(dest, "%s\n", ERROR);
    return 1;
  }

  if ((dir1 = opendir(TOPICS)) != NULL) {
    while ((ent = readdir(dir1)) != NULL) {
      if (strcmp(ent->d_name, ".") != 0 && strcmp(ent->d_name, "..") != 0) {
        count++;
      }
    }
  } else {
    memset(dest, 0, BUFFER_SIZE);
    sprintf(dest, "%s\n", ERROR);
    return 1;
  }

  if (count == 99) {
    sprintf(dest + strlen(dest), "%s\n", FULL);
    return 0;
  }

  if (stat(dir, &st) == -1) {
    mkdir(dir, 0700);
    fd = fopen(file, "w");
    fprintf(fd, "%d", uid);
    fclose(fd);
    sprintf(dest + strlen(dest), "%s\n", OK);
    return 0;
  } else {
    sprintf(dest + strlen(dest), "%s\n", DUP);
    return 0;
  }
}

int handleQuestionList(char *info, char *dest) {
  char path[32], user[64], topic[16];
  char questions[100][32];
  DIR *dir;
  struct dirent *ent;
  int i = 0, id, err, c;
  FILE *f = NULL;
  char question[BUFFER_SIZE / 2];
  char filename[BUFFER_SIZE];

  err = sscanf(info, "%s", topic);
  if (err < 0) {
    memset(dest, 0, BUFFER_SIZE);
    sprintf(dest, "%s\n", ERROR);
    return 1;
  }

  // Build path to topic dir
  err = sprintf(path, "%s/%s", TOPICS, topic);
  if (err < 0) {
    memset(dest, 0, BUFFER_SIZE);
    sprintf(dest, "%s\n", ERROR);
    return 1;
  }
  sprintf(dest, "%s ", QUESTION_LIST_RESPONSE);

  // Read all questions from topic
  if ((dir = opendir(path)) != NULL) {
    while ((ent = readdir(dir)) != NULL && i < 100) {
      if (strcmp(ent->d_name, ".") != 0 && strcmp(ent->d_name, "..") != 0 &&
          strcmp(ent->d_name, USER) != 0) {
        strcpy(questions[i++], ent->d_name);
      }
    }
  } else {
    memset(dest, 0, BUFFER_SIZE);
    sprintf(dest, "%s\n", ERROR);
    return 1;
  }

  err = sprintf(user, "%s/%s", path, USER);
  if (err < 0) {
    memset(dest, 0, BUFFER_SIZE);
    sprintf(dest, "%s\n", ERROR);
    return 1;
  }

  // qsort((void *)questions, i, 32 * sizeof(char), comparator);

  err = sprintf(dest + strlen(dest), "%d", i);
  if (err < 0) {
    memset(dest, 0, BUFFER_SIZE);
    sprintf(dest, "%s\n", ERROR);
    return 1;
  }

  for (int s = 0; s < i; s++) {
    c = 0;
    // path to question dir
    memset(question, 0, 16);
    err = sprintf(question, "%s/%s/%s", TOPICS, topic, questions[s]);
    if (err < 0) {
      memset(dest, 0, BUFFER_SIZE);
      sprintf(dest, "%s\n", ERROR);
      return 1;
    }
    err = sprintf(filename, "%s/%s/%s", question, DATA, USER);
    if (err < 0) {
      memset(dest, 0, BUFFER_SIZE);
      sprintf(dest, "%s\n", ERROR);
      return 1;
    }

    f = fopen(filename, "r");
    if (f == NULL) {
      memset(dest, 0, BUFFER_SIZE);
      sprintf(dest, "%s\n", ERROR);
      return 1;
    }
    err = fscanf(f, "%d", &id);
    fclose(f);
    if (err <= 0) {
      memset(dest, 0, BUFFER_SIZE);
      sprintf(dest, "%s\n", ERROR);
      return 1;
    }

    if ((dir = opendir(question)) != NULL) {
      while ((ent = readdir(dir)) != NULL && i < 100) {
        if (strcmp(ent->d_name, ".") != 0 && strcmp(ent->d_name, "..") != 0 &&
            strcmp(ent->d_name, DATA) != 0) {
          c++;
        }
      }
    } else {
      memset(dest, 0, BUFFER_SIZE);
      sprintf(dest, "%s\n", ERROR);
      return 1;
    }

    err = sprintf(dest + strlen(dest), " %s:%d:%d", questions[s], id, c);
    if (err < 0) {
      memset(dest, 0, BUFFER_SIZE);
      sprintf(dest, "%s\n", ERROR);
      return 1;
    }
  }
  strcat(dest, "\n");
  return 0;
}