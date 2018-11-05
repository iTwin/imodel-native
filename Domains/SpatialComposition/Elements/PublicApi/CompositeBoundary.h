#pragma once

#include "CompositeElement.h"

BEGIN_SPATIALCOMPOSITION_NAMESPACE

struct CompositeBoundary : CompositeElement
    {
    protected:
        explicit CompositeBoundary(CreateParams const& params) : CompositeElement(params) {}
    };

END_SPATIALCOMPOSITION_NAMESPACE
