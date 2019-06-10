/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <BuildingSpatial/Domain/BuildingSpatialMacros.h>
#include <SpatialComposition/Elements/SpatialCompositionElementsApi.h>

BEGIN_BUILDINGSPATIAL_NAMESPACE

struct Story : SPATIALCOMPOSITION_NAMESPACE_NAME::CompositeElement, SPATIALCOMPOSITION_NAMESPACE_NAME::ICompositeVolume
    {
    typedef SPATIALCOMPOSITION_NAMESPACE_NAME::CompositeElement T_Super;
    protected:
        explicit Story(CreateParams const& params) : T_Super(params) {}
    };

END_BUILDINGSPATIAL_NAMESPACE