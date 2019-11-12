/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
//__BENTLEY_INTERNAL_ONLY__

#pragma once

///////// bcdtmEsri/////////
BENTLEYDTM_Private       int bcdtmFormatEsri_callBackFunction(DTMFeatureType dtmFeatureType,DTMUserTag userTag, DTMFeatureId dtmFeatureId,DPoint3d *featurePtsP,size_t numFeaturePts,void *userP) ;
BENTLEYDTMFORMATS_EXPORT        int bcdtmFormatEsri_exportEsriShapeFileDtmObject(BC_DTM_OBJ *dtmP,wchar_t *esriShapeFileP) ;
BENTLEYDTMFORMATS_EXPORT        int bcdtmFormatEsri_importEsriShapeFileDtmObject(BC_DTM_OBJ *dtmP,DTMFeatureType dtmFeatureType,wchar_t *esriShapeFileP) ;
