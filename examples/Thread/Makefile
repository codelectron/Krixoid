CC=g++
CFLAGS=-c -Wall 
LDFLAGS=-lkrixoid
SOURCES=main.cpp ThreadApp.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=ThreadApp

all: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@ 

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	$(RM) $(EXECUTABLE) $(OBJECTS)
