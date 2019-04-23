#pragma once
#include <SpatialComposition/Domain/SpatialCompositionMacros.h>
#include "CompositeElement.h"

SPATIALCOMPOSITION_REFCOUNTED_PTR_AND_TYPEDEFS(CompositeBoundary)

BEGIN_SPATIALCOMPOSITION_NAMESPACE

struct CompositeBoundary : CompositeElement
    {
    typedef CompositeElement T_Super;
    protected:
        explicit CompositeBoundary(CreateParams const& params) : T_Super(params) {}
    };

END_SPATIALCOMPOSITION_NAMESPACE
