CC = gcc
CFLAGS = -std=c99 -Wall -pedantic -O3 -Wmissing-field-initializers -fsanitize=address -g
INCLUDES = -I .
LIBS = -lpthread

.PHONY : clean cleanall

all : objectstore client

objectstore: objectstore.c libworker.a
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $< $(LIBS)

libworker.a: worker.o worker.h
	ar rvs $@ $<

client: client.c libaccess.a
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $< $(LIBS)

libaccess.a: access.o access.h
	ar rvs $@ $<

worker.o: worker.c util.h
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $< $(LIBS)

client.o: client.c util.h
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $< $(LIBS)
	
clean:
		@echo "Pulizia"
		-rm -f *.o
		-rm -f *.a
		-rm -f *.log
		-rm -f *.sock
		-rm -rf data