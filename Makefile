CC=g++
CFLAGS=-c -Wall -Wextra -ansi -pedantic -std=c++11
#CFLAGS=-c -Wall -Wextra
LDFLAGS=
SOURCES=src/ndet.cpp lib/tinyxml2.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=ndet

all: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@
	
	
.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f ndet src/*.o src/*~ *~ 
  # $(EXECUTABLE)
