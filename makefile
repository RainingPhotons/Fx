CPP=g++
CFLAGS=-Wall
OBJ=client.o 

%.o: %.c
	$(CPP) -c -o $@ $^ $(CFLAGS)

lights: $(OBJ)
	$(CPP) -o $@ $^ $(CFLAGS)

.PHONY: clean

clean:
	rm -f *.o lights 
