all: robotnik.o brygadzista.o archiwista.o

robotnik.o: robotnik.c
	gcc robotnik.c -lm -o robotnik.o

brygadzista.o: brygadzista.c
	gcc brygadzista.c -lm -o brygadzista.o

archiwista.o: archiwista.c
	gcc archiwista.c -lm -o archiwista.o
