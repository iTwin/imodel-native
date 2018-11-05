#pragma once

BEGIN_SPATIALCOMPOSITION_NAMESPACE

struct CompositeElement : Dgn::SpatialLocationElement
    {
    protected:
        explicit CompositeElement(CreateParams const& params) : Dgn::SpatialLocationElement(params) {}
    };

END_SPATIALCOMPOSITION_NAMESPACE
