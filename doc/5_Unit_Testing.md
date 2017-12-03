# Unit Testing Framework

In this chapter we'll build a unit testing framework that will help us
developing the rest of the kernel.

## Why build it yourself?

As far as I understand it, there's really no good arguments for building
your own unit testing framework. There are lots of finished alternatives
available allready, and using them will let you focus on your main
project instead.

I'll write my own, simply because I feel like it. If you don't, then use
an existing one.

## Requirements

I don't have very many requirements for the testing framework. But it's important that:

- It runs fast
- It's clear what failed
- Tests are isolated so that they don't influence each other

I also want the tests to be "close" to the code. I was thinking of
including them in the actual source file, but instead opted to put them
in a separate file with the same name, but with a `.tt` extension. (tt
for Thomas-Test).

## Designing the framework

The framework will be in two parts; a c source file to include in each
test, and a shell script to run all tests.

Let's start with the shell script.

It should scan the supplied directories for `.tt` files, compile them and run them:

`ttest`
```sh
#!/bin/sh

dirs = src/kernel

main()
{
  for dir in $dirs; do
    local files=`find $dir -name "*.tt"
    for suite in $files; do
      test_exec="${suite}"est
      cc -x c $suite.c -o $test_exec -I $dir/include -I toolchain -DTTEST`
      $test_exec
      rm $suite.c $test_exec
    done
  done
}
```

`$test_exec` contains the name of the compiled test executable. It will get the
extension `.ttest`. Since gcc doesn't know how to compile `.tt` files we tell
it that it contains c code with the `-x`  switch.

Now let's take a look at the c source file (well... it's really a header file
*acting* like a c source file...)

This should contain the `main()` function of the test which should run each
test in turn and keep track of any failures. But first, we have a struct to
keep track of everything related to a test.

`toolchain/ttest.h`
```c
...
struct tt_test
{
  char *name;
  int(*test)(void);
  int status;
  char *output;
};

struct tt_test *tt_tests;
struct tt_test *tt_current;
int tt_test_count = 0;
...
```

Next for the main function. For insolation, each test is forked off and run as
a separate process. Errors are passed back to the main process through a pipe.

`toolchain/ttest.h`
```c
...
int tt_pipe[2];

int main(int argc, char **argv)
{
  for(int i = 0; i < tt_test_count; i++)
  {
    tt_current = &tt_tests[i];

    fflush(stdout);
    pipe(tt_pipe);

    int pid;
    if(!(pid = fork()))
    {
      close(tt_pipe[0]); // Close read end of pipe
      exit(tt_current->test());
    }

    close(tt_pipe[1]); // Close write end of pipe
    read(tt_pipe[0], tt_current->output, TT_BUFFER_SIZE);
    close(tt_pipe[0]);

    int status;
    waitpid(pid, &status, 0);
    if((tt_current->status = WEXITSTATUS(status)))
    {
      printf("F");
    } else {
      printf(".");
    }
  }

  for(int i = 0; i < tt_test_cound; i++)
  {
    if(tt_tests[i].status)
      printf("%s: %s\n", tt_tests[i].name, tt_tests[i].output)
  }
}
```

This is, of course severey simplified. I also have a lot of prettification,
keeping track of the number of failures, checking if the test crashed before
finishing and such...

Ok, so how is the `tt_tests` array populated?

Well. There's a macro to define each test in the `.tt`-file:

`toolchain/ttest.h`
```c
...
#define TEST(name) \
  int ttt_##name(); \
  __attribute__((constructor)) void tttr_##name() { \
    tt_register(#name, ttt_##name); \
  } \
    int ttt_#name()
```

This looks... weird... What does it do? Let's try expanding it.

I'll tell you right away that it's supposed to be used for a function definition, so using it like:

```c
TEST(adder_adds_two_numbers)
{
  // Test goes here
}
```

will expand to

```c
int ttt_adder_adds_two_numbers();
__attribute__((constructor)) void tttr_adder_adds_two_numbers()
{
  tt_register("adder_adds_two_numbers", ttt_adder_adds_two_numbers);
}
int ttt_adder_adds_two_numbers()
{
  // Test goes here
}
```

So it declares, and then defines a function. But what's the stuff in between?

It's magic - that's what.

The `__attribute__((constructor))`  is a gcc specific attribute which tells the
compiler that the function `tttr_adder_adds_two_numbers`  should run as soon as
it gets into scope - which is when the test loads and, most importantly, before
`main()`  is called.

So `tt_register` just has to set up an entry into the `tt_tests`  array, and
the rest will take care of itself:

`toolchain/ttest.h`
```c
...
void tt_register(char *name, int (*fn)(void))
{
  tt_tests = realloc(tt_tests, (tt_test_count+1)*sizeof(struct tt_test));

  struct tt_test *t = &tt_tests[tt_test_count++];
  t->name = name;
  t->test = fn
  t->status = 1
  t->output = malloc(TT_BUFFER_SIZE)
}
...
```
The final part needed for a test is an assertion, which is yet another macro:

`toolchain/ttest.h`
```c
...

#define ASSERT_EQUAL(lhs, rhs) do { \
  int tt_lhs = (int)(lhs); \
  int tt_rhs = (int)(rhs); \
  if(tt_lhs != tt_rhs) { \
    dprintf(tt_pipe[1], "Got <%d> but expected <%d>\n", tt_lhs, tt_rhs); \
    return 1; \
  } \
} while(0);
...
```

The arguments of the assertion macro are copied immediately. That way
they are only evaluated once.

And that's it!

## Using the framework

A typical source file with tests can look like this

`adder.c`
```c
int adder(int a, int b)
{
  return 5;
}
```

`adder.tt`
```c
#include <ttest.h>
#inclued "adder.c"

TEST(adder_adds_two_numbers)
{
  ASSERT_EQUAL(adder(2,3), 5);
}

TEST(adder_adds_two_other_numbers)
{
  ASSERT_EQUAL(adder(3,4), 7);
}
```

And running it can look like this

```bash
$ d ./ttest
.F
adder_adds_two_other_numbers: Got <5> but expected <7>
```

If you have a c compiler installed you should also be able to run the
test framework locally. This speeds it up a bit.

## Further improvements

Of course this is only a very rudimentary test framework, and a lot of
improvements can be made. But this series isn't about designing a test
framework, and I don't know anything about designing a test framework
anyway, so I'll just leave it here.

I will however note some differences between this version and what's
actually in my code:

- The `ASSERT_EQUAL` macro has a different signature and should in fact
not be used directly.
- Instead use
  - `ASSERT_EQ_INT(lhs, rhs)` for integer comparison
  - `ASSERT_EQ_CHR(lhs, rhs)` for char comparison
  - `ASSERT_EQ_STR(lhs, rhs, len)` for string comparison
- There's also `ASSERT_NEQ_INT` and `ASSERT_NEQ_CHR` for not-equal
assertions
- Macros `BEFORE()` and `AFTER()` define functions that are run before
and after each test respectively. Use them for setup and teardown of the
objects under test
- The printed errors contain the name of the test file and the faulting line
in a format that can be processed automatically by vim and used in the
quickfix list
- Output to terminal is colorized
- The output is colorized
- The shell script handles compilation errors of the tests

If you run the tests locally, everything should work if your c compiler
is gcc.
