/*--------------------------------------------------------------------------------------+
|
|     $Source: Elements/PublicApi/Story.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <BuildingSpatial/Domain/BuildingSpatialMacros.h>
#include <SpatialComposition/Elements/SpatialCompositionElementsApi.h>

BEGIN_BUILDINGSPATIAL_NAMESPACE

struct Story : SPATIALCOMPOSITION_NAMESPACE_NAME::CompositeVolume
    {
    typedef SPATIALCOMPOSITION_NAMESPACE_NAME::CompositeVolume T_Super;
    protected:
        explicit Story(CreateParams const& params) : T_Super(params) {}
    };

END_BUILDINGSPATIAL_NAMESPACE