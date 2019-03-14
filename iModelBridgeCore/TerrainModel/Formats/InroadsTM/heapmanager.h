//---------------------------------------------------------------------------+
// $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//---------------------------------------------------------------------------+
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
