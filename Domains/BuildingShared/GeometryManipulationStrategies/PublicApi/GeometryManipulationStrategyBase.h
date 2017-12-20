/*--------------------------------------------------------------------------------------+
|
|     $Source: GeometryManipulationStrategies/PublicApi/GeometryManipulationStrategyBase.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BUILDING_SHARED_NAMESPACE

#define GMS_V_SET_PROPERTY_TYPE(value_type) \
    virtual void _SetProperty(Utf8CP key, value_type const& value) {}
#define GMS_V_TRYGET_PROPERTY_TYPE(value_type) \
    virtual bool _TryGetProperty(Utf8CP key, value_type& value) { return false; }
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
    protected:
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT GeometryManipulationStrategyBase();

        GMS_PROPERTY_TYPE(int)
        GMS_PROPERTY_TYPE(double)
        GMS_PROPERTY_TYPE(Dgn::DgnElementId)
        GMS_PROPERTY_TYPE(Dgn::DgnElement)
        GMS_PROPERTY_TYPE(Utf8String)
    };

END_BUILDING_SHARED_NAMESPACE