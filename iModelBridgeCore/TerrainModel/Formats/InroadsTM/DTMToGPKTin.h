//---------------------------------------------------------------------------+
// Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//---------------------------------------------------------------------------+

#pragma once

int aecDTM_convertDTMToGPKTin
(
 wchar_t *dtmFilename,
 wchar_t *gpkTinFilename,
 int (*bcdtmInRoads_importGeopakTinFromInroadsDtm)(double maxTriLength, long  numTinPoints, long  numTinFeatures, wchar_t  *geopakTinFileNameP, int (*setGeopakCallBackFunctionsP)())
 );

int aecDTM_exportToGPKTin
(
struct CIVdtmsrf *srfP,
wchar_t *gpkTinFilename,
int (*bcdtmInRoads_importGeopakTinFromInroadsDtm)(double maxTriLength, long  numTinPoints, long  numTinFeatures, wchar_t  *geopakTinFileNameP, int (*setGeopakCallBackFunctionsP)())
);