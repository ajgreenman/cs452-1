#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/resource.h>

#define MAX_VECTOR_SIZE 100
#define PIPE_READ 0
#define PIPE_WRITE 1

void complementer(char *binary_string);
int convert_binary_to_decimal(char *binary_string);

/**************************************************************************
 * Main driver of the program.                                            *
 **************************************************************************/
int main(int argc, char **argv)
{
  if(argc != 3) {
    printf("Usage: $ %s input_a input_b\n", argv[0]);
    return -1;
  }

  pid_t pid;
  FILE *vector_a, *vector_b;
  int pipe1_fd[2], pipe2_fd[2];
  char input_a[MAX_VECTOR_SIZE], input_b[MAX_VECTOR_SIZE];

  vector_a = fopen(argv[1], "r");
  vector_b = fopen(argv[2], "r");

  pid = fork();
  if(pid < 0) {
    fprintf(stderr, "First fork() failed.\n");
    return(1);
  } else if(pid) {
    pid = fork();
    if(pid < 0) {
      fprintf(stderr, "Second fork() failed.\n");
      return(1);
    } else if(pid) {
      // Process 2

      while(fgets(input_a, MAX_VECTOR_SIZE, vector_a) != NULL) {
        // printf("Parent Vector A: %s\n", input_a);
      }
    } else {
      // Process 1

      dup2(pipe1_fd[PIPE_READ], STDIN_FILENO);
    }
  } else {
    // Process 0

    while(fgets(input_b, MAX_VECTOR_SIZE, vector_b) != NULL) {
      // Reads in and processes input_b line by line.

      char *position;
      if((position = strchr(input_b, '\n')) != NULL) {
        *position = '\0'; // Removes newline character from end of buffer.
      }

      complementer(input_b); // Bitwise complement on the binary number.

      dup2(pipe1_fd[PIPE_WRITE], STDOUT_FILENO); // Set up a pipe to write to.
      write(STDOUT_FILENO, (const void *) input_b, (size_t) strlen(input_b) + 1);
      close(pipe1_fd[PIPE_READ]);
      close(pipe1_fd[PIPE_WRITE]);
    }

    exit(0);
  }

  return 0;
}

/**************************************************************************
 * Converts a binary number in string form into a decimal number in       *
 * integer form. The placeholder variable holds each bit in integer form. *
 * The multiplier variables is used to multiply the placeholder by the    *
 * correct power of two. The sum contains the running sum of the binary   *
 * number. The final integer representation of the number, held by the    *
 * variable sum, is returned.                                             *
 **************************************************************************/
int convert_binary_to_decimal(char *binary_string)
{
  int length = strlen(binary_string), sum = 0, multiplier = 0, placeholder;

  for(int i = length - 1; i >= 0; i--) {
    placeholder = binary_string[i] - '0';
    sum += placeholder * (1 << multiplier++);
  }

  return sum;
}

/**************************************************************************
 * Performs bitwise complement on a binary number in string form.         *
 **************************************************************************/
void complementer(char *binary_string)
{
  int length = strlen(binary_string);

  for(int i = 0; i < length; i++) {
    if(binary_string[i] == '0') {
      binary_string[i] = '1';
    } else {
      binary_string[i] = '0';
    }
  }
}