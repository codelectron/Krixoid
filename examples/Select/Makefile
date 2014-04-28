CC=g++
CFLAGS=-c -Wall 
LDFLAGS=-lkrixoid
SOURCES=main.cpp SelectApp.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=SelectApp

all: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@ 

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	$(RM) $(EXECUTABLE) $(OBJECTS)
