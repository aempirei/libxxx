CXX = g++
CPPFLAGS = -Isrc
CXXFLAGS = -Wall -W -pedantic -std=gnu++11 -O2
LIBFLAGS = -Llib -lxxx -lpthread -lboost_regex
TARGETS = lib/libxxx.a bin/demo

.PHONY : all wipe clean $(TARGETS)

all : lib bin $(TARGETS)

lib:

bin:

clean :
	rm -f src/*~ src/*.o $(TARGETS)
	rm -rf bin lib

src/libxxx.o: src/libxxx.cc src/libxxx.hh

src/demo.o: src/demo.cc src/libxxx.hh

lib/libxxx.a: src/libxxx.o
	if [ ! -d lib ]; then mkdir -vp lib; fi
	ar crfv $@ $^ 

bin/demo: lib/libxxx.a src/demo.o
	if [ ! -d bin ]; then mkdir -vp bin; fi
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBFLAGS)
