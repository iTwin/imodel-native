//---------------------------------------------------------------------------+
// $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//---------------------------------------------------------------------------+
#include "stdafx.h"

#include "heapmanager.h"

DPoint3d *hmgrVertices_allocate
    (
    DPoint3d **ppVerts,
    int num
    )
    {
    DPoint3d *pVerts = (DPoint3d*) calloc ( 1, num * sizeof(DPoint3d) );
    if ( ppVerts )
        *ppVerts = pVerts;
    return ( pVerts );
    }

void hmgrVertices_free
    (
    DPoint3d *pVerts
    )
    {
    if ( pVerts )
        {
        free( pVerts );
        }
    }
