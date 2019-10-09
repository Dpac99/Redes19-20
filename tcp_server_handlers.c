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
  }

  if (i < 1) {
    return 2;
  }

  if (strlen(topic) + strlen(question) + 1 != infoSize) {
    return 1;
  }

  return 0;
}

void handleGetQuestion(char *info, char *dest) {
  char topic[16], question[16], path[64], filename[128], *data, ext[4];
  char imgName[16], answers[100][32];
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
    return;
  }

  // Build path to question dir
  err = sprintf(path, "%s/%s/%s/%s", TOPICS, topic, question, DATA);
  if (err < 0) {
    memset(dest, 0, BUFFERSIZE);
    strcpy(dest, ERROR);
    return;
  }

  // Read uid of question
  err = sprintf(filename, "%s/%s", path, USER);
  if (err < 0) {
    memset(dest, 0, BUFFERSIZE);
    strcpy(dest, ERROR);
    return;
  }

  f = fopen(filename, "r");
  if (f == NULL) {
    memset(dest, 0, BUFFERSIZE);
    strcpy(dest, ERROR);
    return;
  }

  err = fscanf(f, "%d", &uid);
  if (err == EOF) {
    fclose(f);
    memset(dest, 0, BUFFERSIZE);
    strcpy(dest, ERROR);
    return;
  }
  fclose(f);

  // Get question info
  err = sprintf(filename, "%s/%s", path, INFO);
  if (err < 0) {
    memset(dest, 0, BUFFERSIZE);
    strcpy(dest, ERROR);
    return;
  }

  stat(filename, &st);

  size = st.st_size;

  data = (char *)malloc(sizeof(char) * (size + 1));
  if (data == NULL) {
    memset(dest, 0, BUFFERSIZE);
    strcpy(dest, ERROR);
    return;
  }

  f = fopen(filename, "r");
  if (f == NULL) {
    memset(dest, 0, BUFFERSIZE);
    strcpy(dest, ERROR);
    return;
  }

  fread(data, sizeof(char), size, f);
  if (feof(f) == 0) {
    memset(dest, 0, BUFFERSIZE);
    strcpy(dest, ERROR);
    return;
  }

  fclose(f);

  // Print question info to response buffer
  err = sprintf(dest + strlen(dest), "%d %s ", size, data);
  if (err < 0) {
    memset(dest, 0, BUFFERSIZE);
    strcpy(dest, ERROR);
    return;
  }

  free(data);

  // Check if image and extension if yes
  memset(filename, 0, strlen(filename));
  err = sprintf(filename, "%s/%s ", path, IMG_DATA);
  if (err < 0) {
    memset(dest, 0, BUFFERSIZE);
    strcpy(dest, ERROR);
    return;
  }

  f = fopen(filename, "r");
  if (f == NULL) {
    memset(dest, 0, BUFFERSIZE);
    strcpy(dest, ERROR);
    return;
  }

  fscanf(f, "%d %s", &qIMG, ext);

  fclose(f);

  // Get image

  if (qIMG == 1) {
    memset(filename, 0, 128);
    err = sprintf(filename, "%s/%s.%s", path, IMG, ext);
    if (err < 0) {
      memset(dest, 0, BUFFERSIZE);
      strcpy(dest, ERROR);
      return;
    }

    stat(filename, &st);

    size = st.st_size;

    data = (char *)malloc(sizeof(char) * (size + 1));
    if (data == NULL) {
      memset(dest, 0, BUFFERSIZE);
      strcpy(dest, ERROR);
      return;
    }

    f = fopen(filename, "r");
    if (f == NULL) {
      memset(dest, 0, BUFFERSIZE);
      strcpy(dest, ERROR);
      return;
    }

    fread(data, sizeof(char), size, f);
    if (feof(f) == 0) {
      memset(dest, 0, BUFFERSIZE);
      strcpy(dest, ERROR);
      return;
    }

    fclose(f);

    err = sprintf(dest + strlen(dest), "%d [%s %d %s] ", qIMG, ext, size, data);
    if (err < 0) {
      memset(dest, 0, BUFFERSIZE);
      strcpy(dest, ERROR);
      return;
    }

    free(data);
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
  for (; i >= size; i--) {
    memset(filename, 0, 128);
    err = sprintf(filename, "%s/%s/%s/%s/%s", TOPICS, topic, question,
                  answers[i], ANS_INFO);
    if (err < 0) {
      memset(dest, 0, BUFFERSIZE);
      strcpy(dest, ERROR);
      return;
    }

    stat(filename, &st);

    size = st.st_size;

    data = (char *)malloc(sizeof(char) * (size + 1));
    if (data == NULL) {
      memset(dest, 0, BUFFERSIZE);
      strcpy(dest, ERROR);
      return;
    }

    f = fopen(filename, "r");
    if (f == NULL) {
      memset(dest, 0, BUFFERSIZE);
      strcpy(dest, ERROR);
      return;
    }

    fread(data, sizeof(char), size, f);
    if (feof(f) == 0) {
      memset(dest, 0, BUFFERSIZE);
      strcpy(dest, ERROR);
      return;
    }

    fclose(f);

    // Print answer data to dest

    err = sprintf(dest + strlen(dest), "%d %s ", size, data);
    if (err < 0) {
      memset(dest, 0, BUFFERSIZE);
      strcpy(dest, ERROR);
      return;
    }

    free(data);

    // Check if image and extension if yes
    memset(filename, 0, strlen(filename));
    err = sprintf(filename, "%s/%s/%s/%s/%s", TOPICS, topic, question,
                  answers[i], IMG_DATA);
    if (err < 0) {
      memset(dest, 0, BUFFERSIZE);
      strcpy(dest, ERROR);
      return;
    }

    f = fopen(filename, "r");
    if (f == NULL) {
      memset(dest, 0, BUFFERSIZE);
      strcpy(dest, ERROR);
      return;
    }

    memset(ext, 0, 4);
    fscanf(f, "%d %s", &qIMG, ext);

    fclose(f);

    // Get image

    if (qIMG == 1) {
      memset(filename, 0, 128);
      err = sprintf(filename, "%s/%s.%s", path, IMG, ext);
      if (err < 0) {
        memset(dest, 0, BUFFERSIZE);
        strcpy(dest, ERROR);
        return;
      }

      stat(filename, &st);

      size = st.st_size;

      data = (char *)malloc(sizeof(char) * (size + 1));
      if (data == NULL) {
        memset(dest, 0, BUFFERSIZE);
        strcpy(dest, ERROR);
        return;
      }

      f = fopen(filename, "r");
      if (f == NULL) {
        memset(dest, 0, BUFFERSIZE);
        strcpy(dest, ERROR);
        return;
      }

      fread(data, sizeof(char), size, f);
      if (feof(f) == 0) {
        memset(dest, 0, BUFFERSIZE);
        strcpy(dest, ERROR);
        return;
      }

      fclose(f);

      // Print image
      err =
          sprintf(dest + strlen(dest), "%d [%s %d %s] ", qIMG, ext, size, data);
      if (err < 0) {
        memset(dest, 0, BUFFERSIZE);
        strcpy(dest, ERROR);
        return;
      }

      free(data);
    }
  }

  return 0;
}
