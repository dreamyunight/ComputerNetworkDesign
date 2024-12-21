#include <cstdio>
#include <cstdlib>
#include "util.h"

void errif(int condition, const char *errmsg) {
    if (condition == -1) {
      perror(errmsg);
      exit(EXIT_FAILURE);
    }
}