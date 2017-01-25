all: robotnik.o brygadzista.o archiwista.o

robotnik.o: robotnik.c
	gcc robotnik.c -lm -o robotnik.o

brygadzista.o: brygadzista.c
	gcc brygadzista.c -lrt -lm -lssl -lcrypto -o brygadzista.o

archiwista.o: archiwista.c
	gcc archiwista.c -lm -lssl -lcrypto -o archiwista.o
