all:
	# compile empty C program
	gcc -o empty empty.c

	# trace empty C program
	strace -o empty.trace ./empty

	# compile five syscall C program
	gcc -o part1 part1.c

	# trace five syscall program
	strace -o part1.trace ./part1
clean:
	rm part1;
	rm empty;
