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

#include <ScalableMeshPCH.h>
#include "Heap.h"

void Heap::swap(int i, int j)
{
    heap_node tmp = ref(i);
    ref(i) = ref(j);
    ref(j) = tmp;
    ref(i).obj->setHeapPos(i);
    ref(j).obj->setHeapPos(j);
}

void Heap::upheap(int i)
{
    if (i == 0) return;

    if (ref(i).import > ref(parent(i)).import) {
        swap(i, parent(i));
        upheap(parent(i));
    }
}

void Heap::downheap(int i)
{
    if (i >= size) return; // perhaps just extracted the last

    int largest = i,
        l = left(i),
        r = right(i);

    if (l<size && ref(l).import > ref(largest).import) largest = l;
    if (r<size && ref(r).import > ref(largest).import) largest = r;

    if (largest != i) {
        swap(i, largest);
        downheap(largest);
    }
}

void Heap::insert(Heapable *t, float v)
{
    if (size == maxLength())
    {
        //cerr << "NOTE: Growing heap from " << size << " to " << 2 * size << endl;
        resize(2 * size);
    }
    int i = size++;
    ref(i).obj = t;
    ref(i).import = v;
    ref(i).obj->setHeapPos(i);
    upheap(i);
}

void Heap::update(Heapable *t, float v)
{
    int i = t->getHeapPos();

    if (i >= size)
    {
        //cerr << "WARNING: Attempting to update past end of heap!" << endl;
        return;
    }
    else if (i == NOT_IN_HEAP)
    {
        //cerr << "WARNING: Attempting to update object not in heap!" << endl;
        return;
    }
    float old = ref(i).import;
    ref(i).import = v;

    if (v<old)
        downheap(i);
    else
        upheap(i);
}

// Remove the top element from the heap
// in putting its flag "notInHeap" to true.
heap_node* Heap::extractTop()
{
    if (size < 1)
    {
        return 0;
    }
    else
    {
        swap(0, size-1);
        size--;
        downheap(0);
        ref(size).obj->notInHeap();
        return &ref(size);
    }
}

// Get the top element without removing it.
heap_node* Heap::getRefOnTop()
{ 
    if (size < 1)
    {
        return (heap_node *)NULL;
    }
    else
    {
        return &ref(0);
    }
}

heap_node *Heap::kill(int i)
{
    if (i >= size)
    {
        //cerr << "WARNING: Attempt to delete invalid heap node." << endl;
    }
    swap(i, size - 1);
    size--;
    ref(size).obj->notInHeap();

    if (ref(i).import < ref(size).import)
    {
        downheap(i);
    }    
    else
    {
        upheap(i);
    }
    return &ref(size);
}