PRODUCT=shell

CC=gcc
CCFLAGS=-DDEBUG -Wall -g -c
LNKOPT=-lm

OBJECTS=alloc.o shell.o

all: $(PRODUCT)

$(PRODUCT): $(OBJECTS)
	$(CC) $(LNKOPT) -o $(PRODUCT) $(OBJECTS)
	cp $(PRODUCT) ../bin/$(PRODUCT)
	
%.o: %.c
	$(CC) $(CCFLAGS) -o $*.o $<

clean:
	@-rm *.o
	@-rm *~ef 
