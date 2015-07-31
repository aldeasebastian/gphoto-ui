all: lib
clean:
		rm -f libgphotoapi.a
		rm -f *.o
run: all
		./test
lib: api.o
		ar cr libgphotoapi.a api.o
api.o: api.c api.h
		gcc -c api.c -o api.o
