/*--------------------------------------------------------------------------------------+
|
|     $Source: Grids/Strategies/PublicApi/LinePlacementStrategy.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

GRIDS_REFCOUNTED_PTR_AND_TYPEDEFS(LinePointsPlacementStrategy)
GRIDS_REFCOUNTED_PTR_AND_TYPEDEFS(LinePointLengthPlacementStrategy)

BEGIN_GRIDS_NAMESPACE

struct EXPORT_VTABLE_ATTRIBUTE LinePointsPlacementStrategy : CurvePlacementStrategy
    {
    DEFINE_T_SUPER(CurvePlacementStrategy);

    DPoint3d m_startPoint, m_endPoint;
    bool m_startPointInUse, m_endPointInUse;
    protected:
        LinePointsPlacementStrategy() = default;

        virtual ICurvePrimitivePtr _GetCurvePrimitive() override;
        
        virtual BentleyStatus _SetPropertyValuePoint3d(Utf8CP propertyName, const DPoint3d & value) override;
        virtual BentleyStatus _GetPropertyValuePoint3d(Utf8CP propertyName, DPoint3d & value) const override;

        virtual BentleyStatus _AddPoint(DPoint3d point) override;
        virtual void _Reset() override;
        
        // Should not by used by line tools
        virtual void _SetDynamicPoint(DPoint3d point) override {};
        virtual void _UnsetDynamicPoint() override {};
        virtual void _AcceptDynamicPoint() override {};
        virtual bool _IsInDynamics() const { return false; }

    public:
        static GRIDSTRATEGIES_EXPORT LinePointsPlacementStrategyPtr Create() { return new LinePointsPlacementStrategy(); }

        GRIDSTRATEGIES_EXPORT BentleyStatus SetStartPoint(DPoint3d const & startPoint) { return _SetPropertyValuePoint3d(GRIDS_PROP_StartPoint, startPoint); }
        GRIDSTRATEGIES_EXPORT BentleyStatus GetStartPoint(DPoint3d & startPoint) const { return _GetPropertyValuePoint3d(GRIDS_PROP_StartPoint, startPoint); }

        GRIDSTRATEGIES_EXPORT BentleyStatus SetEndPoint(DPoint3d const & endPoint) { return _SetPropertyValuePoint3d(GRIDS_PROP_EndPoint, endPoint); }
        GRIDSTRATEGIES_EXPORT BentleyStatus GetEndPoint(DPoint3d & endPoint) const { return _GetPropertyValuePoint3d(GRIDS_PROP_EndPoint, endPoint); }
    };

struct EXPORT_VTABLE_ATTRIBUTE LinePointLengthAnglePlacementStrategy : CurvePlacementStrategy
    {
    DEFINE_T_SUPER(CurvePlacementStrategy);

    private:
        double m_length;
        double m_angle;
        DPlane3d m_workingPlane;

    protected:
        LinePointLengthAnglePlacementStrategy(DPlane3d const & workingPlane) : m_workingPlane(workingPlane) {}

        virtual ICurvePrimitivePtr _GetCurvePrimitive() override;

        virtual BentleyStatus _SetPropertyValuePoint3d(Utf8CP propertyName, const DPoint3d & value) override;
        virtual BentleyStatus _GetPropertyValuePoint3d(Utf8CP propertyName, DPoint3d & value) const override;

        virtual BentleyStatus _SetPropertyValueDouble(Utf8CP propertyName, const double & value) override;
        virtual BentleyStatus _GetPropertyValueDouble(Utf8CP propertyName, double & value) const override;

        virtual void _SetWorkingPlane(DPlane3d const & plane) { m_workingPlane = plane; }
        virtual DPlane3d _GetWorkingPlane() const { return m_workingPlane; }

        virtual BentleyStatus _AddPoint(DPoint3d point) override;
    public:
        static GRIDSTRATEGIES_EXPORT LinePointLengthAnglePlacementStrategyPtr Create(DPlane3d const& plane);

        GRIDSTRATEGIES_EXPORT BentleyStatus SetPoint(DPoint3d const & point) { return _SetPropertyValuePoint3d(GRIDS_PROP_Point, point); }
        GRIDSTRATEGIES_EXPORT BentleyStatus GetPoint(DPoint3d & point) const { return _GetPropertyValuePoint3d(GRIDS_PROP_Point, point); }

        GRIDSTRATEGIES_EXPORT BentleyStatus SetLength(double const & length) { return _SetPropertyValueDouble(GRIDS_PROP_Length, length); }
        GRIDSTRATEGIES_EXPORT BentleyStatus GetLength(double & length) const { return _GetPropertyValueDouble(GRIDS_PROP_Length, length); }

        GRIDSTRATEGIES_EXPORT BentleyStatus SetAngle(double const & angle) { return _SetPropertyValueDouble(GRIDS_PROP_Angle, angle); }
        GRIDSTRATEGIES_EXPORT BentleyStatus GetAngle(double & angle) const { return _GetPropertyValueDouble(GRIDS_PROP_Angle, angle); }

        GRIDSTRATEGIES_EXPORT void          SetWorkingPlane(DPlane3d const & plane) { return _SetWorkingPlane(plane); }
        GRIDSTRATEGIES_EXPORT DPlane3d      GetWorkingPlane() const { return _GetWorkingPlane(); }
    };
END_GRIDS_NAMESPACE