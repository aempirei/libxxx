CXX = g++
CPPFLAGS = -Isrc
CXXFLAGS = -Wall -W -pedantic -std=gnu++11 -O2
LIBFLAGS = -Llib -lxxx -lboost_regex
TARGETS = lib/libxxx.a bin/xxx
INSTALL_PATH = /usr/local
SOURCES = src/xxx-ast.cc src/xxx-rule.cc src/xxx-predicate.cc src/xxx-grammar.cc src/xxx-q.cc
OBJECTS = src/xxx-ast.o src/xxx-rule.o src/xxx-predicate.o src/xxx-grammar.o src/xxx-q.o

.PHONY: all wipe clean install test library

all: $(TARGETS)

clean:
	rm -f src/*.o $(TARGETS)
	rm -rf bin lib

install:
	install -m 644 lib/libxxx.a $(INSTALL_PATH)/lib
	install -m 644 src/xxx.hh $(INSTALL_PATH)/include
	install -m 755 bin/xxx $(INSTALL_PATH)/bin

test: $(TARGETS)
	./bin/xxx -apg ./examples/p-lang.xxx < ./examples/example.p-lang

library: lib/libxxx.a

lib/libxxx.a: $(OBJECTS)
	if [ ! -d lib ]; then mkdir -vp lib; fi
	ar crfv $@ $^ 

bin/xxx: src/xxx.o lib/libxxx.a
	if [ ! -d bin ]; then mkdir -vp bin; fi
	$(CXX) $(CXXFLAGS) -o $@ $< $(LIBFLAGS)
