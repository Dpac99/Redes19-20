#include "consts.h"

int parseGetQuestion(char *info, char *topic, char *question) {
  int infoSize = strlen(info);
  int i = 0;
  char s[2] = " ";
  char *token;
  token = strtok(info, s);
  while (token != NULL) {
    if (i == 0) {
      strcpy(topic, token);
    } else if (i == 1) {
      strcpy(question, token);
    } else {
      return 1;
    }
    i++;
  }

  if (i < 2) {
    return 2;
  }

  if (strlen(topic) + strlen(question) + 1 != infoSize) {
    return 1;
  }

  return 0;
}

int handleGetQuestion(char *info, char *dest) {
  char topic[16], question[16], path[64], filename[128], *data, ext[4];
  char answers[100][32];
  FILE *f;
  struct dirent *ent;
  DIR *dir;
  int uid, err, qIMG, i = 1, limit;
  struct stat st;
  off_t size;

  strcpy(dest, GET_QUESTION_RESPONSE);

  // Parse info
  err = parseGetQuestion(info, topic, question);
  if (err > 0) {
    if (err == 1) {
      strcat(dest, NOK);
    } else {
      strcat(dest, END_OF_FILE);
    }
    return 0;
  }

  // Build path to question dir
  err = sprintf(path, "%s/%s/%s/%s", TOPICS, topic, question, DATA);
  if (err < 0) {
    memset(dest, 0, BUFFER_SIZE);
    strcpy(dest, ERROR);
    return 1;
  }

  // Read uid of question
  err = sprintf(filename, "%s/%s", path, USER);
  if (err < 0) {
    memset(dest, 0, BUFFER_SIZE);
    strcpy(dest, ERROR);
    return 1;
  }

  if (access(filename, R_OK) == 0) {
    f = fopen(filename, "r");
    if (f == NULL) {
      memset(dest, 0, BUFFER_SIZE);
      strcpy(dest, ERROR);
      return 1;
    }

    err = fscanf(f, "%d", &uid);
    if (err == EOF) {
      fclose(f);
      memset(dest, 0, BUFFER_SIZE);
      strcpy(dest, ERROR);
      return 1;
    }
    fclose(f);
  } else {
    memset(dest, 0, BUFFER_SIZE);
    strcpy(dest, ERROR);
    return 1;
  }

  // Get question info
  err = sprintf(filename, "%s/%s", path, Q_INFO);
  if (err < 0) {
    memset(dest, 0, BUFFER_SIZE);
    strcpy(dest, ERROR);
    return 1;
  }

  if (stat(filename, &st) == 0) {

    size = st.st_size;

    data = (char *)malloc(sizeof(char) * (size + 1));
    if (data == NULL) {
      memset(dest, 0, BUFFER_SIZE);
      strcpy(dest, ERROR);
      return 1;
    }

    f = fopen(filename, "r");
    if (f == NULL) {
      memset(dest, 0, BUFFER_SIZE);
      strcpy(dest, ERROR);
      return 1;
    }

    fread(data, sizeof(char), size, f);
    if (feof(f) == 0) {
      memset(dest, 0, BUFFER_SIZE);
      strcpy(dest, ERROR);
      return 1;
    }

    fclose(f);

    // Print question info to response buffer
    err = sprintf(dest + strlen(dest), "%d %s ", (int)size, data);
    if (err < 0) {
      memset(dest, 0, BUFFER_SIZE);
      strcpy(dest, ERROR);
      return 1;
    }

    free(data);
  } else {
    memset(dest, 0, BUFFER_SIZE);
    strcpy(dest, ERROR);
    return 1;
  }

  // Check if image and extension if yes
  memset(filename, 0, strlen(filename));
  err = sprintf(filename, "%s/%s ", path, IMG_DATA);
  if (err < 0) {
    memset(dest, 0, BUFFER_SIZE);
    strcpy(dest, ERROR);
    return 1;
  }

  if (access(filename, R_OK) == 0) {
    f = fopen(filename, "r");
    if (f == NULL) {
      memset(dest, 0, BUFFER_SIZE);
      strcpy(dest, ERROR);
      return 1;
    }

    fscanf(f, "%d %s", &qIMG, ext);

    fclose(f);
  } else {
    memset(dest, 0, BUFFER_SIZE);
    strcpy(dest, ERROR);
    return 1;
  }

  // Get image

  if (qIMG == 1) {
    memset(filename, 0, 128);
    err = sprintf(filename, "%s/%s.%s", path, IMG, ext);
    if (err < 0) {
      memset(dest, 0, BUFFER_SIZE);
      strcpy(dest, ERROR);
      return 1;
    }

    if (stat(filename, &st) == 0) {

      size = st.st_size;

      data = (char *)malloc(sizeof(char) * (size + 1));
      if (data == NULL) {
        memset(dest, 0, BUFFER_SIZE);
        strcpy(dest, ERROR);
        return 1;
      }

      f = fopen(filename, "r");
      if (f == NULL) {
        memset(dest, 0, BUFFER_SIZE);
        strcpy(dest, ERROR);
        return 1;
      }

      fread(data, sizeof(char), size, f);
      if (feof(f) == 0) {
        memset(dest, 0, BUFFER_SIZE);
        strcpy(dest, ERROR);
        return 1;
      }

      fclose(f);

      err = sprintf(dest + strlen(dest), "%d [%s %d %s] ", qIMG, ext, (int)size,
                    data);
      if (err < 0) {
        memset(dest, 0, BUFFER_SIZE);
        strcpy(dest, ERROR);
        return 1;
      }

      free(data);
    } else {
      memset(dest, 0, BUFFER_SIZE);
      strcpy(dest, ERROR);
      return 1;
    }
  }

  // Get all answers and get most recent 10
  if ((dir = opendir(path)) != NULL) {

    // Read dir
    while ((ent = readdir(dir)) != NULL) {
      if (strcmp(ent->d_name, ".") != 0 && strcmp(ent->d_name, "..") != 0 &&
          strcmp(ent->d_name, DATA) != 0) {
        strcpy(answers[i - 1], ent->d_name);
        i++;
      }
    }
  } else {
    memset(dest, 0, BUFFER_SIZE);
    strcpy(dest, ERROR);
    return 1;
  }

  // Find N
  if (i > 10) {
    limit = i - 10;
    sprintf(dest + strlen(dest), "%d ", 10);
  } else {
    limit = 0;
    sprintf(dest + strlen(dest), "%d ", i);
  }

  // Last 10 (all answers are filed as answer_XX, so the last will be the most
  // recent)
  for (; i >= limit; i--) {
    memset(filename, 0, 128);
    err = sprintf(filename, "%s/%s/%s/%s/%s", TOPICS, topic, question,
                  answers[i], ANS_DATA);
    if (err < 0) {
      memset(dest, 0, BUFFER_SIZE);
      strcpy(dest, ERROR);
      return 1;
    }

    stat(filename, &st);

    size = st.st_size;

    data = (char *)malloc(sizeof(char) * (size + 1));
    if (data == NULL) {
      memset(dest, 0, BUFFER_SIZE);
      strcpy(dest, ERROR);
      return 1;
    }

    if (access(filename, R_OK) == 0) {

      f = fopen(filename, "r");
      if (f == NULL) {
        memset(dest, 0, BUFFER_SIZE);
        strcpy(dest, ERROR);
        return 1;
      }

      fread(data, sizeof(char), size, f);
      if (feof(f) == 0) {
        memset(dest, 0, BUFFER_SIZE);
        strcpy(dest, ERROR);
        return 1;
      }

      fclose(f);

      // Print answer data to dest
      err = sprintf(dest + strlen(dest), "%d %s ", (int)size, data);
      if (err < 0) {
        memset(dest, 0, BUFFER_SIZE);
        strcpy(dest, ERROR);
        return 1;
      }

      free(data);
    } else {
      memset(dest, 0, BUFFER_SIZE);
      strcpy(dest, ERROR);
      return 1;
    }

    // Check if image and extension if yes
    memset(filename, 0, strlen(filename));
    err = sprintf(filename, "%s/%s/%s/%s/%s", TOPICS, topic, question,
                  answers[i], IMG_DATA);
    if (err < 0) {
      memset(dest, 0, BUFFER_SIZE);
      strcpy(dest, ERROR);
      return 1;
    }

    if (access(filename, R_OK) == 0) {
      f = fopen(filename, "r");
      if (f == NULL) {
        memset(dest, 0, BUFFER_SIZE);
        strcpy(dest, ERROR);
        return 1;
      }

      memset(ext, 0, 4);
      fscanf(f, "%d %s", &qIMG, ext);

      fclose(f);
    } else {
      memset(dest, 0, BUFFER_SIZE);
      strcpy(dest, ERROR);
      return 1;
    }

    // Get image

    if (qIMG == 1) {
      memset(filename, 0, 128);
      err = sprintf(filename, "%s/%s.%s", path, IMG, ext);
      if (err < 0) {
        memset(dest, 0, BUFFER_SIZE);
        strcpy(dest, ERROR);
        return 1;
      }

      if (stat(filename, &st) == 0) {

        size = st.st_size;

        data = (char *)malloc(sizeof(char) * (size + 1));
        if (data == NULL) {
          memset(dest, 0, BUFFER_SIZE);
          strcpy(dest, ERROR);
          return 1;
        }

        f = fopen(filename, "r");
        if (f == NULL) {
          memset(dest, 0, BUFFER_SIZE);
          strcpy(dest, ERROR);
          return 1;
        }

        fread(data, sizeof(char), size, f);
        if (feof(f) == 0) {
          memset(dest, 0, BUFFER_SIZE);
          strcpy(dest, ERROR);
          return 1;
        }

        fclose(f);

        // Print image
        err = sprintf(dest + strlen(dest), "%d [%s %d %s] ", qIMG, ext,
                      (int)size, data);
        if (err < 0) {
          memset(dest, 0, BUFFER_SIZE);
          strcpy(dest, ERROR);
          return 1;
        }

        free(data);
      } else {
        memset(dest, 0, BUFFER_SIZE);
        strcpy(dest, ERROR);
        return 1;
      }
    }
  }

  return 0;
}

int parseSubmitQuestion(char *info, int *id, char *topic, char *question,
                        int *size, char *data, int *qIMG, char *ext, int *iSize,
                        char *iData) {

  int infoSize = strlen(info);
  char s[2] = " ";
  int i = 0, err;
  char *token, ssize[8], sid[8], sqImg[2], siSize[8];
  token = strtok(info, s);
  while (token != NULL) {
    switch (i++) {
    case 0:
      strcpy(sid, token);
      err = sscanf(token, "%d", id);
      if (err < 0) {
        return 1;
      }
      break;
    case 1:
      strcpy(topic, token);
      break;
    case 2:
      strcpy(question, token);
      break;
    case 3:
      strcpy(ssize, token);
      err = sscanf(token, "%d", size);
      if (err < 0) {
        return 1;
      }
      break;
    case 4:
      strcpy(data, token);
      break;
    case 5:
      strcpy(sqImg, token);
      err = sscanf(token, "%d", qIMG);
      if (err < 0) {
        return 1;
      }
      break;
    case 6:
      strcpy(ext, token);
      break;
    case 7:
      strcpy(siSize, token);
      err = sscanf(token, "%d", iSize);
      if (err < 0) {
        return 1;
      }
      break;
    case 8:
      strcpy(iData, token);
      break;
    default:
      break;
    }
  }

  if (i != 6 && i != 9) {
    return 1;
  }

  if (strlen(topic) + strlen(question) + strlen(sid) + strlen(ssize) +
          strlen(data) + strlen(sqImg) + strlen(ext) + strlen(siSize) +
          strlen(iData) + i - 1 !=
      infoSize) {
    return 1;
  }

  if (*qIMG != 0 && *qIMG != 1) {
    return 1;
  }

  return 0;
}

int handleSubmitQuestion(char *info, char *dest) {

  char topic[16], question[16], data[BUFFER_SIZE], ext[4], idata[BUFFER_SIZE];
  char path[BUFFER_SIZE], filename[2 * BUFFER_SIZE];
  int id, size, qIMG, iSize, err, fileN = 0;
  FILE *f;
  DIR *d;
  struct dirent *ent;

  memset(dest, 0, BUFFER_SIZE);
  strcpy(dest, SUBMIT_QUESTION_RESPONSE);

  err = parseSubmitQuestion(info, &id, topic, question, &size, data, &qIMG, ext,
                            &iSize, idata);
  if (err > 0) {
    strcat(dest, END_OF_FILE);
    return 0;
  }

  // Info parsed correctly, procede to build question dir and data

  err = sprintf(path, "%s/%s", TOPICS, topic); // Topics/topic_name
  if (err < 0) {
    memset(dest, 0, BUFFER_SIZE);
    strcpy(dest, ERROR);
    return 1;
  }

  if ((d = opendir(path)) == NULL) { // Check if topic dir exists
    memset(dest, 0, BUFFER_SIZE);
    strcpy(dest, ERROR);
    return 1;
  } else {
    while ((ent = readdir(d)) != NULL) {
      fileN++;
    }
  }
  if (fileN > 100) { // Questions 01 to 99 + . + .. = 101. Dir is full if fileN
                     // == 101 or >100
    strcat(dest, FULL);
    return 0;
  }

  err = sprintf(path + strlen(path), "/%s",
                question); // Build path for question dir
  if (err < 0) {
    memset(dest, 0, BUFFER_SIZE);
    strcpy(dest, ERROR);
    return 1;
  }

  if (access(path, F_OK) == -1) { // Check if question already exists
    err = mkdir(path, 0700);
    if (err < 0) {
      memset(dest, 0, BUFFER_SIZE);
      strcpy(dest, ERROR);
      return 1;
    }
  } else {
    memset(dest, 0, BUFFER_SIZE);
    strcpy(dest, SUBMIT_QUESTION_RESPONSE);
    strcat(dest, DUP);
    return 0;
  }

  err = sprintf(path + strlen(path), "/%s",
                DATA); // Build path for QUESTION_DATA dir
  if (err < 0) {
    memset(dest, 0, BUFFER_SIZE);
    strcpy(dest, ERROR);
    return 1;
  }

  err = mkdir(path, 0700); // Make data dir
  if (err < 0) {
    memset(dest, 0, BUFFER_SIZE);
    strcpy(dest, ERROR);
    return 1;
  }

  err = sprintf(filename, "%s/%s", path, USER); // Make user id file and fill it
  if (err < 0) {
    memset(dest, 0, BUFFER_SIZE);
    strcpy(dest, ERROR);
    return 1;
  }

  f = fopen(filename, "w");
  if (f == NULL) {
    memset(dest, 0, BUFFER_SIZE);
    strcpy(dest, ERROR);
    return 1;
  }

  err = fprintf(f, "%d", id);
  if (err < 0) {
    memset(dest, 0, BUFFER_SIZE);
    strcpy(dest, ERROR);
    return 1;
  }

  fclose(f);

  memset(filename, 0, BUFFER_SIZE); // Make question data file and fill it
  err = sprintf(filename, "%s/%s", path, Q_INFO);
  if (err < 0) {
    memset(dest, 0, BUFFER_SIZE);
    strcpy(dest, ERROR);
    return 1;
  }

  f = fopen(filename, "w");
  if (f == NULL) {
    memset(dest, 0, BUFFER_SIZE);
    strcpy(dest, ERROR);
    return 1;
  }

  err = fprintf(f, "%s", data);
  if (err < 0) {
    memset(dest, 0, BUFFER_SIZE);
    strcpy(dest, ERROR);
    return 1;
  }

  fclose(f);

  memset(filename, 0, BUFFER_SIZE); // Make image info file and fill it
  err = sprintf(filename, "%s/%s", path, IMG_DATA);
  if (err < 0) {
    memset(dest, 0, BUFFER_SIZE);
    strcpy(dest, ERROR);
    return 1;
  }

  f = fopen(filename, "w");
  if (f == NULL) {
    memset(dest, 0, BUFFER_SIZE);
    strcpy(dest, ERROR);
    return 1;
  }
  if (qIMG == 0) {
    err = fprintf(f, "%d %s", qIMG, "nil");
    if (err < 0) {
      memset(dest, 0, BUFFER_SIZE);
      strcpy(dest, ERROR);
      return 1;
    }
  } else {
    err = fprintf(f, "%d %s", qIMG, ext);
    if (err < 0) {
      memset(dest, 0, BUFFER_SIZE);
      strcpy(dest, ERROR);
      return 1;
    }
  }

  fclose(f);

  if (qIMG == 1) {
    memset(filename, 0, BUFFER_SIZE); // Make image data file and fill it
    err = sprintf(filename, "%s/%s.%s", path, IMG, ext);
    if (err < 0) {
      memset(dest, 0, BUFFER_SIZE);
      strcpy(dest, ERROR);
      return 1;
    }

    f = fopen(filename, "w");
    if (f == NULL) {
      memset(dest, 0, BUFFER_SIZE);
      strcpy(dest, ERROR);
      return 1;
    }

    err = fprintf(f, "%s", idata);
    if (err < 0) {
      memset(dest, 0, BUFFER_SIZE);
      strcpy(dest, ERROR);
      return 1;
    }

    fclose(f);
  }

  strcat(dest, OK);
  return 0;
}

int parseSubmitAnswer(char *info, int *id, char *topic, char *question,
                      int *size, char *data, int *aIMG, char *ext, int *iSize,
                      char *iData) {

  int infoSize = strlen(info);
  char s[2] = " ";
  int i = 0, err;
  char *token, ssize[8], sid[8], saImg[2], siSize[8];
  token = strtok(info, s);
  while (token != NULL) {
    switch (i++) {
    case 0:
      strcpy(sid, token);
      err = sscanf(token, "%d", id);
      if (err < 0) {
        return 1;
      }
      break;
    case 1:
      strcpy(topic, token);
      break;
    case 2:
      strcpy(question, token);
      break;
    case 3:
      strcpy(ssize, token);
      err = sscanf(token, "%d", size);
      if (err < 0) {
        return 1;
      }
      break;
    case 4:
      strcpy(data, token);
      break;
    case 5:
      strcpy(saImg, token);
      err = sscanf(token, "%d", aIMG);
      if (err < 0) {
        return 1;
      }
      break;
    case 6:
      strcpy(ext, token);
      break;
    case 7:
      strcpy(siSize, token);
      err = sscanf(token, "%d", iSize);
      if (err < 0) {
        return 1;
      }
      break;
    case 8:
      strcpy(iData, token);
      break;
    default:
      break;
    }
  }

  if (i != 6 && i != 9) {
    return 1;
  }

  if (strlen(topic) + strlen(question) + strlen(sid) + strlen(ssize) +
          strlen(data) + strlen(saImg) + strlen(ext) + strlen(siSize) +
          strlen(iData) + i - 1 !=
      infoSize) {
    return 1;
  }

  if (*aIMG != 0 && *aIMG != 1) {
    return 1;
  }

  return 0;
}

int handleSubmitAnswer(char *info, char *dest) {

  char topic[16], question[16], data[BUFFER_SIZE], ext[4], idata[BUFFER_SIZE];
  char path[BUFFER_SIZE], filename[2 * BUFFER_SIZE];
  int id, size, aIMG, iSize, err, fileN = 0;
  FILE *f;
  DIR *d;
  struct dirent *ent;

  memset(dest, 0, BUFFER_SIZE);
  strcpy(dest, SUBMIT_QUESTION_RESPONSE);

  err = parseSubmitQuestion(info, &id, topic, question, &size, data, &aIMG, ext,
                            &iSize, idata);
  if (err > 0) {
    strcat(dest, END_OF_FILE);
    return 0;
  }

  // Info parsed correctly, procede to build question dir and data

  err = sprintf(path, "%s/%s/%s", TOPICS, topic, question); // Topics/topic_name
  if (err < 0) {
    memset(dest, 0, BUFFER_SIZE);
    strcpy(dest, ERROR);
    return 1;
  }

  if ((d = opendir(path)) == NULL) { // Check if topic/question dir exists
    memset(dest, 0, BUFFER_SIZE);
    strcpy(dest, ERROR);
    return 1;
  } else {
    while ((ent = readdir(d)) != NULL) {
      fileN++;
    }
  }
  if (fileN >
      101) { // Answers 01 to 99 + . + .. + DATA = 102. Dir is full if fileN
             // == 102 or >101
    strcat(dest, FULL);
    return 0;
  }

  err = sprintf(path + strlen(path), "/%s%d", "ANSWER_",
                fileN - 2); // Build path for answer dir, ANSWER_01 to ANSWER_99
  if (err < 0) {
    memset(dest, 0, BUFFER_SIZE);
    strcpy(dest, ERROR);
    return 1;
  }

  err = mkdir(path, 0700);
  if (err < 0) {
    memset(dest, 0, BUFFER_SIZE);
    strcpy(dest, ERROR);
    return 1;
  }

  err = sprintf(filename, "%s/%s", path,
                ANS_DATA); // Make ANS_DATA.txt file and fill it
  if (err < 0) {
    memset(dest, 0, BUFFER_SIZE);
    strcpy(dest, ERROR);
    return 1;
  }

  f = fopen(filename, "w");
  if (f == NULL) {
    memset(dest, 0, BUFFER_SIZE);
    strcpy(dest, ERROR);
    return 1;
  }

  err = fprintf(f, "%s", data);
  if (err < 0) {
    memset(dest, 0, BUFFER_SIZE);
    strcpy(dest, ERROR);
    return 1;
  }

  fclose(f);

  memset(filename, 0, BUFFER_SIZE); // Make image info file and fill it
  err = sprintf(filename, "%s/%s", path, IMG_DATA);
  if (err < 0) {
    memset(dest, 0, BUFFER_SIZE);
    strcpy(dest, ERROR);
    return 1;
  }

  f = fopen(filename, "w");
  if (f == NULL) {
    memset(dest, 0, BUFFER_SIZE);
    strcpy(dest, ERROR);
    return 1;
  }
  if (aIMG == 0) {
    err = fprintf(f, "%d %s", aIMG, "nil");
    if (err < 0) {
      memset(dest, 0, BUFFER_SIZE);
      strcpy(dest, ERROR);
      return 1;
    }
  } else {
    err = fprintf(f, "%d %s", aIMG, ext);
    if (err < 0) {
      memset(dest, 0, BUFFER_SIZE);
      strcpy(dest, ERROR);
      return 1;
    }
  }

  fclose(f);

  if (aIMG == 1) {
    memset(filename, 0, BUFFER_SIZE); // Make image data file and fill it
    err = sprintf(filename, "%s/%s.%s", path, IMG, ext);
    if (err < 0) {
      memset(dest, 0, BUFFER_SIZE);
      strcpy(dest, ERROR);
      return 1;
    }

    f = fopen(filename, "w");
    if (f == NULL) {
      memset(dest, 0, BUFFER_SIZE);
      strcpy(dest, ERROR);
      return 1;
    }

    err = fprintf(f, "%s", idata);
    if (err < 0) {
      memset(dest, 0, BUFFER_SIZE);
      strcpy(dest, ERROR);
      return 1;
    }

    fclose(f);
  }

  strcat(dest, OK);
  return 0;
}
