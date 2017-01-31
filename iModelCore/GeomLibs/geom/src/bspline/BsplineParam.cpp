/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/bspline/BsplineParam.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include <memory.h>
#include <stdlib.h>
#include <math.h>
#if defined (INCLUDE_PPL)
    #include <Bentley\Iota.h>
    #include <ppl.h>
    //#define USE_PPL
    #if !defined (USE_PPL)
        #include <algorithm>
    #endif
#endif
BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Earlin.Lutz             08/07
+---------------+---------------+---------------+---------------+---------------+------*/
int BsplineParam::NumberAllocatedKnots () const
    {
    return (closed ? numPoles + 2*order - 1 : numPoles + order);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Earlin.Lutz             08/07
+---------------+---------------+---------------+---------------+---------------+------*/
int BsplineParam::NumberAllocatedKnots (int numPoles, int order, int closed)
    {
    return (closed ? numPoles + 2*order - 1 : numPoles + order);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Earlin.Lutz             08/07
+---------------+---------------+---------------+---------------+---------------+------*/
int BsplineParam::NumberInteriorKnots (int numPoles, int order, int closed)
    {
    return (closed ? numPoles - 1 : numPoles - order);
    }
END_BENTLEY_GEOMETRY_NAMESPACE