/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/linestyle/LsElem.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
/*----------------------------------------------------------------------+
|                                                                       |
|   This file is the API to the type 95 level 8 elements.  Since the    |
|   definition is only known here, all use must go through this API.    |
|   This way consistancy can be guaranteed.                             |
|                                                                       |
+----------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>

#include    <DgnPlatform/DgnCore/LsLocal.h>

struct ElmIDMapEntry
    {
    LsLocation          location;
    DgnElementId        elmID;
    DgnElementP pEntryElm;
    } ;

#define CREATE_ELM_ID_ENTRY(entry, pLocation, eID, elm) \
                        (entry).location.SetFrom (pLocation); \
                        (entry).elmID   = (eID);  \
                        (entry).pEntryElm = (elm);

Public int      lstyleElm_appendLineStyleDefElm
(
DgnElementP pEntryElement,     /* => element to append*/
DgnDbP        dgnFile             /* => */
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Chuck.Kirschman 01/06
+---------------+---------------+---------------+---------------+---------------+------*/
double    BentleyApi::lsutil_getMasterToUorConversionFactor
(
DgnModelP        destDgnModel
)
    {
    double          scale = 1.0;
#ifdef DGNV10FORMAT_CHANGES_WIP
    bool            bSrcCreated = false;
    double          uorFactor = 1.0;
    DgnModelP    defaultDgnModel;

    scale = dgnModel_getUorPerMaster (destDgnModel->GetDgnModelP ());

    mdlDgnModel_getDefaultDgnModel (&defaultDgnModel, &bSrcCreated, destDgnModel, true, false, false);
    mdlDgnModel_getUorScaleBetweenModels (&uorFactor, destDgnModel, defaultDgnModel);
    scale *= uorFactor;

    if (bSrcCreated)
        mdlDgnModel_free (&defaultDgnModel);
#endif

    return scale;
    }

