CC      = g++
FLAGS   = 
TARGET  = libkrixoid.so.0.1
TARGETNAME = libkrixoid.so
CFLAGS     = -fPIC  -shared -Wl,-soname,$(TARGETNAME) -lrt 
SOURCES = Serial.cpp Timer.cpp Krixoid.cpp PThread.cpp Select.cpp TCPClient.cpp TCPServer.cpp  Twitter.cpp tweet.cpp utils.cpp 
HEADERS = Serial.h Timer.h Krixoid.h PThread.h Select.h TCPClient.h TCPServer.h   Twitter.h tweet.h utils.h
OBJECTS = Serial.o Timer.o Krixoid.o PThread.o Select.o TCPClient.o TCPServer.o  Twitter.o tweet.o utils.o

VERSION = 0.1
DESTDIR=/usr
PREFIX=/local


all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(FLAGS) $(CFLAGS) $(DEBUGFLAGS) -o $(TARGET) $(OBJECTS) 
#%.o : %.c
#	$(CC) $(CFLAGS) -c $<
clean:
	$(RM) $(OBJECTS)
	$(RM) $(TARGET)

install:
	@install -m  755 -d  	$(DESTDIR)$(PREFIX)/include
	@install -m 0644 Serial.h $(DESTDIR)$(PREFIX)/include	
	@install -m 0644 Krixoid.h $(DESTDIR)$(PREFIX)/include	
	@install -m 0644 Timer.h $(DESTDIR)$(PREFIX)/include	
	@install -m 0644 PThread.h $(DESTDIR)$(PREFIX)/include	
	@install -m 0644 TCPClient.h $(DESTDIR)$(PREFIX)/include	
	@install -m 0644 TCPServer.h $(DESTDIR)$(PREFIX)/include	
	@install -m 0644 Select.h $(DESTDIR)$(PREFIX)/include	
	@install -m 0644 Twitter.h $(DESTDIR)$(PREFIX)/include	
	@install -m 0755 $(TARGET) $(DESTDIR)$(PREFIX)/lib/$(TARGET)
	@ln -sf $(DESTDIR)$(PREFIX)/lib/$(TARGET) $(DESTDIR)$(PREFIX)/lib/$(TARGETNAME)
	@ldconfig
