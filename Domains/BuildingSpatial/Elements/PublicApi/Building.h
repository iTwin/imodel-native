#pragma once

#include <BuildingSpatial/Domain/BuildingSpatialMacros.h>
BEGIN_BUILDINGSPATIAL_NAMESPACE
USING_NAMESPACE_SPATIALCOMPOSITION

struct Building : CompositeVolume
    {
    typedef CompositeVolume T_Super;
    protected:
        explicit Building(CreateParams const& params) : CompositeVolume(params) {}
    };

END_BUILDINGSPATIAL_NAMESPACE