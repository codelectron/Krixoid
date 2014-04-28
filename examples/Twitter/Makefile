CC=g++
CFLAGS=-c -Wall 
LDFLAGS=-lkrixoid -lcurl -loauth
SOURCES=main.cpp TwitterApp.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=TwitterApp

all: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@ 

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	$(RM) $(EXECUTABLE) $(OBJECTS)
