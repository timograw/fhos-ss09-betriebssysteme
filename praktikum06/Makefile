PRODUCT=threads

CC=gcc
CCFLAGS=-DDEBUG -DMEMDEBUG -Wall -g -c
LNKOPT=-lm -lsocket -lnsl -lrt

OBJECTS=alloc.o threads.o memcheck.o socket.o list.o

all: $(PRODUCT) download

$(PRODUCT): $(OBJECTS)
	@echo "Linking \"$@\" because \"$?\" has changed"
	$(CC) $(LNKOPT) -o $(PRODUCT) $(OBJECTS)
	cp $(PRODUCT) ../bin/$(PRODUCT)
	@echo 
	
download:
	@echo "Creating download directory"
	mkdir download
	@echo
	
%.o: %.c # Alle C Dateien zu Objektdateien compilieren
	@echo "Compiling \"$@\" because \"$?\" has changed"
	$(CC) $(CCFLAGS) -o $*.o $<
	@echo 

.PHONY: clean
clean:
	-rm $(PRODUCT)
	-rm *~
	-rm $(OBJECTS)
