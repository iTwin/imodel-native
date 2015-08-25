/*--------------------------------------------------------------------------------------+
|
|   Code taken from Michael Garland's demo application called "QSlim" (version 1.0)
|   which intends to demonstrate an algorithm of mesh simplification based on
|   Garland and Heckbert(1997) "Surface Simplification Using Quadric Error Metrics".
|   The code of the demo is said to be in the public domain.
|   See: http://www.cs.cmu.edu/afs/cs/Web/People/garland/quadrics/qslim10.html
|
|   $Revision: 1.0 $
|       $Date: 2014/09/17 $
|     $Author: Christian.Cote $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include "Array.h"

#define NOT_IN_HEAP -47

class Heapable
{
    private:
        int token;
    public:
        Heapable() { notInHeap(); }
        inline int isInHeap() { return token != NOT_IN_HEAP; }
        inline void notInHeap() { token = NOT_IN_HEAP; }
        inline int getHeapPos() { return token; }
        inline void setHeapPos(int t) { token = t; }
};

class heap_node 
{
    public:
        float import;
        Heapable *obj;
        heap_node() { obj = NULL; import = 0.0; }
        heap_node(Heapable *t, float i = 0.0) { obj = t; import = i; }
        heap_node(const heap_node& h) { import = h.import; obj = h.obj; }
};

class Heap : public array<heap_node> 
{        
        void swap(int i, int j);
        int parent(int i) { return (i - 1) / 2; }
        int left(int i) { return 2 * i + 1; }
        int right(int i) { return 2 * i + 2; }
        void upheap(int i);
        void downheap(int i);

    public:

        // The actual size of the heap (the number of elements valid or not):
        int size;
        Heap() : array<heap_node>() { size = 0; }
        Heap(int s) : array<heap_node>(s) { size = 0; }
        void insert(Heapable *, float);
        void update(Heapable *, float);
        heap_node *extractTop();
        heap_node *getRefOnTop();
        heap_node *kill(int i);
};
