//---------------------------------------------------------------------------+
// $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//---------------------------------------------------------------------------+
/*----------------------------------------------------------------------------*/
/* dtmutifnc.h                                       aec    08-Feb-1994       */
/*----------------------------------------------------------------------------*/
/* Function prototypes for various DTM utilities functions.                   */
/*----------------------------------------------------------------------------*/

#pragma once

/*----------------------------------------------------------------------------*/
/* Dependent include files                                                    */
/*----------------------------------------------------------------------------*/

#include <dtmstr.h>

/*----------------------------------------------------------------------------*/
/* Function prototypes                                                        */
/*----------------------------------------------------------------------------*/

void aecDTM_computeSurfaceRange
(
   struct CIVdtmsrf *srfP               /* => surface to use                   */
);

void aecDTM_computeSurfaceRangeForce
(
   struct CIVdtmsrf *srfP               /* => surface to use                   */
);

void aecDTM_computeSurfaceFileRange
(
   struct CIVdtmfil *filP               /* => surface file to use              */
);

void aecDTM_updateSurfaceRange
(
   struct CIVdtmsrf *srfP,              /* => surface to use                   */
   struct CIVdtmpnt *pntP               /* => point to check                   */
);

void aecDTM_setRangePointCoordinates
(
   DPoint3d *pP,                        /* <=> array of 4 coordinates          */
   DPoint3d *rangeP                     /*  => lower left & upper right coord. */
);

void aecDTM_generateUniqueSurfaceName
(
   wchar_t *newNameP,                   /* <= unique name                      */
   wchar_t *oldNameP,                   /* => name to check                    */
   struct CIVdtmprj *prjP               /* => DTM project (or NULL)            */
);

void aecDTM_applyMaximumTriangleLength
(
   struct CIVdtmsrf *srfP,              /* => surface we're using              */
   double sizeSquared,                  /* => length to check, squared.        */
   struct CIVdtmtin *tP                 /* => triangle to check                */
);

void aecDTM_applyMaximumTriangleLengthAndMarkRangeTriangles
(
   struct CIVdtmsrf *srfP,              /* => surface to use                   */
   double maxTriLength                  /* => max. tri. side length            */
);

int aecDTM_crossingCheck    /* <= TRUE if error                         */
(
   CIVdtmsrf *srfP,                     /* => surface to check                 */
   int option,                          /* => option DTM_C_INSIDE, etc.        */
   int (*intFuncP)(                     /* => intersect function               */
    wchar_t *name1,                     /* => segment one name                   */
    wchar_t *name2,                     /* => segment two name                   */
    struct CIVdtmpnt *,                /* => segment one                      */
    struct CIVdtmpnt *,                /* => segment two                      */
    void * ),                          /* => user data pointer                */
   void *mdlDescP,                      /* => MDL Descriptor                   */
   void *userDataP,                     /* => user data pointer                */
   BOOL bDisableLog = FALSE,            /* => disables the log file, optional    */
   CMapStringToString *ftrGuidsP = NULL /* => only process these features   */
);

int aecDTM_crossingCheckExterior  /* TRUE if error                 */
(
   CIVdtmsrf *srfP                      /* => surface to check                 */
);

int aecDTM_fixTolerance     /* <= TRUE if error                    */
(
   CIVdtmsrf *pSrf                      /* => surface to fix                   */
);

int aecDTM_generateFeaturesFromPoints
(
   struct CIVdtmsrf *                   /* => surface                          */
);

int aecDTM_generateFeatureStyles
(
   struct CIVdtmsrf *srf
);

void aecDTM_generateUniqueFeatureName
(
   wchar_t *newNameP,                 /* <= unique name                        */
   wchar_t *oldNameP,                 /* => name to check                      */
   struct CIVdtmsrf *srfP             /* => DTM surface (or NULL)              */
);

int aecDTM_featureFileFromPointFile
(
   long fileType
);

int aecDTM_pointFileFromFeatureFile
(
   long fileType
);

void aecDTM_setFeatureFlag
(
    struct CIVdtmftr *ftrP,
    struct CIVdtmsrf *srfP,
    byte *flagP
);

int aecDTM_getFeatureInfo
(
    CIVdtmftr *ftrP,                      /*  => feature                      */
    CIVdtmsrf *srfP,                      /*  => surface containing feature   */
    GUID      *guidP,                     /* <=  feature's GUID (or NULL)     */
    long      *typeP,                     /* <=  feature type (or NULL)       */
    wchar_t      nameP[DTM_C_NAMSIZ],     /* <=  feature name (or NULL)       */
    wchar_t      descP[DTM_C_NAMSIZ],     /* <=  feature description (or NULL)*/
    wchar_t      parentP[DTM_C_NAMSIZ],   /* <=  parent feature name (or NULL)*/
    CIVdtmpnt **pntsPP,                   /* <=  feature's points (or NULL)   */
    long      *numPntsP,                  /* <=  number of points (or NULL)   */
    double       *pntDensityP,            /* <=  point density (or NULL)      */
    CIVdtmstynam **stylesPP,              /* <=  feature's styles (or NULL)   */
    long      *numStylesP,                /* <=  number of styles (or NULL)   */
    CIVdtmpaynam **payItemsPP,            /* <=  feature's pay items (or NULL)*/
    long         *numPayItemsP,           /* <=  number of pay items (or NULL)*/
    byte   *flagP                      /* <=  feature's flag (or NULL)     */
);

int aecDTM_setFeatureInfo
(
    CIVdtmftr    *ftrP,                   /* <=> feature                      */
    CIVdtmsrf    *srfP,                   /*  => surface containing feature   */
    long         opt,                     /*  => operational information      */
    GUID         *guidP,                  /*  => feature's GUID (or NULL)     */
    long         *typeP,                  /*  => feature type (or NULL)       */
    wchar_t         nameP[DTM_C_NAMSIZ],  /*  => feature name (or NULL)       */
    wchar_t         descP[DTM_C_NAMSIZ],  /*  => feature description (or NULL)*/
    wchar_t         parentP[DTM_C_NAMSIZ],/*  => parent feature name (or NULL)*/
    CIVdtmpnt    *pntsP,                  /*  => feature's points (or NULL)   */
    long         numPnts,                 /*  => number of points (or NULL)   */
    double       *pntDensityP,            /*  => point density (or NULL)      */
    CIVdtmstynam *stylesP,                /*  => feature's styles (or NULL)   */
    long         numStyles,               /*  => number of styles (or NULL)   */
    CIVdtmpaynam *payItemsP,              /*  => pay items (or NULL)          */
    long         numPayItems,             /*  => # of pay items               */
    byte      *flagP,                  /*  => feature's flag (or NULL)     */
    BOOL         bReTin                   /*  => retriangulate (usually TRUE) */
);

long aecDTM_calcFeaturePntAllocSize  /* <=  Number to allocate     */
(
    long numPnts                                /* <=  Number of points       */
);

int aecDTM_densifyFeaturePoints
(
  long *numOutputPntsP,                /* <= # of points in list              */
  CIVdtmpnt **outputPntsPP,            /* <= list of computed points          */
  long numInputPnts,                   /* => # vertices in polyline           */
  CIVdtmpnt *inputPntsP,               /* => polyline vertices                */
  double interval                      /* => interval to use                  */
);
