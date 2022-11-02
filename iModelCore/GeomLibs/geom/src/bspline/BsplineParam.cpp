/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include <memory.h>
#include <stdlib.h>
#include <math.h>

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
int BsplineParam::NumberAllocatedKnots () const
    {
    return (closed ? numPoles + 2*order - 1 : numPoles + order);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
int BsplineParam::NumberAllocatedKnots (int numPoles, int order, int closed)
    {
    return (closed ? numPoles + 2*order - 1 : numPoles + order);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
int BsplineParam::NumberInteriorKnots (int numPoles, int order, int closed)
    {
    return (closed ? numPoles - 1 : numPoles - order);
    }
END_BENTLEY_GEOMETRY_NAMESPACE