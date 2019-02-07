src = $(wildcard *.c)
obj = $(src:.c=.o)

LDFLAGS = -lz -lm

phase1: $(obj)
        $(CC) -o $@ $^ $(LDFLAGS)

.PHONY: clean
clean:
        rm -f $(obj) phase1

