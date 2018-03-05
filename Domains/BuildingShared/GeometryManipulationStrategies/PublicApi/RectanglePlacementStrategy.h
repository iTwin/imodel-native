/*--------------------------------------------------------------------------------------+
|
|     $Source: GeometryManipulationStrategies/PublicApi/RectanglePlacementStrategy.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BUILDING_SHARED_NAMESPACE

//=======================================================================================
// @bsiclass                                     Mindaugas.Butkus               03/2018
//=======================================================================================
struct RectanglePlacementStrategy : public GeometryPlacementStrategy
    {
    DEFINE_T_SUPER(GeometryPlacementStrategy)

    private:
        CurveVectorManipulationStrategyPtr m_manipulationStrategy;

        DPoint3d m_rotationPoint;
        bool m_rotationPointSet;
        
        DPoint3d m_dynamicRotationPoint;
        bool m_dynamicRotationPointSet;

        bvector<DPoint3d> CalculateLast3Point(DPoint3d start, DPoint3d rotationPoint, DPoint3d end) const;

    protected:
        RectanglePlacementStrategy(CurveVectorManipulationStrategyR manipulationStrategy);

        virtual GeometryManipulationStrategyCR _GetManipulationStrategy() const override { return *m_manipulationStrategy; }
        virtual GeometryManipulationStrategyR _GetManipulationStrategyForEdit() override { return *m_manipulationStrategy; }

        virtual void _AddKeyPoint(DPoint3dCR newKeyPoint) override;
        virtual void _AddDynamicKeyPoint(DPoint3dCR newDynamicKeyPoint) override;
        virtual void _ResetDynamicKeyPoint() override;
        virtual void _PopKeyPoint() override;
        virtual bool _IsComplete() const override;
        virtual bool _CanAcceptMorePoints() const override;
        virtual bvector<DPoint3d> _GetKeyPoints() const override;
        virtual IGeometryPtr _FinishGeometry() const override;

        virtual BentleyStatus _TryGetProperty(Utf8CP key, RotMatrix& value) const override;

    public:
        static constexpr Utf8CP prop_Rotation() { return "Rotation"; }

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT static RectanglePlacementStrategyPtr Create();
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT static RectanglePlacementStrategyPtr Create(CurveVectorManipulationStrategyR manipulationStrategy);
    };

END_BUILDING_SHARED_NAMESPACE