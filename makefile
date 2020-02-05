CPP=g++
C=gcc
CFLAGS=-Wall -g -std=c++11 -Wno-deprecated-declarations
OBJDIR=obj
OBJ=$(addprefix $(OBJDIR)/, client.o hsluv.o util.o generateProg.o)
MONITOR_OBJ=$(addprefix $(OBJDIR)/, monitor.o util.o)
FLUIDSYNTH_PKG=`pkg-config fluidsynth --libs`
SDL_PKG=`sdl2-config --cflags --libs`

all: lights monitor

$(OBJDIR)/%.o: %.cc
	$(CPP) -c -o $@ $^ $(CFLAGS)

$(OBJDIR)/%.o: %.c
	$(C) -c -o $@ $^ $(CFLAGS)

lights: $(OBJ)
	$(CPP) -o $@ $^ $(CFLAGS) -lpthread $(FLUIDSYNTH_PKG) $(SDL_PKG)

monitor: $(MONITOR_OBJ)
	$(CPP) -o $@ $^ $(CFLAGS) -lpthread

.PHONY: clean

clean:
	rm -f $(OBJDIR)/*.o lights
