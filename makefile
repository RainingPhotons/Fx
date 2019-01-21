CC=gcc
CFLAGS=-Wall
OBJ=client.o 

%.o: %.c
	$(CC) -c -o $@ $^ $(CFLAGS)

lights: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

.PHONY: clean

clean:
	rm -f *.o lights 
