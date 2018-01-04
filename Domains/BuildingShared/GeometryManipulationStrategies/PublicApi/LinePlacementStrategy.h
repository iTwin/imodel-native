/*--------------------------------------------------------------------------------------+
|
|     $Source: GeometryManipulationStrategies/PublicApi/LinePlacementStrategy.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

BUILDING_SHARED_REFCOUNTED_PTR_AND_TYPEDEFS(LinePointsPlacementStrategy)
BUILDING_SHARED_REFCOUNTED_PTR_AND_TYPEDEFS(LinePointLengthAnglePlacementStrategy)

BEGIN_BUILDING_SHARED_NAMESPACE

struct EXPORT_VTABLE_ATTRIBUTE LinePointsPlacementStrategy : CurvePrimitivePlacementStrategy
    {
    DEFINE_T_SUPER(CurvePrimitivePlacementStrategy);

    protected:
        LinePointsPlacementStrategy() : T_Super(LineManipulationStrategy::Create().get()) {};

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _AddKeyPoint(DPoint3dCR newKeyPoint) override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _AddDynamicKeyPoint(DPoint3dCR newDynamicKeyPoint) override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _AddDynamicKeyPoints(bvector<DPoint3d> const& newDynamicKeyPoints) override;
    public:
        static GEOMETRYMANIPULATIONSTRATEGIES_EXPORT LinePointsPlacementStrategyPtr Create() { return new LinePointsPlacementStrategy(); }
    };

struct EXPORT_VTABLE_ATTRIBUTE LinePointLengthAnglePlacementStrategy : CurvePrimitivePlacementStrategy
    {
    DEFINE_T_SUPER(CurvePrimitivePlacementStrategy);

    private:
        double m_length = 0;
        double m_angle = 0;

        DPlane3d m_workingPlane;

    protected:
        LinePointLengthAnglePlacementStrategy(DPlane3d const & workingPlane) : T_Super(LineManipulationStrategy::Create().get()), m_workingPlane(workingPlane) {}

        virtual void _SetWorkingPlane(DPlane3d const & plane);
        virtual DPlane3d _GetWorkingPlane() const { return m_workingPlane; }

        virtual void _SetProperty(Utf8CP key, const double & value) override;
        virtual BentleyStatus _TryGetProperty(Utf8CP key, double & value) const override;
    
        void _SetLength(double const & length);
        double _GetLength() const;

        void _SetAngle(double const & angle);
        double _GetAngle() const;

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _AddKeyPoint(DPoint3dCR newKeyPoint) override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _AddDynamicKeyPoint(DPoint3dCR newDynamicKeyPoint) override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _AddDynamicKeyPoints(bvector<DPoint3d> const& newDynamicKeyPoints) override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _PopKeyPoint();

        BentleyStatus CalculateEndPoint(DPoint3dR endPoint);
        void UpdateEndPoint();
    public:
        static GEOMETRYMANIPULATIONSTRATEGIES_EXPORT LinePointLengthAnglePlacementStrategyPtr Create(DPlane3d const& plane) { return new LinePointLengthAnglePlacementStrategy(plane); }

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT void          SetWorkingPlane(DPlane3d const & plane) { return _SetWorkingPlane(plane); }
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT DPlane3d      GetWorkingPlane() const { return _GetWorkingPlane(); }
    };
END_BUILDING_SHARED_NAMESPACE