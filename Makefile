CC = gcc
CFLAGS = -std=c99 -pedantic -Wall -O3 -Wmissing-field-initializers -D_POSIX_C_SOURCE=200809L -fsanitize=address -g -fno-omit-frame-pointer
INCLUDES = -I .
LIBS = -lpthread -L.


.PHONY : clean cleanall

all : objectstore client

objectstore: objectstore.c worker.c worker.h util.h
	$(CC) $(CFLAGS) $(INCLUDES) $^ -o $@.o $(LIBS)


client: client.c libaccess.a
	$(CC) $(CFLAGS) $(INCLUDES) $^ -o $@.o  $(LIBS)

libaccess.a: access.h access.o
	ar rvs $@ $^

access: access.c util.h
	$(CC) $(CFLAGS) $(INCLUDES) $^ -o $@.o $(LIBS)

clean:
		@echo "Pulizia"
		-rm -f *.o
		-rm -f *.a
		-rm -f *.log
		-rm -f *.sock
		-rm -rf data