DIR_INC = ./include
DIR_LIB = ./lib

SHAREDNAME = libcts.so

CPPFLAGS = -g -Wall -fPIC -I$(DIR_INC) 

LDFLAGS := -L$(DIR_LIB)
LDFLAGS += -lmsc -lrt -ldl -lpthread -shared

OBJECTS := $(patsubst %.cpp,%.o,$(wildcard *.cpp))
OBJECTS += $(patsubst %.c,%.o,$(wildcard *.c))

$(SHAREDNAME) : $(OBJECTS)
	g++ $(CPPFLAGS) $^ -o $@ $(LDFLAGS)

%.o : %.cpp
	g++ -c $(CPPFLAGS) $< -o $@

clean:
	@rm -f *.o $(SHAREDNAME)
