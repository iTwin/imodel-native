/*--------------------------------------------------------------------------------------+
|
|     $Source: Drainage/PublicAPI/drainage.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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
    BENTLEYDTMDRAINAGE_EXPORT static DTMStatusInt CreateDrainageTables(TerrainModel::BcDTMP dtm, DTMDrainageTables*& dtmDrainageTablesPP);
    BENTLEYDTMDRAINAGE_EXPORT static DTMStatusInt TraceMaximumDescent(TerrainModel::BcDTMP dtm, DTMDrainageTables* dtmDrainageTablesP, double minDepth, double x, double y, DTMFeatureCallback callBackFunctionP, void* userP) ;
    BENTLEYDTMDRAINAGE_EXPORT static DTMStatusInt TraceMaximumAscent(TerrainModel::BcDTMP dtm, DTMDrainageTables* dtmDrainageTablesP, double minDepth, double x, double y, DTMFeatureCallback callBackFunctionP, void* userP) ;
    BENTLEYDTMDRAINAGE_EXPORT static DTMStatusInt DeterminePonds(TerrainModel::BcDTMP dtm, DTMDrainageTables* dtmDrainageTablesP, DTMFeatureCallback callBackFunctionP, void *userP) ;
    BENTLEYDTMDRAINAGE_EXPORT static DTMStatusInt CalculatePondForPoint(TerrainModel::BcDTMP dtm, double x, double y, double minDepth, bool& pondCalculatedP, double& pondElevationP, double& pondDepthP, double& pondAreaP, double& pondVolumeP, BENTLEY_NAMESPACE_NAME::TerrainModel::DTMDynamicFeatureArray& pondFeatures);
    BENTLEYDTMDRAINAGE_EXPORT static DTMStatusInt TraceCatchmentForPoint(TerrainModel::BcDTMP dtm, DPoint3d tracePoint, double maxPondDepth, bool& catchmentDetermined, DPoint3d& sumpPoint, bvector<DPoint3d>& catchmentPts);
    BENTLEYDTMDRAINAGE_EXPORT static DTMStatusInt CreateDepressionDtm(TerrainModel::BcDTMP dtmP, BC_DTM_OBJ*& depressionDtmPP, DTMFeatureCallback callBackFunctionP, void *userP);
    BENTLEYDTMDRAINAGE_EXPORT static DTMStatusInt CreateAndCheckDrainageTables(TerrainModel::BcDTMP dtmP) ;
    BENTLEYDTMDRAINAGE_EXPORT static DTMStatusInt ReturnLowPoints(TerrainModel::BcDTMP dtmP,DTMFeatureCallback loadFunctionP,TerrainModel::DTMFenceParamsCR fence, void* userP ) ;
    BENTLEYDTMDRAINAGE_EXPORT static DTMStatusInt ReturnNoneFalseLowPoints(TerrainModel::BcDTMP dtmP,double falseLowDepth,DTMFeatureCallback loadFunctionP,TerrainModel::DTMFenceParamsCR fence, void* userP ) ;
    BENTLEYDTMDRAINAGE_EXPORT static DTMStatusInt ReturnHighPoints(TerrainModel::BcDTMP dtmP,DTMFeatureCallback loadFunctionP,TerrainModel::DTMFenceParamsCR fence, void* userP ) ;
    BENTLEYDTMDRAINAGE_EXPORT static DTMStatusInt ReturnSumpLines(TerrainModel::BcDTMP dtmP,DTMFeatureCallback loadFunctionP,TerrainModel::DTMFenceParamsCR fence,void* userP) ;
    BENTLEYDTMDRAINAGE_EXPORT static DTMStatusInt ReturnZeroSlopeSumpLines(TerrainModel::BcDTMP dtmP,DTMFeatureCallback loadFunctionP,TerrainModel::DTMFenceParamsCR fence,void* userP) ;
    BENTLEYDTMDRAINAGE_EXPORT static DTMStatusInt ReturnRidgeLines(TerrainModel::BcDTMP dtmP,DTMFeatureCallback loadFunctionP,TerrainModel::DTMFenceParamsCR fence,void* userP) ;
    BENTLEYDTMDRAINAGE_EXPORT static DTMStatusInt ReturnZeroSlopePolygons(TerrainModel::BcDTMP dtmP,DTMFeatureCallback loadFunctionP,TerrainModel::DTMFenceParamsCR fence,void* userP) ;
    BENTLEYDTMDRAINAGE_EXPORT static DTMStatusInt CreateRefinedDrainageDtm(TerrainModel::BcDTMP dtmP,TerrainModel::DTMFenceParamsCR fence, TerrainModel::BcDTMPtr* refinedDtmPP) ;
    BENTLEYDTMDRAINAGE_EXPORT static DTMStatusInt ReturnCatchments(TerrainModel::BcDTMP dtmP,class DTMDrainageTables* drainageTablesP,double falseLowDepth,bool refineOption, DTMFeatureCallback loadFunctionP,TerrainModel::DTMFenceParamsCR  fence,void* userP) ;

};

