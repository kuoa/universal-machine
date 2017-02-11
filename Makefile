CC=gcc
CFLAGS=-O3 -Wall -Werror -std=c11
SRC=src
TEST=test_files
EXE=vm


all:
	$(CC) $(CFLAGS) $(SRC)/vm.c -o $(EXE)

codex:
	make all; ./$(EXE) $(TEST)/codex.umz

sandmark:
	make all; ./$(EXE) $(TEST)/sandmark.umz

clean:
	rm vm
