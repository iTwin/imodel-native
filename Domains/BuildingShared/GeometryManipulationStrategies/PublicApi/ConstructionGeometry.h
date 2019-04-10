/*--------------------------------------------------------------------------------------+
|
|     $Source: GeometryManipulationStrategies/PublicApi/ConstructionGeometry.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BUILDING_SHARED_NAMESPACE

#define CONSTRUCTION_GEOMTYPE_Invalid                       -1

#define BASE_CONSTRUCTION_GEOMTYPE_Generic                  0
#define CONSTRUCTION_GEOMTYPE_GenericCurveVector            1
#define CONSTRUCTION_GEOMTYPE_GenericICurvePrimitive        2

#define BASE_CONSTRUCTION_GEOMTYPE_Arc                      100
#define CONSTRUCTION_GEOMTYPE_ArcCenter                     (BASE_CONSTRUCTION_GEOMTYPE_Arc + 0)
#define CONSTRUCTION_GEOMTYPE_ArcCenterToStart              (BASE_CONSTRUCTION_GEOMTYPE_Arc + 1)
#define CONSTRUCTION_GEOMTYPE_ArcCenterToEnd                (BASE_CONSTRUCTION_GEOMTYPE_Arc + 2)
#define CONSTRUCTION_GEOMTYPE_ArcEdge                       (BASE_CONSTRUCTION_GEOMTYPE_Arc + 3)
#define CONSTRUCTION_GEOMTYPE_ArcInfiniteLine               (BASE_CONSTRUCTION_GEOMTYPE_Arc + 4)
#define CONSTRUCTION_GEOMTYPE_ArcInvalidKeyPoint            (BASE_CONSTRUCTION_GEOMTYPE_Arc + 5)

#define BASE_CONSTRUCTION_GEOMTYPE_Spline                   200
#define CONSTRUCTION_GEOMTYPE_SplinePolePoints              (BASE_CONSTRUCTION_GEOMTYPE_Spline + 0)
#define CONSTRUCTION_GEOMTYPE_SplinePoleLines               (BASE_CONSTRUCTION_GEOMTYPE_Spline + 1)
#define CONSTRUCTION_GEOMTYPE_SplineFitPoints               (BASE_CONSTRUCTION_GEOMTYPE_Spline + 2)
#define CONSTRUCTION_GEOMTYPE_SplineStartTangentPoint       (BASE_CONSTRUCTION_GEOMTYPE_Spline + 3)
#define CONSTRUCTION_GEOMTYPE_SplineEndTangentPoint         (BASE_CONSTRUCTION_GEOMTYPE_Spline + 4)
#define CONSTRUCTION_GEOMTYPE_SplineStartTangentLine        (BASE_CONSTRUCTION_GEOMTYPE_Spline + 5)
#define CONSTRUCTION_GEOMTYPE_SplineEndTangentLine          (BASE_CONSTRUCTION_GEOMTYPE_Spline + 6)

//=======================================================================================
// @bsiclass                                     Mindaugas.Butkus               02/2019
//=======================================================================================
struct ConstructionGeometry
    {
    private:
        IGeometryPtr m_geometry;
        int m_type;

    public:
        ConstructionGeometry()
            : m_geometry(nullptr)
            , m_type(CONSTRUCTION_GEOMTYPE_Invalid)
            {}

        ConstructionGeometry(IGeometryCR geometry, int type)
            : m_geometry(geometry.Clone())
            , m_type(type)
            {}

        IGeometryPtr GetGeometry() const { return m_geometry; }
        int GetType() const { return m_type; }
        bool IsValid() const { return CONSTRUCTION_GEOMTYPE_Invalid != m_type && m_geometry.IsValid(); }
    };

END_BUILDING_SHARED_NAMESPACE