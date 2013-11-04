CXX = g++
CPPFLAGS = -Isrc
CXXFLAGS = -Wall -W -pedantic -std=gnu++11 -O2
LIBFLAGS = -Llib -llang -lpthread -lboost_regex
TARGETS = lib/liblang.a bin/demo

.PHONY : all wipe clean $(TARGETS)

all : lib bin $(TARGETS)

lib:

bin:

clean :
	rm -f src/*~ src/*.o $(TARGETS)
	rm -rf bin lib

src/liblang.o: src/liblang.cc src/liblang.hh

src/demo.o: src/demo.cc src/liblang.hh

lib/liblang.a: src/liblang.o
	if [ ! -d lib ]; then mkdir -vp lib; fi
	ar crfv $@ $^ 

bin/demo: lib/liblang.a src/demo.o
	if [ ! -d bin ]; then mkdir -vp bin; fi
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBFLAGS)
