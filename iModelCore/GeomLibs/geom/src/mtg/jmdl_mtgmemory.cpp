/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include "mtgintrn.h"
BEGIN_BENTLEY_GEOMETRY_NAMESPACE
// EDL March 31 2009 -- EmbeddedIntArray substitution looks easy -- no conditionals.
END_BENTLEY_GEOMETRY_NAMESPACE
BEGIN_BENTLEY_GEOMETRY_NAMESPACE
/*----------------------------------------------------------------------+
|                                                                       |
|   Local defines                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
#define MTG_DELETED_NODEID (-2)     /* also defined in jmdl_mtgbase.cxx */

#ifdef CompileAppendLabels

/**
* Appends to each node of this instance the given number of labels with
* the given properties.
*
* @param        pGraph     <=> labels added
* @param        pMask        => array of properties of new labels
* @param        pDefault     => array of default values for new labels
* @param        pTag         => array of tags for new labels
* @param        numToAdd     => # labels to add
* @return false iff error
* @see #dropLabels
* @bsimethod                                    DavidAssaf      12/98
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlMTGGraph_appendLabels
(
MTGGraph        *pGraph,
const int       *pMask,
const int       *pDefault,
const int       *pTag,
int             numToAdd
)
    {
    EmbeddedStructArray *pNodeListHdr;      /* node list */
    EmbeddedIntArray            *pOldNodeListHdr;   /* copy of original node list */

    const int       *pReadPtr;      /* read ptr into original node list */
    int             *pWritePtr;     /* write ptr into node list */
    int             *pGraphTag;     /* array of MTG label tags */
    int             numNodes;       /* # nodes (active and free) */
    int             numInts;        /* # integers in new node list */
    int             numLabels;      /* # labels per new node */
    int             oldNumLabels;   /* # labels per original node */
    int             nodeSize;       /* # ints per new node */
    int             oldNodeSize;    /* # ints per orginal node */
    int             charsToAdd;     /* # chars added to label blocks */
    int             charsToCopy;    /* # chars in an old node */
    int             i, j;

    /* ptr parameter error traps */
    if  (!pGraph || !pMask || !pDefault || !pTag)
        return false;

    /* set MTG ptrs and values */
    numNodes        = jmdlMTGGraph_getNodeIdCount (pGraph);
    pGraphTag       = pGraph->labelTag;
    oldNodeSize     = pGraph->numIntPerNode;
    oldNumLabels    = pGraph->numLabelPerNode;
    nodeSize        = oldNodeSize + numToAdd;
    numLabels       = oldNumLabels + numToAdd;
    numInts         = nodeSize * numNodes;
    charsToAdd      = numToAdd * sizeof (int);
    charsToCopy     = oldNodeSize * sizeof (int);

    /* quantity error trap */
    if  (numToAdd < 0 || numLabels > MTG_MAX_LABEL_PER_NODE)
        return false;
    else if (numToAdd == 0)
        return true;

    /* tag error trap: don't want to duplicate tags */
    for (i = 0; i < oldNumLabels; i++)
        for (j = 0; j < numLabels; j++)
            if (pGraphTag[i] == pTag[j])
                return false;

    /*
    Create old node list, expand new node list, and get ptrs into node lists
    (reallocation of pNodes is done before the write ptr is defined, so this
    ptr will be valid the whole time).
    */
    pNodeListHdr = &pGraph->intArray_hdr;     /* EmbeddedStructArray w/ int blocking */
    pOldNodeListHdr = jmdlEmbeddedIntArray_grab ();
#if defined (INCLUDE_CRTDBG)
    omdlVArray_copy2 (&pOldNodeListHdr->vbArray, pNodeListHdr, __FILE__, __LINE__);
#else
    omdlVArray_copy (&pOldNodeListHdr->vbArray, pNodeListHdr);
#endif
    omdlVArray_setBufferSize (pNodeListHdr, numInts);
    pWritePtr = (int *) omdlVArray_getPtr (pNodeListHdr, 0);
    pReadPtr  = jmdlEmbeddedIntArray_getConstPtr (pOldNodeListHdr, 0);

    /* add labels to each node in new node list */
    for (
        i = 0;
        i < numNodes;
        i++, pReadPtr += oldNodeSize, pWritePtr += numToAdd
        )
        {
        /* copy oldNodeSize ints from original node list */
        memcpy (pWritePtr, pReadPtr, charsToCopy);

        /* append default values of new labels to node list */
        pWritePtr += oldNodeSize;
        memcpy (pWritePtr, pDefault, charsToAdd);
        }
    jmdlEmbeddedIntArray_drop (pOldNodeListHdr);

    /* new (larger) size (in ints) of node list */
    pNodeListHdr->count = numInts;

    /* set remaining relevant MTG fields/arrays */
    pGraph->numIntPerNode   = nodeSize;
    pGraph->numLabelPerNode = numLabels;
    memcpy (pGraph->labelMask + oldNumLabels,           pMask,      charsToAdd);
    memcpy (pGraph->defaultLabelValue + oldNumLabels,   pDefault,   charsToAdd);
    memcpy (pGraphTag + oldNumLabels,                   pTag,       charsToAdd);

    return true;
    }
#endif

/**
* Deletes the last numToDrop labels from each node of this instance.
*
* @param    pGraph      <=> labels dropped
* @param    numToDrop    => #labels to drop from end of each label block
* @return false iff error
* @see #appendLabels
* @bsimethod                                    DavidAssaf      1/99
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlMTGGraph_dropLabels
(
MTGGraph        *pGraph,
int             numToDrop
)
    {
    EmbeddedStructArray *pNodeListHdr;    /* node list */

    const int       *pReadPtr;      /* read ptr into node list */
    int             *pWritePtr;     /* write ptr into node list */
    int             numNodes;       /* # nodes (active and free) */
    int             numLabels;      /* # labels per new node */
    int             oldNumLabels;   /* # labels per original node */
    int             nodeSize;       /* # ints per new node */
    int             oldNodeSize;    /* # ints per orginal node */
    int             charsToDelete;  /* # chars deleted from label blocks */
    int             charsToCopy;    /* # chars in an old node */
    int             i;

    /* ptr parameter error trap */
    if (!pGraph)
        return false;

    /* set MTG ptrs and values */
    numNodes        = jmdlMTGGraph_getNodeIdCount (pGraph);
    oldNodeSize     = pGraph->numIntPerNode;
    oldNumLabels    = pGraph->numLabelPerNode;
    numLabels       = oldNumLabels - numToDrop;
    nodeSize        = oldNodeSize - numToDrop;
    charsToDelete   = numToDrop * sizeof (int);
    charsToCopy     = nodeSize * sizeof (int);

    /* quantity error trap */
    if (numToDrop < 0 || numLabels < 0)
        return false;
    else if (numToDrop == 0)
        return true;

    /*
    Get ptrs to rewrite new node list inline.  Ptrs valid the whole time
    because no new nodes are created (and hence the node list is not realloc'd).
    */
    pNodeListHdr = &pGraph->intArray_hdr;     /* EmbeddedStructArray w/ int blocking */
    pWritePtr = (int *) omdlVArray_getPtr (pNodeListHdr, 0);
    pReadPtr  = (int *) omdlVArray_getConstPtr (pNodeListHdr, 0);

    /*
    Delete labels from each node in the list, but leave list allocated to
    current size.
    */
    for (
        i = 0;
        i < numNodes;
        i++, pReadPtr += oldNodeSize, pWritePtr += nodeSize
        )
        memcpy (pWritePtr, pReadPtr, charsToCopy);

    /* new (smaller) size (in ints) of node list */
    pNodeListHdr->count = nodeSize * numNodes;

    /* set remaining relevant MTG fields/arrays */
    pGraph->numIntPerNode   = nodeSize;
    pGraph->numLabelPerNode = numLabels;
    memset (pGraph->labelMask + numLabels,          0, charsToDelete);
    memset (pGraph->defaultLabelValue + numLabels,  0, charsToDelete);
    memset (pGraph->labelTag + numLabels,           0, charsToDelete);

    return true;
    }


/**
* Compress this instance by deleting obsolete/unused nodes and by dropping the
* last numDroppedLabels labels of each remaining node.
* Warning: external references to or undropped labels that store MTG node IDs
* will no longer be valid.
*
* @param    pGraph              <=> on output: compressed graph
* @param    obsoleteNodeMask     => mask of obsoleted nodes
* @param    numLabelsToDrop      => #labels to drop from end of each label block
* @return false iff error
* @see #dropLabels
* @bsimethod                                    DavidAssaf      2/99
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlMTGGraph_compress
(
MTGGraph        *pGraph,
MTGMask         obsoleteNodeMask,
int             numLabelsToDrop
)
    {
    EmbeddedStructArray *pOldNodeListHdr;   /* old node list */
    EmbeddedStructArray *pNewNodeListHdr;   /* new node list */
    EmbeddedIntArray            *pIdMapHdr;         /* maps old nodeId to new nodeId */

    const int   *pReadPtr;          /* read ptr into old node list */
    const int   *pIdMapBase;        /* ptr to start of Id map */
    int         *pNode;             /* ptr to start of an MTG node */
    int         newNumNodes;        /* # non-obsolete MTG nodes */
    int         oldNumNodes;        /* # obs & non-obs MTG nodes */
    int         nodeSize;           /* # ints per new node */
    int         i, j;

    /* ptr parameter error trap */
    if (!pGraph)
        return false;

    /* trim labels (but not too many) */
    if (!jmdlMTGGraph_dropLabels (pGraph, numLabelsToDrop))
        return false;

    /* "free" all nodes masked as obsolete */
    if (obsoleteNodeMask)
        {
        MTGARRAY_SET_LOOP (nodeId, pGraph)
            {
            /* delete 2 obsolete edgemates (ignore if edge deleted already) */
            if (jmdlMTGGraph_getMask (pGraph, nodeId, obsoleteNodeMask))
                jmdlMTGGraph_dropEdge (pGraph, nodeId);
            }
        MTGARRAY_END_SET_LOOP (nodeId, pGraph)
        }

    /* get ptr to read old nodelist; it's always valid b/c list not reallocated */
    pOldNodeListHdr = &pGraph->intArray_hdr; /* EmbeddedStructArray w/ int blocking */
    pReadPtr = (int *) omdlVArray_getConstPtr (pOldNodeListHdr, 0);

    /* make new nodelist of exactly the right size */
    newNumNodes = pGraph->numActiveNodes;
    nodeSize = pGraph->numIntPerNode;
    pNewNodeListHdr = omdlVArray_new (sizeof (int));
    omdlVArray_setExactBufferSize (pNewNodeListHdr, newNumNodes * nodeSize);

    /* setup nodeId mapping */
    oldNumNodes = jmdlMTGGraph_getNodeIdCount (pGraph);
    pIdMapHdr = jmdlEmbeddedIntArray_grab ();
    jmdlVArrayInt_extend (pIdMapHdr, oldNumNodes);

    /*
    Copy non-"freed" nodes from old node list to new node list.
    Create mapping from old nodeId (i) to new nodeId (j).
    */
    for (
        i = j = 0;
        i < oldNumNodes;
        i++, pReadPtr += nodeSize
        )
        if (jmdlMTGGraph_isValidNodeId (pGraph, i))
            {
            omdlVArray_setArray (pNewNodeListHdr, (char *) pReadPtr, -1, nodeSize);
            jmdlVArrayInt_set (pIdMapHdr, j++, i);
            }

    /* strip graph of its original node list */
    omdlVArray_releaseMem (pOldNodeListHdr);

    /* remap vSucc and fSucc of each node in new node list to the new nodeIds */
    for (
        i = 0,
        pIdMapBase = jmdlEmbeddedIntArray_getConstPtr (pIdMapHdr, 0),
        pNode = (int *) omdlVArray_getPtr (pNewNodeListHdr, 0);
        i < newNumNodes;
        i++, pNode += nodeSize
        )
        {
        *pNode          = pIdMapBase[*pNode];       /* set vSucc */
        *(pNode + 1)    = pIdMapBase[*(pNode + 1)]; /* set fSucc */
        }
    jmdlEmbeddedIntArray_drop (pIdMapHdr);

    /* insert new node list into MTGGraph and set its fields */
    pGraph->intArray_hdr = *pNewNodeListHdr;
    pGraph->firstFreeNodeId = MTG_DELETED_NODEID;
    pGraph->numFreeNodes = 0;

    return true;
    }

END_BENTLEY_GEOMETRY_NAMESPACE
