//---------------------------------------------------------------------------------------------
// Copyright (c) Bentley Systems, Incorporated. All rights reserved.
// See COPYRIGHT.md in the repository root for full copyright notice.
//---------------------------------------------------------------------------------------------
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
    void aecDTM_setConvertTinToDTMFunction
        (
        int (*pFunc)
        (
        WCharCP tinP,
        int (*tinStatsCallBackFunctionP)(long numRandomPoints, long numFeaturePoints, long numTriangles, long NumFeatures),
        int (*tinRandomPointsCallBackFunctionP)(long pntIndex, double X, double Y, double Z),
        int (*tinFeaturePointsCallBackFunctionP)(long pntIndex, double X, double Y, double Z),
        int (*tinTrianglesCallBackFunctionP)(long trgIndex, long pntIndex1, long pntIndex2, long pntIndex3, long voidTriangle, long side1TrgIndex, long side2TrgIndex, long side3TrgIndex),
        int (*tinFeaturesCallBackFunctionP)(long dtmFeatureType, __int64 dtmUsertag, __int64 dtmFeatureId, long *pointIndicesP, long numPointIndices)
        )
        );
#ifndef CREATE_STATIC_LIBRARIES
    __declspec(dllexport)
#endif
        int inroadsTM_setConvertGPKTinToDTMFunction
        (
        int (*pFunc)
        (
        WCharCP tinFileNameP,
        int (*tinStatsCallBackFunctionP)(long numRandomPoints, long numFeaturePoints, long numTriangles, long NumFeatures),
        int (*tinRandomPointsCallBackFunctionP)(long pntIndex, double X, double Y, double Z),
        int (*tinFeaturePointsCallBackFunctionP)(long pntIndex, double X, double Y, double Z),
        int (*tinTrianglesCallBackFunctionP)(long trgIndex, long pntIndex1, long pntIndex2, long pntIndex3, long voidTriangle, long side1TrgIndex, long side2TrgIndex, long side3TrgIndex),
        int (*tinFeaturesCallBackFunctionP)(long dtmFeatureType, __int64 dtmUsertag, __int64 dtmFeatureId, long *pointIndicesP, long numPointIndices)
        )
        )
        {
        aecDTM_setConvertTinToDTMFunction (pFunc);
        return SUCCESS;
        }
    int aecDTM_convertTinToDTM
        (
        CIVdtmsrf **srfPP,
        void *tinP,
        WCharCP name,
        WCharCP description,
        int updateExplorer,
        int updateSurface
        );
#ifndef CREATE_STATIC_LIBRARIES
    __declspec(dllexport)
#endif
        int inroadsTM_convertGPKTinToDTM
        (
        WCharCP tinFileNameP,
        WCharCP dtmFileNameP,
        WCharCP name,
        WCharCP description,
        int dtmVersion
        )
        {
        struct CIVdtmsrf *srfP = NULL;
        aecDTM_projectSurfaceInitialize ();
        int sts = aecDTM_convertTinToDTM(&srfP, (void *)tinFileNameP, name, description, FALSE, FALSE);
        if (sts == SUCCESS)
            {
            if (srfP != NULL)
                {
                sts = aecDTM_save (srfP, dtmFileNameP, dtmVersion);
                aecDTM_deleteSurface (NULL, srfP, FALSE);
                }
            else
                sts = ERROR;
            }
        return sts;
        }
    int aecDTM_convertDTMToGPKTin
        (
        wchar_t *dtmFilename,
        wchar_t *gpkTinFilename,
        int (*bcdtmInRoads_importGeopakTinFromInroadsDtm)(double maxTriLength, long  numTinPoints, long  numTinFeatures, wchar_t  *geopakTinFileNameP, int (*setGeopakCallBackFunctionsP)())
        );
#ifndef CREATE_STATIC_LIBRARIES
    __declspec(dllexport)
#endif
        extern"C" int inroadsTM_convertDTMToGPKTin (wchar_t *dtmFileNameP, wchar_t *tinFileNameP, int (*bcdtmInRoads_importGeopakTinFromInroadsDtm)(double maxTriLength, long  numTinPoints, long  numTinFeatures, wchar_t  *geopakTinFileNameP, int (*setGeopakCallBackFunctionsP)()))
        {
        aecDTM_projectSurfaceInitialize ();
        return aecDTM_convertDTMToGPKTin (dtmFileNameP, tinFileNameP, bcdtmInRoads_importGeopakTinFromInroadsDtm);
        }
    void aecDTM_addCPFeature (__int64 featureID, LPWSTR featureName, LPWSTR featureDefinition);
    void aecDTM_clearCPFeatureNamesList ();
#ifndef CREATE_STATIC_LIBRARIES
__declspec(dllexport)
#endif
void inroadsTM_addCPFeature (__int64 featureID, LPWSTR featureName, LPWSTR featureDefinition)
    {
    aecDTM_addCPFeature (featureID, featureName, featureDefinition);
    }
#ifndef CREATE_STATIC_LIBRARIES
__declspec(dllexport)
#endif
void inroadsTM_clearCPFeatureNamesList ()
    {
    aecDTM_clearCPFeatureNamesList ();
    }