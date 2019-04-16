/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once

BEGIN_SPATIALCOMPOSITION_NAMESPACE

struct CompositeVolume : CompositeElement
    {
    typedef CompositeElement T_Super;
    protected:
        explicit CompositeVolume(CreateParams const& params) : CompositeElement(params) {}

    };

END_SPATIALCOMPOSITION_NAMESPACE
