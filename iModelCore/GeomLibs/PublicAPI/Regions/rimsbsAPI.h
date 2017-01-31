/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Regions/rimsbsAPI.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once


/*__BENTLEY_INTERNAL_ONLY__*/
#include <Mtg/MtgApi.h>
#include "Regions/regionsAPI.h"
BEGIN_BENTLEY_GEOMETRY_NAMESPACE
#define RIMSBS_NULL_CURVE_ID (-1)

struct RIMSBS_ElementHeader
    {
    int     type;
    int     groupId;
    int     userInt;
    void    *pUserData;
    void    *pGeometryData;
    int     alternateCurveId;
    int64_t userInt64;
    };

struct RIMSBS_Context
    {
    bvector <RIMSBS_ElementHeader> m_geometry;
    int                     currGroupId;
    RIMSBS_Context ();
    ~RIMSBS_Context ();
    };

typedef enum
    {
    RIMSBS_NoGeometry       = 0,
    RIMSBS_CurveInterval    = 1,
    RIMSBS_DEllipse3d       = 2,
    RUNSBS_DEllipse4d       = 3,
    RIMSBS_MSBsplineCurve   = 4,
    RIMSBS_DataCarrier      = 5,
    RIMSBS_CurveChain      = 6
    } RIMSBS_GeometryType;





typedef struct
    {
    int parentId;       /* Parent curve id within RIMSBS indexing */
    double s0;          /* 0.0 = start of parent */
    double s1;          /* 1.0 = end of parent */
    int partialCurveId; /* (optional) standalone curve that is this portion by itself. */
    } RIMSBS_CurveIntervalStruct;

struct RIMSBS_CurveChainStruct : public bvector <int>
    {
    int m_primaryCurveId;
    RIMSBS_CurveChainStruct (int primaryCurveId);
    };

typedef int RIMSBS_CurveId;
END_BENTLEY_GEOMETRY_NAMESPACE

#include    <Regions/rimsbs/rimsbs_context.fdf>
#include    <Regions/rimsbs/rimsbs_eval.fdf>
#include    <Regions/rimsbs/rimsbs_intersect.fdf>
#include    <Regions/rimsbs/rimsbs_intersect1.fdf>


