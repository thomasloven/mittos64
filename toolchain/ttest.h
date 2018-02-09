#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>


#define TT_FAIL(error, ...) dprintf(tt_pipe[1], "\"%s\" Line %d: %s >> " error "\n", tt_current->filename, __LINE__, tt_current->name, __VA_ARGS__);


#define ASSERT_EQUAL(type, pf, lhs, rhs) do { \
  type tt_lhs = (type)(lhs); \
  type tt_rhs = (type)(rhs); \
  if(tt_lhs != tt_rhs) { \
    TT_FAIL("Expected <%" pf "> got <%" pf ">", tt_rhs, tt_lhs); \
    return 1; \
  } \
  return 0; \
}while(0);

#define ASSERT_NOT_EQUAL(type, pf, lhs, rhs) do { \
  type tt_lhs = (type)(lhs); \
  type tt_rhs = (type)(rhs); \
  if(tt_lhs == tt_rhs) { \
    TT_FAIL("Got <%" pf "> but expected anything else", tt_rhs); \
    return 1; \
  } \
  return 0; \
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
  return 0; \
}while(0);

#define ASSERT_EQ_INT(lhs, rhs) ASSERT_EQUAL(int, "d", lhs, rhs)
#define ASSERT_NEQ_INT(lhs, rhs) ASSERT_NOT_EQUAL(int, "d", lhs, rhs)
#define ASSERT_EQ_CHR(lhs, rhs) ASSERT_EQUAL(char, "c", lhs, rhs)
#define ASSERT_NEQ_CHR(lhs, rhs) ASSERT_NOT_EQUAL(char, "c", lhs, rhs)
#define ASSERT_EQ_STR(lhs, rhs, n) ASSERT_STRN(lhs, rhs, n)

#define TEST(name) \
  int ttt_##name(); \
  __attribute__((constructor)) void tttr_##name() { \
    tt_register(#name, ttt_##name, __FILE__); \
  } \
    int ttt_##name()

#define BEFORE() void tt_before()
#define AFTER() void tt_after()

void __attribute__((weak)) tt_before(void);
void __attribute__((weak)) tt_after(void);

#ifndef TT_BUFFER_SIZE
  #define TT_BUFFER_SIZE 512
#endif

struct tt_test
{
  char *filename;
  char *name;
  int (*test)(void);
  int status;
  char *output;
};

// Global variables
int tt_pipe[2];
int tt_color = 1;
int tt_verbose = 0;
int tt_silent = 0;
struct tt_test *tt_tests;
struct tt_test *tt_current;
int tt_max_name_len = 0;
int tt_test_count = 0;

void tt_register(char *name, int (*fn)(void), char *filename)
{
  tt_tests = realloc(tt_tests, (tt_test_count+1)*sizeof(struct tt_test));

  struct tt_test *t = &tt_tests[tt_test_count++];
  t->filename = filename;
  t->name = name;
  t->test = fn;
  t->status = 1;
  t->output = malloc(TT_BUFFER_SIZE);
  if(strlen(name) > tt_max_name_len)
    tt_max_name_len = strlen(name);
}


#define TT_CLR_RED ((tt_color)?"\x1b[31m":"")
#define TT_CLR_GRN ((tt_color)?"\x1b[32m":"")
#define TT_CLR_YEL ((tt_color)?"\x1b[33m":"")
#define TT_CLR_RES ((tt_color)?"\x1b[0m":"")

int main(int argc, char **argv)
{
  if(!isatty(1)) tt_color = 0;
  int opt;
  while((opt = getopt(argc, argv, "vscn")) != -1)
  {
    switch(opt)
    {
      case 'v':
        tt_verbose = 1;
        tt_silent = 0;
        break;
      case 's':
        tt_silent = 1;
        tt_verbose = 0;
        break;
      case 'c':
        tt_color = 1;
        break;
      case 'n':
        tt_color = 0;
        break;
    }
  }

  int ok = 0;
  int failed = 0;
  int crashed = 0;

  if(!tt_silent)
    printf("\n%s\n", tt_tests[0].filename);

  for(int i = 0; i < tt_test_count; i++)
  {
    tt_current = &tt_tests[i];

    fflush(stdout);
    pipe(tt_pipe);

    int pid;
    if(!(pid = fork()))
    {
      // Run test
      close(tt_pipe[0]);
      if(tt_before) tt_before();
      int result = tt_current->test();
      if(tt_after) tt_after();
      exit(result);
    }

    // Capture test output
    close(tt_pipe[1]);
    read(tt_pipe[0], tt_current->output, TT_BUFFER_SIZE);
    close(tt_pipe[0]);

    // Determine if test passed or not
    int status;
    waitpid(pid, &status, 0);
    if(!WIFEXITED(status))
    {
      crashed++;
      tt_current->status = -1;
    } else if(WEXITSTATUS(status)) {
      failed++;
      tt_current->status = WEXITSTATUS(status);
    } else {
      ok++;
      tt_current->status = 0;
    }

    // Output progress
    if(tt_verbose)
      printf("%3d/%3d  %-*s ", i+1, tt_test_count,
          tt_max_name_len, tt_current->name);
    switch(tt_current->status)
    {
      case 0:
        if(tt_verbose)
          printf("[%sOK%s]\n", TT_CLR_GRN, TT_CLR_RES);
        else
          printf("%s.%s", TT_CLR_GRN, TT_CLR_RES);
        break;
      case -1:
        if(tt_verbose)
          printf("[%sCRASHED%s]\n", TT_CLR_RED, TT_CLR_RES);
        else
          printf("%sC%s", TT_CLR_RED, TT_CLR_RES);
        break;
      default:
        if(tt_verbose)
          printf("[%sFAILED%s]\n", TT_CLR_YEL, TT_CLR_RES);
        else
          printf("%sF%s", TT_CLR_YEL, TT_CLR_RES);
    }

  }

  int retval = failed+crashed;

  // Print summary
  if(!tt_silent)
  {
    printf("\n%s", (retval)?TT_CLR_RED:TT_CLR_GRN);
    printf("%d tests, %d failures", tt_test_count, retval);
    printf("%s\n", TT_CLR_RES);
  }

  if(retval && tt_silent)
    printf("\n");

  // Print any errors
  for(int i = 0; i < tt_test_count; i++)
  {
    if(tt_tests[i].status)
      printf("%s", tt_tests[i].output);
    free(tt_tests[i].output);
  }


  return retval;
}
