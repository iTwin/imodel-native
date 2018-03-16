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
// A base class for custom property types to use with SetProperty and TryGetProperty
// methods of GeometryManipulationStrategyBase.
//
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
        GMS_V_SET_TRYGET_PROPERTY_TYPE(RotMatrix)
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
        //! Set/TryGet a bool property.
        GMS_SET_TRYGET_PROPERTY_TYPE(bool)
        //! Set/TryGet an int property.
        GMS_SET_TRYGET_PROPERTY_TYPE(int)
        //! Set/TryGet a double property.
        GMS_SET_TRYGET_PROPERTY_TYPE(double)
        //! Set/TryGet a DVec3d property.
        GMS_SET_TRYGET_PROPERTY_TYPE(DVec3d)
        //! Set/TryGet a DPlane3d property.
        GMS_SET_TRYGET_PROPERTY_TYPE(DPlane3d)
        //! Set/TryGet a RotMatrix property.
        GMS_SET_TRYGET_PROPERTY_TYPE(RotMatrix)
        //! Set/TryGet a Utf8String property.
        GMS_SET_TRYGET_PROPERTY_TYPE(Utf8String)
        //! Set/TryGet a bvector<double> property.
        GMS_SET_TRYGET_PROPERTY_TYPE(bvector<double>)
        //! Set/TryGet a bvector<Utf8String> property.
        GMS_SET_TRYGET_PROPERTY_TYPE(bvector<Utf8String>)
        //! Set/TryGet a custom type property.
        GMS_SET_TRYGET_PROPERTY_TYPE(GeometryManipulationStrategyProperty)

        //! Retrieve added key points. If dynamic key point is set - it contains that dynamic key point.
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT bvector<DPoint3d> GetKeyPoints() const;

        //! Check if the dynamic key point is set.
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT bool IsDynamicKeyPointSet() const;
        //! Restores the state of key point collection to the one before setting a dynamic key point.
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT void ResetDynamicKeyPoint();

        //! Check if geometry can be created from added key points.
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT bool IsComplete() const;
        //! Check if more key points can be added.
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT bool CanAcceptMorePoints() const;

        //! Create geometry from added key points.
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT IGeometryPtr FinishGeometry() const;
    };

END_BUILDING_SHARED_NAMESPACE