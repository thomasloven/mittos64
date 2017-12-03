#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>

typedef void (*tt_test)(void);

char *tt_filename;
char *tt_current_test;
int tt_fd[2];
int tt_color = 1;

extern tt_test tt_tests[];

#ifndef TT_BUFFER_SIZE
  #define TT_BUFFER_SIZE 512
#endif

#define TT_CLR_RED ((tt_color)?"\x1b[31m":"")
#define TT_CLR_GRN ((tt_color)?"\x1b[32m":"")
#define TT_CLR_BLU ((tt_color)?"\x1b[34m":"")
#define TT_CLR_RES ((tt_color)?"\x1b[0m":"")

#define TT_FAIL(error, ...) dprintf(tt_fd[1], "\"%s\" Line %d: %s >> " error "\n", tt_filename, __LINE__, tt_current_test, __VA_ARGS__);

#define ASSERT_EQUAL(type, pf, lhs, rhs) do { \
  type tt_lhs = (type)(lhs); \
  type tt_rhs = (type)(rhs); \
  if(tt_lhs == tt_rhs) return; \
  TT_FAIL("Expected <%" pf "> got <%" pf ">", tt_rhs, tt_lhs); \
  exit(1); \
}while(0);
#define ASSERT_NOT_EQUAL(type, pf, lhs, rhs) do { \
  type tt_lhs = (type)(lhs); \
  type tt_rhs = (type)(rhs); \
  if(tt_lhs != tt_rhs) return; \
  TT_FAIL("Got <%" pf "> but expected anything else", tt_rhs); \
  exit(1); \
}while(0);

#define ASSERT_STRN(lhs, rhs, n) do { \
  size_t tt_n = (size_t)(n); \
  char *tt_lhs = malloc(tt_n); \
  char *tt_rhs = malloc(tt_n); \
  memcpy(tt_lhs, (lhs), tt_n); \
  memcpy(tt_rhs, (rhs), tt_n); \
  tt_rhs[tt_n] = tt_lhs[tt_n] = '\0'; \
  if(!(strncmp(tt_lhs, tt_rhs, tt_n))) {free(tt_lhs); free(tt_rhs); return;} \
  TT_FAIL("Expected <%s> got <%s>\n", tt_rhs, tt_lhs); \
  free(tt_lhs); free(tt_rhs); \
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
      failed = 1;
      errors[failures] = buffer;
      buffer = malloc(TT_BUFFER_SIZE);
    }
    int status;
    waitpid(pid, &status, 0);
    if(!WIFEXITED(status))
    {
      failed = 1;
      sprintf(buffer, "\"%s\" >> TEST %d CRASHED\n", tt_filename, i+1);
      errors[failures] = buffer;
      buffer = malloc(TT_BUFFER_SIZE);
    }
    close(tt_fd[0]);
    if(failed)
    {
      failures++;
      printf("%sF%s", TT_CLR_RED, TT_CLR_RES);
    }
    else
    {
      printf("%s.%s", TT_CLR_GRN, TT_CLR_RES);
    }
    i++;
  }

  printf("\n");
  /* printf("%s----------------------------------------%s\n", TT_CLR_BLU, TT_CLR_RES); */
  printf("Ran %d tests in %s\n", i, tt_filename);
  free(buffer);
  if(failures)
  {
    printf("%sFAILED%s (failures=%d)\n", TT_CLR_RED, TT_CLR_RES, failures);
    i = 0;
    while(errors[i])
    {
  printf("%s========================================%s\n", TT_CLR_BLU, TT_CLR_RES);
      printf("%s", errors[i]);
      free(errors[i]);
      i++;
    }
  }
  free(errors);
  return failures;
}
