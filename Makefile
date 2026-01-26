CXX ?= c++
AR ?= ar

LIB = libSMLParser.a
SRCS = src/sml_parser.cpp
OBJS = $(SRCS:.cpp=.o)
CXXFLAGS = -std=c++11 -Iinclude -O2 -Wall

all: $(LIB)

$(LIB): $(OBJS)
	$(AR) rcs $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -f $(LIB) $(OBJS)
