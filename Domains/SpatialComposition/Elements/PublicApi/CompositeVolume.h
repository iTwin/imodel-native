#pragma once

#include "CompositeElement.h"

BEGIN_SPATIALCOMPOSITION_NAMESPACE

struct CompositeVolume : CompositeElement
    {
    protected:
        explicit CompositeVolume(CreateParams const& params) : CompositeElement(params) {}
    };

END_SPATIALCOMPOSITION_NAMESPACE
