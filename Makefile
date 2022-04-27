all: os
os: main.c test.h
	gcc -o os main.c