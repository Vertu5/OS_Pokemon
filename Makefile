CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++11 -O2 -lrt
OBJS =  # Add your object files here if needed

all: img-search

img-search: main.cpp $(OBJS)
	$(CXX) $(CXXFLAGS) main.cpp -o img-search $(OBJS)

%.o: %.cpp %.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

