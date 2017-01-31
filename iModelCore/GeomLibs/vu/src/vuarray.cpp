/*--------------------------------------------------------------------------------------+
|
|     $Source: vu/src/vuarray.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE
#include <stdlib.h>

UsageSums g_vuDroppedArraySize = UsageSums  ();

vuArray::vuArray ()
    {
    nRead = 0;
    grabCount = 0;
    }

void vuArray::RecordGrab ()
    {
    assert (grabCount == 0);
    grabCount++;
    }

void vuArray::RecordDrop ()
    {
    g_vuDroppedArraySize.Accumulate ((double)capacity ());
    assert (grabCount == 1);
    grabCount--;
    }


/*---------------------------------------------------------------------------------**//**
@description Get any available array from the graph header.
@remarks The application must return the array via ~mvu_returnArray
@param graphP IN OUT graph header
@return a pointer to the borrowed array header.
@group "VU Node Arrays"
@bsimethod                                                    Earlin.Lutz     10/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP VuArrayP vu_grabArray
(
VuSetP graphP
)
    {
    VuArrayP data = NULL;
    if (graphP->mFreeArrays.size () > 0)
        {
        data = graphP->mFreeArrays.back ();
        graphP->mFreeArrays.pop_back ();
        data->clear ();
        }
    else
        data = new vuArray ();
    graphP->mNumberOfArraysGrabbed++;
    data->RecordGrab ();
    return data;
    }



/*---------------------------------------------------------------------------------**//**
@description Return an array previously grabbed via ~mvu_grabArray.
@param graphP IN OUT graph header
@param arrayP IN pointer to array to drop.
@group "VU Node Arrays"
@bsimethod                                                    Earlin.Lutz     10/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void            vu_returnArray
(
VuSetP graphP,
VuArrayP arrayP
)
    {
    arrayP->RecordDrop ();
    assert (graphP->mNumberOfArraysGrabbed > 0);
    graphP->mNumberOfArraysGrabbed--;
    graphP->mFreeArrays.push_back (arrayP);
    }

/*---------------------------------------------------------------------------------**//**
@description Allocates a new array header.
@return pointer to array header allocated from the heap
@group "VU Node Arrays"
@see vu_arrayFree
@bsimethod                                                      Earlin.Lutz     11/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP VuArrayP        vu_arrayNew
(
)
    {
    return new vuArray ();
    }

/*---------------------------------------------------------------------------------**//**
@description Free both the dynamic and header parts of an array.
@param headerP IN OUT pointer to array header
@group "VU Node Arrays"
@see vu_arrayNew
@bsimethod                                                      Earlin.Lutz     11/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void            vu_arrayFree
(
VuArrayP        headerP
)
    {
    if (NULL != headerP)
        delete headerP;
    }

int vu_arrayCapacity (VuArrayP headerP)
    {
    return (int)headerP->capacity ();
    }
/*---------------------------------------------------------------------------------**//**
@description Empty (clear) the node array.
@remarks Memory from previous use remains allocated, but the formal array size (cf. ~mvu_arraySize) drops to zero.
@param headerP IN OUT pointer to array header
@group "VU Node Arrays"
@bsimethod                                                      Earlin.Lutz     11/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void            vu_arrayClear
(
VuArrayP        headerP
)
    {
    headerP->clear ();
    }

/*---------------------------------------------------------------------------------**//**
@description Query the number of nodes in the array.
@param headerP IN pointer to array header
@return number of nodes in the array
@group "VU Node Arrays"
@bsimethod                                                      Earlin.Lutz     11/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP unsigned int    vu_arraySize
(
VuArrayP        headerP
)
    {
    return (int)headerP->size ();
    }

/*---------------------------------------------------------------------------------**//**
@description Access the node at position zero-based index i in the array.
@param headerP IN pointer to array header
@param i IN unsigned index
@return node from array, or NULL if index is out of range
@group "VU Node Arrays"
@bsimethod                                                      Earlin.Lutz     11/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP VuP             vu_arrayGetVuP
(
VuArrayP        headerP,
unsigned int    i
)
    {
    if ((size_t)i < headerP->size ())
        {
        return headerP->at (i);
        }
    else
        {
        return NULL;
        }
    }

/*---------------------------------------------------------------------------------**//**
@description Read from specified 0-based index in an array, interpreting out-of-bounds index modulo the number of entries in the array.
@param headerP IN pointer to array header
@param i IN signed (!) index to access
@return the VuP at position i mod n (i.e., treat i as a cyclic index), or NULL if the array is empty.
@group "VU Node Arrays"
@bsimethod                                                      Earlin.Lutz     11/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP VuP             vu_arrayGetCyclicVuP
(
VuArrayP        headerP,
int             i
)
    {
    VuP         nodeP = NULL;
    int n = (int)headerP->size ();
    if ( n > 0 )
        {
        while ( i >= n)
            {
            i -= n;
            }
        while ( i < 0)
            {
            i += n;
            }
        nodeP = headerP->at((size_t)i);
        }
    return nodeP;
    }

/*---------------------------------------------------------------------------------**//**
@description Replace the VuP at 0-based index i in the array.
@remarks Invalid i is ignored.
@param headerP IN OUT pointer to array header
@param i IN index of replacement
@param P IN replacement node
@group "VU Node Arrays"
@bsimethod                                                      Earlin.Lutz     11/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void            vu_arrayReplaceVuP
(
VuArrayP        headerP,                /* array in which an element is to be replaced */
unsigned int    i,              /* index of replacement */
VuP             P               /* new value */
)
    {
    if (i < headerP->size ())
        {
        headerP->at((size_t)i) = P;
        }

    }

/*---------------------------------------------------------------------------------**//**
@description Replace each occurrance of oldP by newP.
@param headerP IN OUT pointer to array header
@param oldP IN search node
@param newP IN replacement node
@group "VU Node Arrays"
@bsimethod                                                      Earlin.Lutz     11/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     vu_arrayReplaceMatched
(
VuArrayP        headerP,        /* array in which an element is to be replaced */
VuP             oldP,           /* => old VuP */
VuP             newP            /* => new VuP */
)
    {
    for (size_t i = 0, n = headerP->size (); i < n; i++)
        {
        if (headerP->at(i) == oldP)
            headerP->at(i) = newP;
        }
    }

/*---------------------------------------------------------------------------------**//**
@description Prepare for sequential 'reading' of the array.
@remarks Only one read process is active at any time; each call to this function resets the read pointer.
@param headerP IN OUT pointer to array header
@group "VU Node Arrays"
@bsimethod                                                      Earlin.Lutz     11/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void            vu_arrayOpen
(
VuArrayP        headerP
)
    {
    headerP->nRead = 0;
    }

/*---------------------------------------------------------------------------------**//**
@description Read the next node from an array.
@remarks The array must have been opened for reading with ~mvu_arrayOpen.
@param headerP IN OUT pointer to array header
@param P OUT result node, or NULL if read has finished
@return true if a node was read, false if not
@group "VU Node Arrays"
@bsimethod                                                      Earlin.Lutz     11/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool         vu_arrayRead
(
VuArrayP        headerP,
VuP            *P
)
    {
    bool        stat;
    if (headerP->nRead < headerP->size ())
        {
        *P = headerP->at(headerP->nRead++);
        stat = 1;
        }
    else
        {
        *P = (VuP) 0;
        stat = 0;
        }
    return stat;
    }

/*---------------------------------------------------------------------------------**//**
@description In an array being read via ~mvu_arrayOpen and ~mvu_arrayRead, delete the
    VU that was most recently read, filling its position from an arbitrary unread VU.
@param headerP IN OUT pointer to array header
@group "VU Node Arrays"
@bsimethod                                                      Earlin.Lutz     11/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void vu_arrayRemoveCurrent
(
VuArrayP        headerP
)
    {
    if (headerP->nRead == headerP->size () && headerP->size () > 0)
        {
        /* The final vu was just read --- just back over it */
        headerP->pop_back();
        headerP->nRead--;
        }
    else if (headerP->nRead > 0 &&  headerP->size () > headerP->nRead)
        {
        headerP->at(--headerP->nRead) = headerP->back ();
        headerP->pop_back ();
        }
    }

/*---------------------------------------------------------------------------------**//**
@description Replace the node just returned by the ~mvu_arrayOpen/~mvu_arrayRead mechanism.
@param headerP IN OUT pointer to array header
@param P IN replacement node
@group "VU Node Arrays"
@bsimethod                                                      Earlin.Lutz     11/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void            vu_arrayReplaceCurrent
(
VuArrayP        headerP,
VuP             P
)
    {
    if( headerP->nRead > 0 )
        vu_arrayReplaceVuP(headerP, (unsigned int)headerP->nRead - 1 , P);
    }

/*---------------------------------------------------------------------------------**//**
@description In an array being read via ~mvu_arrayOpen and ~mvu_arrayRead, shift the
    last n entries of the array to just before the read pointer.
@remarks The array's node count is reduced accordingly.
@param headerP IN OUT pointer to array header
@param n IN number of nodes to move
@group "VU Node Arrays"
@bsimethod                                                      Earlin.Lutz     11/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void vu_arrayDeleteBlock
(
VuArrayP        headerP,
unsigned int n
)
    {
    if ( n > headerP->nRead )
        n = (unsigned int)headerP->nRead;

    if ( headerP->nRead >= headerP->size ())
        return;
    for (size_t i = 0, j = headerP->nRead - n, k = headerP->size () - n; i < (size_t)n; i++)
        headerP->at (j) = headerP->at (k);
    headerP->nRead -= n;
    headerP->resize (headerP->size () - n);
    }

/*---------------------------------------------------------------------------------**//**
@description In an array being read via ~mvu_arrayOpen and ~mvu_arrayRead, replace
    the n pointers just before the read pointer.
@remarks Do nothing if fewer than n slots precede the pointer.
@param headerP IN OUT pointer to array header
@param blockPP IN pointer to contiguous block of node pointers
@param n IN number of nodes to replace
@group "VU Node Arrays"
@bsimethod                                                      Earlin.Lutz     11/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void vu_arrayReplaceBlock
(
VuArrayP        headerP,
VuP             *blockPP,
unsigned int    n
)
    {
    if ( n > headerP->nRead )
        return;
    for (size_t i = 0, i0 = headerP->nRead - n; i < (size_t)n; i++)
        headerP->at(i0 + i) = blockPP[i];
    }

/*---------------------------------------------------------------------------------**//**
@description Read multiple nodes from the array to a contiguous buffer.
@param headerP IN OUT pointer to array header
@param blockPP OUT pointer to contiguous block of n or more node pointers
@param n IN number of nodes requested
@return number of nodes copied
@group "VU Node Arrays"
@bsimethod                                                      Earlin.Lutz     11/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int vu_arrayReadBlock
(
VuArrayP        headerP,
VuP             *blockPP,
unsigned int n
)
    {
    size_t nread = 0;
    while ( headerP->nRead < headerP->size () && nread < n )
        {
        blockPP[ nread++ ] = headerP->at (headerP->nRead++);
        }
    return (unsigned int) nread;
    }

/*---------------------------------------------------------------------------------**//**
@description Add a contiguous block of nodes to the array.
@param headerP IN OUT pointer to array header
@param blockPP IN pointer to contiguous block of n or more node pointers
@param n IN number of nodes in block
@group "VU Node Arrays"
@bsimethod                                                      Earlin.Lutz     11/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void vu_arrayAddBlock
(
VuArrayP        headerP,
VuP             *blockPP,
unsigned int n
)
    {
    for (unsigned int  i = 0 ; i < n ; i++ )
        headerP->push_back (blockPP[i]);
    }

/*---------------------------------------------------------------------------------**//**
@description Add a null node to the array.
@param headerP IN OUT pointer to array header
@group "VU Node Arrays"
@bsimethod                                                      Earlin.Lutz     11/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void            vu_arrayAddNull
(
VuArrayP        headerP
)
    {
    if (headerP)
      headerP->push_back (NULL);
    }

/*---------------------------------------------------------------------------------**//**
@description Add a node to a node array.
@remarks This function will <em>not</em> add a null node to the array.
@param headerP IN OUT pointer to array header
@param nodeP IN node to add
@group "VU Node Arrays"
@bsimethod                                                      Earlin.Lutz     11/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void            vu_arrayAdd
(
VuArrayP        headerP,                /* <=> Array being extended */
VuP             nodeP
)
    {
    if (nodeP != NULL && headerP != NULL)
        headerP->push_back (nodeP);
    }

/*---------------------------------------------------------------------------------**//**
@description Remove and return the last node in an array.
@param headerP IN OUT pointer to array header
@return removed node or NULL if array empty
@group "VU Node Arrays"
@bsimethod                                                      Earlin.Lutz     11/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP VuP             vu_arrayRemoveLast
(
VuArrayP        headerP
)
{
    VuP             P = (VuP) 0;
    if (headerP->size() > 0)
        {
        P = headerP->back ();
        headerP->pop_back ();
        }
    return P;
    }

/*---------------------------------------------------------------------------------**//**
@description Reverse the node order in an array.
@param headerP IN OUT pointer to array header
@group "VU Node Arrays"
@bsimethod                                                      Earlin.Lutz     11/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void            vu_arrayReverse
(
VuArrayP        headerP
)
    {
    if ( headerP && headerP->size () > 1)
        {
        for(size_t    i = 0, j = headerP->size () - 1 ;
                i < j ;
                i++, j-- )
            {
            std::swap (headerP->at (i), headerP->at (j));
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
@description Sort the array with a caller-supplied qsort-style comparison function.
@param headerP IN OUT pointer to array header
@param compare IN comparison function
@group "VU Node Arrays"
@bsimethod                                                      Earlin.Lutz     11/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void            vu_arraySort0
(
VuArrayP        headerP,
VuSortFunction0 compare
)
    {
    if (headerP->size () > 1)
        qsort (&headerP->at(0), (int)headerP->size (), sizeof (VuP), (int (*)(const void *,const void *)) compare);
    }

#ifdef CompileSortBuckets
#define FIX_BUCKET(b,m) ( b >= 0 && b < m ? b : m)
/*---------------------------------------------------------------------------------**//**
@nodoc
@description Sort the array into contiguous blocks, the nodes within which lie in the same predefined bucket.
@remarks This is a linear time operation: a couple of passes through the buckets and VUs, but each is just a single sweep.
@remarks The internal (private, undocumented) data field of each node should be prefilled with an integer bucket number in the range
    0..mBucket-1, e.g., using ~mvu_setInternalDataPAsInt.
@remarks All invalid buckets are grouped to a single bucket placed after the final bucket after sorting.  This means that the caller must
    dimension the bucketStart and bucketCount arrays to allow for this extra position.
@param graphP IN OUT graph header (for sorting mask)
@param HP IN OUT array header
@param mBucket IN number of buckets.  Note that arrays should be dimensioned one larger to handle illegal bucket numbers.
@param bucketStart OUT array of start indices. Allocated by caller, initialized here, indexed 0..mBucket.
@param bucketCount OUT array giving number of nodes in each bucket. Allocated by caller, initialized here, indexed 0..mBucket.
@group "VU Node Arrays"
@bsimethod                                                      Earlin.Lutz     11/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void            vu_sortBuckets
(
VuSetP          graphP,
VuArrayP        HP,
int             mBucket,
int            *bucketStart,
int            *bucketCount
)
    {
    int             b, v;       /* Bucket and vu indices */
    int             nVu = HP->nVu;
    VuP            *array = HP->arrayP;
    VuMask          mask = vu_grabMask (graphP);
    VuP             P;
    /*    Clear the counters */
    for (b = 0; b <= mBucket; b++)
        {
        bucketCount[b] = 0;
        }

    /*    Accumulate counters */
    for (v = 0; b < nVu; b++)
        {
        b = VU_GET_LABEL_AS_INT (array[v]);
        b = FIX_BUCKET (b, mBucket);
        bucketCount[b]++;
        VU_CLRMASK (array[v], mask);
        }

    /*    Set the start pointers  */
    for (bucketStart[0] = 0, b = 0; b < mBucket; b++)
        {
        bucketStart[b + 1] = bucketStart[b] + bucketCount[b];
        }

    /*    Move the vu's to the proper places.
       Loop invariants:
       All vu's prior to position v are in
       their proper place.
       Within each bucket, bucketStart points to the
       start of UNplaced vus.

       To extend the invariant, the vu at position v is swapped
       forward to its proper bucket and marked as a correctly
       placed vu.  Its bucketStart is then
       advanced.   The start position v is continually reexamined
       until the vu there becomes marked.

       Termination guarantee: Each step reduces the number of
       improper bucket placements by one.  Hence the vu at v
       must eventually get marked and v can be advanced.
     */
    for (v = 0; v < nVu; v++)
        {
        while (!VU_GETMASK ((P = array[v]), mask))
            {
            b = VU_GET_LABEL_AS_INT (P);
            b = FIX_BUCKET (b, mBucket);
            array[v] = array[bucketStart[v]];
            array[bucketStart[v]] = P;
            bucketStart[v] += 1;
            VU_SETMASK (P, mask);
            }
        }

    /*    Reset the start indices */
    for (bucketStart[0] = 0, b = 0; b < mBucket; b++)
        {
        bucketStart[b + 1] = bucketStart[b] + bucketCount[b];
        }

    vu_returnMask (graphP, mask);
    }
#endif
/*======================================================================+

    Code Section for VuArray-used-as-a-heap

    The array is an implicit binary tree.  Entry 0 is the root.
    The left and right children of entry i are 2i+1 and 2i+2.

+======================================================================*/

#define PARENT(i) ( ((i)-1) >> 1 )
#define LEFT(i) (2*(i)+1)
#define RIGHT(i)        (2*(i)+2)
#define BELOW(HA,i,j) (vu_compareLexicalUV0((HA)+(i),(HA)+(j)) < 0)
#define INTREE(i,n) ((i) < (n))
#define TOP 0
#define SWAP(HA,i,j)            \
        {VuP E = (HA)[(i)];     \
         (HA)[(i)]=(HA)[(j)];   \
         (HA)[(j)]=E;           \
        }

/*---------------------------------------------------------------------------------**//**
@bsimethod                                                      Earlin.Lutz     11/04
+---------------+---------------+---------------+---------------+---------------+------*/
static int vuHeap_shuffleUp    /* <= The index where the original A[i] entry ends up */
(
VuP     heapSortedArrayP[],     /* <=> The heap array in which the shuffle is applied */
int             i               /* => The index where the shuffle starts */
)
    {
    while (i > 0)
        {
        int parent = PARENT (i);
        if (BELOW(heapSortedArrayP, i,parent))
            {
            SWAP (heapSortedArrayP, i, parent);
            i = parent;
            }
        else
            {
            break;
            }
        }
    return i;
    }

/*---------------------------------------------------------------------------------**//**
@bsimethod                                                      Earlin.Lutz     10/94
+---------------+---------------+---------------+---------------+---------------+------*/
static int vuHeap_shuffleDown  /* <= The index where the original A[i] entry ends up */
(
VuP     heapSortedArrayP[],     /* <=> The heap array in which the shuffle is applied */
int             nA,             /* => The total number of entries in the heap. */
int             i               /* => The index where the shuffle starts */
)
    {
    /*************************************************************************
    ** Possible configurations going down, where (i left right) indicates values of i and its subtrees:
    **
    ** The full case list for child-parent relations is:
    **       (1 2 3) or (1 3 2)              QUIT
    **       (2 3 1) or (2 1 3)              exchange with the "1" and continue down.
    **       (3 1 2) or (3 2 1)              same
    **       (1 2 X)                         QUIT
    **       (2 1 X)                         exchange with the "1".  Since tree is balanced, there are no more children.
    **
    ** The actual shuffle-down reduces to finding the smaller child and pushing onward only in that direction.
    **************************************************************************/
    int smallerChild, left, right;
    for (;;)
        {
        smallerChild = left = LEFT (i);
        right = RIGHT (i);

        if (INTREE (right, nA) && BELOW(heapSortedArrayP, right, left))
            {
                /* NB: The INTREE test for right assured that left is also
                        INTREE and the BELOW test is valid */
                smallerChild = right;
            }

        if (INTREE( smallerChild, nA) && BELOW(heapSortedArrayP,smallerChild,i))
            {
            SWAP (heapSortedArrayP, smallerChild, i);
            i = smallerChild;
            }
        else
            {
            break;
            }
        }

    return i;
    }

/*---------------------------------------------------------------------------------**//**
@description Remove the minimum entry from the heap.
@remarks The returned node is not nulled if the heap is empty.
@param headerP IN OUT pointer to array header
@param nodePP OUT node pointer
@return true if a node is returned, false if heap is empty at call time.
@group "VU Node Heap"
@bsimethod                                                      Earlin.Lutz     11/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     vu_heapRemoveMin
(
VuArrayP headerP,
VuP *nodePP
)
    {
    bool        stat = 0;
    if (headerP && headerP->size () > 0)
        {
        *nodePP = headerP->at(0);
        headerP->at (0) = headerP->back ();
        headerP->pop_back ();
        vuHeap_shuffleDown (&headerP->at(0), (unsigned int)headerP->size (), TOP);
        stat = 1;
        }
    return stat;
    }

/*---------------------------------------------------------------------------------**//**
@description Insert a node into the heap.
@param headerP IN OUT pointer to array header
@param nodeP IN node pointer
@group "VU Node Heap"
@bsimethod                                                      Earlin.Lutz     11/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void            vu_heapInsert
(
VuArrayP headerP,
VuP nodeP
)
    {
    /* Put the vu at the end of the array */
    vu_arrayAdd(headerP,nodeP);
    /* Shuffle logic pulls it back up to its proper place */
    vuHeap_shuffleUp (&headerP->at (0), (unsigned int)headerP->size () - 1);
    }

/*---------------------------------------------------------------------------------**//**
@description Sort an array into heap structure, using ~mvu_compareLexicalUV0 as comparison function.
@param headerP IN OUT pointer to array header
@group "VU Node Heap"
@bsimethod                                                      Earlin.Lutz     11/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void            vu_sortArrayToHeap
(
VuArrayP headerP
)
    {
    vu_arraySort0 (headerP, vu_compareLexicalUV0);
    }
END_BENTLEY_GEOMETRY_NAMESPACE
