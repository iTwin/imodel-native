#pragma once

#include <BuildingSpatial/Domain/BuildingSpatialMacros.h>
BEGIN_BUILDINGSPATIAL_NAMESPACE
USING_NAMESPACE_SPATIALCOMPOSITION

struct Story : CompositeVolume
    {
    typedef CompositeVolume T_Super;
    protected:
        explicit Story(CreateParams const& params) : CompositeVolume(params) {}
    };

END_BUILDINGSPATIAL_NAMESPACE