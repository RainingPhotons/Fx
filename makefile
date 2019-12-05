CPP=g++
CFLAGS=-Wall -g -std=c++11
OBJ=client.o hsluv.o

%.o: %.cc
	$(CPP) -c -o $@ $^ $(CFLAGS)

lights: $(OBJ)
	$(CPP) -o $@ $^ $(CFLAGS) -lpthread

.PHONY: clean

clean:
	rm -f *.o lights 
