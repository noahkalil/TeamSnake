#include <stdio.h>
#include <stdlib.h>

int main() {
  char*   buffer = NULL;
  size_t  n;

  getline(&buffer, &n, stdin);
  printf("You typed: %s", buffer);

  free(buffer);
  return(0);
}
