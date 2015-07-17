//---------------------------------------------------------------------------+
// $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//---------------------------------------------------------------------------+
#include "stdafx.h"
#include "inroadstm.h"

#ifndef CREATE_STATIC_LIBRARIES
__declspec(dllexport)
#endif
    int inroadsTM_load             /* <= TRUE if error                    */
    (
struct CIVdtmsrf **srfPP,            /* <= new, loaded surface              */
    int *fileWasTTN,                     /* <= TRUE if TTN load (or NULL)       */
struct CIVdtmprj *prjP,              /* => DTM project (or NULL)            */
    wchar_t *fileNameP                   /* => file to load                     */
    )
    {
    aecDTM_projectSurfaceInitialize ();
    return aecDTM_load (srfPP, fileWasTTN, prjP, fileNameP);
    }


#ifndef CREATE_STATIC_LIBRARIES
__declspec(dllexport)
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
    )
    {
    return aecDTM_sendAllFeatures (mdlDescP, srfP, opt, typmsk, usrfncP, datP);
    }


#ifndef CREATE_STATIC_LIBRARIES
__declspec(dllexport)
#endif
    int inroadsTM_sendAllTriangles  /* <= TRUE if error                    */
    (
    void *mdlDescP,                      /* => mdl app desc. (or NULL)          */
struct CIVdtmsrf *srfP,              /* => surface with triangles           */
    int opt,                             /* => options                          */
    int (*usrfncP)(void *,long,          /* => your function                    */
    DPoint3d *,struct CIVdtmtin *,unsigned long),
    void *datP                           /* => your data                        */
    )
    {
    return aecDTM_sendAllTriangles (mdlDescP, srfP, opt, usrfncP, datP);
    }


#ifndef CREATE_STATIC_LIBRARIES
__declspec(dllexport)
#endif
    int inroadsTM_getSurfacePerimeter    /* <= TRUE if error                    */
    (
    long *nvrtP,                         /* <= # of perimeter vertices          */
    DPoint3d **vrtPP,                    /* <= per. verts. (free)        */
struct CIVdtmsrf *srfP               /* => surface to use                   */
    )
    {
    return aecDTM_getSurfacePerimeter (nvrtP, vrtPP, srfP);
    }


#ifndef CREATE_STATIC_LIBRARIES
    __declspec(dllexport)
#endif
        int inroadsTM_deleteSurface    /* <= TRUE if error                    */
        (
    struct CIVdtmprj *prjP,              /* => DTM prj w/ srf (or NULL)         */
    struct CIVdtmsrf *srfP,              /* => surface to delete                */
        int emptyOnly                        /* => TRUE: just empty surface         */
        )
        {
        return aecDTM_deleteSurface (prjP, srfP, emptyOnly);
        }
