CC = gcc
CFLAGS = -W
LDFLAGS = -lm

SRCS = monitor.c
OBJS = $(SRCS:.c=.o)
EXEC = monitor

all: $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o $(EXEC)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJS)
