CXX=g++
CXXSTD=--std=c++17
CXXOPTIMFLAGS=-O3 -g
CXXDEBUGFLAGS=-g -O0 -rdynamic
CXXFLAGS=-Wall $(CXXOPTIMFLAGS)
LIBS=-lncurses -lpthread

# find source files
SOURCEDIR := $(shell pwd)
BUILDDIR := $(shell pwd)
SOURCES := $(shell find $(SOURCEDIR) -path $(SOURCEDIR)/benchmark -prune -o -name '*.cpp' -print)
SOURCES := $(filter-out $(SOURCEDIR)/main.cpp, $(SOURCES))
SOURCES := $(filter-out $(SOURCEDIR)/server/server.cpp, $(SOURCES))
SOURCES := $(filter-out $(SOURCEDIR)/client/client.cpp, $(SOURCES))
SOURCES := $(filter-out $(SOURCEDIR)/commandline/example.cpp, $(SOURCES))
#OBJECTS := $(addprefix $(BUILDDIR)/,$(notdir $(SOURCES:.cpp=.o)))
OBJECTS := $(SOURCES:.cpp=.o)
OBJECTS_MAIN := $(OBJECTS) $(SOURCEDIR)/main.o
OBJECTS_SERVER := $(OBJECTS) $(SOURCEDIR)/server/server.o
OBJECTS_CLIENT := $(OBJECTS) $(SOURCEDIR)/client/client.o

.PHONY: all main server client

all: main server client

main: main.x

server: server.x

client: client.x

# link
main.x: $(OBJECTS_MAIN)
	$(CXX) $(CXXFLAGS) $(OBJECTS_MAIN) -o main.x $(LIBS)

server.x: $(OBJECTS_SERVER)
	$(CXX) $(CXXFLAGS) $(OBJECTS_SERVER) -o server.x $(LIBS)

client.x: $(OBJECTS_CLIENT)
	$(CXX) $(CXXFLAGS) $(OBJECTS_CLIENT) -o client.x $(LIBS)

# pull dependencies for existing .o files
-include $(OBJECTS_MAIN:.o=.d)
-include $(OBJECTS_SERVER:.o=.d)
-include $(OBJECTS_CLIENT:.o=.d)

# compile and generate dependency info
%.o: %.cpp %.d
	$(CXX) $(CXXSTD) -c $(CXXFLAGS) $*.cpp -o $*.o
	$(CXX) $(CXXSTD) -MM $(CXXFLAGS) $*.cpp > $*.d

# empty rule for dependency files
%.d: ;

clean:
	rm -f *core *.o *.d src/*.o src/*.d
