#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <ctype.h>

extern void die(const char *);


struct termios orig_termios;


inline void putbit(unsigned char, unsigned);
void putbit(unsigned char c, unsigned pos)
{
  putchar('0' + !!(c&(1<<pos)) );
}

inline void print_charbi(const char*, unsigned char);
void print_charbi(const char* infopre, unsigned char c)
{
  // This sets the first 3 bits to b010 to get the base.
  unsigned char base = (c & ~(0xE0)) | 0x40;
  printf(" (%s, base '%c', '^%c')", infopre, base, base);
}

void print_char_info(unsigned char c)
{
  printf("%3d ", c);

  printf("[0x%02x] ", c);

  putbit(c, 7);
  putchar('_');
  putbit(c, 6);
  putbit(c, 5);
  putchar('_');
  putbit(c, 4);
  putbit(c, 3);
  putbit(c, 2);
  putbit(c, 1);
  putbit(c, 0);


  switch (c & 0xE0) { // 3 first bits.
  case 0x40: // b010 - normal uppercase column
  case 0x60: // b011 - normal lowercase column
  case 0x20: // b001 - normal printable punctuation column
    if (c == 0x7f) // Special case: delete character.
      print_charbi("DEL control char", c);
    else
      printf(" ('%c')", c);
    break;
  case 0x00: // b000 - actual control char.
    //To get the base char: c + '@' (or c|'@').
    print_charbi("control char", c);
    break;
  case 0x80: // b100 - modified control char (same char but with MSB=1)
    print_charbi("modified control char", c);
    break;
  default: // something else: has MSB=1 but is not a modified control char.
    print_charbi("other char", c);
    break;
  }

  putchar('\r');
  putchar('\n');
}



void disable_raw_mode() {
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1)
    die("tcsetattr");
}

void enable_raw_mode() {
  // Don't mess with user's terminal configuration.
  // Backup original config before setting attributes.

  if (tcgetattr(STDIN_FILENO, &orig_termios) == -1)
    die("tcgetattr");
  
  // Register atexit handler here.
  // We don't process signals as we are capturing '^C', etc.
  // If we receive a signal this is bad news enough, so we
  // won't try to retore the termios initial state.
  atexit(disable_raw_mode);

  struct termios raw = orig_termios;

  /*
   * ECHO=0:   disable printing pressed keys.
   * ICANON=0: read char by char instead of line by line.
   * ISIG=0:
   *   [VINTR]  don't send SIGINT  on Ctrl-C (0x02, End of Text) [Signal]
   *   [VEOF]   don't send EOF     on Ctrl-D (0x03, End of Transmission)
   *   [VSUSP]  don't send SIGTSTP on Ctrl-Z (0x1A, Substitute) [Signal]
   *                               or Ctrl-Y on MacOS (0x19, End of Medium)
   *   [VQUIT]  don't send SIGQUIT on Ctrl-\ (0x1C, File Separator) [Signal]
   *   [VDSUSP] not supported by POSIX (nor Linux, for that matter) [Signal]
   *            Special SIGTSTP on Ctrl-Y.
   * 
   * IEXTEN=0:
   *   [VEOL2]  don't send EOL2    on Ctrl-@ (0x00, NULL)
   *            Alternative EOL. 
   *   [LNEXT]  don't send LNEXT   on Ctrl-V (0x16, Synchronous Idle),
   *            Means "the next character should be treated literally" (like Ctrl-V + Ctrl-L).
   *   [VREPRINT] don't send Reprint on Ctrl-R (0x12, Device Control 2).
   *             Also: ¿Used by readline for backwards search?
   *   [VERASE] don't send WERASE  on Ctrl-W (0x17, End Of Transmission Block).
   * 
   * Note: the key combinations can be changed with raw.c_cc[_V_ACTION_] = _KEYCODE_;
   */
  raw.c_lflag &= ~(ECHO|ICANON|ISIG|IEXTEN);

  /*
   * OPOST=0: Disable output post-processing.
   *          Note that this also disables conversion from '\n' to '\r\n',
   *          (even if ONLCR=1) so from now on we have to use '\r\n' manually
   *          (but do NOT write any file with '\r\n'!!).
   *          Keyboard RETURN actually generates '\r' (0x0D, Carriage Return) that 
   *          can be generated with Ctrl-M, but as by default ICRNL=1 (we set later to 0),
   *          it sends by default '\n' instead.
   *          '\n' (0x0A, Line Feed) and can be generated with Ctrl-J.
   */
  raw.c_oflag &= ~(OPOST);

  /*
   * IXON=0:
   *   disable XON/XOFF in-band flow control.
   *   XOFF is generated with Ctrl-S (0x13, Device Control 3)
   *   XON  is generated with Ctrl-Q (0x11, Device Control 1)
   *   Note: these are NOT signals, they are something that affects directly the (P)TTY.
   * ICRNL=0:
   *   By default (=1) this would translate '\r' to '\n', this also includes Ctrl-M sending
   *   '\n' 0x0A (=Ctrl-J) instead of '\r' 0x0D.
   *   Setting to 0 disables this and both the Key RETURN and Ctrl-M correctly generate '\r' 0x0D.
   *   Also, Ctrl-Return generates '\n' 0x0A regardless of this option.
   * BRKINT=0:
   *   Disables SIGINT on BREAK condition (this is related to RS232 voltage signaling and not relevant here).
   * INPCK=0:
   *   Disable input parity check.
   * ISTRIP=0:
   *   Do not force 8th bit to 0 (for 7-bit ASCII terminals compatibility?).
   */
  raw.c_iflag &= ~(IXON|ICRNL|BRKINT|INPCK|ISTRIP);


  /*
   * CSIZE=CS8:
   *   Set 8-bits per character.
   */
  raw.c_cflag &= ~(CSIZE); // Clear CSIZE Flags.
  raw.c_cflag |=  (CSIZE | CS8); // Actually CS8 == CSIZE, but whatever...

  /*
   * MIN == 0, TIME > 0:
   * read(2) will return immediately if *any* (MIN==0) non-zero number
   * of bytes is available. If there are no (zero) bytes available it will
   * wait for available chars during TIME*0.1 seconds, returning 0 on timeout.i
   *
   * NOTE: on WSL, VTIME is ignored (assume it is always 0).
   */
  raw.c_cc[VMIN] = 0;
  raw.c_cc[VTIME] = 1;

  // Commit changes:
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1)
    die("tcsetattr");

}


