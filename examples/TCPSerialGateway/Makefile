CC=g++
CFLAGS=-c -Wall 
LDFLAGS=-lkrixoid
SOURCES=main.cpp TCPSerialGatewayApp.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=TCPSerialGatewayApp

all: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@ 

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	$(RM) $(EXECUTABLE) $(OBJECTS)
