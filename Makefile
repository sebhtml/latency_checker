CFLAGS=-O3 -Wall -Wextra -ansi 
CC=mpicc



all: latency_checker

latency_checker: main.o process.o
	$(CC) $(CFLAGS) main.o process.o -o latency_checker

main.o: main.c
	$(CC) $(CFLAGS) -c main.c -o main.o

process.o: process.c
	$(CC) $(CFLAGS) -c process.c -o process.o

test:
	mpiexec -n 3 ./latency_checker
