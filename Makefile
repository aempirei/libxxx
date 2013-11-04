CXX = g++
CPPFLAGS = -Isrc
CXXFLAGS = -Wall -W -pedantic -std=gnu++11 -O2
LIBFLAGS = -Llib -lxxx -lboost_regex
TARGETS = lib/libxxx.a bin/demo
INSTALL_PATH = /usr/local

.PHONY: all wipe clean $(TARGETS)

all: lib bin $(TARGETS)

lib:

bin:

clean:
	rm -f src/*~ src/*.o $(TARGETS)
	rm -rf bin lib

install:
	install -m 644 lib/libxxx.a $(INSTALL_PATH)/lib
	install -m 644 src/xxx.hh $(INSTALL_PATH)/include

test: $(TARGETS)
	./bin/demo < examples/source.demo

src/libxxx.o: src/xxx.cc src/xxx.hh

src/demo.o: src/demo.cc src/xxx.hh

lib/libxxx.a: src/xxx.o
	if [ ! -d lib ]; then mkdir -vp lib; fi
	ar crfv $@ $^ 

bin/demo: lib/libxxx.a src/demo.o
	if [ ! -d bin ]; then mkdir -vp bin; fi
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBFLAGS)
