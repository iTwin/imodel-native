//---------------------------------------------------------------------------+
// $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
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
    unsigned char *flgsP,
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
    unsigned char **pntFlgsPP,
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
    unsigned char *pntflgsP,
    long numPnts
);

int aecFeature_ftrpntsToDpnt3ds
(
    DPoint3d **dpnt3dsPP,
    unsigned char **pntflgsPP,
    CFeaturePnt *ftrPntsP,
    long numPnts
);

#include "FeatureClass.h"
