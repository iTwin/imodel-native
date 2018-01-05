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
BUILDING_SHARED_REFCOUNTED_PTR_AND_TYPEDEFS(LinePointsLengthPlacementStrategy)

BEGIN_BUILDING_SHARED_NAMESPACE

struct LinePlacementStrategy : CurvePrimitivePlacementStrategy
    {
    private:
        LineManipulationStrategyPtr m_manipulationStrategy = LineManipulationStrategy::Create();
        
    protected:
        virtual GeometryManipulationStrategyCR _GetManipulationStrategy() const override { return *m_manipulationStrategy; }
        virtual GeometryManipulationStrategyR _GetManipulationStrategyR() override { return *m_manipulationStrategy; }
        LineManipulationStrategyCR GetLineManipulationStrategy() const { return *m_manipulationStrategy; }
        LineManipulationStrategyR GetLineManipulationStrategyR() { return *m_manipulationStrategy; }
    };

struct EXPORT_VTABLE_ATTRIBUTE LinePointsPlacementStrategy : LinePlacementStrategy
    {
    DEFINE_T_SUPER(LinePlacementStrategy);

    protected:
        LinePointsPlacementStrategy() : T_Super() {};

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _AddKeyPoint(DPoint3dCR newKeyPoint) override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _AddDynamicKeyPoint(DPoint3dCR newDynamicKeyPoint) override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _AddDynamicKeyPoints(bvector<DPoint3d> const& newDynamicKeyPoints) override;
    public:
        static GEOMETRYMANIPULATIONSTRATEGIES_EXPORT LinePointsPlacementStrategyPtr Create() { return new LinePointsPlacementStrategy(); }
    };

struct EXPORT_VTABLE_ATTRIBUTE LinePointLengthAnglePlacementStrategy : LinePlacementStrategy
    {
    DEFINE_T_SUPER(LinePlacementStrategy);

    private:
        double m_length = 0;
        double m_angle = 0;

        DPlane3d m_workingPlane;

    protected:
        LinePointLengthAnglePlacementStrategy(DPlane3d const & workingPlane) : T_Super(), m_workingPlane(workingPlane) {}

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

struct EXPORT_VTABLE_ATTRIBUTE LinePointsLengthPlacementStrategy : LinePlacementStrategy
    {
    DEFINE_T_SUPER(LinePlacementStrategy);

    private:
        double m_length = 0;
        DVec3d m_direction;

    protected:
        LinePointsLengthPlacementStrategy() : T_Super() {}

        virtual void _SetProperty(Utf8CP key, const double & value) override;
        virtual BentleyStatus _TryGetProperty(Utf8CP key, double & value) const override;

        void _SetLength(double const & length);
        double _GetLength() const;

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _AddKeyPoint(DPoint3dCR newKeyPoint) override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _AddDynamicKeyPoint(DPoint3dCR newDynamicKeyPoint) override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _AddDynamicKeyPoints(bvector<DPoint3d> const& newDynamicKeyPoints) override;

        BentleyStatus AdjustEndPoint();
    public:
        static GEOMETRYMANIPULATIONSTRATEGIES_EXPORT LinePointsLengthPlacementStrategyPtr Create() { return new LinePointsLengthPlacementStrategy(); }
    };

END_BUILDING_SHARED_NAMESPACE