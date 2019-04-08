/*--------------------------------------------------------------------------------------+
|
|     $Source: GeometryManipulationStrategies/PublicApi/LinePlacementStrategy.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
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
        virtual GeometryManipulationStrategyR _GetManipulationStrategyForEdit() override { return *m_manipulationStrategy; }
        LineManipulationStrategyCR GetLineManipulationStrategy() const { return *m_manipulationStrategy; }
        LineManipulationStrategyR GetLineManipulationStrategyForEdit() { return *m_manipulationStrategy; }
        virtual CurvePrimitiveManipulationStrategyCR _GetCurvePrimitiveManipulationStrategy() const override { return *m_manipulationStrategy; }
        virtual CurvePrimitiveManipulationStrategyR _GetCurvePrimitiveManipulationStrategyForEdit() override { return *m_manipulationStrategy; }

    public:
        static constexpr Utf8CP prop_Length() { return "Length"; }
        static constexpr Utf8CP prop_Angle() { return "Angle"; }
        static constexpr Utf8CP prop_WorkingPlane() { return "WorkingPlane"; }


        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT static LinePlacementStrategyPtr Create(LinePlacementStrategyType strategyType, LineManipulationStrategyR manipulationStrategy);
    };

struct LinePointsPlacementStrategy : LinePlacementStrategy
    {
    DEFINE_T_SUPER(LinePlacementStrategy);

    protected:
        LinePointsPlacementStrategy() : T_Super() {};
        LinePointsPlacementStrategy(LineManipulationStrategyR manipulationStrategy) : T_Super(manipulationStrategy) {}

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _AddKeyPoint(DPoint3dCR newKeyPoint) override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _AddDynamicKeyPoint(DPoint3dCR newDynamicKeyPoint) override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _AddDynamicKeyPoints(bvector<DPoint3d> const& newDynamicKeyPoints) override;
    public:
        static GEOMETRYMANIPULATIONSTRATEGIES_EXPORT LinePointsPlacementStrategyPtr Create() { return new LinePointsPlacementStrategy(); }
        static GEOMETRYMANIPULATIONSTRATEGIES_EXPORT LinePointsPlacementStrategyPtr Create(LineManipulationStrategyR manipulationStrategy) { return new LinePointsPlacementStrategy(manipulationStrategy); }
    };

struct LinePointLengthAnglePlacementStrategy : LinePlacementStrategy
    {
    DEFINE_T_SUPER(LinePlacementStrategy);

    private:
        double m_length = 0;
        double m_angle = 0;

        DPlane3d m_workingPlane;

    protected:
        LinePointLengthAnglePlacementStrategy(DPlane3d const & workingPlane);
        LinePointLengthAnglePlacementStrategy(LineManipulationStrategyR manipulationStrategy, DPlane3d const & workingPlane);


        virtual void _SetWorkingPlane(DPlane3d const & plane);
        virtual DPlane3d _GetWorkingPlane() const { return m_workingPlane; }

        virtual void _SetProperty(Utf8CP key, const double & value) override;
        virtual BentleyStatus _TryGetProperty(Utf8CP key, double & value) const override;

        virtual void _SetProperty(Utf8CP key, DPlane3d const& value) override;
        virtual BentleyStatus _TryGetProperty(Utf8CP key, DPlane3d & value) const override;

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
        static GEOMETRYMANIPULATIONSTRATEGIES_EXPORT LinePointLengthAnglePlacementStrategyPtr Create(LineManipulationStrategyR manipulationStrategy, DPlane3d const& plane) { return new LinePointLengthAnglePlacementStrategy(manipulationStrategy, plane); }
    };

//=======================================================================================
// @bsiclass                                     Mindaugas.Butkus               01/2018
//=======================================================================================
struct MetesAndBounds : GeometryManipulationStrategyProperty
    {
    typedef bpair<Utf8String, double> ValuePair;
    DEFINE_T_SUPER(GeometryManipulationStrategyProperty)

    private:
        bvector<ValuePair> m_value;

    public:
        MetesAndBounds()
            : T_Super()
            {}
        MetesAndBounds(Utf8CP directionString, double length)
            : T_Super()
            , m_value({ValuePair(directionString, length)})
            {}
        MetesAndBounds(ValuePair const& value)
            : T_Super()
            , m_value({value})
            {}
        MetesAndBounds(bvector<ValuePair> const& value)
            : T_Super()
            , m_value(value)
            {}
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT MetesAndBounds(bvector<Utf8String> directionStrings, bvector<double> lengths);

        bvector<ValuePair> const& GetValue() const { return m_value; }
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
        LineMetesAndBoundsPlacementStrategy(DPlane3d plane);

        virtual void _SetProperty(Utf8CP key, GeometryManipulationStrategyProperty const& value) override;
        virtual BentleyStatus _TryGetProperty(Utf8CP key, GeometryManipulationStrategyProperty& value) const override;

    public:
        static constexpr Utf8CP prop_MetesAndBounds() { return "MetesAndBounds"; }

        static LineMetesAndBoundsPlacementStrategyPtr Create(DPlane3d const& plane) { return new LineMetesAndBoundsPlacementStrategy(plane); }
    };

struct LinePointsLengthPlacementStrategy : LinePlacementStrategy
    {
    DEFINE_T_SUPER(LinePlacementStrategy);

    private:
        double m_length = 0;
        DVec3d m_direction;

    protected:
        LinePointsLengthPlacementStrategy();
        LinePointsLengthPlacementStrategy(LineManipulationStrategyR manipulationStrategy);

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
        static GEOMETRYMANIPULATIONSTRATEGIES_EXPORT LinePointsLengthPlacementStrategyPtr Create(LineManipulationStrategyR manipulationStrategy) { return new LinePointsLengthPlacementStrategy(manipulationStrategy); }
    };

struct LinePointsAnglePlacementStrategy : LinePlacementStrategy
    {
    DEFINE_T_SUPER(LinePlacementStrategy);

    private:
        double m_angle = 0;
        DPlane3d m_workingPlane;

    protected:
        LinePointsAnglePlacementStrategy(DPlane3d plane) : T_Super(), m_workingPlane(plane) {}
        LinePointsAnglePlacementStrategy(LineManipulationStrategyR manipulationStrategy, DPlane3d plane);

        virtual void _SetProperty(Utf8CP key, const double & value) override;
        virtual BentleyStatus _TryGetProperty(Utf8CP key, double & value) const override;

        virtual void _SetProperty(Utf8CP key, DPlane3d const& value) override;
        virtual BentleyStatus _TryGetProperty(Utf8CP key, DPlane3d & value) const override;

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
        static GEOMETRYMANIPULATIONSTRATEGIES_EXPORT LinePointsAnglePlacementStrategyPtr Create(LineManipulationStrategyR manipulationStrategy, DPlane3d plane) { return new LinePointsAnglePlacementStrategy(manipulationStrategy, plane); }
    };


END_BUILDING_SHARED_NAMESPACE