TARGETEXE := kcv

CFLAGS = -pipe -Wall -Wextra -pedantic -Wno-unused-parameter -std=c99

# default target
all: $(TARGETEXE) 
.PHONY: all


$(TARGETEXE): $(TARGETEXE).c term.c
	$(CC) -o $@ $^ $(CFLAGS)


clean:
	rm -f $(TARGETEXE)
.PHONY: clean

