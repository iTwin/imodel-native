/*--------------------------------------------------------------------------------------+
|
|     $Source: GeometryManipulationStrategies/PublicApi/GeometryManipulationStrategy.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

BUILDING_SHARED_REFCOUNTED_PTR_AND_TYPEDEFS(GeometryManipulationStrategy)

BEGIN_BUILDING_SHARED_NAMESPACE

//=======================================================================================
// @bsiclass                                     Mindaugas.Butkus               12/2017
//=======================================================================================
struct GeometryManipulationStrategy : public GeometryManipulationStrategyBase
    {
    DEFINE_T_SUPER(GeometryManipulationStrategyBase)

    private:
        bvector<DPoint3d> m_keyPoints;
        bvector<DPoint3d> m_keyPointsWithDynamicKeyPoint;
        bool m_dynamicKeyPointSet;

    protected:
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT GeometryManipulationStrategy();

        bvector<DPoint3d> const& GetAcceptedKeyPoints() const { return m_keyPoints; }
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual bvector<DPoint3d> const& _GetKeyPoints() const override;

        virtual bool _IsDynamicKeyPointSet() const override { return m_dynamicKeyPointSet; }
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _AppendDynamicKeyPoint(DPoint3dCR newDynamicKeyPoint) override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _AppendDynamicKeyPoints(bvector<DPoint3d> const& newDynamicKeyPoints) override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _InsertDynamicKeyPoint(DPoint3dCR newDynamicKeyPoint, size_t index) override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _InsertDynamicKeyPoints(bvector<DPoint3d> const& newDynamicKeyPoints, size_t index) override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _UpdateDynamicKeyPoint(DPoint3dCR newDynamicKeyPoint, size_t index) override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _UpdateDynamicKeyPoints(bvector<DPoint3d> const& newDynamicKeyPoints, size_t index) override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _ResetDynamicKeyPoint() override;

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _AppendKeyPoint(DPoint3dCR);

    public:
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT void AppendKeyPoint(DPoint3dCR newKeyPoint);
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT void InsertKeyPoint(DPoint3dCR newKeyPoint, size_t index);
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT void ReplaceKeyPoint(DPoint3dCR newKeyPoint, size_t index);
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT void PopKeyPoint();
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT void RemoveKeyPoint(size_t index);
    };

END_BUILDING_SHARED_NAMESPACE