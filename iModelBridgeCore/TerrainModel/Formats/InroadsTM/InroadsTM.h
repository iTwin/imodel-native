//---------------------------------------------------------------------------+
// $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//---------------------------------------------------------------------------+

//-----------------------------------------------------------------------------
//
// MicroStation, InRoads, InRail and MDL are registered trademarks and 
// "MicroCSL" is a trademark of Bentley Systems, Inc.
//
// Limited permission is hereby granted to reproduce and modify this
// copyrighted material provided that the resulting code is used only in 
// conjunction with Bentley Systems products under the terms of the license 
// agreement provided therein, and that this notice is retained in its entirety 
// in any such reproduction or modification.
//
//-----------------------------------------------------------------------------
//
// RDLinkDTM.h -- SelectCad Surface SDK Declarations for C, C++
//
// This file contains only the Const, Typedef
// and Functions definitions for SelectCad Surface SDK.
//
//-----------------------------------------------------------------------------

#pragma once

//#include <Bentley\stg\guid.h>
#include "portable.h"
#include "dtmcon.h"
#include "txtsiz.h"
#include "dtmstr.h"


#ifndef CREATE_STATIC_LIBRARIES
#ifdef _CIVDTM
__declspec(dllexport)
#else
#endif
#endif
int inroadsTM_load             /* <= TRUE if error                    */
(
struct CIVdtmsrf **srfPP,            /* <= new, loaded surface              */
    int *fileWasTTN,                     /* <= TRUE if TTN load (or NULL)       */
struct CIVdtmprj *prjP,              /* => DTM project (or NULL)            */
    wchar_t *fileNameP                   /* => file to load                     */
    );

#ifndef CREATE_STATIC_LIBRARIES
#ifdef _CIVDTM
__declspec(dllexport)
#else
#endif
#endif
int inroadsTM_sendAllFeatures  /* <= TRUE if error                    */
(
void *mdlDescP,                      /* => mdl app desc. (or NULL)          */
struct CIVdtmsrf *srfP,              /* => surface with triangles           */
    int opt,                             /* => options                          */
    int typmsk,                          /* => point type (zero for all)        */
    int (*usrfncP)(void *,               /* => your function                    */
struct CIVdtmsrf *, int,
struct CIVdtmftr *),
    void *datP                           /* => your data                        */
    );


#ifndef CREATE_STATIC_LIBRARIES
#ifdef _CIVDTM
__declspec(dllexport)
#else
#endif
#endif
int inroadsTM_sendAllTriangles  /* <= TRUE if error                    */
(
void *mdlDescP,                      /* => mdl app desc. (or NULL)          */
struct CIVdtmsrf *srfP,              /* => surface with triangles           */
    int opt,                             /* => options                          */
    int (*usrfncP)(void *, long,          /* => your function                    */
    DPoint3d *, struct CIVdtmtin *, unsigned long),
    void *datP                           /* => your data                        */
    );

#ifndef CREATE_STATIC_LIBRARIES
#ifdef _CIVDTM
__declspec(dllexport)
#else
#endif
#endif
int inroadsTM_getSurfacePerimeter    /* <= TRUE if error                    */
(
long *nvrtP,                         /* <= # of perimeter vertices          */
DPoint3d **vrtPP,                    /* <= per. verts. (free)        */
struct CIVdtmsrf *srfP               /* => surface to use                   */
    );

#ifndef CREATE_STATIC_LIBRARIES
#ifdef _CIVDTM
__declspec(dllexport)
#else
#endif
#endif
int inroadsTM_deleteSurface    /* <= TRUE if error                    */
(
struct CIVdtmprj *prjP,              /* => DTM prj w/ srf (or NULL)         */
struct CIVdtmsrf *srfP,              /* => surface to delete                */
    int emptyOnly                        /* => TRUE: just empty surface         */
    );

