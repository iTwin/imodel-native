#pragma once

#include "CompositeElement.h"

BEGIN_SPATIALCOMPOSITION_NAMESPACE

struct CompositeVolume : CompositeElement
    {
    typedef CompositeElement T_Super;
    protected:
        explicit CompositeVolume(CreateParams const& params) : CompositeElement(params) {}

    };

END_SPATIALCOMPOSITION_NAMESPACE
