/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
/* DO NOT EDIT!  THIS FILE IS GENERATED. */

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/**
* Allocates a new vertex use array.
* @param void
* @return pointer to the newly allocated and initialized array.
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP MTG_NodeArrayHeader * jmdlMTGNodeArray_new
(
void
);

/**
* Frees the header and array for a MTG_NodeArrayHeader *.
* @param pArrayHeader IN      Pointer to the array header to be freed
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGNodeArray_free
(
MTG_NodeArrayHeader *pArrayHeader
);

/**
* Inquires the heap memory size currently allocated to the MTG_NodeArrayHeader *
* @return  size in bytes.
* @param  pArrayHeader  IN
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int jmdlMTGNodeArray_getHeapMemorySize
(
MTG_NodeArrayHeader * pArrayHeader
);

/**
* Clears a dynamic array.
* @param  pArrayHeader  IN      Array whose counter size is be cleared
* @see
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGNodeArray_clear
(
MTG_NodeArrayHeader *        pArrayHeader
);

/**
* @return  the number of MTGNodeId pointers in a dynamic array.
* @param  pArrayHeader  IN      Array whose size is being queried
* @see
* @return  int
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int    jmdlMTGNodeArray_getCount
(
MTG_NodeArrayHeader *        pArrayHeader
);

/**
* @return  the MTGNodeId at position i (zerobased) in a dynamic array. Returns
*       NULL if the index is out of range.
* @param pArrayHeader IN      Array being accessed
* @param i            IN      index to access
* @see
* @return MTGNodeId
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP MTGNodeId jmdlMTGNodeArray_get  /* node id at index i */
(

MTG_NodeArrayHeader *        pArrayHeader,
        int i
);

/**
* @return  the MTGNodeId at position (i mod n) in a dynamic array, i.e  treats
*       i as a cyclic index.  Returns NULL if the array is empty.
* @param pArrayHeader IN      Array being accessed
* @param i IN      index to access.  Out of range values
                                                are treated cyclically.
* @see
* @return MTGNodeId
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP MTGNodeId jmdlMTGNodeArray_getCyclic     /* OUT     value at index i */
(
MTG_NodeArrayHeader *        pArrayHeader,
int             i
);

/**
* Replaces the MTGNodeId at position i in a dynamic array. Invalid i is
* ignored.
* @param pArrayHeader   IN OUT  array in which an element is to be replaced
* @param i  IN      index of replacement
* @param nodeId IN      new value
* @see
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGNodeArray_replace
(
MTG_NodeArrayHeader *        pArrayHeader,
 int    i,
MTGNodeId             nodeId
);

/**
* Replaces each occurrance of oldP by newP.
* @param pArrayHeader IN OUT  array in which an element is to be replaced
* @param pOld IN      old MTGNodeId
* @param pNew IN      new MTGNodeId
* @see
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGNodeArray_replaceMatched
(
MTG_NodeArrayHeader *        pArrayHeader,
MTGNodeId             pOld,
MTGNodeId             pNew
);

/**
* Prepare for sequential 'reading' of the dynamic array.  Only one
* read process is activve at any time; each call to 'open' the reading
* resets the read pointer.
* @param pArrayHeader   IN      array whose entries are about to be read
* @see
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGNodeArray_open
(
MTG_NodeArrayHeader *        pArrayHeader
);

/**
* @param pArrayHeader   IN      <=>Array being read
* @param pNodeId OUT     element read from the array. null if read is complete
* @see
* @return int
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int jmdlMTGNodeArray_read     /* OUT     true if an element was read. */
(
MTG_NodeArrayHeader *    pArrayHeader,
MTGNodeId          *pNodeId
);

/**
* In an array being read via jmdlMTGNodeArray_open and jmdlMTGNodeArray_read, delete the
* vu that was most recently read, filling its position from an arbitrary
* unread VU.
* @param pArrayHeader   IN      Array being read
* @see
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGNodeArray_removeCurrent
(
MTG_NodeArrayHeader *        pArrayHeader
);

/**
* Replace the MTGNodeId just returned by the open/read mechanism.
* @param pArrayHeader   IN      array in which an element is to be replaced
* @param nodeId      IN      new value
* @see
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGNodeArray_replaceCurrent
(
MTG_NodeArrayHeader *        pArrayHeader,
MTGNodeId             nodeId
);

/**
* In an array being read via jmdlMTGNodeArray_open and jmdlMTGNodeArray_read, shift the
* last n pointers of the array to just before the read pointer.
* @param  pArrayHeader  IN      Array being read
* @param  n IN      .
* @see
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGNodeArray_deleteBlock
(
MTG_NodeArrayHeader *        pArrayHeader,
 int n
);

/**
* In an array being read via jmdlMTGNodeArray_open and jmdlMTGNodeArray_read, replace
* the n pointers just before the read pointer.  NOOP if fewer than
* n slots precede the pointer.
* @param pArrayHeader   IN      Array being read
* @param pNodeArray IN      block of vertex use pointers
* @param n  IN      .
* @see
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGNodeArray_replaceBlock
(
MTG_NodeArrayHeader *        pArrayHeader,
MTGNodeId             *pNodeArray,
 int n
);

/**
* Read up to n elements from the array.  Return count of items read
* @param pArrayHeader   IN      Array being read
* @param pNodeArray OUT     Buffer, must be n elements or more
* @param n IN      number of elements to read
* @see
* @return int
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int jmdlMTGNodeArray_readBlock
(
MTG_NodeArrayHeader *        pArrayHeader,
MTGNodeId             *pNodeArray,
 int n
);

/**
* @description Write n elements to the array.
* @param pArrayHeader   IN OUT  Array being appended to.
* @param pNodeArray IN      Buffer, must be n elements or more
* @param n  IN      # elements to add
* @see
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGNodeArray_addBlock
(
MTG_NodeArrayHeader *        pArrayHeader,
MTGNodeId             *pNodeArray,
 int n
);

/**
* @param pArrayHeader IN OUT  Array being extended
* @param nodeId IN      Value to insert in pArrayHeader
* @see
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGNodeArray_add
(
MTG_NodeArrayHeader *        pArrayHeader,
MTGNodeId             nodeId
);

/**
* @param pArrayHeader IN      Array whose last element is being removed
* @see
* @return MTGNodeId
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP MTGNodeId jmdlMTGNodeArray_removeLast /* OUT     element take from the end of pArrayHeader */
(
MTG_NodeArrayHeader *        pArrayHeader
);

/**
* @param pArrayHeader   IN      Array whose members are to be reversed
* @see
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGNodeArray_reverse
(
MTG_NodeArrayHeader *        pArrayHeader
);

/**
* @param pArrayHeader   IN      Array whose members are to be sorted
* @param MTG_SortFunction  compare                  Comparison function
* @see
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGNodeArray_sort
(
MTG_NodeArrayHeader *        pArrayHeader,
MTG_SortFunction  compare
);

END_BENTLEY_GEOMETRY_NAMESPACE

