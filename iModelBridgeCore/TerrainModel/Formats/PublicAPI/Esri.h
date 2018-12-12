/*--------------------------------------------------------------------------------------+
|
|     $Source: Formats/PublicAPI/Esri.h $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//__BENTLEY_INTERNAL_ONLY__

#pragma once

///////// bcdtmEsri/////////
BENTLEYDTM_Private       int bcdtmFormatEsri_callBackFunction(DTMFeatureType dtmFeatureType,DTMUserTag userTag, DTMFeatureId dtmFeatureId,DPoint3d *featurePtsP,size_t numFeaturePts,void *userP) ;
BENTLEYDTMFORMATS_EXPORT        int bcdtmFormatEsri_exportEsriShapeFileDtmObject(BC_DTM_OBJ *dtmP,wchar_t *esriShapeFileP) ;
BENTLEYDTMFORMATS_EXPORT        int bcdtmFormatEsri_importEsriShapeFileDtmObject(BC_DTM_OBJ *dtmP,DTMFeatureType dtmFeatureType,wchar_t *esriShapeFileP) ;
