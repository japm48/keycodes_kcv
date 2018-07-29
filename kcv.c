// Simple KeyCode Viewer for *nix terminals.

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>



void die(const char *s)
{
  perror(s);
  exit(1);
}

extern void enable_raw_mode();
//extern void disable_raw_mode();
extern void print_char_info(unsigned char);

#define DEFAULT_EXIT_KEY 'q'

int main(int argc, char** argv) {
  enable_raw_mode();

  char c;
  int r;
  
  char exit_key = DEFAULT_EXIT_KEY;
  if(argc > 1)
    exit_key = argv[1][0];

  while (1) {
    c = '\0';

    // Non-blocking read, as c_cc[VTIME] != 0
    // Note that this will not allow inputting a null character (using Ctrl-@).
    r = read(STDIN_FILENO, &c, 1); 
    if (r == -1 && errno != EAGAIN)
      die("read");
    if (r == 0)
      continue;

    print_char_info(c);

    if (c == exit_key) break;
  }

  return 0;
}

