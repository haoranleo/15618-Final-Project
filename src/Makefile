CC=g++
CXXFLAGS=-g -Wall -MMD -std=c++11 -O3 -pthread

LDFLAGS= -lm
BUILDDIR=bin

all: benchmark

C_FINE_GRAINED_FILES = main.cpp benchmark.cpp coarse_grained_BST.cpp fine_grained_BST.cpp lock_free_BST.cpp cycletimer.cpp test_BST.cpp
H_FINE_GRAINED_FILES = BST_utils.h BST.h benchmark.h coarse_grained_BST.h fine_grained_BST.h lock_free_BST.h cycletimer.h test_BST.h

benchmark: $(C_FINE_GRAINED_FILES) $(H_FINE_GRAINED_FILES)
	$(CC) $(CXXFLAGS) -o benchmark $(C_FINE_GRAINED_FILES) $(LDFLAGS)
	mv benchmark $(BUILDDIR)

clean:
	rm -rf $(BUILDDIR)/*