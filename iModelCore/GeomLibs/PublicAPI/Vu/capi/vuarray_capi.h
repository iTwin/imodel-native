/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
/* DO NOT EDIT!  THIS FILE IS GENERATED. */

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/*---------------------------------------------------------------------------------**//**
@description Allocates a new array header.
@return pointer to array header allocated from the heap
@group "VU Node Arrays"
@see vu_arrayFree
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP VuArrayP        vu_arrayNew
(
);

/*---------------------------------------------------------------------------------**//**
@description Free both the dynamic and header parts of an array.
@param headerP IN OUT pointer to array header
@group "VU Node Arrays"
@see vu_arrayNew
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void            vu_arrayFree
(
VuArrayP        headerP
);

/*---------------------------------------------------------------------------------**//**
@description Compute the total number of bytes allocated to the array.
@remarks This count includes both the header and the dynamically allocated contiguous buffer.
@param headerP IN pointer to array header
@return Byte count
@group "VU Node Arrays"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int vu_arrayGetHeapMemorySize
(
VuArrayP headerP
);

/*---------------------------------------------------------------------------------**//**
@description Return the pre-allocated capacity of the array.
@param headerP IN pointer to array header
@return number of VuP pointers.
@group "VU Node Arrays"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int vu_arrayCapacity
(
VuArrayP headerP
);

/*---------------------------------------------------------------------------------**//**
@description Empty (clear) the node array.
@remarks Memory from previous use remains allocated, but the formal array size (cf. ~mvu_arraySize) drops to zero.
@param headerP IN OUT pointer to array header
@group "VU Node Arrays"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void            vu_arrayClear
(
VuArrayP        headerP
);

/*---------------------------------------------------------------------------------**//**
@description Query the number of nodes in the array.
@param headerP IN pointer to array header
@return number of nodes in the array
@group "VU Node Arrays"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP unsigned int    vu_arraySize
(
VuArrayP        headerP
);

/*---------------------------------------------------------------------------------**//**
@description Access the node at position zero-based index i in the array.
@param headerP IN pointer to array header
@param i IN unsigned index
@return node from array, or NULL if index is out of range
@group "VU Node Arrays"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP VuP             vu_arrayGetVuP
(
VuArrayP        headerP,
unsigned int    i
);

/*---------------------------------------------------------------------------------**//**
@description Read from specified 0-based index in an array, interpreting out-of-bounds index modulo the number of entries in the array.
@param headerP IN pointer to array header
@param i IN signed (!) index to access
@return the VuP at position i mod n (i.e., treat i as a cyclic index), or NULL if the array is empty.
@group "VU Node Arrays"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP VuP             vu_arrayGetCyclicVuP
(
VuArrayP        headerP,
int             i
);

/*---------------------------------------------------------------------------------**//**
@description Replace the VuP at 0-based index i in the array.
@remarks Invalid i is ignored.
@param headerP IN OUT pointer to array header
@param i IN index of replacement
@param P IN replacement node
@group "VU Node Arrays"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void            vu_arrayReplaceVuP
(
VuArrayP        headerP,                /* array in which an element is to be replaced */
unsigned int    i,              /* index of replacement */
VuP             P               /* new value */
);

/*---------------------------------------------------------------------------------**//**
@description Replace each occurrance of oldP by newP.
@param headerP IN OUT pointer to array header
@param oldP IN search node
@param newP IN replacement node
@group "VU Node Arrays"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     vu_arrayReplaceMatched
(
VuArrayP        headerP,        /* array in which an element is to be replaced */
VuP             oldP,           /* IN      old VuP */
VuP             newP            /* IN      new VuP */
);

/*---------------------------------------------------------------------------------**//**
@description Prepare for sequential 'reading' of the array.
@remarks Only one read process is active at any time; each call to this function resets the read pointer.
@param headerP IN OUT pointer to array header
@group "VU Node Arrays"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void            vu_arrayOpen
(
VuArrayP        headerP
);

/*---------------------------------------------------------------------------------**//**
@description Read the next node from an array.
@remarks The array must have been opened for reading with ~mvu_arrayOpen.
@param headerP IN OUT pointer to array header
@param P OUT result node, or NULL if read has finished
@return true if a node was read, false if not
@group "VU Node Arrays"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool         vu_arrayRead
(
VuArrayP        headerP,
VuP            *P
);

/*---------------------------------------------------------------------------------**//**
@description In an array being read via ~mvu_arrayOpen and ~mvu_arrayRead, delete the
    VU that was most recently read, filling its position from an arbitrary unread VU.
@param headerP IN OUT pointer to array header
@group "VU Node Arrays"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void vu_arrayRemoveCurrent
(
VuArrayP        headerP
);

/*---------------------------------------------------------------------------------**//**
@description Replace the node just returned by the ~mvu_arrayOpen/~mvu_arrayRead mechanism.
@param headerP IN OUT pointer to array header
@param P IN replacement node
@group "VU Node Arrays"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void            vu_arrayReplaceCurrent
(
VuArrayP        headerP,
VuP             P
);

/*---------------------------------------------------------------------------------**//**
@description In an array being read via ~mvu_arrayOpen and ~mvu_arrayRead, shift the
    last n entries of the array to just before the read pointer.
@remarks The array's node count is reduced accordingly.
@param headerP IN OUT pointer to array header
@param n IN number of nodes to move
@group "VU Node Arrays"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void vu_arrayDeleteBlock
(
VuArrayP        headerP,
unsigned int n
);

/*---------------------------------------------------------------------------------**//**
@description In an array being read via ~mvu_arrayOpen and ~mvu_arrayRead, replace
    the n pointers just before the read pointer.
@remarks Do nothing if fewer than n slots precede the pointer.
@param headerP IN OUT pointer to array header
@param blockPP IN pointer to contiguous block of node pointers
@param n IN number of nodes to replace
@group "VU Node Arrays"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void vu_arrayReplaceBlock
(
VuArrayP        headerP,
VuP             *blockPP,
unsigned int    n
);

/*---------------------------------------------------------------------------------**//**
@description Read multiple nodes from the array to a contiguous buffer.
@param headerP IN OUT pointer to array header
@param blockPP OUT pointer to contiguous block of n or more node pointers
@param n IN number of nodes requested
@return number of nodes copied
@group "VU Node Arrays"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int vu_arrayReadBlock
(
VuArrayP        headerP,
VuP             *blockPP,
unsigned int n
);

/*---------------------------------------------------------------------------------**//**
@description Add a contiguous block of nodes to the array.
@param headerP IN OUT pointer to array header
@param blockPP IN pointer to contiguous block of n or more node pointers
@param n IN number of nodes in block
@group "VU Node Arrays"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void vu_arrayAddBlock
(
VuArrayP        headerP,
VuP             *blockPP,
unsigned int n
);

/*---------------------------------------------------------------------------------**//**
@description Add a null node to the array.
@param headerP IN OUT pointer to array header
@group "VU Node Arrays"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void            vu_arrayAddNull
(
VuArrayP        headerP
);

/*---------------------------------------------------------------------------------**//**
@description Add a node to a node array.
@remarks This function will <em>not</em> add a null node to the array.
@param headerP IN OUT pointer to array header
@param nodeP IN node to add
@group "VU Node Arrays"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void            vu_arrayAdd
(
VuArrayP        headerP,                /* IN OUT  Array being extended */
VuP             nodeP
);

/*---------------------------------------------------------------------------------**//**
@description Remove and return the last node in an array.
@param headerP IN OUT pointer to array header
@return removed node or NULL if array empty
@group "VU Node Arrays"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP VuP             vu_arrayRemoveLast
(
VuArrayP        headerP
);

/*---------------------------------------------------------------------------------**//**
@description Reverse the node order in an array.
@param headerP IN OUT pointer to array header
@group "VU Node Arrays"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void            vu_arrayReverse
(
VuArrayP        headerP
);

/*---------------------------------------------------------------------------------**//**
@description Sort the array with a caller-supplied qsort-style comparison function.
@param headerP IN OUT pointer to array header
@param compare IN comparison function
@group "VU Node Arrays"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void            vu_arraySort0
(
VuArrayP        headerP,
VuSortFunction0 compare
);

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
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void            vu_sortBuckets
(
VuSetP          graphP,
VuArrayP        HP,
int             mBucket,
int            *bucketStart,
int            *bucketCount
);

/*---------------------------------------------------------------------------------**//**
@description Remove the minimum entry from the heap.
@remarks The returned node is not nulled if the heap is empty.
@param headerP IN OUT pointer to array header
@param nodePP OUT node pointer
@return true if a node is returned, false if heap is empty at call time.
@group "VU Node Heap"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     vu_heapRemoveMin
(
VuArrayP headerP,
VuP *nodePP
);

/*---------------------------------------------------------------------------------**//**
@description Insert a node into the heap.
@param headerP IN OUT pointer to array header
@param nodeP IN node pointer
@group "VU Node Heap"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void            vu_heapInsert
(
VuArrayP headerP,
VuP nodeP
);

/*---------------------------------------------------------------------------------**//**
@description Sort an array into heap structure, using ~mvu_compareLexicalUV0 as comparison function.
@param headerP IN OUT pointer to array header
@group "VU Node Heap"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void            vu_sortArrayToHeap
(
VuArrayP headerP
);

END_BENTLEY_GEOMETRY_NAMESPACE

