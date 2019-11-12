//---------------------------------------------------------------------------------------------
// Copyright (c) Bentley Systems, Incorporated. All rights reserved.
// See COPYRIGHT.md in the repository root for full copyright notice.
//---------------------------------------------------------------------------------------------
#pragma once

#include "aecuti.h"

DPoint3d* hmgrVertices_allocate
    (
    DPoint3d** ppVerts,
    int num
    );

void hmgrVertices_free
    (
    DPoint3d* pVerts
    );
