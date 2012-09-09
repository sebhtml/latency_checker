CFLAGS=-O3 -Wall -ansi -DASSERT
CC=mpicc



all: latency_checker

latency_checker: main.o process.o
	$(CC) $(CFLAGS) main.o process.o -o latency_checker

main.o: main.c
	$(CC) $(CFLAGS) -c main.c -o main.o

process.o: process.c
	$(CC) $(CFLAGS) -c process.c -o process.o

test: latency_checker
	mpiexec -n 4 -output-filename latency_checker ./latency_checker

clean:
	rm -rf main.o process.o latency_checker
