#include "consts.h"

int max(int x, int y) {
  if (x > y)
    return x;
  else
    return y;
}

int comparator(const void *p, const void *q) {
  char p_c[32], q_c[32];
  struct stat p_s = {0}, q_s = {0};
  sprintf(p_c, "%s/%s", TOPICS, (char *)p);
  sprintf(q_c, "%s/%s", TOPICS, (char *)q);
  stat(p_c, &p_s);
  stat(q_c, &q_s);

  return (int)(p_s.st_mtim.tv_sec - q_s.st_mtim.tv_sec);
}

int sizeOfNumber(int n) {
  if (n < 10) {
    return 1;
  } else {
    return 1 + sizeOfNumber(n / 10);
  }
}

int parseCommand(char *buffer, char *commandArgs[]) {
  char *token;
  int i, count = 0, size = strlen(buffer);
  token = strtok(buffer, " ");

  i = -1; // Num of spaces = num of tokens - 1
  while ((token != NULL) && (i < COMMANDS)) {
    if (strlen(token) <= ARG_SIZE) {
      count += strlen(token);
      strcpy(commandArgs[++i], token);
      token = strtok(NULL, " ");
    } else {
      return INVALID;
    }
  }
  if ((i >= COMMANDS) && (token != NULL)) {
    return INVALID;
  }

  if ((count + i) !=
      size) { // Checks if each token is separated by 1 single space.
    printf("More spaces than expected\n");
    return INVALID;
  } else {
    return VALID;
  }
}

int isValidTopic(char *topic) {
  if (strlen(topic) <= TOPIC_SIZE) {
    return VALID;
  } else {
    return INVALID;
  }
}

int isValidId(char *userId) {
  if (strlen(userId) == USER_ID_SIZE) {
    return VALID;
  } else {
    return INVALID;
  }
}

int isnumber(char *number) {
  int i, size;
  size = strlen(number);
  for (i = 0; i < size; i++) {
    if (!isdigit(number[i]) && (number[i] != '\0')) {
      return INVALID;
    }
  }
  return VALID;
}

bool fileExists(char *filename) {
  struct stat buff;
  return (stat(filename, &buff) == 0);
}

char *copyFile(char *filename) {
  char *aux = 0;
  char *content;
  FILE *fp;
  int size, status;

  fp = fopen(filename, "r");
  if (fp == NULL) {
    printf("Error opening file %s.\n", filename);
    return NULL;
  }
  status = fseek(fp, 0L, SEEK_END);
  size = ftell(fp);
  if (status == -1 || size == -1) {
    printf("Error finding %s size.\n", filename);
    return NULL;
  }

  aux = (char *)malloc(size * sizeof(char));
  if (aux == NULL) {
    printf("Error allocating memory.\n");
    return NULL;
  }
  content = (char *)malloc(size * sizeof(char));
  if (content == NULL) {
    printf("Error allocating memory.\n");
    return NULL;
  }

  rewind(fp);

  if (aux) {
    fread(aux, 1, size, fp);
    if (ferror(fp) != 0) {
      printf("Error reading file %s.\n", filename);
      return NULL;
    }
  }

  strcpy(content, aux);
  printf("%s", content);
  free(aux);

  status = fclose(fp);
  if (status == -1 || size == -1) {
    printf("Error closing file %s.\n", filename);
    return NULL;
  }
  return content;
}

void shiftLeft(char *string, int shiftLength) {

  int i, size = strlen(string);
  if (shiftLength >= size) {
    memset(string, '\0', size);
    return;
  }

  for (i = 0; i < size - shiftLength; i++) {
    string[i] = string[i + shiftLength];
    string[i + shiftLength] = '\0';
  }
}

void deleteDir(const char path[]) {
  size_t path_len;
  char *full_path;
  DIR *dir;
  struct stat stat_path, stat_entry;
  struct dirent *entry;

  // stat for the path
  stat(path, &stat_path);

  // if path does not exists or is not dir - exit with status -1
  if (S_ISDIR(stat_path.st_mode) == 0) {
    fprintf(stderr, "%s: %s\n", "Is not directory", path);
    exit(-1);
  }

  // if not possible to read the directory for this user
  if ((dir = opendir(path)) == NULL) {
    fprintf(stderr, "%s: %s\n", "Can`t open directory", path);
    exit(-1);
  }

  // the length of the path
  path_len = strlen(path);

  // iteration through entries in the directory
  while ((entry = readdir(dir)) != NULL) {

    // skip entries "." and ".."
    if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
      continue;

    // determinate a full path of an entry
    full_path = calloc(path_len + strlen(entry->d_name) + 1, sizeof(char));
    strcpy(full_path, path);
    strcat(full_path, "/");
    strcat(full_path, entry->d_name);

    // stat for the entry
    stat(full_path, &stat_entry);

    // recursively remove a nested directory
    if (S_ISDIR(stat_entry.st_mode) != 0) {
      deleteDir(full_path);
      continue;
    }

    // remove a file object
    if (unlink(full_path) != 0)
      printf("\tCan`t remove a file: %s\n", full_path);
  }

  // remove the devastated directory and close the object of it
  if (rmdir(path) != 0)
    printf("Can`t remove a directory: %s\n", path);

  closedir(dir);
}
