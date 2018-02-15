/*--------------------------------------------------------------------------------------+
|
|     $Source: GeometryManipulationStrategies/PublicApi/GeometryManipulationStrategyBase.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BUILDING_SHARED_NAMESPACE

#define GMS_V_SET_PROPERTY_TYPE(value_type) \
    virtual void _SetProperty(Utf8CP key, value_type const& value) {}

#define GMS_V_TRYGET_PROPERTY_TYPE(value_type) \
    virtual BentleyStatus _TryGetProperty(Utf8CP key, value_type& value) const { return BentleyStatus::ERROR; }

#define GMS_V_SET_TRYGET_PROPERTY_TYPE(value_type) \
    GMS_V_SET_PROPERTY_TYPE(value_type) \
    GMS_V_TRYGET_PROPERTY_TYPE(value_type)

#define GMS_SET_PROPERTY_TYPE(value_type) \
    GEOMETRYMANIPULATIONSTRATEGIES_EXPORT void SetProperty(Utf8CP key, value_type const& value);

#define GMS_TRYGET_PROPERTY_TYPE(value_type) \
    GEOMETRYMANIPULATIONSTRATEGIES_EXPORT BentleyStatus TryGetProperty(Utf8CP key, value_type& value) const;

#define GMS_SET_TRYGET_PROPERTY_TYPE(value_type) \
    GMS_SET_PROPERTY_TYPE(value_type) \
    GMS_TRYGET_PROPERTY_TYPE(value_type)

//=======================================================================================
// @bsiclass                                     Mindaugas.Butkus               01/2018
//=======================================================================================
struct GeometryManipulationStrategyProperty
    {
    protected:
        GeometryManipulationStrategyProperty() {}
        virtual ~GeometryManipulationStrategyProperty() {}
    };

//=======================================================================================
// @bsiclass                                     Mindaugas.Butkus               12/2017
//=======================================================================================
struct GeometryManipulationStrategyBase : RefCountedBase
    {
    private:
        GeometryManipulationStrategyBase() {}

        friend struct GeometryManipulationStrategy;
        friend struct GeometryPlacementStrategy;

    protected:
        virtual void _OnPropertySet(Utf8CP key) {}

        GMS_V_SET_TRYGET_PROPERTY_TYPE(bool)
        GMS_V_SET_TRYGET_PROPERTY_TYPE(int)
        GMS_V_SET_TRYGET_PROPERTY_TYPE(double)
        GMS_V_SET_TRYGET_PROPERTY_TYPE(DVec3d)
        GMS_V_SET_TRYGET_PROPERTY_TYPE(DPlane3d)
        GMS_V_SET_TRYGET_PROPERTY_TYPE(Dgn::DgnElementId)
        GMS_V_SET_TRYGET_PROPERTY_TYPE(Dgn::DgnElement)
        GMS_V_SET_TRYGET_PROPERTY_TYPE(Utf8String)
        GMS_V_SET_TRYGET_PROPERTY_TYPE(bvector<double>)
        GMS_V_SET_TRYGET_PROPERTY_TYPE(bvector<Utf8String>)
        GMS_V_SET_TRYGET_PROPERTY_TYPE(GeometryManipulationStrategyProperty)

        virtual bvector<DPoint3d> _GetKeyPoints() const = 0;

        virtual bool _IsDynamicKeyPointSet() const = 0;
        virtual void _ResetDynamicKeyPoint() = 0;

        virtual bool _IsComplete() const = 0;
        virtual bool _CanAcceptMorePoints() const = 0;

        virtual IGeometryPtr _FinishGeometry() const = 0;

    public:
        GMS_SET_TRYGET_PROPERTY_TYPE(bool)
        GMS_SET_TRYGET_PROPERTY_TYPE(int)
        GMS_SET_TRYGET_PROPERTY_TYPE(double)
        GMS_SET_TRYGET_PROPERTY_TYPE(DVec3d)
        GMS_SET_TRYGET_PROPERTY_TYPE(DPlane3d)
        GMS_SET_TRYGET_PROPERTY_TYPE(Dgn::DgnElementId)
        GMS_SET_TRYGET_PROPERTY_TYPE(Dgn::DgnElement)
        GMS_SET_TRYGET_PROPERTY_TYPE(Utf8String)
        GMS_SET_TRYGET_PROPERTY_TYPE(bvector<double>)
        GMS_SET_TRYGET_PROPERTY_TYPE(bvector<Utf8String>)
        GMS_SET_TRYGET_PROPERTY_TYPE(GeometryManipulationStrategyProperty)

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT bvector<DPoint3d> GetKeyPoints() const;

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT bool IsDynamicKeyPointSet() const;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT void ResetDynamicKeyPoint();

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT bool IsComplete() const;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT bool CanAcceptMorePoints() const;

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT IGeometryPtr FinishGeometry() const;
    };

END_BUILDING_SHARED_NAMESPACE