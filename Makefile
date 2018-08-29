

CC = gcc -g  
LIBS=-lncurses 
SRC= rpvi.c
OBJ= rpvi.o

.c.o:
	$(CC) -c $<

all:	$(OBJ) 
	$(CC) -o rpvi $(OBJ) $(LIBS)
