

CC = gcc -g  
LIBS=-lncursesw
SRC= rpvi.c
OBJ= rpvi.o

.c.o:
	$(CC) -c $<

all:	$(OBJ) 
	$(CC) -o rpvi $(OBJ) $(LIBS)
