CC      = g++
FLAGS   = 
TARGET  = libkrixoid.so.0.1
TARGETNAME = libkrixoid.so
CFLAGS     = -fPIC  -shared -Wl,-soname,$(TARGETNAME) -lrt 
SOURCES = Serial.cpp Timer.cpp Krixoid.cpp
HEADERS = Serial.h Timer.h Krixoid.h 
OBJECTS = Serial.o Timer.o Krixoid.o


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
	@install -m 0755 $(TARGET) $(DESTDIR)$(PREFIX)/lib/$(TARGET)
	@ln -sf $(DESTDIR)$(PREFIX)/lib/$(TARGET) $(DESTDIR)$(PREFIX)/lib/$(TARGETNAME)
	@ldconfig
