CPP=g++
CFLAGS=-Wall -g -std=c++11 -Wno-deprecated-declarations
OBJDIR=obj
OBJ=$(addprefix $(OBJDIR)/, client.o hsluv.o util.o sound.o)
FLUIDSYNTH_PKG=`pkg-config fluidsynth --libs`
SDL_PKG=`sdl2-config --cflags --libs`

all: lights

$(OBJDIR)/%.o: %.cc
	$(CPP) -c -o $@ $^ $(CFLAGS)

lights: $(OBJ)
	$(CPP) -o $@ $^ $(CFLAGS) -lpthread $(FLUIDSYNTH_PKG) $(SDL_PKG)

.PHONY: clean

clean:
	rm -f $(OBJDIR)/*.o lights
