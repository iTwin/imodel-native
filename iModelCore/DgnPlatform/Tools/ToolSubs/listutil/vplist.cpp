/*--------------------------------------------------------------------------------------+
|
|     $Source: Tools/ToolSubs/listutil/vplist.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include    <stdlib.h>
#include    <string.h>

#include    <RmgrTools/Tools/sort.h>
#include    <DgnPlatform/Tools/vplist.h>

/*----------------------------------------------------------------------+
|                                                                       |
|   Local defines                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
#if !defined (MAX)
#   define MAX(a,b)     ((a)>(b)? (a) : (b))
#   define MIN(a,b)     ((a)<(b)? (a) : (b))
#endif

#define GROW_PROPORTIONATELY
#define VPLIST_GROWTH_FRACTION  3   /* this means that the list grows by 1/3 of its current size every time  */
#define VPLIST_MIN_GROWTH_UNIT  5   /* this means that we'll always grow by at least 5 entries */

USING_NAMESPACE_BENTLEY

/*----------------------------------------------------------------------+
|                                                                       |
| name          vpList_define
|                                                                       |
| author        SWW                                      4/92           |
|                                                                       |
+----------------------------------------------------------------------*/
Public  VPList  *vpList_define
(                               /* <=  pList */
VPList          *pList,         /* <= */
int              nInit          /* => */
)
    {
    BeAssert (pList && 0<=nInit);

    /*  optional:  allocate initial chunk of memory */
    if ((pList->nAllocated = nInit) > 0)
        {
        pList->p = (void **) malloc (pList->nAllocated * sizeof (void *));

        if (NULL == pList->p)       /* out of memory error => 0-length list */
            pList->nAllocated = 0;
        }
    else
        {
        pList->p = 0;
        }

    /* list has no members initially */
    pList->count        = 0;

    return      pList;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          vpList_new
|                                                                       |
| author        SWW                                      4/92           |
|                                                                       |
+----------------------------------------------------------------------*/
Public  VPList  *vpList_new
(                               /* <=  new VPList */
void
)
    {
    return vpList_define (static_cast<VPList*>(malloc (sizeof (VPList))), 0);
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          vpList_n                                                |
|                                                                       |
| author        SamWilson                               02/95           |
|                                                                       |
+----------------------------------------------------------------------*/
Public int      vpList_n
(
const VPList    *p
)
    {
    if (p == NULL || p->p == NULL)
        return 0;

    return p->count;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          vpList_newCopy                                          |
|                                                                       |
| author        SamWilson                               11/92           |
|                                                                       |
+----------------------------------------------------------------------*/
Public VPList   *vpList_newCopy
(                               /* <=  new list (cc of source) */
VPList          *source         /*  => list to copy */
)
    {
    VPList              *pList;

    pList = vpList_define (static_cast<VPList*>(malloc (sizeof (VPList))), source->nAllocated);

    return      vpList_copy (pList, source);
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          vpList_growBy -- make the list longer
|                                                                       |
| author        SWW                                      4/92           |
|                                                                       |
+----------------------------------------------------------------------*/
Public  int     vpList_growBy
(                               /* <= ERROR if not possible */
VPList  *pList,                 /*  => the list */
int     xlen                    /*  => how much to add (at least) */
)
    {
    int     newlen;
    void    **pp;

    BeAssert (xlen > 0);
    if (xlen <= 0)
        return  ERROR;

    /*-------------------------------------------------------------------
        Decide how long the array has to become
    -------------------------------------------------------------------*/
#   if defined (GROW_PROPORTIONATELY)
        {
        int   added;
        int   necessary = pList->nAllocated + xlen;

        for (newlen = pList->nAllocated;  newlen < necessary; )
            {
            added = (newlen / VPLIST_GROWTH_FRACTION);

            if (added < VPLIST_MIN_GROWTH_UNIT)
                added = VPLIST_MIN_GROWTH_UNIT;

            newlen += added;
            }
        }
#   else
        {
        int     nChunks;

        nChunks = (xlen + (pList->nChunk-1)) / pList->nChunk;

        nChunks = MAX (nChunks, 1);

        newlen = pList->nAllocated + (nChunks * pList->nChunk);
        }
#   endif

    /*-------------------------------------------------------------------
        realloc to new size
    -------------------------------------------------------------------*/
    pp = static_cast<void**>(realloc (pList->p, newlen * sizeof (void*)));

    if (!pp)
        {
#ifdef NOISY_ERRORS
        printf ("***** ERROR vpList_growBy: cannot grow to %d x 4\n", newlen);
#endif
        return ERROR;
        }

    /*  update if successful */
    pList->p    = pp;
    pList->nAllocated  = newlen;

    return SUCCESS;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          vpList_destroy -- free private storage used in VPList   |
|                                                                       |
| author        SWW                                      4/92           |
|                                                                       |
+----------------------------------------------------------------------*/
Public  void    vpList_destroy
(
VPList  *pList                  /* <=> */
)
    {
    BeAssert (pList);
    if (pList->p)
        {
        free (pList->p);
        pList->p  = 0;
        }
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          vpList_free -- free the VPList's conents && the VPList itself |
|                                                                       |
| author        SWW                                      4/92           |
|                                                                       |
+----------------------------------------------------------------------*/
Public  void    *vpList_free
(                               /* <=  NULL */
VPList  *pList                  /* <=> */
)
    {
    vpList_destroy (pList);
    free (pList);
    return NULL;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          vpList_empty -- empty out the list -- contain 0 items   |
|                                                                       |
| author        SWW                                      4/92           |
|                                                                       |
+----------------------------------------------------------------------*/
Public  VPList  *vpList_empty
(
VPList  *pList
)
    {
    pList->count = 0;
    return pList;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          vpList_reverse -- reverse the order of the items on list        |
|                                                                       |
| author        SamWilson                               1/93            |
|                                                                       |
+----------------------------------------------------------------------*/
Public VPList   *vpList_reverse
(
VPList          *l
)
    {
    int i, n=l->count, mid=n/2-1, last=n-1;

    for (i=0; i<=mid; ++i)
        {
        void *tmp = l->p[i];
        l->p[i] = l->p[last-i];
        l->p[last-i] = tmp;
        }

    return l;
    }


/*======================================================================+
|                                                                       |
|   unit item access functions -- add/drop/pop/dequeue/get/find         |
|                                                                       |
+======================================================================*/
/*----------------------------------------------------------------------+
|                                                                       |
| name          vpList_put -- replace (or add) item i                   |
|                                                                       |
| author        SamWilson                               5/93            |
|                                                                       |
+----------------------------------------------------------------------*/
Public void     vpList_put
(
VPList          *pList,         /* <=> */
void            *dataPtr,       /*  => */
int             i               /*  => index (or -1 for end) */
)
    {
    if (i < 0)
        {
        if (pList->count)
            i = pList->count - 1;
        else
            i = 0;
        }

    if (i >= pList->count)
        {
        if (i >= pList->nAllocated)
            if (vpList_growBy (pList, (i + 1) - pList->nAllocated) == ERROR)
                return;

        memset (&pList->p[pList->count], 0, (pList->nAllocated - pList->count) * sizeof (void*));

        pList->count = i + 1;
        }

    BeAssert (i < pList->count);
    BeAssert (pList->count <= pList->nAllocated);

    pList->p[i] = dataPtr;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          vpList_add -- add item to end of list                   |
|               (aka vpList_push)                                       |
|                                                                       |
| author        SWW                                      3/92           |
|                                                                       |
+----------------------------------------------------------------------*/
Public int      vpList_add
(                               /*<=  index at which object was added */
VPList          *pList,         /* => lst */
void            *dataPtr        /* => ->data */
)
    {
    if (pList->count+1 > pList->nAllocated)
        if (vpList_growBy (pList, 1) != SUCCESS)
            return -1;

    BeAssert (pList->count <= pList->nAllocated);

    pList->p[pList->count++] = dataPtr;

    return pList->count-1;
    }


/*----------------------------------------------------------------------+
|                                                                       |
| name          vpList_find -- find an item in an lst by data pointer value
|                                                                       |
| author        SWW                                      4/92           |
|                                                                       |
+----------------------------------------------------------------------*/
Public  int     vpList_find
(                               /* <= found index (pList->count if not found) */
const VPList    *pList,         /* => VPList */
const void      *dataPtr        /* => dataPtr of node to find */
)
    {
    void        **p, **end;
                                BeAssert (pList);

    p = pList->p;
    end = p + pList->count;

    for (; p<end; p++)
        {
        if (*p == dataPtr)
            return  static_cast<int>(p - pList->p);
        }

    return  pList->count;
    }


/*----------------------------------------------------------------------+
|                                                                       |
| name          vpList_addUnique -- add if not already in list
|                                                                       |
| author        SWW                                      3/92           |
|                                                                       |
+----------------------------------------------------------------------*/
Public int      vpList_addUnique
(                               /*<=  dataPtr or NULL if error */
VPList          *pList,         /* => lst */
void            *dataPtr        /* => ->data */
)
    {
    int index;

    if ((index = vpList_find (pList, dataPtr)) < pList->count)
        return index;

    return vpList_add (pList, dataPtr);
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          vpList_dropItems

        drop n items from list, starting from index i.

        (may be called from inside vpList_1/vpList_next iterator)

        Req: i and n must be within the bounds of the list.
|                                                                       |
| author        SamWilson                               5/93            |
|                                                                       |
+----------------------------------------------------------------------*/
Public void     vpList_dropItems
(                               /* <=  dropped item */
VPList          *pList,         /* <=> */
int             i,              /*  => index of first item to drop (or -1 for end) */
int             nItems          /*  => # items to drop (including i) */
)
    {
    int     nleft;

    if (nItems <= 0)
        return;

    if (i==-1)
        i = pList->count ? pList->count-1: 0;

    if (i<0 || pList->count<=i)
        {
        return;
        }

    if (nItems<0 || nItems > pList->count-i)
        nItems = pList->count - i;

    nleft = pList->count-i-nItems;
    if (nleft > 0)
        memmove (pList->p+i, pList->p+i+nItems, nleft * sizeof(pList->p));

    pList->count -= nItems;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          vpList_drop -- find and drop the specified item from the list
|                                                                       |
| author        SWW                                      4/92           |
|                                                                       |
+----------------------------------------------------------------------*/
Public StatusInt    vpList_drop
(
VPList          *pList,         /* => VPList */
const void      *dataPtr        /* => dataPtr of node to drop */
)
    {
    void        **p, **end;
                                BeAssert (pList);

    p = pList->p;
    end = p + pList->count;

    for (; p<end; p++)
        {
        if (*p == dataPtr)
            {
            if (p < (end-1))
                {
                memmove (p, p+1, (end-(p+1)) * sizeof (void *));
                }

            pList->count--;
            return  SUCCESS;
            }
        }

    return  ERROR;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          vpList_get -- return data pointer stored at index i     |
|                                                                       |
| author        SWW                                      4/92           |
|                                                                       |
+----------------------------------------------------------------------*/
Public void     *vpList_get
(
const VPList    *pList,         /* =>                       */
int             i               /* => index (or -1 for end) */
)
    {                           BeAssert (pList);
    if (i < 0)
        i = pList->count ? pList->count-1 : 0;
    if (pList->p == NULL)
        return NULL;

    return (0 <= i && i < pList->count) ? pList->p[i] : NULL;
    }


/*----------------------------------------------------------------------+
|                                                                       |
| name          vpList_first
|                                                                       |
| author        SamWilson                                6/92           |
|                                                                       |
+----------------------------------------------------------------------*/
Public  void    *vpList_first
(
const VPList    *pList
)
    {
    return vpList_get (pList, 0);
    }


/*----------------------------------------------------------------------+
|                                                                       |
| name          vpList_isFound -- is item on list?
|                                                                       |
| author        SamWilson                                4/92           |
|                                                                       |
+----------------------------------------------------------------------*/
Public  bool    vpList_isFound
(                               /*<=  true if dataPtr is on the list */
const VPList    *pList,         /* => list to search */
const void      *dataPtr        /* => what to look for */
)
    {
    return vpList_find (pList, dataPtr) < pList->count;
    }


/*======================================================================+
|                                                                       |
|   lst-lst operations                                                  |
|                                                                       |
+======================================================================*/

/*----------------------------------------------------------------------+
|                                                                       |
| name          vpList_copy                                             |
|                                                                       |
| author        SamWilson                               11/92           |
|                                                                       |
+----------------------------------------------------------------------*/
Public VPList   *vpList_copy
(
VPList          *target,        /* <=  */
const VPList    *source         /*  => */
)
    {
    if (source->count)          /* (must check, to avoid use of 0-pointers) */
        {
        if (target->nAllocated < source->count)
            if (vpList_growBy (target, source->count - target->nAllocated) != SUCCESS)
                return NULL;

        memcpy (target->p, source->p, (target->count = source->count) * sizeof (void *));
        }

    return      target;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          vpList_dropList -- pList SetDifference lst2
|                                                                       |
| author        SamWilson                               1/93            |
|                                                                       |
+----------------------------------------------------------------------*/
Public VPList   *vpList_dropList
(                               /* <=   pList */
VPList          *pList,         /* <=>  (items dropped) */
const VPList    *lst2           /*  =>  items to drop */
)
    {
    int         i;
    for (i=0; i<lst2->count; ++i)
        vpList_drop (pList, lst2->p[i]);

    return  pList;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          vpList_appendListUnique -- target SetUnion source               |
|                                                                       |
| author        SamWilson                               11/92           |
|                                                                       |
+----------------------------------------------------------------------*/
Public VPList   *vpList_appendListUnique
(                               /* <=  target w/ (new items from) source appended */
VPList          *target,        /* <=  */
const VPList    *source         /*  => */
)
    {
    int         i;

    for (i=0; i<source->count; ++i)
        vpList_addUnique (target, source->p[i]);

    return      target;
    }


