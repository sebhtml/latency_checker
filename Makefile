all:
	mpicxx -O3 -Wall -Wextra -ansi main.c -o latency_checker

test:
	mpiexec -n 3 ./latency_checker
