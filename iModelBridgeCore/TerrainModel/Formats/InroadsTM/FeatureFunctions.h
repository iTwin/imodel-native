//---------------------------------------------------------------------------+
// $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//---------------------------------------------------------------------------+
#pragma once

#include "FeatureDefs.h"

//---------------------------------------------------------------------------
// Typedefs
//---------------------------------------------------------------------------

typedef void * FeatureHandle;

//---------------------------------------------------------------------------
// Function Prototypes
//---------------------------------------------------------------------------

int aecFeature_create
(
    FeatureHandle *ftrHndlP
);

void aecFeature_destroy 
(
    FeatureHandle ftrHndl
);

int aecFeature_loadFromDTMExteriorBoundary
(
    FeatureHandle ftrHndl,
    struct CIVdtmsrf *srfP
);

int aecFeature_saveToDTM
(
    FeatureHandle ftrHndl,
    struct CIVdtmsrf *srfP,
    long opt,
    BOOL bReTin
);

int aecFeature_addPoints
(
    FeatureHandle ftrHndl,
    CFeaturePnt *ftrPntsP,
    DPoint3d *pntsP,
    byte *flgsP,
    long numPnts
);

int aecFeature_getPointCount
(
    FeatureHandle ftrHndl
);

int aecFeature_getPoints
(
    FeatureHandle ftrHndl,
    CFeaturePnt **ftrPntsPP,
    DPoint3d **pntsPP,
    byte **pntFlgsPP,
    long *numPntsP
);

int aecFeature_removePoints
(
    FeatureHandle ftrHndl,
    long index,
    long numPnts
);

int aecFeature_dpnt3dsToFtrpnts
(
    CFeaturePnt **ftrPntsPP,
    DPoint3d *dpnt3dsP,
    byte *pntflgsP,
    long numPnts
);

int aecFeature_ftrpntsToDpnt3ds
(
    DPoint3d **dpnt3dsPP,
    byte **pntflgsPP,
    CFeaturePnt *ftrPntsP,
    long numPnts
);

#include "FeatureClass.h"
