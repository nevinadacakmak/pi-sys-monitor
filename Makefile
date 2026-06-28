CC = gcc
CFLAGS = -Wall -Wextra -std=c99
CPPFLAGS = -I.
LDFLAGS =
LDLIBS = -lm

TARGETS = sysmon

.PHONY: all clean

all: $(TARGETS)

sysmon: main.o metrics.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o $(TARGETS)