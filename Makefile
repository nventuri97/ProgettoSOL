CC = gcc
CFLAGS = -std=c99 -pedantic -Wall -O3 -Wmissing-field-initializers -D_POSIX_C_SOURCE=200809L -fsanitize=address -g -fno-omit-frame-pointer
INCLUDES = -I .
LIBS = -lpthread -L.


.PHONY : clean test

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

test: 
		@echo 'Inizio fase di test'
		for((i=0;i<50;i++))
		do
			./client.o client$i 1 >>testou.log &
		done

		wait

		for((i=0;i<30;i++))
		do
			./client.o client$i 2 >>testou.log &
		done

		for((i=30;i<50;i++))
		do
			./client.o client$i 3 >>testou.log &
		done

		wait