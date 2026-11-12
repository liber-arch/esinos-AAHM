CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Werror -g -Iinclude

SRC = $(wildcard src/*.c src/*/*.c)
OBJ = $(SRC:.c=.o)

esinos : $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

clean:
	rm -f $(OBJ) $(OBJ:.o=.d) esinos

.PHONY: clean

-include $(OBJ:.o=.d)
