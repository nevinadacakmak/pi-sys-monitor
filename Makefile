CC = gcc
CFLAGS = -Wall -Wextra -std=c99
CPPFLAGS = -I.
LDFLAGS =
LDLIBS = -lm

TARGETS = socket_server socket_client sysmon

.PHONY: all clean

all: $(TARGETS)

socket_server: socket_server.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

sysmon: main.o metrics.o socket_client.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o $(TARGETS)