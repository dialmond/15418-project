# Compiler and flags
CXX = g++
CXXFLAGS = -Wall -O3 -std=c++20 -m64 -I.

MPICXX = mpic++

# Target executable
TARGET = render

# Source files
SRC = material.cpp scenes.cpp

OBJS = main.o material.o scenes.o

SCATTER_PROB ?=

ifeq ($(SCATTER_PROB),)
    CUSTOM_DEFINE =
else
    CUSTOM_DEFINE = -DSCATTER_PROB=$(SCATTER_PROB)
endif

# Build rules
all: $(TARGET)

$(TARGET)_scatter: main.cpp $(SRC)
	$(CXX) $(CXXFLAGS) -fopenmp $(CUSTOM_DEFINE) -o $(TARGET)_scatter main.cpp $(SRC)

$(TARGET): main.cpp $(SRC)
	$(CXX) $(CXXFLAGS) -fopenmp $(CUSTOM_DEFINE) -o $(TARGET) main.cpp $(SRC)


mpi: $(OBJS)
	$(MPICXX) $(CXXFLAGS) -DP_MPI=1 -o render_mpi $(OBJS)

%.o: %.cpp
	$(MPICXX) $(CXXFLAGS) -DP_MPI=1 -c -o $@ $<

demo: demo.cpp $(SRC)
	$(CXX) $(CXXFLAGS) -fopenmp -lSDL2 -o demo demo.cpp $(SRC) -lSDL2

compare: compare.cpp
	$(CXX) $(CXXFLAGS) -o compare compare.cpp

clean:
	rm -f $(TARGET)
	rm -f *.o

.PHONY: all clean

