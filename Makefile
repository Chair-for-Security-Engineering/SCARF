CC=gcc
CPP=g++

all: scarf_c

scarf_c: scarf_c.c scarf_c.h
	$(CC) -o scarf_c scarf_c.c
	