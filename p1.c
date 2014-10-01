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
#define DIAG // Set to have diagnostic messages printed during execution.

void complementer(char *binary_string);
long long convert_binary_to_decimal(char *binary_string);
void convert_decimal_to_binary(FILE *output, long long decimal_value, int length);

/**************************************************************************
 * Main driver of the program.                                            *
 **************************************************************************/
int main(int argc, char **argv)
{
  if(argc != 3) {
    printf("Usage: $ %s input_a input_b\n", argv[0]);
    return -1;
  }

  #ifdef DIAG
    printf("Starting program...\n");
    sleep(1);
  #endif

  // Declare variables.
  pid_t pid;
  FILE *vector_a, *vector_b, *output;
  int pipe1_fd[2], pipe2_fd[2], length;;
  long long decimal_value, decimal_value2;
  char input_a[MAX_VECTOR_SIZE], input_b[MAX_VECTOR_SIZE];

  // Open input and output files.
  vector_a = fopen(argv[1], "r");
  vector_b = fopen(argv[2], "r");
  output = fopen("output.txt", "w");

  // Create pipes.
  if(pipe(pipe1_fd)) {
    perror("Plumbing problem.\n");
    exit(1);
  }

  if(pipe(pipe2_fd)) {
    perror("Plumbing problem.\n");
    exit(1);
  }

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

      #ifdef DIAG
        printf("Process 2 created.\n");
        sleep(2);
      #endif

      close(pipe1_fd[PIPE_READ]); // Close the read portion of pipe 1.
      close(pipe1_fd[PIPE_WRITE]); // Close the write portion of pipe 1.
      close(pipe2_fd[PIPE_WRITE]); // Close the write portion of pipe 2.

      while(fgets(input_a, MAX_VECTOR_SIZE, vector_a) != NULL) {
        // Reads in and process input_a line by line.

        char *position;
        if((position = strchr(input_a, '\n')) != NULL) {
          *position = '\0'; // Removes newline character from end of buffer.
        }

        #ifdef DIAG
          printf("Process 2: Read line '%s' from input_a file.\n", input_a);
        #endif

        length = strlen(input_a); // Get the length in bits of input_a.
        decimal_value = convert_binary_to_decimal(input_a); // Convert the binary number to decimal for easier math.

        #ifdef DIAG
          printf("Process 2: Converted '%s' to integer value %lld.\n", input_a, decimal_value);
        #endif

        read(pipe2_fd[PIPE_READ], input_b, sizeof(input_b)); // Reads in the two's complimented input_b from the pipe.
        #ifdef DIAG
          printf("Process 2: Read '%s' from pipe 2.\n", input_b);
        #endif

        decimal_value2 = strtoll(input_b,  NULL, 10); // Convert the binary number to decimal for easier math.
        #ifdef DIAG
          printf("Process 2: Converted '%s' to integer value %lld.\n", input_b, decimal_value2);
        #endif

        decimal_value += decimal_value2; // Add input_a with input_b.

        #ifdef DIAG
          printf("Process 2: Added %lld with %lld to get %lld.\n", decimal_value - decimal_value2, decimal_value2, decimal_value);
        #endif

        convert_decimal_to_binary(output, decimal_value, length); // Convert the decimal number back to a binary string.

        #ifdef DIAG
          printf("Process 2: Converting %lld to binary and writing to file \"output.txt\".\n", decimal_value);
        #endif

        #ifdef DIAG
          sleep(3);
        #endif
      }

      #ifdef DIAG
        printf("Process 2: No more input, closing pipe and exiting.\n");
      #endif

      close(pipe2_fd[PIPE_READ]); // Close the read portion of pipe 2.

      exit(0);
    } else {
      // Process 1

      #ifdef DIAG
        printf("Process 1 created.\n");
        sleep(2);
      #endif

      close(pipe1_fd[PIPE_WRITE]); // Close the write portion of pipe 1.
      close(pipe2_fd[PIPE_READ]); // Close the read portion of pipe 2.

      while(read(pipe1_fd[PIPE_READ], input_b, sizeof(input_b))) {
        #ifdef DIAG
          printf("Process 1: Read '%s' from pipe 1.\n", input_b);
        #endif

        decimal_value = convert_binary_to_decimal(input_b); // Convert the binary number to decimal for easier math.
        #ifdef DIAG
          printf("Process 1: Converted '%s' to integer value %lld.\n", input_b, decimal_value);
        #endif

        decimal_value++; // Increments the decimal value by 1.
        #ifdef DIAG
          printf("Process 1: Incremented by 1 to %lld.\n", decimal_value);
        #endif

        sprintf(input_b, "%lld", decimal_value); // Convert from long long to string.

        #ifdef DIAG
          printf("Process 1: Writing to pipe 2.\n");
        #endif
        // Write the incremented number to the pipe.
        write(pipe2_fd[PIPE_WRITE], (const void *) input_b, (size_t) strlen(input_b) + 1);

        #ifdef DIAG
          sleep(3);
        #endif
      }

      #ifdef DIAG
        printf("Process 1: No more input, closing pipes and exiting.\n");
      #endif

      close(pipe1_fd[PIPE_READ]); // Close the read portion of pipe 1.
      close(pipe2_fd[PIPE_WRITE]); // Close the write portion of pipe 2.

      exit(0);
    }
  } else {
    // Process 0

    #ifdef DIAG
      printf("Process 0 created.\n");
      sleep(2);
    #endif

    close(pipe1_fd[PIPE_READ]); // Close the read portion of pipe 1.
    close(pipe2_fd[PIPE_READ]); // Close the read portion of pipe 2.
    close(pipe2_fd[PIPE_WRITE]); // Close the write portion of pipe 2.

    while(fgets(input_b, MAX_VECTOR_SIZE, vector_b) != NULL) {
      // Reads in and processes input_b line by line.

      char *position;
      if((position = strchr(input_b, '\n')) != NULL) {
        *position = '\0'; // Removes newline character from end of buffer.
      }

      #ifdef DIAG
        printf("Process 0: Read line '%s' from input_b file.\n", input_b);
      #endif

      complementer(input_b); // Bitwise complement on the binary number.

      #ifdef DIAG
        printf("Process 0: Complemented input to '%s'.\n", input_b);
      #endif

      // Write the complemented number to the pipe.

      #ifdef DIAG
        printf("Process 0: Writing to pipe 1.\n");
      #endif

      write(pipe1_fd[PIPE_WRITE], (const void *) input_b, (size_t) strlen(input_b) + 1);

      #ifdef DIAG
        sleep(3);
      #endif
    }

    #ifdef DIAG
      printf("Process 0: No more input, closing pipe and exiting.\n");
    #endif

    close(pipe1_fd[PIPE_WRITE]); // Close the write portion of pipe 1.

    exit(0);
  }

  return 0;
}

/**************************************************************************
 * Converts a binary number in string form into a decimal number in       *
 * integer form. The placeholder variable holds each bit in integer form. *
 * The multiplier variables is used to multiply the placeholder by the    *
 * correct power of two. The sum contains the running sum of the binary   *
 * number. Since the value is in two's compliment, a special step is      *
 * taken to convert to a negative number. The final integer               *
 * representation of the number, held by the variable sum, is returned.   *
 **************************************************************************/
long long convert_binary_to_decimal(char *binary_string)
{
  int length = strlen(binary_string), sum = 0, multiplier = 0, placeholder;

  for(int i = length - 1; i >= 0; i--) {
    placeholder = binary_string[i] - '0';
    sum += placeholder * (1 << multiplier++);
  }

  if(binary_string[0] == '1') {
    int diff = 1;

    for(int p = 1; p <= length; p++) {
      diff *= 2;
    }

    sum -= diff;
  }

  return (long long) sum;
}

/**************************************************************************
 * Converts a decimal value to binary and writes it to a file. Goes bit   *
 * by bit and writes a 1 for each 1 and a 0 for each 0.                   *
 **************************************************************************/
void convert_decimal_to_binary(FILE *output, long long decimal_value, int length)
{
  int n;

  for(int i = length - 1; i >= 0; i--) {
    n = decimal_value >> i;

    if(n & 1) {
      fprintf(output, "1");
    } else {
      fprintf(output, "0");
    }
  }

  fprintf(output, "\n");
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