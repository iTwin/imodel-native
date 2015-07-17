/*--------------------------------------------------------------------------------------+
|
|     $Source: Drainage/PublicAPI/drainage.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <TerrainModel\Core\bcDTMClass.h>
#include <TerrainModel\Core\IDTM.h>
#include <TerrainModel\Core\dtmdefs.h>
#include <TerrainModel\Core\dtm2dfns.h>

#if defined (CREATE_STATIC_LIBRARIES) || defined (TERRAINMODEL_STATICLIB)
  #define BENTLEYDTMDRAINAGE_EXPORT 
#elif defined (__BENTLEYTMDRAINAGE_BUILD__) || defined (__BENTLEYDTM_BUILD__)
  #define BENTLEYDTMDRAINAGE_EXPORT EXPORT_ATTRIBUTE
#else
  #define BENTLEYDTMDRAINAGE_EXPORT IMPORT_ATTRIBUTE
#endif

class DTMDrainageTables ; 

struct BcDTMDrainage
{
    private:
    BcDTMDrainage();

    public:
    BENTLEYDTMDRAINAGE_EXPORT static DTMStatusInt CreateDrainageTables(BcDTMP dtm, DTMDrainageTables*& dtmDrainageTablesPP);
    BENTLEYDTMDRAINAGE_EXPORT static DTMStatusInt TraceMaximumDescent(BcDTMP dtm, DTMDrainageTables* dtmDrainageTablesP, double minDepth, double x, double y, DTMFeatureCallback callBackFunctionP, void* userP) ;
    BENTLEYDTMDRAINAGE_EXPORT static DTMStatusInt TraceMaximumAscent(BcDTMP dtm, DTMDrainageTables* dtmDrainageTablesP, double minDepth, double x, double y, DTMFeatureCallback callBackFunctionP, void* userP) ;
    BENTLEYDTMDRAINAGE_EXPORT static DTMStatusInt DeterminePonds(BcDTMP dtm, DTMDrainageTables* dtmDrainageTablesP, DTMFeatureCallback callBackFunctionP, void *userP) ;
    BENTLEYDTMDRAINAGE_EXPORT static DTMStatusInt CalculatePondForPoint(BcDTMP dtm, double x, double y, double minDepth, bool& pondCalculatedP, double& pondElevationP, double& pondDepthP, double& pondAreaP, double& pondVolumeP, Bentley::TerrainModel::DTMDynamicFeatureArray& pondFeatures);
    BENTLEYDTMDRAINAGE_EXPORT static DTMStatusInt TraceCatchmentForPoint(BcDTMP dtm, DPoint3d tracePoint, double maxPondDepth, bool& catchmentDetermined, DPoint3d& sumpPoint, bvector<DPoint3d>& catchmentPts);
    BENTLEYDTMDRAINAGE_EXPORT static DTMStatusInt CreateDepressionDtm(BcDTMP dtmP, BC_DTM_OBJ*& depressionDtmPP, DTMFeatureCallback callBackFunctionP, void *userP);
    BENTLEYDTMDRAINAGE_EXPORT static DTMStatusInt CreateAndCheckDrainageTables(BcDTMP dtmP) ;
    BENTLEYDTMDRAINAGE_EXPORT static DTMStatusInt ReturnLowPoints(BcDTMP dtmP,DTMFeatureCallback loadFunctionP,DTMFenceParamsCR fence, void* userP ) ;
    BENTLEYDTMDRAINAGE_EXPORT static DTMStatusInt ReturnNoneFalseLowPoints(BcDTMP dtmP,double falseLowDepth,DTMFeatureCallback loadFunctionP,DTMFenceParamsCR fence, void* userP ) ;
    BENTLEYDTMDRAINAGE_EXPORT static DTMStatusInt ReturnHighPoints(BcDTMP dtmP,DTMFeatureCallback loadFunctionP,DTMFenceParamsCR fence, void* userP ) ;
    BENTLEYDTMDRAINAGE_EXPORT static DTMStatusInt ReturnSumpLines(BcDTMP dtmP,DTMFeatureCallback loadFunctionP,DTMFenceParamsCR fence,void* userP) ;
    BENTLEYDTMDRAINAGE_EXPORT static DTMStatusInt ReturnZeroSlopeSumpLines(BcDTMP dtmP,DTMFeatureCallback loadFunctionP,DTMFenceParamsCR fence,void* userP) ;
    BENTLEYDTMDRAINAGE_EXPORT static DTMStatusInt ReturnRidgeLines(BcDTMP dtmP,DTMFeatureCallback loadFunctionP,DTMFenceParamsCR fence,void* userP) ;
    BENTLEYDTMDRAINAGE_EXPORT static DTMStatusInt ReturnZeroSlopePolygons(BcDTMP dtmP,DTMFeatureCallback loadFunctionP,DTMFenceParamsCR fence,void* userP) ;
    BENTLEYDTMDRAINAGE_EXPORT static DTMStatusInt CreateRefinedDrainageDtm(BcDTMP dtmP,DTMFenceParamsCR fence,Bentley::TerrainModel::BcDTMPtr* refinedDtmPP) ;
    BENTLEYDTMDRAINAGE_EXPORT static DTMStatusInt ReturnCatchments(BcDTMP dtmP,class DTMDrainageTables* drainageTablesP,double falseLowDepth,bool refineOption, DTMFeatureCallback loadFunctionP,DTMFenceParamsCR  fence,void* userP) ;

};

