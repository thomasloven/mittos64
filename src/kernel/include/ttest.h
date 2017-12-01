#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>

typedef void (*tt_test)(void);

char *tt_filename;
char *tt_current_test;
int tt_fd[2];
int tt_color = 1;

extern tt_test tt_tests[];

#ifndef TT_BUFFER_SIZE
  #define TT_BUFFER_SIZE 512
#endif

#define TT_FAIL(error, ...) dprintf(tt_fd[1], "\"%s\" Line %d: %s >> " error "\n", tt_filename, __LINE__, tt_current_test, __VA_ARGS__);

#define ASSERT_EQUAL(type, pf, lhs, rhs) do { \
  type tt_lhs = (lhs); \
  type tt_rhs = (rhs); \
  if(tt_lhs == tt_rhs) return; \
  TT_FAIL("Expected <%" pf "> got <%" pf ">", rhs, lhs); \
  exit(1); \
}while(0);

#define TEST(name, body) void ttt_##name() { tt_current_test = #name; if(tt_before) tt_before(); body }

#define BEFORE(body) void tt_before() { body }

void __attribute__((weak)) tt_before(void);

int main(int argc, char **argv)
{
  tt_filename = argv[1];

  if(!isatty(1)) tt_color = 0;

  int i = 0;
  while(tt_tests[i++]);

  char **errors = calloc(i, sizeof(char*));
  char *buffer = malloc(TT_BUFFER_SIZE);

  int failures = 0;
  i = 0;
  while(tt_tests[i])
  {
    fflush(stdout);
    pipe(tt_fd);
    int pid;
    if(!(pid = fork()))
    {
      close(tt_fd[0]);
      tt_tests[i]();
      exit(0);
    }

    close(tt_fd[1]);
    int failed = 0;
    int bytes = 0;
    if(bytes = read(tt_fd[0], buffer, TT_BUFFER_SIZE))
    {
      printf("%sF%s", tt_color?"\x1b[31m":"", tt_color?"\x1b[0m":"");
      errors[failures] = buffer;
      failures ++;
      buffer = malloc(TT_BUFFER_SIZE);
    } else {
      printf("%s.%s", tt_color?"\x1b[32m":"", tt_color?"\x1b[0m":"");
    }
    waitpid(pid, 0,0);
    close(tt_fd[0]);
    i++;
  }

  printf("\n");
  printf("%s----------------------------------------%s\n", tt_color?"\x1b[32m":"", tt_color?"\x1b[0m":"");
  printf("Ran %d tests in %s\n", i, tt_filename);
  free(buffer);
  if(failures)
  {
    printf("%sFAILED%s (failures=%d)\n", tt_color?"\x1b[31m":"", tt_color?"\x1b[0m":"", failures);
    i = 0;
    while(errors[i])
    {
  printf("%s========================================%s\n", tt_color?"\x1b[1;34m":"", tt_color?"\x1b[0m":"");
      printf("%s", errors[i]);
      free(errors[i]);
      i++;
    }
  }
  free(errors);
  return failures;
}
