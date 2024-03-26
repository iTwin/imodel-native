/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

/**
* Allocate a new header from the system heap.
* @return pointer to the header.
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public EmbeddedIntPtrPairArray *jmdlEmbeddedIntPtrPairArray_new
(
void
);

/**
* Initialize a given EmbeddedIntPtrPairArray header.
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public void jmdlEmbeddedIntPtrPairArray_init
(
EmbeddedIntPtrPairArray     *pHeader
);

/**
* Free both the header and is associated memory.
* @return always null.
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public EmbeddedIntPtrPairArray *jmdlEmbeddedIntPtrPairArray_free
(
EmbeddedIntPtrPairArray *pHeader
);

/**
* @param pHeader
* @see
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public void jmdlEmbeddedIntPtrPairArray_empty
(
EmbeddedIntPtrPairArray *pHeader
);

/**
* Release all memory attached to the header, and reinitialize the header
* as an empty array with no buffer.
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public void jmdlEmbeddedIntPtrPairArray_releaseMem
(
EmbeddedIntPtrPairArray *pHeader
);

/**
* Grab (borrow) an array from the cache.
* @return pointer to the borrowed header.
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public EmbeddedIntPtrPairArray *jmdlEmbeddedIntPtrPairArray_grab
(
void
);

/**
* Drop (return) an array to the cache.
* @return always returns null.
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public EmbeddedIntPtrPairArray *jmdlEmbeddedIntPtrPairArray_drop
(
EmbeddedIntPtrPairArray     *pHeader
);

/**
* Swap the contents (counts and associated memory) of two headers.
* @param pHeader0 IN OUT first array header
* @param pHeader1 IN OUT second array header
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public void     jmdlEmbeddedIntPtrPairArray_swapContents
(
EmbeddedIntPtrPairArray     *pHeader0,
EmbeddedIntPtrPairArray     *pHeader1
);

/**
* Ensure the buffer is at least a specified minimum size.
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public bool    jmdlEmbeddedIntPtrPairArray_ensureCapacity
(
EmbeddedIntPtrPairArray     *pHeader,
int                 n
);

/**
* Reallocate the buffer to accommodate exactly n pairs.
* NOTE: this will truncate the contents of this instance if its count is
* greater than n.
*
* @param    n       Number of pairs to accommodate, no more, no less.
* @return false if unable to reallocate the buffer.
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public bool    jmdlEmbeddedIntPtrPairArray_setExactBufferSize
(
EmbeddedIntPtrPairArray     *pHeader,
int                 n
);

/**
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public int jmdlEmbeddedIntPtrPairArray_getCount
(
const EmbeddedIntPtrPairArray *pHeader
);

/**
* Add an IntPtrPair to the array.
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public bool    jmdlEmbeddedIntPtrPairArray_addIntPtrPair
(
EmbeddedIntPtrPairArray       *pHeader,
const IntPtrPair                    *pPair
);

/**
* Insert at specified position, shifting others to higher positions as needed.
* @param pPair IN pair to insert.
* @param index IN index at which the pair is to appear in the array.
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public bool    jmdlEmbeddedIntPtrPairArray_insertIntPtrPair
(
        EmbeddedIntPtrPairArray         *pHeader,
const   IntPtrPair                *pPair,
        int                     index
);

/**
* @param pHeader IN OUT header of array receiveing pairs
* @param pPair IN array of pairs to add
* @param n IN number of pairs to add
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public bool     jmdlEmbeddedIntPtrPairArray_addIntPtrPairArray
(
      EmbeddedIntPtrPairArray       *pHeader,
const IntPtrPair              *pPair,
      int                   n
);

/**
* @param pHeader IN OUT header of array receiveing pairs
* @param pPair IN array of pairs to add
* @param index IN index location for adding the array
* @param n IN number of pairs to add
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public bool    jmdlEmbeddedIntPtrPairArray_insertIntPtrPairArray
(
      EmbeddedIntPtrPairArray       *pHeader,
const IntPtrPair              *pPair,
      int                   index,
      int                   n
);

/**
* Copy up to nreq pairs out of the array into a buffer.
* @param pBuffer OUT buffer of pairs.
* @nGot   nGot OUT number of pairs placed in buffer.
* @i0 IN index of first pair to access.
* @nreq IN number of pairs requested.
* @return true if at least one pair was returned.
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public bool    jmdlEmbeddedIntPtrPairArray_getIntPtrPairArray
(
const EmbeddedIntPtrPairArray   *pHeader,
IntPtrPair                *pBuffer,
int                     *nGot,
int                     i0,
int                     nreq
);

/**
* Get a pair from a specified index.
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public bool     jmdlEmbeddedIntPtrPairArray_getIntPtrPair
(
const EmbeddedIntPtrPairArray   *pHeader,
IntPtrPair                *pFpair,
int                     index
);

/**
* Drop a contiguous block of entries.  Copy higher indices back down.
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public bool     jmdlEmbeddedIntPtrPairArray_dropRange
(
EmbeddedIntPtrPairArray   *pHeader,
int                     index,
int                                         nDrop
);

/**
* Store a pair at specified index.
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public bool     jmdlEmbeddedIntPtrPairArray_setIntPtrPair
(
EmbeddedIntPtrPairArray   *pHeader,
IntPtrPair          *pFpair,
int               index
);

/**
* Add n uninitialized pairs to the array.
* @param pHeader IN OUT array from which to get block
* @param n IN number of entries requested
* @return Temporary pointer to block.
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public IntPtrPair *jmdlEmbeddedIntPtrPairArray_getBlock
(
EmbeddedIntPtrPairArray *pHeader,
int             n
);

/**
* @param pVertex Packed vertex array
* @param int            maxVertex vertex array limit
* @param pHeader master vertex array
* @param pIndex index array
* @param int            nIndex               number of vertices
* @return number of succesful dereferences.
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public int jmdlEmbeddedIntPtrPairArray_getIndexedIntPtrPairArray
(
const   EmbeddedIntPtrPairArray *pHeader,
        IntPtrPair        *pVertex,
        int             maxVertex,
        int             *pIndex,
        int             nIndex
);

/**
* Get a pointer to a position in the array.  This pointer may become invalid
* due to modifications of the array.
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public IntPtrPair* jmdlEmbeddedIntPtrPairArray_getPtr
(
EmbeddedIntPtrPairArray *pHeader,
      int        index
);

/**
* Get a pointer to a position in the array.  This pointer may become invalid
* due to modifications of the array.
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public const IntPtrPair* jmdlEmbeddedIntPtrPairArray_getConstPtr
(
const EmbeddedIntPtrPairArray *pHeader,
      int        index
);

/**
* @param pHeader
* @param index1
* @param index2
* @see
* @return SUCCESS if
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public bool     jmdlEmbeddedIntPtrPairArray_swapValues
(
EmbeddedIntPtrPairArray *pHeader,
int             index1,
int             index2
);

/**
* @param pDestHeader
* @param pSourceHeader
* @see
* @return SUCCESS if
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public bool     jmdlEmbeddedIntPtrPairArray_copy
(
EmbeddedIntPtrPairArray *pDestHeader,
const EmbeddedIntPtrPairArray *pSourceHeader
);

/**
* @param pDestHeader
* @param VBArray_SortFunction pFunction
* @see
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public void jmdlEmbeddedIntPtrPairArray_sort
(
EmbeddedIntPtrPairArray *pDestHeader,
VBArray_SortFunction pFunction
);

/**
* Add to the array.
* @param iData IN integer part to insert.
* @param pData IN pointer part to insert.
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public bool    jmdlEmbeddedIntPtrPairArray_addIntPtr
(
EmbeddedIntPtrPairArray       *pHeader,
int                             iData,
void                            *pData
);

/**
* Insert at specified position, shifting others to higher positions as needed.
* @param iData IN integer part to insert.
* @param pData IN pointer part to insert.
* @param index IN index at which the pair is to appear in the array.
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public bool    jmdlEmbeddedIntPtrPairArray_insertIntPtr
(
EmbeddedIntPtrPairArray     *pHeader,
int                         iData,
void                        *pData,
int                         index
);

/**
* Find the (first) index at iDatg is matched.
* @param pIndex IN index where iData is found.
* @param ppData OUT pointer value
* @param iData IN integer part to find
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public bool    jmdlEmbeddedIntPtrPairArray_searchForInt
(
const   EmbeddedIntPtrPairArray         *pHeader,
        int                     *pIndex,
        void                    **ppData,
        int                     iData
);

