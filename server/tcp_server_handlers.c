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

int handleGetQuestion(int fd) {
  char buffer[BUFFER_SIZE];
  char topic[16], question[16], ext[4];
  char resp[BUFFER_SIZE];
  char path[64];
  char filename[128];
  int err;
  char eol[] = {"\n\0\0"};
  char errS[5];
  int uid, size, qimg;
  int nread = 0, nleft, chunk;
  DIR *dir;
  FILE *f;
  struct dirent *ent;
  fd_set mask;
  struct timeval timeout;
  timeout.tv_sec = 0;
  timeout.tv_usec = 0;
  char answers[100][16];

  sprintf(errS, "%s\n", ERROR);

  FD_ZERO(&mask);
  FD_SET(fd, &mask);
  memset(buffer, 0, BUFFER_SIZE);

  // Read info from socket
  while (select(fd + 1, &mask, NULL, NULL, &timeout)) {
    FD_ZERO(&mask);
    FD_SET(fd, &mask);
    nread += read(fd, buffer + nread, BUFFER_SIZE - nread);
    if (nread == -1) {
      writeTCP(fd, errS, 4);
      return -1;
      ;
    }
  }

  printf("%s", buffer);

  printf("\n\tSENT: ");

  char s[8];
  err = sprintf(s, "%s ", GET_QUESTION_RESPONSE);
  if (err < 0) {
    writeTCP(fd, errS, 4);
    return -1;
    ;
  }

  err = writeTCP(fd, s, 4);
  if (err < 1) {
    writeTCP(fd, errS, 4);
    return -1;
    ;
  }

  // Parse info
  err = parseGetQuestion(buffer, topic, question);
  if (err != 0) {
    writeTCP(fd, errS, 4);
    return -1;
    ;
  }

  // path = TOPICS/{topic}/{question}
  err = sprintf(path, "%s/%s/%s", TOPICS, topic, question);
  if (err < 0) {
    writeTCP(fd, errS, 4);
    return -1;
    ;
  }

  err = fileExists(path);
  if (err == 0) { // Check if path exists
    writeTCP(fd, "EOF\n", 4);
    return 0;
  } else if (err == -1) {
    writeTCP(fd, errS, 4);
    return -1;
    ;
  }

  // File that contains uid of the question submitter
  err = sprintf(filename, "%s/%s/%s", path, DATA, USER);
  if (err < 0) {
    writeTCP(fd, errS, 4);
    return -1;
    ;
  }

  f = fopen(filename, "r");
  if (f == NULL) {
    writeTCP(fd, errS, 4);
    return -1;
    ;
  }

  err = fscanf(f, "%d", &uid);
  if (err < 0) {
    writeTCP(fd, errS, 4);
    return -1;
    ;
  }

  fclose(f);

  // Get data size
  memset(filename, 0, 128);
  err = sprintf(filename, "%s/%s/%s", path, DATA, QDATA);
  if (err < 0) {
    writeTCP(fd, errS, 4);
    return -1;
    ;
  }

  size = fileExists(filename);
  if (size <= 0) {
    writeTCP(fd, errS, 4);
    return -1;
    ;
  }

  memset(resp, 0, BUFFER_SIZE);
  err = sprintf(resp, "%d %d ", uid, size);
  if (err < 0) {
    writeTCP(fd, errS, 4);
    return -1;
    ;
  }

  err = writeTCP(fd, resp, strlen(resp));
  if (err < 0) {
    writeTCP(fd, errS, 4);
    return -1;
    ;
  }

  f = fopen(filename, "r");
  if (f == NULL) {
    writeTCP(fd, errS, 4);
    return -1;
    ;
  }

  nleft = size;
  while (nleft > 0) {
    memset(buffer, 0, BUFFER_SIZE);
    chunk = nleft < BUFFER_SIZE ? nleft : BUFFER_SIZE;
    err = fread(buffer, chunk, sizeof(char), f);
    if (err != chunk && feof(f)) {
      writeTCP(fd, errS, 4);
      return -1;
      ;
    }
    err = writeTCP(fd, buffer, chunk);
    if (err < 0) {
      writeTCP(fd, errS, 4);
      return -1;
      ;
    }
    nleft -= chunk;
  }

  fclose(f);

  // Check for images

  memset(filename, 0, 128);
  err = sprintf(filename, "%s/%s/%s", path, DATA, IMG_DATA);
  if (err < 0) {
    writeTCP(fd, errS, 4);
    return -1;
    ;
  }

  f = fopen(filename, "r");
  if (f == NULL) {
    writeTCP(fd, errS, 4);
    return -1;
    ;
  }

  // read qimg and ext
  err = fscanf(f, "%d %s", &qimg, ext);
  if (err < 0) {
    writeTCP(fd, errS, 4);
    return -1;
    ;
  }

  if (qimg == 0) {
    char s[4];
    err = sprintf(s, " %d", qimg);
    if (err < 0) {
      writeTCP(fd, errS, 4);
      return -1;
      ;
    }
    err = writeTCP(fd, s, 3);
    if (err < 0) {
      writeTCP(fd, errS, 4);
      return -1;
      ;
    }
  } else { // If qimg == 1
    memset(filename, 0, 128);
    err = sprintf(filename, "%s/%s/%s.%s", path, DATA, IMG, ext);
    if (err < 0) {
      writeTCP(fd, errS, 4);
      return -1;
      ;
    }

    // get size of data
    size = fileExists(filename);
    if (size < 1) {
      writeTCP(fd, errS, 4);
      return -1;
      ;
    }

    // write qimg, ext and size
    char s[16];
    err = sprintf(s, " %d %s %d ", qimg, ext, size);
    if (err < 0) {
      writeTCP(fd, errS, 4);
      return -1;
      ;
    }

    err = writeTCP(fd, s, strlen(s));
    if (err < 0) {
      writeTCP(fd, errS, 4);
      return -1;
      ;
    }

    // write imgdata
    f = fopen(filename, "r");
    if (f == NULL) {
      writeTCP(fd, errS, 4);
      return -1;
      ;
    }

    nleft = size;
    while (nleft > 0) {
      memset(buffer, 0, BUFFER_SIZE);
      chunk = nleft < BUFFER_SIZE ? nleft : BUFFER_SIZE;
      err = fread(buffer, chunk, sizeof(char), f);
      if (err != chunk && feof(f)) {
        writeTCP(fd, errS, 4);
        return -1;
        ;
      }
      err = writeTCP(fd, buffer, chunk);
      if (err < 0) {
        writeTCP(fd, errS, 4);
        return -1;
        ;
      }
      nleft -= chunk;
    }

    fclose(f);
  }

  // Get N of answers, aka, the answer number of the last

  int i = 0;
  int N;
  dir = opendir(path);
  if (dir != NULL) {
    while ((ent = readdir(dir)) != NULL) {
      if ((strcmp(ent->d_name, ".") * strcmp(ent->d_name, "..") *
           strcmp(ent->d_name, DATA)) != 0) {
        strcpy(answers[i++], ent->d_name);
      }
    }
  }

  if (i > 10) {
    N = 10;
  } else {
    N = i;
  }

  memset(s, 0, 8);
  err = N > 0 ? sprintf(s, " %d ", N) : sprintf(s, " %d%s", N, eol);
  if (err < 0) {
    writeTCP(fd, errS, 4);
    return -1;
    ;
  }

  err = writeTCP(fd, s, strlen(s));
  if (err < 0) {
    writeTCP(fd, errS, 4);
    return -1;
  }

  // Get info and data for last 10 answers. Also writes
  while (N-- > 0) {
    memset(filename, 0, 128);

    // Get dir name
    err = sprintf(filename, "%s/%s", path, answers[N]);
    if (err < 0) {
      writeTCP(fd, errS, 4);
      return -1;
    }

    char AN[3];
    getAnswerNumber(filename, AN);

    char s[8];
    memset(s, 0, 8);
    err = sprintf(s, "%s ", AN);
    if (err < 0) {
      writeTCP(fd, errS, 4);
      return -1;
    }

    err = writeTCP(fd, s, strlen(s));
    if (err < 1) {
      writeTCP(fd, errS, 4);
      return -1;
    }

    // Get user id
    err = sprintf(filename + strlen(filename), "/%s", USER);
    if (err < 0) {
      writeTCP(fd, errS, 4);
      return -1;
    }

    f = fopen(filename, "r");
    if (f == NULL) {
      writeTCP(fd, errS, 4);
      return -1;
    }

    err = fscanf(f, "%d", &uid);
    if (err < 0) {
      writeTCP(fd, errS, 4);
      return -1;
    }

    fclose(f);

    // Get dataSize
    memset(filename, 0, 128);
    err = sprintf(filename, "%s/%s/%s", path, answers[N], ANS_DATA);
    if (err < 0) {
      writeTCP(fd, errS, 4);
      return -1;
    }

    size = fileExists(filename);
    if (size < 1) {
      writeTCP(fd, errS, 4);
      return -1;
    }

    // Write all info so far
    memset(resp, 0, BUFFER_SIZE);
    err = sprintf(resp, "%d %d ", uid, size);
    if (err < 0) {
      writeTCP(fd, errS, 4);
      return -1;
    }

    err = writeTCP(fd, resp, strlen(resp));
    if (err < 0) {
      writeTCP(fd, errS, 4);
      return -1;
    }

    // Open, read and send data file

    f = fopen(filename, "r");
    if (f == NULL) {
      writeTCP(fd, errS, 4);
      return -1;
    }

    nleft = size;
    while (nleft > 0) {
      memset(buffer, 0, BUFFER_SIZE);
      chunk = nleft < BUFFER_SIZE ? nleft : BUFFER_SIZE;
      err = fread(buffer, chunk, sizeof(char), f);
      if (err != chunk && feof(f)) {
        writeTCP(fd, errS, 4);
        return -1;
      }
      err = writeTCP(fd, buffer, chunk);
      if (err < 0) {
        writeTCP(fd, errS, 4);
        return -1;
      }
      nleft -= chunk;
    }

    fclose(f);

    // get image info

    memset(filename, 0, 128);
    err = sprintf(filename, "%s/%s/%s", path, answers[N], IMG_DATA);
    if (err < 0) {
      writeTCP(fd, errS, 4);
      return -1;
    }

    f = fopen(filename, "r");
    if (f == NULL) {
      writeTCP(fd, errS, 4);
      return -1;
    }

    // read qimg and ext
    err = fscanf(f, "%d %s", &qimg, ext);
    if (err < 0) {
      writeTCP(fd, errS, 4);
      return -1;
    }

    if (qimg == 0) {
      char s[8];
      err = N == 0 ? sprintf(s, " %d ", qimg) : sprintf(s, " %d%s", qimg, eol);
      if (err < 0) {
        writeTCP(fd, errS, 4);
        return -1;
      }
      err = writeTCP(fd, s, 4);
      if (err < 0) {
        writeTCP(fd, errS, 4);
        return -1;
      }
    } else { // If qimg == 1
      memset(filename, 0, 128);
      err = sprintf(filename, "%s/%s/%s.%s", path, answers[N], IMG, ext);
      if (err < 0) {
        writeTCP(fd, errS, 4);
        return -1;
      }

      // get size of data
      size = fileExists(filename);
      if (size < 1) {
        writeTCP(fd, errS, 4);
        return -1;
      }

      // write qimg, ext and size
      char s[16];
      err = sprintf(s, " %d %s %d ", qimg, ext, size);
      if (err < 0) {
        writeTCP(fd, errS, 4);
        return -1;
      }

      err = writeTCP(fd, s, strlen(s));
      if (err < 0) {
        writeTCP(fd, errS, 4);
        return -1;
      }

      // write imgdata
      f = fopen(filename, "r");
      if (f == NULL) {
        writeTCP(fd, errS, 4);
        return -1;
      }

      nleft = size;
      while (nleft > 0) {
        memset(buffer, 0, BUFFER_SIZE);
        chunk = nleft < BUFFER_SIZE ? nleft : BUFFER_SIZE;
        err = fread(buffer, chunk, sizeof(char), f);
        if (err != chunk && feof(f)) {
          writeTCP(fd, errS, 4);
          return -1;
        }
        err = writeTCP(fd, buffer, chunk);
        if (err < 0) {
          writeTCP(fd, errS, 4);
          return -1;
        }
        nleft -= chunk;
      }

      fclose(f);

      if (N == 0) {
        writeTCP(fd, eol, 1);
      }
    }
  }

  return 0;
}

int parseSubmitQuestion(char *buffer, int *uid, char *topic, char *question,
                        int *size) {

  int i = 0, t = 0, q = 0;

  int state = 0;

  while (state != 4) { // get id
    switch (state) {
    case 0: // UID
      *uid *= 10;
      *uid += buffer[i] - '0';
      break;
    case 1: // topic
      topic[t++] = buffer[i];
      break;
    case 2: // Question
      question[q++] = buffer[i];
      break;
    case 3: // Size
      *size *= 10;
      *size += buffer[i] - '0';
      break;
    }
    i++;
    if (buffer[i] == ' ') {
      switch (state) {
      case 1:
        topic[t] = 0;
        break;
      case 2:
        question[q] = 0;
        break;
      default:
        break;
      }
      state++;
      i++;
    }
  }

  if (sizeOfNumber(*uid) != 5 || strlen(topic) == 0 || strlen(question) == 0 ||
      sizeOfNumber(*size) > 10) {
    return -1;
    ;
  }

  return i;
}

int parseSubmitQuestionImage(char *buffer, char *ext, int *size) {

  int i;
  // First 3 chars are " ", QIMG, " "
  for (i = 0; i < 3; i++) {
    ext[i] = buffer[3 + i];
  }
  if (buffer[3 + i++] != ' ') {
    return -1;
    ;
  }

  while (buffer[3 + i] != ' ') {
    *size *= 10;
    *size += buffer[3 + i++] - '0';
  }

  return i + 4;
}

int handleSubmitQuestion(int fd) {
  char buffer[BUFFER_SIZE];
  int uid = 0, size = 0, qimg, isize = 0;
  char topic[16], question[16], ext[4];
  char path[64], filename[128];
  int nread, nleft, nproc, err, nprocimg, nleftimg, ndata;
  char response[16];
  DIR *dir;
  struct dirent *ent;
  FILE *f;
  fd_set mask;
  struct timeval timeout;
  timeout.tv_sec = 0;
  timeout.tv_usec = 0;

  FD_ZERO(&mask);
  FD_SET(fd, &mask);

  sprintf(response, "%s ", SUBMIT_QUESTION_RESPONSE);

  nread = read(fd, buffer,
               39); // 4 spaces + max_topic + max_question + max_size + uid_size

  printf("%s", buffer);

  nproc = parseSubmitQuestion(buffer, &uid, topic, question, &size);
  if (nproc < 0) {
    sprintf(response + strlen(response), "%s\n", NOK);
    write(fd, response, strlen(response));
    return 0;
  }

  // Check number of questions
  err = sprintf(path, "%s/%s", TOPICS, topic);
  if (err < 0) {
    writeTCP(fd, "ERR\n", 4);
    return -1;
  }

  int c = 0;
  dir = opendir(path);
  if (dir != NULL) {
    while ((ent = readdir(dir)) != NULL) {
      if ((strcmp(ent->d_name, ".") * strcmp(ent->d_name, "..") *
           strcmp(ent->d_name, USER)) != 0) {
        c++;
      }
    }
  }

  if (c == 99) {
    sprintf(response + strlen(response), "%s\n", FULL);
    writeTCP(fd, response, strlen(response));
    return 0;
  }

  // Check if everything exists
  err = sprintf(path + strlen(path), "/%s", question);
  if (err < 0) {
    writeTCP(fd, "ERR\n", 4);
    return -1;
  }

  dir = opendir(path);
  if (dir != NULL) {
    sprintf(response + strlen(response), "%s\n", DUP);
    write(fd, response, strlen(response));
    return 0;
  } else if (errno != ENOENT) {
    writeTCP(fd, "ERR\n", 4);
    return -1;
  }

  err = mkdir(path, 0700); // make question dir
  if (err < 0) {
    writeTCP(fd, "ERR\n", 4);
    return -1;
  }

  // make data dir

  err = sprintf(path + strlen(path), "/%s", DATA);
  if (err < 0) {
    writeTCP(fd, "ERR\n", 4);
    return -1;
  }

  err = mkdir(path, 0700); // make question dir
  if (err < 0) {
    writeTCP(fd, "ERR\n", 4);
    return -1;
  }

  // make question_uid file

  err = sprintf(filename, "%s/%s", path, USER);
  if (err < 0) {
    deleteDir(path);
    writeTCP(fd, "ERR\n", 4);
    return -1;
  }

  f = fopen(filename, "w");
  if (f == NULL) {
    deleteDir(path);
    writeTCP(fd, "ERR\n", 4);
    return -1;
  }

  err = fprintf(f, "%d", uid);
  if (err < 0) {
    deleteDir(path);
    writeTCP(fd, "ERR\n", 4);
    return -1;
  }

  fclose(f);

  // make data file, to be filled

  memset(filename, 0, 128);
  err = sprintf(filename, "%s/%s", path, QDATA);
  if (err < 0) {
    deleteDir(path);
    writeTCP(fd, "ERR\n", 4);
    return -1;
  }

  f = fopen(filename, "w");
  if (f == NULL) {
    deleteDir(path);
    writeTCP(fd, "ERR\n", 4);
    return -1;
  }

  // read rest of data

  nleft = size - (nread - nproc);
  int naux = nleft;

  if (nleft > 0) {

    for (int k = nproc; k < nread; k++) {
      fputc(buffer[k], f);
    }
    memset(buffer, 0, BUFFER_SIZE);
    while (naux > 0) {
      int chunk =
          naux < BUFFER_SIZE ? naux : BUFFER_SIZE; // read in BUFFER_SIZE chunks
                                                   // until nleft is smaller
      int n = read(fd, buffer, chunk);
      naux -= n;
      for (int l = 0; l < n; l++) {
        fputc(buffer[l], f);
      }
      printf("%s", buffer);
      memset(buffer, 0, BUFFER_SIZE);
    }
  } else {
    for (int l = 0; l < size; l++) {
      fputc(buffer[nproc + l], f);
    }
  }

  fclose(f);

  if (nleft > -3) { // If we havent already read qimg
    memset(buffer, 0, BUFFER_SIZE);
    read(fd, buffer, 3); // read qimg
    ndata = 3;
    nread = 3;
  } else {
    shiftLeft(buffer, nproc + size);
    // Shift so in both cases buffer=[" ", QIMG," "|| "\n", ...]
    ndata = nread - nproc - size;
  }

  if (buffer[0] != ' ') {
    deleteDir(path);
    sprintf(response + strlen(response), "%s\n", NOK);
    write(fd, response, strlen(response));
    return 0;
  }

  // Open IMG_DATA.txt
  memset(filename, 0, 128);
  err = sprintf(filename, "%s/%s", path, IMG_DATA);
  if (err < 0) {
    deleteDir(path);
    writeTCP(fd, "ERR\n", 4);
    return -1;
  }

  f = fopen(filename, "w");
  if (f == NULL) {
    deleteDir(path);
    writeTCP(fd, "ERR\n", 4);
    return -1;
  }

  qimg = buffer[1] - '0';

  printf(" %d ", qimg);

  if (qimg == 0) {
    if (buffer[2] != '\n') {
      deleteDir(path);
      sprintf(response + strlen(response), "%s\n", NOK);
      write(fd, response, strlen(response));
      return 0;
    }

    fprintf(f, "%d %s", qimg, "nil"); // nil is a placeholder
    fclose(f);
  } else if (qimg == 1) {
    int nimg = ndata;
    // If we read into the img area, this tells us how much

    if (nimg > 0) {
      if (nimg >= 18) { // means we read " 1 ext size "
        nprocimg = parseSubmitQuestionImage(buffer, ext, &isize);
        nleftimg = nimg - nprocimg;
      } else { // mean we have nothing guaranteed
        int limit = 18;
        int nreadimg = nimg;
        while (nreadimg < limit &&
               select(fd + 1, &mask, NULL, NULL,
                      &timeout)) { // Read until max_size is guaranteed or
                                   // nothing more to read
          nreadimg += read(fd, buffer + nreadimg, limit - nreadimg);
          FD_ZERO(&mask);
          FD_SET(fd, &mask);
          printf("%s", buffer);
        }

        nprocimg = parseSubmitQuestionImage(buffer, ext, &isize);
        nleftimg = nreadimg - nprocimg;
      }
    } else {
      int n = 3;
      int limit = 18;
      do { // Read until max_size is guaranteed or nothing more to read
        n += read(fd, buffer + n, limit - n);
        FD_ZERO(&mask);
        FD_SET(fd, &mask);
        printf("%s", buffer);
      } while (n < limit && select(fd + 1, &mask, NULL, NULL, &timeout));

      nprocimg = parseSubmitQuestionImage(buffer, ext, &isize);
      nleftimg = n - nprocimg;
    }

    // Fill IMG_DATA.txt file

    fprintf(f, "%d %s", qimg, ext);
    fclose(f);

    // Open IMG.ext file
    memset(filename, 0, 128);
    err = sprintf(filename, "%s/%s.%s", path, IMG, ext);
    if (err < 0) {
      deleteDir(path);
      memset(response, 0, 16);

      sprintf(response, "%s\n", ERROR);
      write(fd, response, strlen(response));
      return -1;
      ;
    }

    f = fopen(filename, "w");
    if (f == NULL) {
      deleteDir(path);
      memset(response, 0, 16);

      sprintf(response, "%s\n", ERROR);
      write(fd, response, strlen(response));
      return -1;
      ;
    }

    // Read image data
    int ndataimg = isize - nleftimg;
    if (ndataimg <= 0) { // We already read all the data
      for (int i = 0; i < isize; i++) {
        fputc(buffer[nprocimg + i], f);
      }
    } else {
      for (int i = 0; i < nleftimg; i++) {
        fputc(buffer[nprocimg + i], f);
      }

      int naux = ndataimg;
      int chunk = 0;
      while (naux > 0) {
        memset(buffer, 0, BUFFER_SIZE);
        chunk = naux < BUFFER_SIZE ? naux
                                   : BUFFER_SIZE; // read in BUFFER_SIZE chunks
                                                  // until nleft is smaller
        int n = read(fd, buffer, chunk);
        naux -= n;
        for (int l = 0; l < n; l++) {
          fputc(buffer[l], f);
        }
        printf("%s", buffer);
      }

      if (buffer[chunk - 1] != '\n') {
        deleteDir(path);
        sprintf(response + strlen(response), "%s\n", NOK);
        write(fd, response, strlen(response));
        return 0;
      }
    }

    fclose(f);
  }

  sprintf(response + strlen(response), "%s\n", OK);
  write(fd, response, strlen(response));

  printf("\n");
  return 0;
}

int handleSubmitAnswer(int fd) {
  char buffer[BUFFER_SIZE];
  int uid = 0, size = 0, qimg, isize = 0;
  char topic[16], question[16], ext[4];
  char path[64], filename[128];
  int nread, nleft, nproc, err, nprocimg, nleftimg, ndata;
  char response[16];
  DIR *dir;
  struct dirent *ent;
  FILE *f;
  fd_set mask;
  struct timeval timeout;
  timeout.tv_sec = 0;
  timeout.tv_usec = 0;

  FD_ZERO(&mask);
  FD_SET(fd, &mask);

  sprintf(response, "%s ", SUBMIT_ANSWER_RESPONSE);

  nread = read(fd, buffer,
               39); // 4 spaces + max_topic + max_question + max_size + uid_size

  printf("%s", buffer);

  nproc = parseSubmitQuestion(buffer, &uid, topic, question, &size);
  if (nproc < 0) {
    sprintf(response + strlen(response), "%s\n", NOK);
    write(fd, response, strlen(response));
    return 0;
  }

  // Check number of answers
  err = sprintf(path, "%s/%s/%s", TOPICS, topic, question);
  if (err < 0) {
    writeTCP(fd, "ERR\n", 4);
    return -1;
  }

  int c = 0;
  dir = opendir(path);
  if (dir != NULL) {
    while ((ent = readdir(dir)) != NULL) {
      if ((strcmp(ent->d_name, ".") * strcmp(ent->d_name, "..") *
           strcmp(ent->d_name, DATA)) != 0) {
        c++;
      }
    }
  }

  if (c == 99) {
    sprintf(response + strlen(response), "%s\n", FULL);
    writeTCP(fd, response, strlen(response));
    return 0;
  }

  char AN[4];
  if (c < 9) {
    sprintf(AN, "0%d", c + 1);
  } else {
    sprintf(AN, "%d", c + 1);
  }

  // Check if everything exists
  err = sprintf(path + strlen(path), "/%s_%s", question, AN);
  if (err < 0) {
    writeTCP(fd, "ERR\n", 4);
    return -1;
  }

  dir = opendir(path);
  if (dir != NULL) {
    sprintf(response + strlen(response), "%s\n", DUP);
    write(fd, response, strlen(response));
    return 0;
  } else if (errno != ENOENT) {
    writeTCP(fd, "ERR\n", 4);
    return -1;
  }

  err = mkdir(path, 0700); // make answer dir
  if (err < 0) {
    writeTCP(fd, "ERR\n", 4);
    return -1;
  }

  // make answer_uid file

  err = sprintf(filename, "%s/%s", path, USER);
  if (err < 0) {
    deleteDir(path);
    writeTCP(fd, "ERR\n", 4);
    return -1;
  }

  f = fopen(filename, "w");
  if (f == NULL) {
    deleteDir(path);
    writeTCP(fd, "ERR\n", 4);
    return -1;
  }

  err = fprintf(f, "%d", uid);
  if (err < 0) {
    deleteDir(path);
    writeTCP(fd, "ERR\n", 4);
    return -1;
  }

  fclose(f);

  // make data file, to be filled

  memset(filename, 0, 128);
  err = sprintf(filename, "%s/%s", path, ANS_DATA);
  if (err < 0) {
    deleteDir(path);
    writeTCP(fd, "ERR\n", 4);
    return -1;
  }

  f = fopen(filename, "w");
  if (f == NULL) {
    deleteDir(path);
    writeTCP(fd, "ERR\n", 4);
    return -1;
  }

  // read rest of data

  nleft = size - (nread - nproc);
  int naux = nleft;

  if (nleft > 0) {

    for (int k = nproc; k < nread; k++) {
      fputc(buffer[k], f);
    }
    memset(buffer, 0, BUFFER_SIZE);
    while (naux > 0) {
      int chunk =
          naux < BUFFER_SIZE ? naux : BUFFER_SIZE; // read in BUFFER_SIZE chunks
                                                   // until nleft is smaller
      int n = read(fd, buffer, chunk);
      naux -= n;
      for (int l = 0; l < n; l++) {
        fputc(buffer[l], f);
      }
      printf("%s", buffer);
      memset(buffer, 0, BUFFER_SIZE);
    }
  } else {
    for (int l = 0; l < size; l++) {
      fputc(buffer[nproc + l], f);
    }
  }

  fclose(f);

  if (nleft > -3) { // If we havent already read qimg
    memset(buffer, 0, BUFFER_SIZE);
    read(fd, buffer, 3); // read qimg
    ndata = 3;
    nread = 3;
  } else {
    shiftLeft(buffer, nproc + size);
    // Shift so in both cases buffer=[" ", QIMG," "|| "\n", ...]
    ndata = nread - nproc - size;
  }

  if (buffer[0] != ' ') {
    deleteDir(path);
    sprintf(response + strlen(response), "%s\n", NOK);
    write(fd, response, strlen(response));
    return 0;
  }

  // Open IMG_DATA.txt
  memset(filename, 0, 128);
  err = sprintf(filename, "%s/%s", path, IMG_DATA);
  if (err < 0) {
    deleteDir(path);
    writeTCP(fd, "ERR\n", 4);
    return -1;
  }

  f = fopen(filename, "w");
  if (f == NULL) {
    deleteDir(path);
    writeTCP(fd, "ERR\n", 4);
    return -1;
  }

  qimg = buffer[1] - '0';

  printf(" %d ", qimg);

  if (qimg == 0) {
    if (buffer[2] != '\n') {
      deleteDir(path);
      sprintf(response + strlen(response), "%s\n", NOK);
      write(fd, response, strlen(response));
      return 0;
    }

    fprintf(f, "%d %s", qimg, "nil"); // nil is a placeholder
    fclose(f);
  } else if (qimg == 1) {
    int nimg = ndata;
    // If we read into the img area, this tells us how much

    if (nimg > 0) {
      if (nimg >= 18) { // means we read " 1 ext size "
        nprocimg = parseSubmitQuestionImage(buffer, ext, &isize);
        nleftimg = nimg - nprocimg;
      } else { // mean we have nothing guaranteed
        int limit = 18;
        int nreadimg = nimg;
        while (nreadimg < limit &&
               select(fd + 1, &mask, NULL, NULL,
                      &timeout)) { // Read until max_size is guaranteed or
                                   // nothing more to read
          nreadimg += read(fd, buffer + nreadimg, limit - nreadimg);
          FD_ZERO(&mask);
          FD_SET(fd, &mask);
          printf("%s", buffer);
        }

        nprocimg = parseSubmitQuestionImage(buffer, ext, &isize);
        nleftimg = nreadimg - nprocimg;
      }
    } else {
      int n = 3;
      int limit = 18;
      do { // Read until max_size is guaranteed or nothing more to read
        n += read(fd, buffer + n, limit - n);
        FD_ZERO(&mask);
        FD_SET(fd, &mask);
        printf("%s", buffer);
      } while (n < limit && select(fd + 1, &mask, NULL, NULL, &timeout));

      nprocimg = parseSubmitQuestionImage(buffer, ext, &isize);
      nleftimg = n - nprocimg;
    }

    // Fill IMG_DATA.txt file

    fprintf(f, "%d %s", qimg, ext);
    fclose(f);

    // Open IMG.ext file
    memset(filename, 0, 128);
    err = sprintf(filename, "%s/%s.%s", path, IMG, ext);
    if (err < 0) {
      deleteDir(path);
      memset(response, 0, 16);

      sprintf(response, "%s\n", ERROR);
      write(fd, response, strlen(response));
      return -1;
      ;
    }

    f = fopen(filename, "w");
    if (f == NULL) {
      deleteDir(path);
      memset(response, 0, 16);

      sprintf(response, "%s\n", ERROR);
      write(fd, response, strlen(response));
      return -1;
      ;
    }

    // Read image data
    int ndataimg = isize - nleftimg;
    if (ndataimg <= 0) { // We already read all the data
      for (int i = 0; i < isize; i++) {
        fputc(buffer[nprocimg + i], f);
      }
    } else {
      for (int i = 0; i < nleftimg; i++) {
        fputc(buffer[nprocimg + i], f);
      }

      int naux = ndataimg;
      int chunk = 0;
      while (naux > 0) {
        memset(buffer, 0, BUFFER_SIZE);
        chunk = naux < BUFFER_SIZE ? naux
                                   : BUFFER_SIZE; // read in BUFFER_SIZE chunks
                                                  // until nleft is smaller
        int n = read(fd, buffer, chunk);
        naux -= n;
        for (int l = 0; l < n; l++) {
          fputc(buffer[l], f);
        }
        printf("%s", buffer);
      }

      if (buffer[chunk - 1] != '\n') {
        deleteDir(path);
        sprintf(response + strlen(response), "%s\n", NOK);
        write(fd, response, strlen(response));
        return 0;
      }
    }

    fclose(f);
  }

  sprintf(response + strlen(response), "%s\n", OK);
  write(fd, response, strlen(response));

  printf("\n");
  return 0;
}
