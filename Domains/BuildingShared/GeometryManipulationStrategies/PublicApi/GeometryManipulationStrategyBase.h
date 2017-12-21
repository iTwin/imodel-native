/*--------------------------------------------------------------------------------------+
|
|     $Source: GeometryManipulationStrategies/PublicApi/GeometryManipulationStrategyBase.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

BUILDING_SHARED_REFCOUNTED_PTR_AND_TYPEDEFS(GeometryManipulationStrategyBase)

BEGIN_BUILDING_SHARED_NAMESPACE

#define GMS_V_SET_PROPERTY_TYPE(value_type) \
    virtual void _SetProperty(Utf8CP key, value_type const& value) {}
#define GMS_V_TRYGET_PROPERTY_TYPE(value_type) \
    virtual BentleyStatus _TryGetProperty(Utf8CP key, value_type& value) { return BentleyStatus::ERROR; }
#define GMS_V_SET_TRYGET_PROPERTY_TYPE(value_type) \
    GMS_V_SET_PROPERTY_TYPE(value_type) \
    GMS_V_TRYGET_PROPERTY_TYPE(value_type)
#define GMS_SET_PROPERTY_TYPE(value_type) \
    GEOMETRYMANIPULATIONSTRATEGIES_EXPORT void SetProperty(Utf8CP key, value_type const& value);
#define GMS_TRYGET_PROPERTY_TYPE(value_type) \
    GEOMETRYMANIPULATIONSTRATEGIES_EXPORT bool TryGetProperty(Utf8CP key, value_type& value);
#define GMS_SET_TRYGET_PROPERTY_TYPE(value_type) \
    GMS_SET_PROPERTY_TYPE(value_type) \
    GMS_TRYGET_PROPERTY_TYPE(value_type)
#define GMS_PROPERTY_TYPE(value_type) \
    protected: \
        GMS_V_SET_TRYGET_PROPERTY_TYPE(value_type) \
    public: \
        GMS_SET_TRYGET_PROPERTY_TYPE(value_type)

//=======================================================================================
// @bsiclass                                     Mindaugas.Butkus               12/2017
//=======================================================================================
struct GeometryManipulationStrategyBase : RefCountedBase
    {
    enum DynamicKeyPointType
        {
        Insert = 0,
        Update
        };

    private:
        GeometryManipulationStrategyBase() {}

        friend struct GeometryManipulationStrategy;
        friend struct GeometryPlacementStrategy;

    protected:
        GMS_PROPERTY_TYPE(int)
        GMS_PROPERTY_TYPE(double)
        GMS_PROPERTY_TYPE(Dgn::DgnElementId)
        GMS_PROPERTY_TYPE(Dgn::DgnElement)
        GMS_PROPERTY_TYPE(Utf8String)

        virtual bvector<DPoint3d> const& _GetKeyPoints() const = 0;

        virtual bool _IsDynamicKeyPointSet() const = 0;
        virtual void _SetDynamicKeyPoint(DPoint3dCR newDynamicKeyPoint, size_t index, DynamicKeyPointType type) = 0;
        virtual void _ResetDynamicKeyPoint() = 0;

    public:
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT bvector<DPoint3d> const& GetKeyPoints() const;

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT bool IsDynamicKeyPointSet() const;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT void SetDynamicKeyPoint(DPoint3dCR newDynamicKeyPoint, size_t index, DynamicKeyPointType type);
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT void ResetDynamicKeyPoint();
    };

END_BUILDING_SHARED_NAMESPACE