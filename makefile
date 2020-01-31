CPP=g++
CFLAGS=-Wall -g -std=c++11
OBJDIR=obj
OBJ=$(addprefix $(OBJDIR)/, client.o hsluv.o util.o sound.o)

all: lights

$(OBJDIR)/%.o: %.cc
	$(CPP) -c -o $@ $^ $(CFLAGS)

lights: $(OBJ)
	$(CPP) -o $@ $^ $(CFLAGS)  `pkg-config fluidsynth --libs` -lpthread

.PHONY: clean

clean:
	rm -f $(OBJDIR)/*.o lights
