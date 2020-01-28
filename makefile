CPP=g++
CFLAGS=-Wall -g -std=c++11
OBJ=client.o hsluv.o util.o sound.o
OBJ_PROP=prop.o

all: lights prop

%.o: %.cc
	$(CPP) -c -o $@ $^ $(CFLAGS)

lights: $(OBJ)
	$(CPP) -o $@ $^ $(CFLAGS)  `pkg-config fluidsynth --libs` -lpthread

prop: $(OBJ_PROP)
	$(CPP) -o $@ $^ $(CFLAGS)

.PHONY: clean

clean:
	rm -f *.o lights prop
