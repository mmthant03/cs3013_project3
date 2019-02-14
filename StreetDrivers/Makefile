src = $(wildcard *.c)
obj = $(src:.c=.o)

LDFLAGS = -lz -lm

phase2: $(obj)
	$(CC) -pthread -o $@ $^ $(LDFLAGS)

.PHONY: clean
clean:
	rm -f $(obj) phase2
