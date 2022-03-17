CC = gcc
CFLAGS = -Wall -Wextra -g

INCLUDES = -I./src
LIBS =-lm

OBJ = cJSON.o sudoku.o
DIR = src

EXE = sudoku

build: $(EXE)

cJSON.o: $(DIR)/cJSON.c
	$(CC) -c $< $(CFLAGS) $(LIBS) $(INCLUDES)

sudoku.o: $(DIR)/sudoku.c
	$(CC) -c $< $(CFLAGS) $(LIBS) $(INCLUDES)

$(EXE): $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS) $(INCLUDES)


run:
	./$(EXE)

.PHONY: clean

clean:
	@rm -f *.o sudoku
