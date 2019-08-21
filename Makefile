CC = gcc
CFLAGS = -std=c99 -Wall -pedantic -O3 -Wmissing-field-initializers -fsanitize=address -g
INCLUDES = -I .
LIBS = -lpthread

.PHONY : clean cleanall

all : objectstore client

objectstore: objectstore.c worker.c worker.h util.h
	$(CC) $(CFLAGS) $(INCLUDES) $^ -o $@.o $(LIBS)


client: client.c libaccess.a
	$(CC) $(CFLAGS) $(INCLUDES) $^ -o $@.o  $(LIBS)

libaccess.a: access.o access.h
	ar rvs $@ $<

access.o: access.c util.h
	$(CC) $(CFLAGS) $(INCLUDES) $^ -o $@.o $(LIBS)

clean:
		@echo "Pulizia"
		-rm -f *.o
		-rm -f *.a
		-rm -f *.log
		-rm -f *.sock
		-rm -rf data