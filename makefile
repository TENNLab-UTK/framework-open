### TENNLab Open-Source Neuromorphic Framework

# JSP: My goal with this makefile is to keep it simple.  If you want a more
# complex makefile, please create a new file, and then document here how to
# use it.  Please don't modify this file to use your favorite features of make.

# This makefile is super-simple: Its goal is to make one library and two programs:
#
# lib/libframework.a is the library, containing network and processor support.
# bin/network_tool is a command-line tool for creating and manipulating networks.
# bin/processor_tool_risp is a command-line tool for loading networks onto the RISP
#                         simulator and then running the simulator.
#
# You'll note that the processor_tool code is agnostic as to the processor, using
# the processor interface from include/framework.hpp.  It should be clear how to
# compile this for a different processor.

CXX ?= g++

FR_LIB = lib/libframework.a
FR_INC = include/framework.hpp
FR_CFLAGS = -std=c++11 -Wall -Wextra -Iinclude -Iinclude/utils $(CFLAGS)
FR_OBJ = obj/framework.o obj/processor_help.o obj/properties.o

RISP_INC = include/risp.hpp
RISP_OBJ = obj/risp.o obj/risp_static.o

VRISP_INC = include/vrisp.hpp
VRISP_OBJ = obj/vrisp.o obj/vrisp_static.o

all: lib/libframework.a \
     bin/network_tool \
     bin/processor_tool_risp \
	 bin/processor_tool_vrisp

utils: bin/property_pack_tool \
       bin/property_tool

clean:
	rm -f bin/* obj/* lib/*

# ------------------------------------------------------------ 
# The library and two programs.  You should see how to compile the processor_tool
# for a different processor.

lib/libframework.a: $(FR_OBJ) include/framework.hpp
	ar r lib/libframework.a $(FR_OBJ)
	ranlib lib/libframework.a

bin/network_tool: src/network_tool.cpp $(FR_INC) $(FR_LIB)
	$(CXX) $(FR_CFLAGS) -o bin/network_tool src/network_tool.cpp $(FR_LIB)

bin/processor_tool_risp: src/processor_tool.cpp $(FR_INC) $(RISP_INC) $(RISP_OBJ) $(FR_LIB)
	$(CXX) $(FR_CFLAGS) -o bin/processor_tool_risp src/processor_tool.cpp $(RISP_OBJ) $(FR_LIB)

bin/processor_tool_vrisp: src/processor_tool.cpp $(FR_INC) $(VRISP_INC) $(VRISP_OBJ) $(FR_LIB)
	$(CXX) $(FR_CFLAGS) -o bin/processor_tool_vrisp src/processor_tool.cpp $(VRISP_OBJ) $(FR_LIB)

# ------------------------------------------------------------ 
# Utilities.

bin/property_tool: src/property_tool.cpp $(FR_INC) $(FR_LIB)
	$(CXX) $(FR_CFLAGS) -o bin/property_tool src/property_tool.cpp $(FR_LIB)

bin/property_pack_tool: src/property_pack_tool.cpp $(FR_INC) $(FR_LIB)
	$(CXX) $(FR_CFLAGS) -o bin/property_pack_tool src/property_pack_tool.cpp $(FR_LIB)

# ------------------------------------------------------------ 
# Object files

obj/risp.o: src/risp.cpp $(FR_INC) $(RISP_INC)
	$(CXX) -c $(FR_CFLAGS) -o obj/risp.o src/risp.cpp

obj/risp_static.o: src/risp_static.cpp $(FR_INC) $(RISP_INC)
	$(CXX) -c $(FR_CFLAGS) -o obj/risp_static.o src/risp_static.cpp

obj/vrisp.o: src/vrisp.cpp $(FR_INC) $(VRISP_INC)
	$(CXX) -c $(FR_CFLAGS) -o obj/vrisp.o src/vrisp.cpp

obj/vrisp_static.o: src/vrisp_static.cpp $(FR_INC) $(VRISP_INC)
	$(CXX) -c $(FR_CFLAGS) -o obj/vrisp_static.o src/vrisp_static.cpp

obj/framework.o: src/framework.cpp $(FR_INC)
	$(CXX) -c $(FR_CFLAGS) -o obj/framework.o src/framework.cpp

obj/processor_help.o: src/processor_help.cpp $(FR_INC)
	$(CXX) -c $(FR_CFLAGS) -o obj/processor_help.o src/processor_help.cpp

obj/properties.o: src/properties.cpp $(FR_INC)
	$(CXX) -c $(FR_CFLAGS) -o obj/properties.o src/properties.cpp
