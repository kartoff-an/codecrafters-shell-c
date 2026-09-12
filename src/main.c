#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
  // Flush after every printf
  setbuf(stdout, NULL);

  while (1) {
    printf("$ ");

    // reading the user's command
    char input[100];
    fgets(input, sizeof(input), stdin);

    // printing the input not found message
    input[strcspn(input, "\n")] = '\0';
    printf("%s: command not found\n", input);
  }

  return 0;
}
