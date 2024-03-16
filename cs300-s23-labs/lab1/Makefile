CFLAGS = -O3
.PHONY = all clean format
all: reverse_test
%.o: %.c
	gcc ${CFLAGS} -c $^ -o $@
reverse_test: test_reverse.o reverse.o
	gcc ${CFLAGS} $^ -o $@
clean:
	rm -f reverse_test reverse.o test_reverse.o
format:
	clang-format -style=Google -i test_reverse.c reverse.c