/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include    <Bentley/Bentley.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE


#if !defined (__mdlmkn__)
typedef int (*PFToolsSortCompare)(const void *,const void *,const void *);

void util_insertSort
(    
void	*pfirst,
void	*plast,
int	elementsize,
PFToolsSortCompare comparefunc,
void	*optArg
);    

int util_compareLongs
(    
long	y1, 
long	y2
);    

int util_compareDoubles
(    
double	y1,
double	y2
);

int util_tagSort
(    
int    	*tags,
double	*values,
int    	numValues
);    

#endif

#if !defined (__mdlmkn__)
/* not documented */

void	mdlUtil_dlmQuickSort
(    
void	               *pfirst,
void	               *plast,
int	                elementsize,
PFToolsSortCompare      compareFunc, /* => comparison function */
void	               *optArg
);    
#endif

Public void mdlUtil_sortDoubles
(    
double *doubles,	/* <=> array of Doubles to be sorted (in place) */
int    numDoubles,	/* => number of Doubles to sort */
int    ascend		/* => true for ascending order */
);


END_BENTLEY_GEOMETRY_NAMESPACE
