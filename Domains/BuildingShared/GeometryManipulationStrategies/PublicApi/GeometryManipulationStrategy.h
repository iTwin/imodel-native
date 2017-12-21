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
        GeometryManipulationStrategy();

        virtual bvector<DPoint3d> const& _GetKeyPoints() const override;

        virtual bool _IsDynamicKeyPointSet() const override { return m_dynamicKeyPointSet; }
        virtual void _SetDynamicKeyPoint(DPoint3dCR newDynamicKeyPoint, size_t index, DynamicKeyPointType type) override;
        virtual void _ResetDynamicKeyPoint() override;

    public:
        void AppendKeyPoint(DPoint3dCR newKeyPoint);
        void InsertKeyPoint(DPoint3dCR newKeyPoint, size_t index);
        void ReplaceKeyPoint(DPoint3dCR newKeyPoint, size_t index);
        void PopKeyPoint();
        void RemoveKeyPoint(size_t index);
    };

END_BUILDING_SHARED_NAMESPACE