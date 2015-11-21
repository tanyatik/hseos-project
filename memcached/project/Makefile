default: all

CC=g++
CXXWARN=-Wall
CXXFLAGS=-O0 -g -std=c++11 -fno-omit-frame-pointer
CXXFLAGS+=-I. -isystem ./contrib/
LDFLAGS=-g
LDLIBS=-lpthread -lglog

GTEST_OBJS=contrib/gmock-gtest-all.o contrib/gmock_main.o
UT_OBJS=$(patsubst %.cpp,%.o,$(wildcard ut/*_ut.cpp))

OBJS=$(patsubst %.cpp,%.o,$(filter-out main.cpp, $(wildcard *.cpp)))

BINS=run_tests server

COMPILE_RULE=$(CC) $(CXXWARN) $(CXXFLAGS) $< -c -o $@ -MP -MMD -MF deps/$(subst /,-,$@).d
LINK_RULE=$(CC) $(LDFLAGS) $^ -o $@ $(LDLIBS)

run_tests: $(GTEST_OBJS) $(OBJS) $(UT_OBJS)
	$(LINK_RULE)

server: $(OBJS) main.o
	$(LINK_RULE)

%.o: %.cpp
	$(COMPILE_RULE)

all: $(BINS)

clean:
	@rm -f $(BINS)
	@rm -f ./*.o ut/*_ut.o
	@rm -f ./deps/*.d

-include deps/*.d


