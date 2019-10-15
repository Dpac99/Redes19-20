#include "../others/consts.h"
#include "../others/helpers.h"

int parseGetQuestion(char *info, char *topic, char *question) {
  int infoSize = strlen(info);
  int i, j = 0;
  int space = 0;

  for (i = 0; i < infoSize; i++) {
    if (info[i] == ' ') {
      topic[i] = '\0';
      space = 1;
    } else if (!space) {
      topic[i] = info[i];
    } else if (info[i] != '\n') {
      question[j++] = info[i];
    }
  }
  question[j] = '\0';

  if (strlen(topic) + strlen(question) + 2 != infoSize) {
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
  int uid, err, qIMG, i = 0, limit;
  struct stat st;
  off_t size;

  sprintf(dest, "%s ", GET_QUESTION_RESPONSE);

  // Parse info

  err = parseGetQuestion(info, topic, question);
  if (err > 0) {
    sprintf(dest + strlen(dest), "%s\n", ERROR);
    return 0;
  }

  // Build path to topic/question/data dir
  err = sprintf(path, "%s/%s/%s/%s", TOPICS, topic, question, DATA);
  if (err < 0) {
    memset(dest, 0, BUFFER_SIZE);
    sprintf(dest, "%s\n", ERROR);
    return 1;
  }

  if (opendir(path) == NULL &&
      errno == ENOENT) { // Check if topic/question/data exists. If not, return
                         // QGR EOF
    sprintf(dest + strlen(dest), "%s\n", END_OF_FILE);
  }

  // Read uid of question
  err = sprintf(filename, "%s/%s", path, USER);
  if (err < 0) {
    memset(dest, 0, BUFFER_SIZE);
    sprintf(dest, "%s\n", ERROR);
    return 1;
  }

  if (access(filename, F_OK) == 0) {
    f = fopen(filename, "r");
    if (f == NULL) {
      memset(dest, 0, BUFFER_SIZE);
      sprintf(dest, "%s\n", ERROR);
      return 1;
    }

    err = fscanf(f, "%d", &uid);
    if (err == EOF) {
      fclose(f);
      memset(dest, 0, BUFFER_SIZE);
      sprintf(dest, "%s\n", ERROR);
      return 1;
    }
    fclose(f);
  } else {

    memset(dest, 0, BUFFER_SIZE);
    sprintf(dest, "%s\n", ERROR);
    return 1;
  }

  err = sprintf(dest + strlen(dest), "%d ", uid);
  if (err < 0) {

    memset(dest, 0, BUFFER_SIZE);
    sprintf(dest, "%s\n", ERROR);
    return 1;
  }

  // Get question info
  err = sprintf(filename, "%s/%s", path, Q_INFO);
  if (err < 0) {

    memset(dest, 0, BUFFER_SIZE);
    sprintf(dest, "%s\n", ERROR);
    return 1;
  }

  if (stat(filename, &st) == 0) {

    size = st.st_size;

    data = (char *)malloc(sizeof(char) * (size + 1));
    if (data == NULL) {

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

    int nread = 0;
    for (; nread < size; nread++) {
      data[nread] = fgetc(f);
    }

    data[nread] = '\0';

    fclose(f);

    // Print question info to response buffer
    err = sprintf(dest + strlen(dest), "%d %s ", (int)size, data);
    if (err < 0) {

      memset(dest, 0, BUFFER_SIZE);
      sprintf(dest, "%s\n", ERROR);
      return 1;
    }

    free(data);
  } else {

    memset(dest, 0, BUFFER_SIZE);
    sprintf(dest, "%s\n", ERROR);
    return 1;
  }

  // Check if image and extension if yes
  memset(filename, 0, strlen(filename));
  err = sprintf(filename, "%s/%s", path, IMG_DATA);
  if (err < 0) {

    memset(dest, 0, BUFFER_SIZE);
    sprintf(dest, "%s\n", ERROR);
    return 1;
  }

  if (access(filename, F_OK) == 0) {
    f = fopen(filename, "r");
    if (f == NULL) {

      memset(dest, 0, BUFFER_SIZE);
      sprintf(dest, "%s\n", ERROR);
      return 1;
    }

    fscanf(f, "%d %s", &qIMG, ext);

    fclose(f);
  } else {

    memset(dest, 0, BUFFER_SIZE);
    sprintf(dest, "%s\n", ERROR);
    return 1;
  }

  err = sprintf(dest + strlen(dest), "%d ", qIMG);
  if (err < 0) {

    memset(dest, 0, BUFFER_SIZE);
    sprintf(dest, "%s\n", ERROR);
    return 1;
  }

  // Get image

  if (qIMG == 1) {
    memset(filename, 0, 128);
    err = sprintf(filename, "%s/%s.%s", path, IMG, ext);
    if (err < 0) {

      memset(dest, 0, BUFFER_SIZE);
      sprintf(dest, "%s\n", ERROR);
      return 1;
    }

    if (stat(filename, &st) == 0) {

      size = st.st_size;

      data = (char *)malloc(sizeof(char) * (size + 1));
      if (data == NULL) {

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

      int s = 0;
      for (; s < size; s++) {
        data[s] = fgetc(f);
      }

      data[s] = '\0';

      fclose(f);

      err = sprintf(dest + strlen(dest), "%s %d %s ", ext, (int)size, data);
      if (err < 0) {

        memset(dest, 0, BUFFER_SIZE);
        sprintf(dest, "%s\n", ERROR);
        return 1;
      }

      free(data);
    } else {

      memset(dest, 0, BUFFER_SIZE);
      sprintf(dest, "%s\n", ERROR);
      return 1;
    }
  }

  memset(filename, 0, 128);
  err = sprintf(filename, "%s/%s/%s", TOPICS, topic, question);
  if (err < 0) {

    memset(dest, 0, BUFFER_SIZE);
    sprintf(dest, "%s\n", ERROR);
    return 1;
  }

  // Get all answers and get most recent 10
  if ((dir = opendir(filename)) != NULL) {
    i = 0;
    // Read dir
    while ((ent = readdir(dir)) != NULL) {
      if (strcmp(ent->d_name, ".") != 0 && strcmp(ent->d_name, "..") != 0 &&
          strcmp(ent->d_name, DATA) != 0) {
        strcpy(answers[i], ent->d_name);
        i++;
      }
    }
  } else {

    memset(dest, 0, BUFFER_SIZE);
    sprintf(dest, "%s\n", ERROR);
    return 1;
  }

  // Find N

  if (i > 10) {
    limit = i - 9;
    err = sprintf(dest + strlen(dest), "%d", 10);
  } else {
    err = sprintf(dest + strlen(dest), "%d", i);
  }
  if (err < 0) {
    memset(dest, 0, BUFFER_SIZE);
    sprintf(dest, "%s\n", ERROR);
    return 1;
  }

  // Last 10 (all answers are filed as answer_XX, so the last will be the most
  // recent)
  for (; i > limit; i--) {
    memset(filename, 0, 128);
    err = sprintf(filename, "%s/%s/%s/%s", TOPICS, topic, question,
                  answers[i - 1]);
    if (err < 0) {

      memset(dest, 0, BUFFER_SIZE);
      sprintf(dest, "%s\n", ERROR);
      return 1;
    }

    char a0 = answers[i - 1][strlen("ANSWER_")];
    char a1 = answers[i - 1][strlen("ANSWER_") + 1];

    err = sprintf(dest + strlen(dest), " %c%c ", a0, a1);
    if (err < 0) {
      memset(dest, 0, BUFFER_SIZE);
      sprintf(dest, "%s\n", ERROR);
      return 1;
    }

    int aUID;

    char answer[BUFFER_SIZE];

    memset(answer, 0, BUFFER_SIZE);
    err = sprintf(answer, "%s/%s", filename, USER);
    if (err < 0) {
      memset(dest, 0, BUFFER_SIZE);
      sprintf(dest, "%s\n", ERROR);
      return 1;
    }

    // Read Uid of answer
    if (access(answer, R_OK) == 0) {

      f = fopen(answer, "r");
      if (f == NULL) {

        memset(dest, 0, BUFFER_SIZE);
        sprintf(dest, "%s\n", ERROR);
        return 1;
      }

      err = fscanf(f, "%d", &aUID);
      if (err < 1) {
        memset(dest, 0, BUFFER_SIZE);
        sprintf(dest, "%s\n", ERROR);
        return 1;
      }
      fclose(f);

      // Print uid to dest
      err = sprintf(dest + strlen(dest), "%d ", aUID);
      if (err < 0) {
        memset(dest, 0, BUFFER_SIZE);
        sprintf(dest, "%s\n", ERROR);
        return 1;
      }
    } else {

      memset(dest, 0, BUFFER_SIZE);
      sprintf(dest, "%s\n", ERROR);
      return 1;
    }

    memset(answer, 0, BUFFER_SIZE);
    err = sprintf(answer, "%s/%s", filename, ANS_DATA);
    if (err < 0) {
      memset(dest, 0, BUFFER_SIZE);
      sprintf(dest, "%s\n", ERROR);
      return 1;
    }

    stat(answer, &st);

    size = st.st_size;

    data = (char *)malloc(sizeof(char) * (size + 1));
    if (data == NULL) {

      memset(dest, 0, BUFFER_SIZE);
      sprintf(dest, "%s\n", ERROR);
      return 1;
    }

    if (access(answer, R_OK) == 0) {

      f = fopen(answer, "r");
      if (f == NULL) {

        memset(dest, 0, BUFFER_SIZE);
        sprintf(dest, "%s\n", ERROR);
        return 1;
      }

      int c, a = 0;
      while ((c = getc(f)) != EOF) {
        data[a++] = c;
      }

      fclose(f);

      // Print answer data to dest
      err = sprintf(dest + strlen(dest), "%d %s ", (int)size, data);
      if (err < 0) {

        memset(dest, 0, BUFFER_SIZE);
        sprintf(dest, "%s\n", ERROR);
        return 1;
      }

      free(data);
    } else {

      memset(dest, 0, BUFFER_SIZE);
      sprintf(dest, "%s\n", ERROR);
      return 1;
    }

    // Check if image and extension if yes
    memset(answer, 0, BUFFER_SIZE);
    err = sprintf(answer, "%s/%s", filename, IMG_DATA);
    if (err < 0) {
      memset(dest, 0, BUFFER_SIZE);
      sprintf(dest, "%s\n", ERROR);
      return 1;
    }
    if (access(answer, R_OK) == 0) {
      f = fopen(answer, "r");
      if (f == NULL) {

        memset(dest, 0, BUFFER_SIZE);
        sprintf(dest, "%s\n", ERROR);
        return 1;
      }

      memset(ext, 0, 4);
      fscanf(f, "%d %s", &qIMG, ext);
      fclose(f);
    } else {

      memset(dest, 0, BUFFER_SIZE);
      sprintf(dest, "%s\n", ERROR);
      return 1;
    }

    err = sprintf(dest + strlen(dest), "%d", qIMG);
    if (err < 0) {
      memset(dest, 0, BUFFER_SIZE);
      sprintf(dest, "%s\n", ERROR);
      return 1;
    }

    // Get image
    if (qIMG == 1) {
      memset(answer, 0, BUFFER_SIZE);
      err = sprintf(answer, "%s/%s.%s", filename, IMG, ext);
      if (err < 0) {
        memset(dest, 0, BUFFER_SIZE);
        sprintf(dest, "%s\n", ERROR);
        return 1;
      }

      if (stat(answer, &st) == 0) {

        size = st.st_size;

        data = (char *)malloc(sizeof(char) * (size + 1));
        if (data == NULL) {

          memset(dest, 0, BUFFER_SIZE);
          sprintf(dest, "%s\n", ERROR);
          return 1;
        }

        f = fopen(answer, "r");
        if (f == NULL) {

          memset(dest, 0, BUFFER_SIZE);
          sprintf(dest, "%s\n", ERROR);
          return 1;
        }

        int nread = 0;
        for (; nread < size; nread++) {
          data[nread] = fgetc(f);
        }

        data[nread] = '\0';

        fclose(f);

        // Print image
        err = sprintf(dest + strlen(dest), " %s %d %s", ext, (int)size, data);
        if (err < 0) {

          memset(dest, 0, BUFFER_SIZE);
          memset(filename, 0, 128);
          sprintf(dest, "%s\n", ERROR);
          return 1;
        }

        free(data);
      } else {

        memset(dest, 0, BUFFER_SIZE);
        sprintf(dest, "%s\n", ERROR);
        return 1;
      }
    }
  }
  sprintf(dest + strlen(dest), "\n");
  return 0;
}

int parseSubmitQuestion(char *info, int *id, char *topic, char *question,
                        int *size, char **data, int *qIMG, char *ext,
                        int *iSize, char **iData) {

  int infoSize = strlen(info);
  int i = 0, total = 0;
  char rest[BUFFER_SIZE];

  char *token = strtok(info, " ");
  while (i < 4) {
    switch (i) {
    case 0:
      *id = atoi(token);
      break;
    case 1:
      strcpy(topic, token);
      break;
    case 2:
      strcpy(question, token);
      break;
    case 3:
      *size = atoi(token);
      break;
    }
    i++;
    if (i < 4)
      token = strtok(NULL, " ");
  }

  *data = (char *)malloc((*size + 1) * sizeof(char));
  if (*data == NULL) {
    return 1;
  }

  total += strlen(topic) + strlen(question) + sizeOfNumber(*id) +
           sizeOfNumber(*size) + 4;

  int v = 0;
  for (; v < *size; v++) {
    (*data)[v] = info[total + v];
  }

  (*data)[v] = '\0';
  total += strlen(*data) + 1;

  *qIMG = info[total] - '0';

  total += sizeOfNumber(*qIMG) + 1;
  if (*qIMG == 1) {
    for (int j = 0; total + j < infoSize; j++) {
      rest[j] = info[total + j];
    }

    token = strtok(rest, " ");
    strcpy(ext, token);
    token = strtok(NULL, " ");
    *iSize = atoi(token);

    int subtotal = strlen(ext) + sizeOfNumber(*iSize) + 2;
    total += subtotal;

    *iData = (char *)malloc((*iSize + 1) * sizeof(char));
    if (*iData == NULL) {
      return 1;
    }

    int k;
    for (k = 0; k < *iSize; k++) {
      (*iData)[k] = rest[subtotal + k];
    }
    (*iData)[k] = '\0';

    total += strlen(*iData);
  }

  total++;

  if (total != infoSize) {
    return 1;
  }

  if (*qIMG != 0 && *qIMG != 1) {
    return 1;
  }

  return 0;
}

int handleSubmitQuestion(char *info, char *dest) {

  char topic[16], question[16], *data, ext[4], *idata;
  char path[BUFFER_SIZE], filename[2 * BUFFER_SIZE];
  int id, size, qIMG, iSize, err, fileN = 0;
  FILE *f;
  DIR *d;
  struct dirent *ent;

  memset(dest, 0, BUFFER_SIZE);
  err = sprintf(dest, "%s ", SUBMIT_QUESTION_RESPONSE);
  if (err < 0) {
    return 1;
  }

  err = parseSubmitQuestion(info, &id, topic, question, &size, &data, &qIMG,
                            ext, &iSize, &idata);
  if (err > 0) {
    sprintf(dest + strlen(dest), "%s\n", END_OF_FILE);
    return 0;
  }

  // Info parsed correctly, procede to build question dir and data

  err = sprintf(path, "%s/%s", TOPICS, topic); // Topics/topic_name
  if (err < 0) {

    memset(dest, 0, BUFFER_SIZE);
    sprintf(dest, "%s\n", ERROR);
    return 1;
  }

  if ((d = opendir(path)) == NULL) { // Check if topic dir exists

    memset(dest, 0, BUFFER_SIZE);
    sprintf(dest, "%s\n", ERROR);
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
    sprintf(dest, "%s\n", ERROR);
    return 1;
  }

  if (opendir(path)) { // Check if question already exists
    memset(dest, 0, BUFFER_SIZE);
    strcpy(dest, SUBMIT_QUESTION_RESPONSE);
    strcat(dest, DUP);
    return 0;
  } else if (errno == ENOENT) {
    err = mkdir(path, 0700);
    if (err < 0) {

      memset(dest, 0, BUFFER_SIZE);
      sprintf(dest, "%s\n", ERROR);
      return 1;
    }
  }

  err = sprintf(path + strlen(path), "/%s",
                DATA); // Build path for QUESTION_DATA dir
  if (err < 0) {

    memset(dest, 0, BUFFER_SIZE);
    sprintf(dest, "%s\n", ERROR);
    return 1;
  }

  err = mkdir(path, 0700); // Make data dir
  if (err < 0) {

    memset(dest, 0, BUFFER_SIZE);
    sprintf(dest, "%s\n", ERROR);
    return 1;
  }

  err = sprintf(filename, "%s/%s", path, USER); // Make user id file and fill it
  if (err < 0) {

    memset(dest, 0, BUFFER_SIZE);
    sprintf(dest, "%s\n", ERROR);
    return 1;
  }

  f = fopen(filename, "w");
  if (f == NULL) {

    memset(dest, 0, BUFFER_SIZE);
    sprintf(dest, "%s\n", ERROR);
    return 1;
  }

  err = fprintf(f, "%d", id);
  if (err < 0) {

    memset(dest, 0, BUFFER_SIZE);
    sprintf(dest, "%s\n", ERROR);
    return 1;
  }

  fclose(f);

  memset(filename, 0, BUFFER_SIZE); // Make question data file and fill it
  err = sprintf(filename, "%s/%s", path, Q_INFO);
  if (err < 0) {

    memset(dest, 0, BUFFER_SIZE);
    sprintf(dest, "%s\n", ERROR);
    return 1;
  }

  f = fopen(filename, "w");
  if (f == NULL) {

    memset(dest, 0, BUFFER_SIZE);
    sprintf(dest, "%s\n", ERROR);
    return 1;
  }

  err = fprintf(f, "%s", data);
  if (err < 0) {

    memset(dest, 0, BUFFER_SIZE);
    sprintf(dest, "%s\n", ERROR);
    return 1;
  }

  fclose(f);

  memset(filename, 0, BUFFER_SIZE); // Make image info file and fill it
  err = sprintf(filename, "%s/%s", path, IMG_DATA);
  if (err < 0) {

    memset(dest, 0, BUFFER_SIZE);
    sprintf(dest, "%s\n", ERROR);
    return 1;
  }

  f = fopen(filename, "w");
  if (f == NULL) {

    memset(dest, 0, BUFFER_SIZE);
    sprintf(dest, "%s\n", ERROR);
    return 1;
  }
  if (qIMG == 0) {
    err = fprintf(f, "%d %s", qIMG, "nil");
    if (err < 0) {

      memset(dest, 0, BUFFER_SIZE);
      sprintf(dest, "%s\n", ERROR);
      return 1;
    }
  } else {
    err = fprintf(f, "%d %s", qIMG, ext);
    if (err < 0) {

      memset(dest, 0, BUFFER_SIZE);
      sprintf(dest, "%s\n", ERROR);
      return 1;
    }
  }

  fclose(f);

  if (qIMG == 1) {
    memset(filename, 0, BUFFER_SIZE); // Make image data file and fill it
    err = sprintf(filename, "%s/%s.%s", path, IMG, ext);
    if (err < 0) {

      memset(dest, 0, BUFFER_SIZE);
      sprintf(dest, "%s\n", ERROR);
      return 1;
    }

    f = fopen(filename, "w");
    if (f == NULL) {

      memset(dest, 0, BUFFER_SIZE);
      sprintf(dest, "%s\n", ERROR);
      return 1;
    }

    err = fprintf(f, "%s", idata);
    if (err < 0) {

      memset(dest, 0, BUFFER_SIZE);
      sprintf(dest, "%s\n", ERROR);
      return 1;
    }

    fclose(f);
  }

  sprintf(dest + strlen(dest), "%s\n", OK);
  return 0;
}

int parseSubmitAnswer(char *info, int *id, char *topic, char *question,
                      int *size, char **data, int *qIMG, char *ext, int *iSize,
                      char **iData) {

  int infoSize = strlen(info);
  int i = 0, total = 0;
  char rest[BUFFER_SIZE];

  char *token = strtok(info, " ");
  while (i < 4) {
    switch (i) {
    case 0:
      *id = atoi(token);
      break;
    case 1:
      strcpy(topic, token);
      break;
    case 2:
      strcpy(question, token);
      break;
    case 3:
      *size = atoi(token);
      break;
    }
    i++;
    if (i < 4)
      token = strtok(NULL, " ");
  }

  *data = (char *)malloc((*size + 1) * sizeof(char));
  if (*data == NULL) {
    return 1;
  }

  total += strlen(topic) + strlen(question) + sizeOfNumber(*id) +
           sizeOfNumber(*size) + 4;

  int v = 0;
  for (; v < *size; v++) {
    (*data)[v] = info[total + v];
  }

  (*data)[v] = '\0';
  total += strlen(*data) + 1;

  *qIMG = info[total] - '0';

  total += sizeOfNumber(*qIMG) + 1;
  if (*qIMG == 1) {
    for (int j = 0; total + j < infoSize; j++) {
      rest[j] = info[total + j];
    }

    token = strtok(rest, " ");
    strcpy(ext, token);
    token = strtok(NULL, " ");
    *iSize = atoi(token);

    int subtotal = strlen(ext) + sizeOfNumber(*iSize) + 2;
    total += subtotal;

    *iData = (char *)malloc((*iSize + 1) * sizeof(char));
    if (*iData == NULL) {
      return 1;
    }

    int k;
    for (k = 0; k < *iSize; k++) {
      (*iData)[k] = rest[subtotal + k];
    }
    (*iData)[k] = '\0';

    total += strlen(*iData);
  }

  total++;

  if (total != infoSize) {
    return 1;
  }

  if (*qIMG != 0 && *qIMG != 1) {
    return 1;
  }

  return 0;
}

int handleSubmitAnswer(char *info, char *dest) {

  char topic[16], question[16], *data, ext[4], *idata;
  char path[BUFFER_SIZE], filename[2 * BUFFER_SIZE];
  int id, size, qIMG, iSize, err, fileN = 0;
  FILE *f;
  DIR *d;
  struct dirent *ent;

  memset(dest, 0, BUFFER_SIZE);
  err = sprintf(dest, "%s ", SUBMIT_ANSWER_RESPONSE);
  if (err < 0) {
    return 1;
  }

  err = parseSubmitAnswer(info, &id, topic, question, &size, &data, &qIMG, ext,
                          &iSize, &idata);
  if (err > 0) {
    sprintf(dest + strlen(dest), "%s\n", END_OF_FILE);
    return 0;
  }

  // Info parsed correctly, procede to build data

  err = sprintf(path, "%s/%s/%s", TOPICS, topic,
                question); // Topics/topic_name/question
  if (err < 0) {
    memset(dest, 0, BUFFER_SIZE);
    sprintf(dest, "%s\n", ERROR);
    return 1;
  }

  if ((d = opendir(path)) == NULL) { // Check if topic/question dir exists
    memset(dest, 0, BUFFER_SIZE);
    sprintf(dest, "%s\n", ERROR);
    return 1;
  } else {
    while ((ent = readdir(d)) != NULL) {
      fileN++;
    }
  }
  if (fileN >
      101) { // Answers 01 to 99 + . + .. + data = 102. Dir is full if fileN
             // == 102 or >101
    strcat(dest, FULL);
    return 0;
  }

  err = fileN - 2 < 10 ? sprintf(path + strlen(path), "/ANSWER_0%d", fileN - 2)
                       : sprintf(path + strlen(path), "/ANSWER_%d",
                                 fileN - 2); // Build path for answer dir
  if (err < 0) {
    memset(dest, 0, BUFFER_SIZE);
    sprintf(dest, "%s\n", ERROR);
    return 1;
  }

  if (opendir(path)) { // Check if answer already exists
    memset(dest, 0, BUFFER_SIZE);
    strcpy(dest, SUBMIT_QUESTION_RESPONSE);
    strcat(dest, DUP);
    return 0;
  } else if (errno == ENOENT) {
    err = mkdir(path, 0700);
    if (err < 0) {
      memset(dest, 0, BUFFER_SIZE);
      sprintf(dest, "%s\n", ERROR);
      return 1;
    }
  } else {
    memset(dest, 0, BUFFER_SIZE);
    sprintf(dest, "%s\n", ERROR);
    return 1;
  }

  err = sprintf(filename, "%s/%s", path, USER); // Make user id file and fill it
  if (err < 0) {
    memset(dest, 0, BUFFER_SIZE);
    sprintf(dest, "%s\n", ERROR);
    return 1;
  }

  f = fopen(filename, "w");
  if (f == NULL) {
    memset(dest, 0, BUFFER_SIZE);
    sprintf(dest, "%s\n", ERROR);
    return 1;
  }

  err = fprintf(f, "%d", id);
  if (err < 0) {
    memset(dest, 0, BUFFER_SIZE);
    sprintf(dest, "%s\n", ERROR);
    return 1;
  }

  fclose(f);

  memset(filename, 0, BUFFER_SIZE); // Make answer data file and fill it
  err = sprintf(filename, "%s/%s", path, ANS_DATA);
  if (err < 0) {
    memset(dest, 0, BUFFER_SIZE);
    sprintf(dest, "%s\n", ERROR);
    return 1;
  }

  f = fopen(filename, "w");
  if (f == NULL) {
    memset(dest, 0, BUFFER_SIZE);
    sprintf(dest, "%s\n", ERROR);
    return 1;
  }

  err = fprintf(f, "%s", data);
  if (err < 0) {
    memset(dest, 0, BUFFER_SIZE);
    sprintf(dest, "%s\n", ERROR);
    return 1;
  }

  fclose(f);
  memset(filename, 0, BUFFER_SIZE); // Make image info file and fill it
  err = sprintf(filename, "%s/%s", path, IMG_DATA);
  if (err < 0) {
    memset(dest, 0, BUFFER_SIZE);
    sprintf(dest, "%s\n", ERROR);
    return 1;
  }

  f = fopen(filename, "w");
  if (f == NULL) {
    memset(dest, 0, BUFFER_SIZE);
    sprintf(dest, "%s\n", ERROR);
    return 1;
  }
  if (qIMG == 0) {
    err = fprintf(f, "%d %s", qIMG, "nil");
    if (err < 0) {
      memset(dest, 0, BUFFER_SIZE);
      sprintf(dest, "%s\n", ERROR);
      return 1;
    }
  } else {
    err = fprintf(f, "%d %s", qIMG, ext);
    if (err < 0) {
      memset(dest, 0, BUFFER_SIZE);
      sprintf(dest, "%s\n", ERROR);
      return 1;
    }
  }

  fclose(f);

  if (qIMG == 1) {
    memset(filename, 0, BUFFER_SIZE); // Make image data file and fill it
    err = sprintf(filename, "%s/%s.%s", path, IMG, ext);
    if (err < 0) {

      memset(dest, 0, BUFFER_SIZE);
      sprintf(dest, "%s\n", ERROR);
      return 1;
    }

    f = fopen(filename, "w");
    if (f == NULL) {

      memset(dest, 0, BUFFER_SIZE);
      sprintf(dest, "%s\n", ERROR);
      return 1;
    }

    err = fprintf(f, "%s", idata);
    if (err < 0) {

      memset(dest, 0, BUFFER_SIZE);
      sprintf(dest, "%s\n", ERROR);
      return 1;
    }

    fclose(f);
  }

  sprintf(dest + strlen(dest), "%s\n", OK);
  return 0;
}
