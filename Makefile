CXX = g++
CPPFLAGS = -Isrc
CXXFLAGS = -Wall -pedantic -std=gnu++11 -O3
LIBFLAGS = -Llib -lxxx -lboost_regex
TARGETS = lib/libxxx.a bin/xxx
INSTALL_PATH = /usr/local
SOURCES = src/xxx-ast.cc src/xxx-rule.cc src/xxx-predicate.cc src/xxx-grammar.cc src/xxx-q.cc
OBJECTS = src/xxx-ast.o src/xxx-rule.o src/xxx-predicate.o src/xxx-grammar.o src/xxx-q.o

.PHONY: all clean install test library

all: $(TARGETS)

clean:
	rm -f src/*.o $(TARGETS)
	rm -rf bin lib

install:
	install -m 644 lib/libxxx.a $(INSTALL_PATH)/lib
	install -m 644 src/xxx.hh $(INSTALL_PATH)/include
	install -m 755 bin/xxx $(INSTALL_PATH)/bin

test: $(TARGETS)
	./bin/xxx -apg ./test/p-lang.xxx < ./test/example.p-lang

library: lib/libxxx.a

lib/libxxx.a: $(OBJECTS)
	if [ ! -d lib ]; then mkdir -vp lib; fi
	ar crfv $@ $^ 

bin/xxx: src/xxx.o lib/libxxx.a
	if [ ! -d bin ]; then mkdir -vp bin; fi
	$(CXX) $(CXXFLAGS) -o $@ $< $(LIBFLAGS)

src/xxx-xxx.hh:  bin/xxx src/xxx.xxx
	bin/xxx -g src/xxx.xxx -c > src/xxx-xxx.hh
