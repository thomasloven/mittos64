# Debug Output

In this chapter we'll be looking at some ways of getting status information out
of the kernel.

## The printf function and TDD

I really like having access to a printf-like function when developing. It just
makes debugging so much easier.

For further convenience, I want the debug printer to print both to screen and
to a serial port.

Now, I'm doing this as a learning experience, and one thing I've been meaning
to practice is Test-Driven Development. So let's write the debug printing that
way.

Let's start very simple. A function that outputs a character to VGA and to
serial.  Assuming (slightly prematurely) that we'll have two functions `void
vga_write(char c)` and `void serial_write(uint16_t port, uint8_t c)` The test
could look like this:

`src/kernel/boot/debug.tt`
```c
char vga_recv;
char serial_recv;

void vga_write(char c)
{
  vga_recv = c;
}
void serial_write(uint16_t port, uint8_t c)
{
  serial_recv = (char)c;
}

TEST(putch_sends_character_to_vga)
{
  debug_putch('a');
  ASSERT_EQ_CHR(vga_recv, 'a');
}
TEST(putch_sends_character_to_serial)
{
  debug_putch('a');
  ASSERT_EQ_CHR(serial_recv, 'a');
}
```

... which can be made to pass with a simple function like:

`src/kernel/boot/debug.c`
```c
void debug_putch(char c)
{
  vga_write(c);
  serial_write(PORT_COM1, c);
}
```

... with some `#define`s and `#include`s and such, of course.

As I understand it, in efficient TDD, you should write one test at a time, and
then make that pass - so that's what I tried to do, but I'm presenting more
than one test at a time for brevity.

Next is writing strings, which requires an update to the mock functions:

`src/kernel/boot/debug.tt`
```c
char vga_recv[BUFFER_SIZE];
char serial_recv[BUFFER_SIZE];

BEFORE()
{
  for(int i= 0; i < BUFFER_SIZE; i++)
  {
    vga_recv[i] = '\0';
    serial_recv[i] = '\0';
  }
}

void vga_write(char c)
{
  static int i = 0;
  vga_recv[i++] = c;
}
void serial_write(uint16_t port, uint8_t c)
{
  static int i = 0;
  serial_recv[i++] = (char)c;
}

TEST(putsn_writes_string)
{
  char *str = "hello";
  debug_putsn(str, 5);
  ASSERT_EQ_STR(vga_recv, str, BUFFER_SIZE);
}
TEST(putsn_writes_correct_number_of_characters)
{
  char *str = "1234567890";
  debug_putsn(str, 5);
  ASSERT_EQ_STR(vga_recv, "12345", BUFFER_SIZE);
}
```


`src/kernel/boot/debug.c`
```c
debug_putsn(char *s, size_t n)
{
  while(n--)
    debug_putch(*s++);
}
```

Then there's `debug_printf`, which is left as an exercise for the reader - or
you could just look through my code.

> You'll probably find that my implementation, among many other things, is
> influenced by [musl libc](https://www.musl-libc.org/) which is a very clean
> standard library implementation. We'll take a much closer look at musl later...

The names of the tests are:
- `printf_prints_string`
- `printf_does_not_print_percent`
- `printf_prints_binary_number`
- `printf_prints_octal_number`
- `printf_prints_decimal_number`
- `printf_prints_hexadecimal_number`
- `printf_prints_char`
- `printf_prints_passed_string`
- `printf_keeps_printing_after_number`
- `printf_prints_text_around_number`
- `printf_keeps_going_for_unknown_format_specifier`
- `printf_pads_value`
- `printf_pads_more_than_9`
- `printf_zero_pads`

## Printing to screen

Next, let's look at `vga_write()`.

I tried to use TDD for this as well, and that had an unexpected outcome.

In order to test VGA printing, I had to mock up VGA hardware somehow - or at
least the VGA memory mapping.

This brought me two realizations.

First of all, each character on screen is described by a 16 bit value - one
byte for the character and one for the color attribute.

This can be implemented as a structure:

`src/kernel/drivers/vga.c`
```c
struct vga_cell {
  uint8_t c;
  uint8_t f;
}__attribute__((packed));

struct vga_cell buffer[24*80] = (void *)0xB8000;
...
```

The second realization was that each screen line comes right after the previous
one in memory.

This seems obvious, of course - but stay with me.

You've probably gone through some kernel development tutorial at
some point (I'd assume [Brandon Friesen's](http://www.osdever.net/bkerndev/Docs/title.htm),
[James Molloy's](http://www.jamesmolloy.co.uk/tutorial_html/) or
[The osdev.org Bare Bones ](http://wiki.osdev.org/Bare_Bones)).
If so, you may have a `putch()`, `monitor_put()` or
`terminal_putchar()` function which writes a character to screen,
increases a counter, and if it has reached the end of a line, goes
to the next.

See where I'm going with this? There's no point in keeping track on the line
and column separately. All you need is a single counter.

That makes my corresponding function very short and simple:

`src/kernel/drivers/vga.c`
```c
...
uint64_t cursor = 0;

void vga_write(char c)
{
  switch(c)
  {
    case '\n':
      cursor += 80 - (cursor%80);
      break;
    default:
      buffer[cursor++] = (struct vga_cell){.c = c, .f=7};
  }
  scroll();
}
```

`scroll()` scrolls the screen if we pass beyond the last line, and is also very simple:

```c
...
void scroll()
{
  memmove(buffer, &buffer[80], 80*(24-1)*sizeof(struct vga_cell));
  memset(&buffer[80*(24-1)], 0, 80*sizeof(struct vga_cell));
  cursor -= 80;
}
...
```

As a final note, the line `struct vga_cell buffer[24*80] = (void *)0xB8000;` is
a really bad one. You should make sure to use double buffering when dealing
with VGA memory. The reason for this is that *reading* from VGA memory (such as
when scrolling the screen) is really really slow.

## Bonus: PANIC

Sometimes, things go wrong.

When they do, you probably want to know, and perhaps get a chance to find out
why, or even to put things right.

For this reason, I made a `PANIC()` macro. The definition looks like this:

`src/kernel/include/debug.h`
```c
...
#define S(x) #x
#define S_(x) S(x)
#define S__LINE__ S_(__LINE__)

#define PANIC(...) \
  do{ \
    debug_printf("\n\nKernel panic!\n%s:%d\n", __FILE__, __LINE__); \
    debug_printf(__VA_ARGS__); \
    volatile int _override = 0; \
    while(1){ \
      asm("panic_breakpoint_" S__LINE__ ":"); \
      if(_override) break; \
    } \
  }while(0)
...
```

It's pretty simple actually. It just prints an error message, and then loops
infinitely.

However, the loop can be broken out of by changing the value of `_override` by
e.g. breaking in gdb and running

```gdb
(gdb) set _override=1
```

The `asm` line creates a label in the code, which you can add as a breakpoint
in gdb.  The `S__LINE__` stuff and related macros is so that the labels will be
unique if you call `PANIC()` twice in the same source file (gdb will happily
break at any of several labels with the same name, but gcc doesn't like two
equal labels in the same file).

Gdb has a neat command called `rbreak` which sets breakpoints based on a
regular expression. Unfortunately that only works for functions - not labels.
Therefore, I put the following function in gdbinit, to automatically find all
`panic_breakpoint`s and add them to gdb.

`toolchain/gdbinit`
```gdb
...
python
import subprocess
import os
dump = subprocess.Popen(("objdump", "-t", os.environ['BUILDROOT'] + "sysroot/kernel"), stdout=subprocess.PIPE)
lines = subprocess.check_output(('grep', 'panic_breakpoint'), stdin=dump.stdout)
dump.wait()
for line in lines.split('\n'):
  name = line.split(' ')[-1]
  if name:
    gdb.execute('b ' + name, to_string=True)
end
...
```
