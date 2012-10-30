#include the directory with pgl header files
INCLUDEDIR= $(PGLROOT)

#include the directory with boost header files
BOOSTINCLUDEDIR='/usr/local/include'

#include the directory with boost library files
BOOSTLIBDIR='/usr/local/lib'

all: compile

compile:
	g++ example.cpp -O3 -I$(INCLUDEDIR) -I$(BOOSTINCLUDEDIR) -L$(BOOSTLIBDIR) -DNDEBUG -lboost_program_options

debug:
	g++ example.cpp -O0 -g -I$(INCLUDEDIR) -I$(BOOSTINCLUDEDIR) -L$(BOOSTLIBDIR) -Wall -lboost_program_options -DMEMSTATS
	
clean: 
	rm *.out 
