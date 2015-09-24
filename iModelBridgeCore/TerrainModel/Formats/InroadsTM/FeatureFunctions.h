//---------------------------------------------------------------------------+
// $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//---------------------------------------------------------------------------+
#pragma once

#include "FeatureDefs.h"

#define FTR_STYLE_ALL                    0x0000

#define FTR_STYLE_PLAN_SEGS              0x0001
#define FTR_STYLE_PLAN_PNTS              0x0002
#define FTR_STYLE_PLAN_ANNOT             0x0004
#define FTR_STYLE_PLAN_TAG               0x0008

#define FTR_STYLE_PROFILE_SEGS           0x0010
#define FTR_STYLE_PROFILE_CROSSING_PNTS  0x0020
#define FTR_STYLE_PROFILE_PROJECTED_PNTS 0x0040
#define FTR_STYLE_PROFILE_ANNOT          0x0080

#define FTR_STYLE_XS_PNTS                0x0100
#define FTR_STYLE_XS_ANNOT               0x0200
#define FTR_STYLE_XS_PROJECTED_SEGS      0x0400
#define FTR_STYLE_XS_PROJECTED_PNTS      0x0800
#define FTR_STYLE_XS_COMPONENT           0x1000

//---------------------------------------------------------------------------
// Structures
//---------------------------------------------------------------------------

struct FeatureListItem
{
    wchar_t name[DTM_C_NAMSIZ];
    wchar_t style[DTM_C_NAMSIZ];
    wchar_t desc[DTM_C_NAMSIZ];
    struct CIVdtmsrf *srfP;
    BeSQLite::BeGuid  guid;
    BOOL bStyleExists;
    BOOL bPassFilter;
};


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

int aecFeature_findFeatureByLocation
(
    CFeature *featureP,                 /* (o) closest feature */
    DPoint3d *point,                    /* (i) point to search about */
    struct CIVdtmsrf *srfP              /* (i) surface to search (NULL to search all) */
);
