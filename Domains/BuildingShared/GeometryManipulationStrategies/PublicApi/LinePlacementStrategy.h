/*--------------------------------------------------------------------------------------+
|
|     $Source: GeometryManipulationStrategies/PublicApi/LinePlacementStrategy.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BUILDING_SHARED_NAMESPACE

enum class LinePlacementStrategyType
    {
    Points = 0,
    PointLengthAngle,
    PointsLength,
    PointsAngle
    };

struct LinePlacementStrategy : CurvePrimitivePlacementStrategy
    {
    DEFINE_T_SUPER(CurvePrimitivePlacementStrategy)

    private:
        LineManipulationStrategyPtr m_manipulationStrategy;
        
    protected:
        LinePlacementStrategy() : T_Super(), m_manipulationStrategy(LineManipulationStrategy::Create()) {}
        LinePlacementStrategy(LineManipulationStrategyR manipulationStrategy);

        virtual GeometryManipulationStrategyCR _GetManipulationStrategy() const override { return *m_manipulationStrategy; }
        virtual GeometryManipulationStrategyR _GetManipulationStrategyR() override { return *m_manipulationStrategy; }
        LineManipulationStrategyCR GetLineManipulationStrategy() const { return *m_manipulationStrategy; }
        LineManipulationStrategyR GetLineManipulationStrategyR() { return *m_manipulationStrategy; }

    public:
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT static const Utf8CP prop_Length;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT static const Utf8CP prop_Angle;

        static LinePlacementStrategyPtr Create(LinePlacementStrategyType strategyType);
        static LinePlacementStrategyPtr Create(LinePlacementStrategyType strategyType, LineManipulationStrategyR manipulationStrategy);
    };

struct EXPORT_VTABLE_ATTRIBUTE LinePointsPlacementStrategy : LinePlacementStrategy
    {
    DEFINE_T_SUPER(LinePlacementStrategy);

    protected:
        LinePointsPlacementStrategy() : T_Super() {};
        LinePointsPlacementStrategy(LineManipulationStrategyR manipulationStrategy) : T_Super(manipulationStrategy) {}

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _AddKeyPoint(DPoint3dCR newKeyPoint) override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _AddDynamicKeyPoint(DPoint3dCR newDynamicKeyPoint) override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _AddDynamicKeyPoints(bvector<DPoint3d> const& newDynamicKeyPoints) override;
    public:
        static LinePointsPlacementStrategyPtr Create() { return new LinePointsPlacementStrategy(); }
        static LinePointsPlacementStrategyPtr Create(LineManipulationStrategyR manipulationStrategy) { return new LinePointsPlacementStrategy(manipulationStrategy); }
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
        using T_Super::_SetProperty;
        using T_Super::_TryGetProperty;
    
        void SetLength(double const & length);
        double GetLength() const;

        void SetAngle(double const & angle);
        double GetAngle() const;

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

//=======================================================================================
// @bsiclass                                     Mindaugas.Butkus               01/2018
//=======================================================================================
struct MetesAndBounds : GeometryManipulationStrategyProperty
    {
    DEFINE_T_SUPER(GeometryManipulationStrategyProperty)

    private:
        bvector<bpair<Utf8String, double>> m_value;

    public:
        MetesAndBounds(bvector<bpair<Utf8String, double>> const& value)
            : T_Super()
            , m_value(value)
            {}

        bvector<bpair<Utf8String, double>> const& GetValue() const { return m_value; }
    };

//=======================================================================================
// @bsiclass                                     Mindaugas.Butkus               01/2018
//=======================================================================================
struct LineMetesAndBoundsPlacementStrategy : public LinePointLengthAnglePlacementStrategy
    {
    DEFINE_T_SUPER(LinePointLengthAnglePlacementStrategy)

    private:
        Utf8String m_directionString;
        
    protected:
        LineMetesAndBoundsPlacementStrategy(DPlane3d plane) : T_Super(plane), m_directionString("") {}

        virtual void _SetProperty(Utf8CP key, GeometryManipulationStrategyProperty const& value) override;
        virtual BentleyStatus _TryGetProperty(Utf8CP key, GeometryManipulationStrategyProperty& value) const override;

    public:
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT static const Utf8CP prop_MetesAndBounds;

        static LineMetesAndBoundsPlacementStrategyPtr Create(DPlane3d const& plane) { return new LineMetesAndBoundsPlacementStrategy(plane); }
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

        void SetLength(double const & length);
        double GetLength() const;

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _AddKeyPoint(DPoint3dCR newKeyPoint) override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _AddDynamicKeyPoint(DPoint3dCR newDynamicKeyPoint) override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _AddDynamicKeyPoints(bvector<DPoint3d> const& newDynamicKeyPoints) override;

        BentleyStatus AdjustEndPoint();
    public:
        static GEOMETRYMANIPULATIONSTRATEGIES_EXPORT LinePointsLengthPlacementStrategyPtr Create() { return new LinePointsLengthPlacementStrategy(); }
    };

struct EXPORT_VTABLE_ATTRIBUTE LinePointsAnglePlacementStrategy : LinePlacementStrategy
    {
    DEFINE_T_SUPER(LinePlacementStrategy);

    private:
        double m_angle = 0;
        DPlane3d m_workingPlane;

    protected:
        LinePointsAnglePlacementStrategy(DPlane3d plane) : T_Super(), m_workingPlane(plane) {}

        virtual void _SetProperty(Utf8CP key, const double & value) override;
        virtual BentleyStatus _TryGetProperty(Utf8CP key, double & value) const override;

        void SetAngle(double const & angle);
        double GetAngle() const;

        virtual void _SetWorkingPlane(DPlane3d const & plane);
        virtual DPlane3d _GetWorkingPlane() const { return m_workingPlane; }

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _AddKeyPoint(DPoint3dCR newKeyPoint) override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _AddDynamicKeyPoint(DPoint3dCR newDynamicKeyPoint) override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _AddDynamicKeyPoints(bvector<DPoint3d> const& newDynamicKeyPoints) override;

        BentleyStatus AdjustEndPoint();
    public:
        static GEOMETRYMANIPULATIONSTRATEGIES_EXPORT LinePointsAnglePlacementStrategyPtr Create(DPlane3d plane) { return new LinePointsAnglePlacementStrategy(plane); }

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT void          SetWorkingPlane(DPlane3d const & plane) { return _SetWorkingPlane(plane); }
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT DPlane3d      GetWorkingPlane() const { return _GetWorkingPlane(); }
    };


END_BUILDING_SHARED_NAMESPACE