CXX ?= c++
AR ?= ar

LIB = libSMLParser.a
SRCS = src/sml_parser.cpp
OBJS = $(SRCS:.cpp=.o)
CXXFLAGS = -std=c++11 -Iinclude -O2 -Wall -MMD -MP
DEPS = $(OBJS:.o=.d)

all: $(LIB)

$(LIB): $(OBJS)
	$(AR) rcs $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

-include $(DEPS)

clean:
	rm -f $(LIB) $(OBJS) $(DEPS)
