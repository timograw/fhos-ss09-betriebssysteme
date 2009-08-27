PRODUCT=shell

CC=gcc
CCFLAGS=-DDEBUG -DMEMDEBUG -Wall -g -c
LNKOPT=-lm

OBJECTS=alloc.o shell.o memcheck.o

all: $(PRODUCT)

$(PRODUCT): $(OBJECTS)
	@echo "Linking \"$@\" because \"$?\" has changed"
	$(CC) $(LNKOPT) -o $(PRODUCT) $(OBJECTS)
	cp $(PRODUCT) ../bin/$(PRODUCT)
	@echo 
	
%.o: %.c
	@echo "Compiling \"$@\" because \"$?\" has changed"
	$(CC) $(CCFLAGS) -o $*.o $<
	@echo 

.PHONY: clean
clean:
	-rm *.o
	-rm $(PRODUCT)
