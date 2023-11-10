# Makefile
CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++11 -O2 -lrt
OBJS = search-img/imgSearchFunctions.o search-img/main.o  # Mettez à jour le chemin des fichiers objets

all: img-search

img-search: search-img/main.o search-img/imgSearchFunctions.o
	$(CXX) $(CXXFLAGS) search-img/main.o search-img/imgSearchFunctions.o -o img-search

search-img/main.o: search-img/main.cpp
	$(CXX) $(CXXFLAGS) -c search-img/main.cpp -o search-img/main.o

search-img/imgSearchFunctions.o: search-img/imgSearchFunctions.cpp search-img/imgSearchFunctions.h
	$(CXX) $(CXXFLAGS) -c search-img/imgSearchFunctions.cpp -o search-img/imgSearchFunctions.o

clean:
	rm -f img-search search-img/main.o search-img/imgSearchFunctions.o

