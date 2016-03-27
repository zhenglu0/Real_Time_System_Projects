# the compiler: g++ for C++ program, define as g++ for C++
CILKCC = /home/luozheng/Documents/cilkplus-install/bin/g++
RM = rm -f
# compiler flags:
#  -g    adds debugging information to the executable file
#  -Wall turns on most, but not all, compiler warnings
CXXFLAGS  = -g -Wall -Wno-unused-but-set-variable  -Wno-unused-variable \
            -Wno-unused-function
#LDFLAGS =
LINKER_OPTS = -Wl,-rpath=/home/luozheng/Documents/cilkplus-install/lib64/ 
CILK = -fcilkplus -lcilkrts
#SPECIAL_FLAGS =

# the build target executable:
TARGET = helloworld fibonacci

all: $(TARGET)

helloworld: helloworld.cpp
	$(CILKCC) $(CXXFLAGS) $(CILK) $< -o $@ $(LINKER_OPTS)

fibonacci: fibonacci.cpp
	$(CILKCC) $(CXXFLAGS) $(CILK) $< -o $@ $(LINKER_OPTS)

clean:
	$(RM) $(TARGET)
