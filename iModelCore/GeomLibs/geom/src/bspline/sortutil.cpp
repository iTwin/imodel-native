/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/bspline/sortutil.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include "sortutil.h"
BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/* tools subsystem include files */
#define ALLOCATE_LOCAL_BUFFER(_size) _alloca(_size)
//#define FREE_LOCAL_BUFFER(_addr)
void FREE_LOCAL_BUFFER(void *addr) {}
/*----------------------------------------------------------------------+
|                                                                       |
|   External variables                                                  |
|                                                                       |
+----------------------------------------------------------------------*/
/*----------------------------------------------------------------------+
|                                                                       |
|   Local definitions                                                   |
|                                                                       |
+----------------------------------------------------------------------*/
#define MAXITEMSIZE 100
    
/*----------------------------------------------------------------------+
|                                                                       |
|   Local statics                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
/*-----------------------------------------------------------------------
    These three temp buffer pointers (below) used to be three separate
    automatic variables that ate up stack space in the 
    mdlUtil_dlmQuickSort (recursive), util_insertSort and util_exchange
    functions. The stack temp buffers have been replaced by a passable
    structure and two static temp buffer since their contents didn't 
    need to be preserved down thru the recursive calls.
    
    The buffer pointer assignments are done in mdlUtil_dlmQuickSort.
    If the static "staticSwapBuf" is large enough, it will be
    used, otherwise the buffer space is malloc'd.

    
    By passing a single pointer around, we avoid multi-threading issues
    and also reduce stack usage.  Also, we can't use static buffers
    in a thread-safe world, so I shove them in the structure.  Since
    the structure is allocated as a chunk, it is still better to use
    the "static" buffers to hold the data.  If not, additional buffers
    are allocated.
    
    The size, comparison, and optional argument pieces were added to
    the structure because it was convenient to pass a single pointer
    around.    Chuck 5/99
-----------------------------------------------------------------------*/
typedef struct bufstruct
    {
    double      staticSwapBufA [(MAXITEMSIZE+7)/8];
    double      staticSwapBufB [(MAXITEMSIZE+7)/8];
    void        *tempBuf1P;
    void        *tempBuf2P;
    void        *tempBuf3P;

    int     elementsize;                /* => size of individiual array to sort */
    int     (*comparefunc)();           /* => comparison function */
    void    *optArg;                    /* => optional argument passed to comparefunc */
    } SortUtilData;

/*ff MajorPublic Code Section */
/*----------------------------------------------------------------------+
|                                                                       |
|   MajorPublic Code Section                                           |
|                                                                       |
+----------------------------------------------------------------------*/
/*----------------------------------------------------------------------+
|                                                                       |
| name          util_exchange                                           |
|                                                                       |
| author        BJB                                     6/89            |
|                                                                       |
+----------------------------------------------------------------------*/
static void util_exchange
(    
char    *item1,
char    *item2,
int     itemsize,
void    *tempBufP
)    
    {
    memcpy (tempBufP,  item1,     itemsize);
    memcpy (item1,     item2,     itemsize);
    memcpy (item2,     tempBufP,  itemsize);
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          util_iSort (formerly util_insertSort)                   |
|                                                                       |
| description   sorts array by insertion. Translated                    |
|               and modified from Writing Efficient Programs, by        |
|               Jon Louis Bentley, page 64.                             |
|                                                                       |
| author        BJB                                     1/85            |
|                                                                       |
| Modified to take buffer argument to support multiple threads          |
|                                                                       |
+----------------------------------------------------------------------*/
static void util_iSort
(    
void    *pfirstVoidP,
void    *plastVoidP,
int     elementsize,                /* => size of individiual array to sort */
PFToolsSortCompare comparefunc,     /* => comparison function */
void    *optArg,                    /* => optional argument */
void    *tempBufP
)
    {
    char    *pfirst, *plast, *pi, *pj;

    pfirst = (char *) pfirstVoidP;
    plast  = (char *) plastVoidP;
    for (pi = pfirst + elementsize; pi <= plast; pi += elementsize)
        {
        memcpy (tempBufP, pi, elementsize);
        pj = pi;
        while (pj > pfirst && 
              ((*comparefunc)(tempBufP, (pj-elementsize), optArg) < 0))
            {
            memcpy (pj, pj-elementsize, elementsize);
            pj -= elementsize;
            }
        memcpy (pj, tempBufP, elementsize);
        }

    }
    
/*----------------------------------------------------------------------+
|                                                                       |
| name          util_insertSort                                         |
|                                                                       |
| description   sorts array by insertion. Translated                    |
|               and modified from Writing Efficient Programs, by        |
|               Jon Louis Bentley, page 64.                             |
|                                                                       |
|   Modified to pass structure for thread safety.  Chuck 5/99           |
|                                                                       |
| author        BJB                                     1/85            |
|                                                                       |
+----------------------------------------------------------------------*/
Public void util_insertSort
(    
void    *pfirstVoidP,               /* => first element in array of structures */
void    *plastVoidP,                /* => last element in array to sort */
int     elementsize,                /* => size of individiual array to sort */
PFToolsSortCompare comparefunc,     /* => comparison function */
void    *optArg                     /* => optional argument */
)    
    {
    void    *tempBufP = NULL;
    /*-------------------------------------------------------------------
        This allocation is here for callers of util_insertSort other than
        mdlUtil_dlmQuickSort (which now calls util_iSort directly and 
        bypasses this function). At this point there is only one in 
        windutil.c.
    -------------------------------------------------------------------*/
    if ((tempBufP = (void *) ALLOCATE_LOCAL_BUFFER (elementsize))== NULL)
        return;

    util_iSort (pfirstVoidP, plastVoidP, elementsize, comparefunc, optArg, tempBufP);
    
    FREE_LOCAL_BUFFER (tempBufP);
    }
    

/*----------------------------------------------------------------------+
|                                                                       |
| Name:     util_qsort (formerly mdlUtil_dlmQuickSort)                  |
|                                                                       |
| description   uses quiksort algorithm to sort array. Translated       |
|               and modified from Writing Efficient Programs, by        |
|               Jon Louis Bentley, page 117.                            |
|                                                                       |
| author        BJB                                     1/85            |
|                                                                       |
|                                                                       |
|    Modified to take different arguments for thread safety             |
| Author:   ChuckKirschman                              06/99           |
|                                                                       |
+----------------------------------------------------------------------*/
static void util_qsort
(
void         *pfirstVoidP,          /* => first element in array of structures */
void         *plastVoidP,           /* => last element in array to sort */
SortUtilData *sudP,                 /* => working buffers, etc */
int           depth                 /* => how deeply nested we are */
)
    {
    char    *pfirst, *plast, *pi, *pj, *ptemp;
    int      topLevel;
    //void    *tempBuf1P = NULL;
    //void    *tempBuf2P = NULL;

    topLevel = (sudP->tempBuf1P == NULL) ? true : false;

    if (topLevel)
        {
        if (sudP->elementsize <= MAXITEMSIZE)
            {
            sudP->tempBuf1P = (void *) sudP->staticSwapBufA;
            sudP->tempBuf2P = (void *) sudP->staticSwapBufB;
            }
        else
            {
            if ((sudP->tempBuf1P = (void *) ALLOCATE_LOCAL_BUFFER (sudP->elementsize))
                == NULL)
                return;

            if ((sudP->tempBuf2P = (void *) ALLOCATE_LOCAL_BUFFER (sudP->elementsize))
                == NULL)
                {
                FREE_LOCAL_BUFFER (sudP->tempBuf1P);
                return;
                }
            }

        /*------------------------------------------------
            Assumption: tempBuf2 and tempBuf3 can reuse
                        the same buffer space.
        -------------------------------------------------*/
        sudP->tempBuf3P = sudP->tempBuf2P;
        }

    pfirst = (char *) pfirstVoidP;
    plast  = (char *) plastVoidP;

    //  If there are fewer than 15 elements, insertion sort is faster
    //  If we have recursed more than 32 times, we may be in a degenerate case 
    //      => fall back on insertion sort to limit the worst case
    if (plast < (pfirst + 15 * sudP->elementsize) || depth > 32)
        util_iSort (pfirst, plast, sudP->elementsize, (PFToolsSortCompare)sudP->comparefunc, sudP->optArg, sudP->tempBuf3P);
    else
        {
        /* Get halfway through array */
        /* note: this looks like I have elementsize in both numerator and
                denominator, but it must be that way to make sure pi lines
                up with an element */
        //assert(plast >= pfirst);
        pi = pfirst + sudP->elementsize * (((Byte*) plast - (Byte*) pfirst) / 
                                        (2 * sudP->elementsize));

        /* put it into highest element in range */
        memcpy (sudP->tempBuf1P, pi, sudP->elementsize);
        memcpy (pi, plast, sudP->elementsize);
        memcpy (plast, sudP->tempBuf1P, sudP->elementsize);
        
        /* partition around element saved in tempBuf1P */
        for (pi = pfirst, pj = plast; pi <= pj;  )
            {
            while ( (pj >= pfirst) && ((*((PFToolsSortCompare)sudP->comparefunc)) (pj, sudP->tempBuf1P, sudP->optArg) >= 0))
                    pj -= sudP->elementsize;
            while ( ((*((PFToolsSortCompare)sudP->comparefunc)) (pi, sudP->tempBuf1P, sudP->optArg) < 0) && (pi < plast) )
                    pi += sudP->elementsize;
            if (pi < pj)
                {
                util_exchange (pi, pj, sudP->elementsize, sudP->tempBuf3P);
                pi += sudP->elementsize;
                pj -= sudP->elementsize;
                }
            }
            
        /* special case if all elements > tempBuf1P element */
        if (pi == pfirst)
            {
            util_exchange (pfirst, plast, sudP->elementsize, sudP->tempBuf3P);
            pi += sudP->elementsize;
            pj += sudP->elementsize;
            }
            
        /* Recursively sort subarrays */
        ptemp = pi - sudP->elementsize;
        if (pfirst < ptemp) 
            util_qsort (pfirst, ptemp, sudP, depth+1);
        ptemp = pj + sudP->elementsize;
        if (plast > ptemp) 
            util_qsort (ptemp, plast, sudP, depth+1);
        }

    if (topLevel)
        {
        if (sudP->tempBuf1P != (void *) sudP->staticSwapBufA)
            FREE_LOCAL_BUFFER (sudP->tempBuf1P);
        if (sudP->tempBuf2P != (void *) sudP->staticSwapBufB)
            FREE_LOCAL_BUFFER (sudP->tempBuf2P);

        sudP->tempBuf1P = sudP->tempBuf2P = sudP->tempBuf3P = NULL;
        }
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          mdlUtil_dlmQuickSort                                    |
|                                                                       |
| description   uses quiksort algorithm to sort array. Translated       |
|               and modified from Writing Efficient Programs, by        |
|               Jon Louis Bentley, page 117.                            |
|                                                                       |
|   Modified to pass structure for thread safety.  Chuck 5/99           |
|                                                                       |
| author        BJB                                     1/85            |
|                                                                       |
+----------------------------------------------------------------------*/
Public void mdlUtil_dlmQuickSort
(    
void                *pfirstVoidP,   /* => first element in array of structures */
void                *plastVoidP,    /* => last element in array to sort */
int                  elementsize,   /* => size of individiual array to sort */
PFToolsSortCompare   comparefunc,   /* => comparison function */
void                *optArg         /* => optional argument passed to comparefunc */
)    

    {
    SortUtilData *sudP;
    
    /* Create pass structure */
    sudP = (SortUtilData *) ALLOCATE_LOCAL_BUFFER (sizeof(SortUtilData));
    if (NULL == sudP)
        {
        return;
        }
    memset (sudP, 0, sizeof(SortUtilData));
    
    sudP->elementsize = elementsize;
    sudP->comparefunc = (int     (*)())comparefunc;
    sudP->optArg      = optArg;
        
    /* Call quicksort with the new interface */
    util_qsort (pfirstVoidP, plastVoidP, sudP, 0);

    /* Cleanup */
    FREE_LOCAL_BUFFER (sudP);
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          util_ascendLongs                                        |
|                                                                       |
| author        RBB                                     2/90            |
|                                                                       |
+----------------------------------------------------------------------*/
static int     util_ascendLongs
(    
const long *num1,           /* => longs to compare */
const long *num2,
const void *optArgs
)    
    {
    return (*num1 - *num2);
    }


/*----------------------------------------------------------------------+
|                                                                       |
| name          util_descendLongs                                       |
|                                                                       |
| author        RBB                                     2/90            |
|                                                                       |
+----------------------------------------------------------------------*/
static int     util_descendLongs
(    
const long *num1,           /* => longs to compare */
const long *num2,
const void *optArgs
)    
    {
    return (*num2 - *num1);
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          mdlUtil_sortLongs                                               |
|                                                                       |
| author        RBB                                     2/90            |
|                                                                       |
+----------------------------------------------------------------------*/
Public void mdlUtil_sortLongs
(    
long *longs,                /* <=> array of longs to be sorted (in place) */
int   numLongs,             /* => number of longs to sort */
int   ascend                /* => true for ascending order */
)    
    {
    
    mdlUtil_dlmQuickSort (longs, longs+numLongs-1, sizeof(long), 
                        ascend ? (PFToolsSortCompare)util_ascendLongs:(PFToolsSortCompare)util_descendLongs, NULL);
    }


/*----------------------------------------------------------------------+
|                                                                       |
| name          util_ascendStrings                                      |
|                                                                       |
| author        JBG                                     11/90           |
|                                                                       |
+----------------------------------------------------------------------*/
static int     util_ascendStrings
(    
const char **stringPtr1,        /* => strings to compare */
const char **stringPtr2,
const void  *optArgs
)    
    {
    return -strcmp (*stringPtr1, *stringPtr2);
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          util_descendStrings                                     |
|                                                                       |
| author        JBG                                     11/90           |
|                                                                       |
+----------------------------------------------------------------------*/
static int     util_descendStrings
(    
const char **stringPtr1,        /* => strings to compare */
const char **stringPtr2,
const void  *optArgs
)    
    {
    return strcmp (*stringPtr1, *stringPtr2);
    }


/*----------------------------------------------------------------------+
|                                                                       |
| name          mdlUtil_sortStrings                                     |
|                                                                       |
| author        JBG                                     11/90           |
|                                                                       |
+----------------------------------------------------------------------*/
Public void mdlUtil_sortStrings
(    
char **stringPtrsP,         /* <=> support pointers by value of strings. (in place) */
int  numStrings,            /* => number of strings to sort */
int ascend                  /* => true for ascending order */
)    
    {
    mdlUtil_dlmQuickSort (stringPtrsP, stringPtrsP+numStrings-1, sizeof(char **), 
                        ascend ? (PFToolsSortCompare)util_ascendStrings:(PFToolsSortCompare)util_descendStrings, NULL);
    }

#ifdef CompileAll
/*----------------------------------------------------------------------+
|                                                                       |
| name          util_ascendDoubles                                      |
|                                                                       |
| author        RBB                                     2/90            |
|                                                                       |
+----------------------------------------------------------------------*/
static int     util_ascendDoubles
(    
const double *num1,         /* => Doubles to compare */
const double *num2,
const void   *optArgs
)    
    {
    if (*num1 < *num2)
        return (-1);
    else if (*num1 > *num2)
        return (1);
    else
        return (0);
    }


/*----------------------------------------------------------------------+
|                                                                       |
| name          util_descendDoubles                                     |
|                                                                       |
| author        RBB                                     2/90            |
|                                                                       |
+----------------------------------------------------------------------*/
static int     util_descendDoubles
(    
const double    *num1,      /* => Doubles to compare */
const double    *num2,
const void    *optArgs
)    
    {
    if (*num1 > *num2)
        return (-1);
    else if (*num1 < *num2)
        return (1);
    else
        return (0);
    }
#endif


/*----------------------------------------------------------------------+
|                                                                       |
| name          util_compareLongs                                       |
|                                                                       |
| author        RBB                                     5/90            |
|                                                                       |
+----------------------------------------------------------------------*/
Public int util_compareLongs
(    
long    y1, 
long    y2
)    
    {
    if (y1==y2)
        return (0);
    else if (y1<y2)
        return (1);
    else
        return(-1);
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          util_compareDoubles                                     |
|                                                                       |
| author        RBB                                     5/90            |
|                                                                       |
+----------------------------------------------------------------------*/
Public int util_compareDoubles
(    
double  y1,
double  y2
)
    {
    if (y1 == y2)
        return (0);
    else if (y1<y2)
        return (1);
    else
        return(-1);
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          util_tagSortCompare                                     |
|                                                                       |
| author        RayBentley                               10/93          |
|                                                                       |
+----------------------------------------------------------------------*/
static int util_tagSortCompare
(
const int       *index1,
const int       *index2,
const double    *values
)                           
    {
    double  value1 = values[*index1], value2 = values[*index2];
    
    if (value1 < value2)
        return (-1);
    else if (value1 > value2)
        return (1);
    else
        return (0);
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name      util_tagSort                                                |
|                                                                       |
| author    BFP                                         5/90            |
|                                                                       |
+----------------------------------------------------------------------*/
Public int util_tagSort
(    
int     *tags,
double  *values,
int     numValues
)    
    {
    int i;
        
    if (numValues == 0)
        {
        return (ERROR);
        }
    else if (numValues == 1)
        {
        tags[0] = 0;
        return (SUCCESS);
        }
    else
        {
        for (i=0; i < numValues; i++) 
            tags[i] = i;
            
        mdlUtil_dlmQuickSort (tags, tags + numValues - 1, sizeof(int), 
                        (PFToolsSortCompare)util_tagSortCompare, values);
        }
    return SUCCESS;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          util_compareLongPtr                                     |
|                                                                       |
| author        PaulChater                              04/99           |
|                                                                       |
+----------------------------------------------------------------------*/
static int     util_compareLongPtr
(
const void    *y1,
const void    *y2
)
    {
    if (*(long const*)y1==*(long const*)y2)
        return (0);
    else if (*(long const*)y1>*(long const*)y2)
        return (1);
    else
        return(-1);
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          util_bsearch                                            |
|                                                                       |
| author        PaulChater                              04/99           |
|                                                                       |
+----------------------------------------------------------------------*/
Public void     *util_bsearch
(
const void    *pArray,
int           numlongs,
int           t_size,
int           iValue
)
    {
    return  bsearch ((const void*)&iValue, pArray, numlongs, t_size, util_compareLongPtr);
    }


/*----------------------------------------------------------------------+
|                                                                       |
| name          util_bsearch                                            |
|                                                                       |
| author        PaulChater                              04/99           |
|                                                                       |
+----------------------------------------------------------------------*/
Public int      util_bsearchLongs
(
const void    *pArray,
int           numlongs,
int           iValue
)
    {
    long    *p = (long*)bsearch ((const void*)&iValue, pArray, numlongs, sizeof (long), util_compareLongPtr);

    if (p == NULL)
        return  -1;
    return  (int) (p - (long const*)pArray);
    }

#if defined (testsort)
#undef printf               
/*----------------------------------------------------------------------+
|                                                                       |
| name          orderints                                               |
|                                                                       |
+----------------------------------------------------------------------*/
static int     orderints
(    
int *num1,          /* => ints to compare */
int *num2
)    
    {
    return (*num1 - *num2);
    }

/*----------------------------------------------------------------------+
|                                                                       |
|    Program to test sorting algorithm                                  |
|                                                                       |
+----------------------------------------------------------------------*/
main ()
    {
    static int x[2000];
    static int x1[2000];
    char inbuf[80];
    int i, j, numsort=2000, niter, iterations=1, see=0;
    int *pd;
    int *ps;

top:
    printf ("\nNumber of random numbers : ");
    fgets (inbuf, sizeof inbuf, stdin);
    if (*inbuf != '\0')
        numsort = (int)strtol (inbuf, (char **)NULL, 10);
    if (numsort > 2000)
        {
        printf ("Maximum is 2000\n");
        numsort = 2000;
        }

    srand (1);
    for (i=0; i<numsort; i++)
        {
        x[i] = rand();
        }

    printf ("\nSee results (0/1) : ");
    fgets (inbuf, sizeof inbuf, stdin);
    if (*inbuf != '\0')
        see = (int)strtol (inbuf, (char **)NULL, 10);

    printf ("\nNumber of iterations : ");
    fgets (inbuf, sizeof inbuf, stdin);
    if (*inbuf != '\0')
        iterations = (int)strtol (inbuf, (char **)NULL, 10);

    printf ("\nSorting %d numbers %d interations\n", numsort, iterations);
    for (niter=0; niter<iterations; niter++)
        {
        /* could use memcpy here, but this is another common operation */
        for (i=0, pd=x1, ps=x; i < numsort; i++)
            {
            *pd++ = *ps++;
            }

        mdlUtil_dlmQuickSort (x1, &x1[numsort-1], sizeof(int), orderints, NULL);

        /* check for errors */
        for (i=0; i<numsort-1; i++)
            {
            if (x1[i] > x1[i+1]) printf ("sorting error at %d (%d, %d)\n", 
                                            i, x1[i], x1[i+1]);
            }
        }
        
    if (see)
        {
        for (i=0; i<numsort; i+=5)
            {
            printf ("%5d (%08x):  %5d %5d %5d %5d %5d", i, &x1[i], x1[i], x1[i+1], x1[i+2], 
                        x1[i+3], x1[i+4]);
            for (j=i; j<i+5 && j <numsort-1; j++) 
                {
                if (x1[j] > x1[j+1])
                    {
                    printf ("  sorting error!!\n");
                    fgets (inbuf, sizeof inbuf, stdin);
                    }
                }
            printf ("\n");
            }
        }
    goto top;
    return (0);
    }


#endif
END_BENTLEY_GEOMETRY_NAMESPACE
