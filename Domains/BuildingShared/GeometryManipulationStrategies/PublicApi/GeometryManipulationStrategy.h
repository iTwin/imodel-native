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
// A base class for strategies that manipulate existing geometry.
// 
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

        virtual DPoint3d _AdjustPoint(DPoint3d source) const { return source; }

    public:
        //! Add key point to the back of the key point collection. Resets dynamic key points before doing that.
        //! @param[in] newKeyPoint The key point to be added.
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT void AppendKeyPoint(DPoint3dCR newKeyPoint);

        //! Add multiple key points to the back of the key point collection. Resets dynamic key points before doing that.
        //! @param[in] newKeyPoints Key points to be added.
        void AppendKeyPoints(bvector<DPoint3d> const& newKeyPoints);

        //! Insert key point somewhere in the key point collection. Resets dynamic key points before doing that.
        //! @param[in] newKeyPoint The key point to be inserted.
        //! @param[in] index       The index before which the new key point will be inserted.
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT void InsertKeyPoint(DPoint3dCR newKeyPoint, size_t index);

        //! Replace existing key point with a new key point. Resets dynamic key points before doing that.
        //! @param[in] newKeyPoint The key point that will replace existing key point.
        //! @param[in] index       The index of key point to replace.
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT void ReplaceKeyPoint(DPoint3dCR newKeyPoint, size_t index);

        //! Remove key point from the back of the key point collection. Resets dynamic key points before doing that.
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT void PopKeyPoint();

        //! Remove key point from the collection. Resets dynamic key points before doing that.
        //! @parma[in] index The index of key point to remove.
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT void RemoveKeyPoint(size_t index);
        
        //! Remove all key points.
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT void Clear();

        //! Add dynamic key point to the back of the key point collection. Resets current dynamic key points before doing that.
        //! @param[in] newDynamicKeyPoint The dynamic key point to be added.
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT void AppendDynamicKeyPoint(DPoint3dCR newDynamicKeyPoint);

        //! Add multiple dynamic key points to the back of the key point collection. Resets current dynamic key points before doing that.
        //! @param[in] newDynamicKeyPoints Dynamic key points to be added.
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT void AppendDynamicKeyPoints(bvector<DPoint3d> const& newDynamicKeyPoints);

        //! Insert dynamic key point somewhere in the key point collection. Resets current dynamic key points before doing that.
        //! @param[in] newDynamicKeyPoint The dynamic key point to be inserted.
        //! @param[in] index              The index before which the new dynamic key point will be inserted. 
        //!                               The index should be specified as if there were no dynamic key points.
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT void InsertDynamicKeyPoint(DPoint3dCR newDynamicKeyPoint, size_t index);

        //! Insert multiple dynamic key points somewhere in the key point collection. Resets current dynamic key points before doing that.
        //! @param[in] newDynamicKeyPoints Dynamic key points to be inserted.
        //! @param[in] index               The index before which new dynamic key points will be inserted. 
        //!                                The index should be specified as if there were no dynamic key points.
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT void InsertDynamicKeyPoints(bvector<DPoint3d> const& newDynamicKeyPoints, size_t index);

        //! Set dynamic key point somewhere in the key point collection. Resets current dynamic key points before doing that.
        //! @param[in] newDynamicKeyPoint The dynamic key point to be set.
        //! @param[in] index              The index at which the existing key point will be replaced with the new dynamic key point.
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT void UpdateDynamicKeyPoint(DPoint3dCR newDynamicKeyPoint, size_t index);

        //! Set dynamic key points somewhere in the key point collection. Resets current dynamic key points before doing that.
        //! @param[in] newDynamicKeyPoints Dynamic key points to be set.
        //! @param[in] index               The index at which to start replacing existing key points with new dynamic key points.
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT void UpdateDynamicKeyPoints(bvector<DPoint3d> const& newDynamicKeyPoints, size_t index);

        //! Update or append dynamic key point to the key point collection. Resets current dynamic key points before doing that.
        //! @param[in] newDynamicKeyPoint The new dynamic key point.
        //! @param[in] index              The index at which the existing key point will be replaced with the new dynamic key point. 
        //!                               If the given index is bigger than the index of last key point - the new dynamic key point 
        //!                               will be appended to the key point collection.
        void UpsertDynamicKeyPoint(DPoint3d newDynamicKeyPoint, size_t index);

        //! Update or append dynamic key points to the key point collection. Resets current dynamic key points before doing that.
        //! @param[in] newDynamicKeyPoints New dynamic key points.
        //! @param[in] index               The index at which to start replacing existing key points with new dynamic key points.
        //!                                If the given index is bigger than the index of last key point - new dynamic key points
        //!                                will be appended to the key point collection.
        void UpsertDynamicKeyPoints(bvector<DPoint3d> const& newDynamicKeyPoints, size_t index);

        //! Do adjustments to a point if needed. Could be used to project a point to the working plane if the working plane is set.
        //! @paran[in] source The point to adjust.
        DPoint3d AdjustPoint(DPoint3d source) const { return _AdjustPoint(source); }
    };

END_BUILDING_SHARED_NAMESPACE