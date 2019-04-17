/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once

BEGIN_SPATIALCOMPOSITION_NAMESPACE

struct CompositeBoundary : CompositeElement
    {
    protected:
        explicit CompositeBoundary(CreateParams const& params) : CompositeElement(params) {}
    };

END_SPATIALCOMPOSITION_NAMESPACE
