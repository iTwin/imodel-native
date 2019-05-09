/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <SpatialComposition/Domain/SpatialCompositionMacros.h>
#include "CompositeElement.h"

SPATIALCOMPOSITION_REFCOUNTED_PTR_AND_TYPEDEFS(CompositeVolume)

BEGIN_SPATIALCOMPOSITION_NAMESPACE

struct CompositeVolume : CompositeElement
    {
    typedef CompositeElement T_Super;
    protected:
        explicit CompositeVolume(CreateParams const& params) : T_Super(params) {}

    };

END_SPATIALCOMPOSITION_NAMESPACE
