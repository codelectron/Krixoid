CC=g++
CFLAGS=-c -Wall 
LDFLAGS=-lkrixoid
SOURCES=main.cpp TimerApp.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=TimerApp

all: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@ 

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	$(RM) $(EXECUTABLE) $(OBJECTS)
