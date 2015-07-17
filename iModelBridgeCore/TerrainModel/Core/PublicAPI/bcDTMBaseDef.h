/*--------------------------------------------------------------------------------------+
|
|     $Source: Core/PublicAPI/bcDTMBaseDef.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//__BENTLEY_INTERNAL_ONLY__

#ifndef __BCDTMBASEDEF_H__
#define __BCDTMBASEDEF_H__

#ifdef BCIVILDTM_EXPORTS
#ifdef __cplusplus
#include <msgeomstructs.hpp>
inline void DPoint2d::sumOf (DPoint2dCP origin, DPoint2dCP vector, double scale)
    {
    x = (origin->x + (vector->x * scale));
    y = (origin->y + (vector->y * scale));
    }

inline void DPoint2d::setComponents( double newX, double newY)
    {
    x = newX;
    y = newY;
    }

inline double DVec2d::dotProduct (DVec2d const* vec)
    {
    return ((x * vec->x) + (y * vec->y));
    }

inline DVec2d DVec2d::FromXY (double newX, double newY)
    {
    DVec2d n;
    n.x = newX;
    n.y = newY;
    return n;
    }

#endif // #ifdef __cplusplus
#endif
/*__PUBLISH_SECTION_START__*/
/*-------------------------------------------------------------------+
|	Redefine Function Function Declaration Types                	 |
+-------------------------------------------------------------------*/
#define  BENTLEYDTM_Private   static

#include <Bentley\Bentley.h>
#include <Bentley\RefCounted.h>
#include <TerrainModel\TerrainModel.h>

///*-------------------------------------------------------------------+
//|	DTM Definition files				                             |
//+-------------------------------------------------------------------*/
#include  <TerrainModel\Core\DTMDefs.h>
#include  <TerrainModel/Core/dtmfns.h>
#endif 
