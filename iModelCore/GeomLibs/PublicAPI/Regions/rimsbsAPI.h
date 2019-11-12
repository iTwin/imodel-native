/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once


/*__BENTLEY_INTERNAL_ONLY__*/
#include <Mtg/MtgApi.h>
#include "regionsAPI.h"
BEGIN_BENTLEY_GEOMETRY_NAMESPACE
#define RIMSBS_NULL_CURVE_ID (-1)

typedef int RIMSBS_CurveId;
struct RIMSBS_CurveIntervalStruct
    {
    int parentId;       /* Parent curve id within RIMSBS indexing */
    double s0;          /* 0.0 = start of parent */
    double s1;          /* 1.0 = end of parent */
    int partialCurveId; /* (optional) standalone curve that is this portion by itself. */
    RIMSBS_CurveIntervalStruct (int parentId, double s0, double s1, int partialCurveId);
    RIMSBS_CurveIntervalStruct ();
    };

struct RIMSBS_CurveChainStruct : public bvector <int>
    {
    int m_primaryCurveId;
    RIMSBS_CurveChainStruct (int primaryCurveId);
    RIMSBS_CurveChainStruct ();
    };

struct RIMSBS_ElementHeader
    {
    int     type;
    int     groupId;
    int     userInt;
    void    *pUserData;
    ICurvePrimitivePtr curve;
    int     alternateCurveId;
    int64_t userInt64;
// ouch.  These fields are only used for particular curve types . ..  they are dead space for the common types
    RIMSBS_CurveIntervalStruct m_partialCurve;
    RIMSBS_CurveChainStruct m_chainData;
    RIMSBS_ElementHeader ();
    RIMSBS_ElementHeader (
        ICurvePrimitivePtr const &curve,
        int type,
        int userInt,
        void *pUserData,
        int groupId,
        int alternateCurveId = RIMSBS_NULL_CURVE_ID,
        int64_t userInt64 = 0
        );


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

struct RIMSBS_Context
    {
private:
    // curveId is dereferenced without range check -- caller must validate first.
    RIMSBS_ElementHeader &GetElementR (int curveId);

    RG_CurveId ResolveThroughPartialCurve (RG_CurveId              curveId);

public:
    bvector <RIMSBS_ElementHeader> m_geometry;
    int                     currGroupId;
    RIMSBS_Context ();
    ~RIMSBS_Context ();
    void SetCurrentGroupId (int groupId);

    int GetGeometryType (int curveId);
    bool IsCurveChain (RIMSBS_CurveId curveId, RIMSBS_CurveId &primaryCurveId);
    ICurvePrimitiveCP GetICurvePrimitiveCP (int curveId);
    int AddArc (DEllipse3dCR arc, int userInt, void *pUserData);
    int AddDataCarrier (int userInt, void *pUserData);
    int AddCurveChain (int primaryCurveId, int userInt, void *pUserData);
    // WARNING -- the only permitted curve types are bspine and arc!!!
    int AddCurve (ICurvePrimitivePtr &curvePrimitive, int userInt, void *pUserData);
    int AddCurveInterval (RIMSBS_CurveIntervalStruct const &data, int userInt, void *pUserData);

    MSBsplineCurveCP GetMSBsplineCurveCP (int curveIndex);
    void * GetUserDataP (int curveIndex);
    int GetUserInt (int curveIndex);
    bool IsValidCurveIndex (int curveIndex);
    bool GetCurveType (int curveIndex);
    bool TrySetUserInt64 (int curveIndex, int64_t data);
    bool TrySetUserInt   (int curveIndex, int data);
    bool TrySetUserPointer (int curveIndex, void * data);
    bool TrySetGroupId (int curveIndex, int data);
    bool TrySetAlternateCurveId (int curveIndex, int data);

    bool TryGetGroupId (int curveIndex, int &data);
    bool TryGetUserInt64 (int curveIndex, int64_t &data);
    bool TryGetImmediateParent (int curveIndex, int &data);
    bool TryGetMostDistantParent (int curveIndex, int &data);
    bool TryGetCurveChainChild (int curveIndex, int childIndex, int &childCurveId);
    bool TryAddCurveToCurveChain (int curveIndex, int childCurveId);
    bool TryGetArc (int curveIndex, DEllipse3dR arc);
    bool TrySetArc (int curveIndex, DEllipse3dCR arc);

    bool TryGetRange (int curveIndex, DRange3dR range);
    void TransformAllCurves (TransformCR transform);
    bool TryTransformCurve (int curveId, TransformCR transform);
    // collect "some" points on each curve -- bspline poles, ellipse 4 per arc?
    void AppendAllCurveSamplePoints (bvector<DPoint3d> &xyzArray);

    bool  TryGetClosestXYPointOnCurve
        (
        double   *pMinParam,        /* => parameter at closest approach point */
        double   *pMinDistSquared,  /* => squard distance to closest approach point */
        DPoint3d *pMinPoint,        /* => closest approach point */
        DPoint3d *pMinTangent,      /* => tangent vector at closest approach point */
        DPoint3d *pPoint,           /* => space point */
        RG_CurveId  curveId         /* => curve identifier */
        );

    bool TryGetClosestXYPointOnMappedCurve
        (
        double   *pMinParam,        /* => parameter at closest approach point */
        double   *pMinDistSquared,  /* => squard distance to closest approach point */
        DPoint3d *pMinPoint,        /* => closest approach point */
        DPoint3d *pMinTangent,      /* => tangent vector at closest approach point */
        DPoint3d *pPoint,           /* => space point */
        RG_CurveId  curveId,        /* => curve identifier */
        double    s0,               /* => start of active interval */
        double    s1                /* => end param for active interval */
        );

    bool TrySegmentCurveIntersection
        (
        RG_Header               *pRG,               /* => receives declarations of intersections */
        RG_IntersectionList     *pIntersections,    /* <=> list of intersection parameters */
        RG_EdgeData     *pEdgeData0,                /* => segment edge data */
        RG_EdgeData     *pEdgeData1                 /* => curve edge data */
        );

    bool TrySegmentCurveIntersectionMapped
        (
        RG_Header               *pRG,               /* => receives declarations of intersections */
        RG_IntersectionList     *pIntersections,    /* <=> list of intersection parameters */
        RG_EdgeData             *pEdgeData0,        /* => segment edge data */
        RG_EdgeData             *pEdgeData1,        /* => curve edge data, known to be mapped */
        int                     parentCurveId,
        double                  s0,
        double                  s1
        );

    bool TryCurveCurveIntersection
        (
        RG_Header               *pRG,               /* => receives declarations of intersections */
        RG_IntersectionList     *pIntersections,    /* <=> list of intersection parameters */
        RG_EdgeData             *pEdgeData0,        /* => segment edge data */
        RG_EdgeData             *pEdgeData1         /* => curve edge data */
        );

    bool TrySweptProperties
        (
        RG_EdgeData     *pEdgeData,
        double          *pArea,
        double          *pAngle,
        const DPoint3d  *pPoint
        );

    bool TrySweptPropertiesMapped
        (
        double          *pArea,
        double          *pAngle,
        const DPoint3d  *pPoint,
        int             curveId,
        double          s0,
        double          s1
        );

    bool TryGetMappedMSBsplineCurve
        (
        MSBsplineCurve  *pCurve,
        RG_CurveId      curveId,
        double          s0,
        double          s1
        );

    bool TryGetResolvedArc
        (
        DEllipse3dR      arc,
        RG_CurveId      curveId,
        bool            reversed
        );

    bool TryGetCurveInterval
        (
        RG_CurveId      *pParentCurveId,
        double          *pStartFraction,
        double          *pEndFraction,
        RG_CurveId      curveId,
        bool            reversed
        );

    bool TryEvaluate
        (
        DPoint3d        *pXYZ,
        DPoint3d        *pTangent,
        double          *pParam,
        int             nParam,
        RG_CurveId      curveId
        );

    bool TryEvaluateMapped
        (
        DPoint3d        *pXYZ,
        DPoint3d        *pTangent,
        double          *pParam,
        int             nParam,
        RG_CurveId      curveId,
        double          s0,
        double          s1
        );

    bool TryEvaluateDerivatives
        (
        DPoint3d        *pXYZ,
        int             numDerivatives,
        double          param,
        RG_CurveId      curveId
        );

    bool TryEvaluateMappedDerivatives
        (
        DPoint3d        *pXYZ,
        int             numDerivatives,
        double          param,
        RG_CurveId      curveId,
        double          s0,
        double          s1
        );

    bool    TryGetMappedCurveRange
        (
        DRange3dR       range,
        RIMSBS_CurveId  curveId,
        double          s0,
        double          s1
        );

    bool TryCurveCircleIntersectionXY
        (
        bvector<double> *pParameterArray,   /* <= curve parameters of intersections. */
        bvector<DPoint3d>*pPointArray,       /* <= curve points of intersections */
        RG_EdgeData             *pEdgeData,         /* => segment edge data */
        const DPoint3d          *pCenter,           /* => circle center */
        double                  radius
        );

    bool TryIntersectXY_mappedCurve_circle
        (
        bvector<double> *pParameterArray,   /* <= curve parameters of intersections. */
        bvector<DPoint3d>*pPointArray,       /* <= curve points of intersections */
        int                     parentCurveId,
        double                  s0,
        double                  s1,
        const DPoint3d          *pCenter,           /* => circle center */
        double                  radius
        );

    };








END_BENTLEY_GEOMETRY_NAMESPACE

#include    <Regions/rimsbs/rimsbs_context.fdf>
#include    <Regions/rimsbs/rimsbs_eval.fdf>
#include    <Regions/rimsbs/rimsbs_intersect.fdf>
#include    <Regions/rimsbs/rimsbs_intersect1.fdf>


