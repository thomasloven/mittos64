#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>

typedef int (*tt_test)(void);

char *tt_filename;
char *tt_current_test;
int tt_fd[2];
int tt_color = 1;

extern tt_test tt_tests[];
extern char *tt_test_names[];

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
  if(tt_lhs != tt_rhs) { \
  TT_FAIL("Expected <%" pf "> got <%" pf ">", tt_rhs, tt_lhs); \
  return 1; \
  } \
}while(0);
#define ASSERT_NOT_EQUAL(type, pf, lhs, rhs) do { \
  type tt_lhs = (type)(lhs); \
  type tt_rhs = (type)(rhs); \
  if(tt_lhs == tt_rhs) { \
  TT_FAIL("Got <%" pf "> but expected anything else", tt_rhs); \
  return 1; \
  } \
}while(0);

#define ASSERT_STRN(lhs, rhs, n) do { \
  char *tt_lhs = (char *)(lhs); \
  char *tt_rhs = (char *)(rhs); \
  if(!tt_lhs || !tt_rhs) \
  { \
    TT_FAIL("Expected string, got null pointer", 0); \
    return 1; \
  } \
  size_t tt_n = (size_t)(n); \
  char *tt_lhs_c = malloc(tt_n+1); \
  char *tt_rhs_c = malloc(tt_n+1); \
  memcpy(tt_lhs_c, tt_lhs, tt_n); \
  memcpy(tt_rhs_c, tt_rhs, tt_n); \
  tt_rhs_c[tt_n] = tt_lhs_c[tt_n] = '\0'; \
  if(strncmp(tt_lhs_c, tt_rhs_c, tt_n)) { \
  TT_FAIL("Expected <%s> got <%s>", tt_rhs_c, tt_lhs_c); \
  free(tt_lhs_c); free(tt_rhs_c); \
  return 1; \
  } \
  free(tt_lhs_c); free(tt_rhs_c); \
}while(0);

#define ASSERT_EQ_INT(lhs, rhs) ASSERT_EQUAL(int, "d", lhs, rhs)
#define ASSERT_NEQ_INT(lhs, rhs) ASSERT_NOT_EQUAL(int, "d", lhs, rhs)
#define ASSERT_EQ_CHR(lhs, rhs) ASSERT_EQUAL(char, "c", lhs, rhs)
#define ASSERT_NEQ_CHR(lhs, rhs) ASSERT_NOT_EQUAL(char, "c", lhs, rhs)
#define ASSERT_EQ_STR(lhs, rhs, n) ASSERT_STRN(lhs, rhs, n)

#define TEST(name) int ttt_##name()

#define BEFORE() void tt_before()
#define AFTER() void tt_after()

void __attribute__((weak)) tt_before(void);
void __attribute__((weak)) tt_after(void);

int main(int argc, char **argv)
{
  tt_filename = argv[1];

  if(!isatty(1)) tt_color = 0;

  char *buffer = malloc(TT_BUFFER_SIZE);
  char **errors = 0;

  int failures = 0;
  int i = 0;
  while(tt_tests[i])
  {
    fflush(stdout);
    pipe(tt_fd);
    int pid;
    if(!(pid = fork()))
    {
      close(tt_fd[0]);
      tt_current_test = tt_test_names[i];
      if(tt_before) tt_before();
      int result = tt_tests[i]();
      if(tt_after) tt_after();
      exit(result);
    }

    close(tt_fd[1]);
    int status;
    waitpid(pid, &status, 0);
    int failed = 0;
    if(read(tt_fd[0], buffer, TT_BUFFER_SIZE))
    {
      failed = 1;
    }
    close(tt_fd[0]);
    if(!WIFEXITED(status))
    {
      failed = 1;
      sprintf(buffer, "\"%s\" >> TEST %d CRASHED\n", tt_filename, i+1);
    }
    if(failed)
    {
      failures++;
      errors = realloc(errors, failures*sizeof(char *));
      errors[failures-1] = buffer;
      buffer = malloc(TT_BUFFER_SIZE);
      printf("%sF%s", TT_CLR_RED, TT_CLR_RES);
    }
    else
    {
      printf("%s.%s", TT_CLR_GRN, TT_CLR_RES);
    }
    i++;
  }

  printf("\n");
  printf("Ran %d tests in %s\n", i, tt_filename);
  free(buffer);
  if(failures)
  {
    printf("%sFAILED%s (failures=%d)\n", TT_CLR_RED, TT_CLR_RES, failures);
    i = 0;
    printf("%s========================================%s\n", TT_CLR_RED, TT_CLR_RES);
    while(i < failures)
    {
      printf("%s", errors[i]);
      free(errors[i]);
      i++;
    }
    printf("%s========================================%s\n", TT_CLR_RED, TT_CLR_RES);
  free(errors);
  }

  return failures;
}
