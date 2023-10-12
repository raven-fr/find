SDL_HEADERS=/usr/include/SDL2

CFLAGS=-g -Wall -pedantic -std=c99 $$(sdl2-config --cflags)
LFLAGS=$$(sdl2-config --libs)

findgame: main.o world.o random.o generator.o save.o
	$(CC) -o $@ $^ $(LFLAGS)

main.o: world.h random.h save.h
world.o: world.h random.h generator.h save.h
generator.o: world.h random.h
save.o: world.h die.h

.SUFFIXES: .c .o
.c.o:
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f *.o findgame
