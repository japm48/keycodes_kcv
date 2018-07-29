# KeyCode Viewer 

**Simple KeyCode Viewer for \*nix terminals**

This is a simple toy program to get information about the keycodes of the pressed keys, for educational and quick reference purposes.

## Usage

Shows information of every pressed key:

    $ ./kcv
     97 [0x61] 0_11_00001 ('a')
      3 [0x03] 0_00_00011 (control char, base 'C', '^C')

Invoke `make` to compile (hopefully it just works...) and call `./kcv` to execute.

By default, to exit the program, `q` must be pressed (`^C` won't work as it is captured):

    $ ./kcv
    113 [0x71] 0_11_10001 ('q')
    $

To use another exit key, use the first argument:

    $ ./kcv a
    113 [0x71] 0_11_10001 ('q')
     97 [0x61] 0_11_00001 ('a')
    $

For every pressed key it shows:

 * Decimal and hex values of the keycode.

 * Binary separated in 3 bit fields in order:

    * 1 bit that is `1` only when the code does not belong to 7-bit ASCII.

    * 2 bits showing (in general) whether it is a control char (`00`), a symbol (`01`), an uppercase letter (`10`) or a lowercase letter (`11`).

    * Last 5 bits: base key code.

 * Extra info.

## Examples of usage

It is useful for stuff like:

 * Not having to search the ASCII code for `^C`:

         3 [0x03] 0_00_00011 (control char, base 'C', '^C')

 * Finding the relation between `A`, `Shift+A` and `Ctrl+A`:

        97 [0x61] 0_11_00001 ('a')
        65 [0x41] 0_10_00001 ('A')
         1 [0x01] 0_00_00001 (control char, base 'A', '^A')


 * Realising that `Ctrl+A` and `Ctrl+Shift+A` both produce the same results:

         1 [0x01] 0_00_00001 (control char, base 'A', '^A')
         1 [0x01] 0_00_00001 (control char, base 'A', '^A')

 * Finding the difference between `Enter` (CR, `\r`) and `Ctrl+J` (LF, `\n`):

        13 [0x0d] 0_00_01101 (control char, base 'M', '^M')
        10 [0x0a] 0_00_01010 (control char, base 'J', '^J')

 * Realising that some key presses are actually translated as two keycodes, example `á`:

       195 [0xc3] 1_10_00011 (other char, base 'C', '^C')
       161 [0xa1] 1_01_00001 (other char, base 'A', '^A')


