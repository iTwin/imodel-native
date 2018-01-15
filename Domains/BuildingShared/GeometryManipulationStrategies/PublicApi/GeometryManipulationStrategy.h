/*--------------------------------------------------------------------------------------+
|
|     $Source: GeometryManipulationStrategies/PublicApi/GeometryManipulationStrategy.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BUILDING_SHARED_NAMESPACE

//=======================================================================================
// @bsiclass                                     Mindaugas.Butkus               12/2017
//=======================================================================================
struct GeometryManipulationStrategy : public GeometryManipulationStrategyBase
    {
    DEFINE_T_SUPER(GeometryManipulationStrategyBase)

    friend struct GeometryPlacementStrategy;

    private:
        template <typename Func> void ManipulateKeyPoint(Func manipulationFn);

    protected:
        GeometryManipulationStrategy() : T_Super() {}

        virtual void _AppendDynamicKeyPoint(DPoint3dCR newDynamicKeyPoint) = 0;
        virtual void _AppendDynamicKeyPoints(bvector<DPoint3d> const& newDynamicKeyPoints) = 0;
        virtual void _InsertDynamicKeyPoint(DPoint3dCR newDynamicKeyPoint, size_t index) = 0;
        virtual void _InsertDynamicKeyPoints(bvector<DPoint3d> const& newDynamicKeyPoints, size_t index) = 0;
        virtual void _UpdateDynamicKeyPoint(DPoint3dCR newDynamicKeyPoint, size_t index) = 0;
        virtual void _UpdateDynamicKeyPoints(bvector<DPoint3d> const& newDynamicKeyPoints, size_t index) = 0;
        virtual void _UpsertDynamicKeyPoint(DPoint3d newDynamicKeyPoint, size_t index) = 0;
        virtual void _UpsertDynamicKeyPoints(bvector<DPoint3d> const& newDynamicKeyPoints, size_t index) = 0;

        virtual void _OnKeyPointsChanged() {};

        virtual void _AppendKeyPoint(DPoint3dCR newKeyPoint) = 0;
        virtual void _AppendKeyPoints(bvector<DPoint3d> const& newKeyPoints) = 0;
        virtual void _InsertKeyPoint(DPoint3dCR newKeyPoint, size_t index) = 0;
        virtual void _ReplaceKeyPoint(DPoint3dCR newKeyPoint, size_t index) = 0;
        virtual void _PopKeyPoint() = 0;
        virtual void _RemoveKeyPoint(size_t index) = 0;
        virtual void _Clear() = 0;

    public:
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT void AppendKeyPoint(DPoint3dCR newKeyPoint);
        void AppendKeyPoints(bvector<DPoint3d> const& newKeyPoints);
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT void InsertKeyPoint(DPoint3dCR newKeyPoint, size_t index);
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT void ReplaceKeyPoint(DPoint3dCR newKeyPoint, size_t index);
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT void PopKeyPoint();
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT void RemoveKeyPoint(size_t index);
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT void Clear();

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT void AppendDynamicKeyPoint(DPoint3dCR newDynamicKeyPoint);
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT void AppendDynamicKeyPoints(bvector<DPoint3d> const& newDynamicKeyPoints);
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT void InsertDynamicKeyPoint(DPoint3dCR newDynamicKeyPoint, size_t index);
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT void InsertDynamicKeyPoints(bvector<DPoint3d> const& newDynamicKeyPoints, size_t index);
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT void UpdateDynamicKeyPoint(DPoint3dCR newDynamicKeyPoint, size_t index);
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT void UpdateDynamicKeyPoints(bvector<DPoint3d> const& newDynamicKeyPoints, size_t index);
        void UpsertDynamicKeyPoint(DPoint3d newDynamicKeyPoint, size_t index);
        void UpsertDynamicKeyPoints(bvector<DPoint3d> const& newDynamicKeyPoints, size_t index);
    };

END_BUILDING_SHARED_NAMESPACE