CC = gcc
CFLAGS = -Wall -Wextra -std=c99

TARGET1 = socket_server
TARGET2 = socket_client

.PHONY: all
all: $(TARGET1) $(TARGET2)

$(TARGET1): socket_server.o
	$(CC) $(CFLAGS) -o $(TARGET1) socket_server.c

$(TARGET2): socket_client.o
	$(CC) $(CFLAGS) -o $(TARGET2) socket_client.o

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -f *.o $(TARGET1) $(TARGET2)
