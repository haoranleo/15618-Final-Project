CC=g++
CXXFLAGS=-g -Wall -MMD -std=c++11 -O3

LDFLAGS= -lm
BUILDDIR=bin

all: fine_grained

C_FINE_GRAINED_FILES = fine_grained_BST.cpp
H_FINE_GRAINED_FILES = BST_utils.h BST.h fine_grained_BST.h

fine_grained: $(C_FINE_GRAINED_FILES) $(H_FINE_GRAINED_FILES)
	$(CC) $(CXXFLAGS) -o fine_grained_BST $(C_FINE_GRAINED_FILES) $(LDFLAGS)
	mv fine_grained_BST $(BUILDDIR)