CC = gcc
CFLAGS = -Wall -Wextra -g
TARGET = program

SRCS = main.c \
       directory_utils.c \
       file_utils.c \
       help_utils.c \
       permissions_utils.c \
       process_utils.c \
       signal_utils.c

HEADERS = directory_utils.h \
          file_utils.h \
          help_utils.h \
          permissions_utils.h \
          process_utils.h \
          sinal_utils.h

OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)