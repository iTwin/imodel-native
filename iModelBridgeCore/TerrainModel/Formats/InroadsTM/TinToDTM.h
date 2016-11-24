//---------------------------------------------------------------------------+
// $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//---------------------------------------------------------------------------+

void aecDTM_setConvertTinToDTMFunction
(
    int (*pFunc)
    (
        void *tinP,
        int  (*tinStatsCallBackFunctionP)(long numRandomPoints,long numFeaturePoints,long numTriangles,long NumFeatures),
        int  (*tinRandomPointsCallBackFunctionP)(long pntIndex,double X,double Y,double Z),
        int  (*tinFeaturePointsCallBackFunctionP)(long pntIndex,double X,double Y,double Z),
        int  (*tinTrianglesCallBackFunctionP)(long trgIndex,long pntIndex1,long pntIndex2,long pntIndex3,long voidTriangle,long side1TrgIndex,long side2TrgIndex,long side3TrgIndex),
        int  (*tinFeaturesCallBackFunctionP)(long dtmFeatureType,__int64 dtmUsertag,__int64 dtmFeatureId,long *pointIndicesP,long numPointIndices)
    )
);

void aecDTM_setConvertTinToDTMFunctionStdcall
(
    void (__stdcall *pFunc)
    (
        void *tinP,
        int  (*tinStatsCallBackFunctionP)(long numRandomPoints,long numFeaturePoints,long numTriangles,long NumFeatures),
        int  (*tinRandomPointsCallBackFunctionP)(long pntIndex,double X,double Y,double Z),
        int  (*tinFeaturePointsCallBackFunctionP)(long pntIndex,double X,double Y,double Z),
        int  (*tinTrianglesCallBackFunctionP)(long trgIndex,long pntIndex1,long pntIndex2,long pntIndex3,long voidTriangle,long side1TrgIndex,long side2TrgIndex,long side3TrgIndex),
        int  (*tinFeaturesCallBackFunctionP)(long dtmFeatureType,__int64 dtmUsertag,__int64 dtmFeatureId,long *pointIndicesP,long numPointIndices)
    )
);

int aecDTM_convertTinToDTM
( 
    CIVdtmsrf **,
    void *,				// <= Pointer to tin
    WCharCP ,          // <= Surface name
    WCharCP ,          // <= Surface description
    int = 0,			// Update explorer
    int = 0				// Update surface rather than creating a new one
);

void aecDTM_clearCPFeatureNamesList();

void aecDTM_addCPFeature(__int64 featureID, LPWSTR featureName, LPWSTR featureDefinition);

void aecDTM_getCPFeatureInfoByFeatureId(__int64 featureID, const wchar_t* &featureName, const wchar_t* &featureDefinition);
