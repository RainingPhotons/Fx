CPP=g++
CFLAGS=-Wall -g
OBJ=client.o hsluv.o

%.o: %.c
	$(CPP) -c -o $@ $^ $(CFLAGS)

lights: $(OBJ)
	$(CPP) -o $@ $^ $(CFLAGS)

.PHONY: clean

clean:
	rm -f *.o lights 
