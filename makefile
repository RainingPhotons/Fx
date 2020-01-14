
CPP=g++
CFLAGS=-Wall -g
OBJ=shootup.o 

%.o: %.c
	$(CPP) -c -o $@ $^ $(CFLAGS)

lights: $(OBJ)
	$(CPP) -o $@ $^ $(CFLAGS)

.PHONY: clean

clean:
	rm -f *.o lights
