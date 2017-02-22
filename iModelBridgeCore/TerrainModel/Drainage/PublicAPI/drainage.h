/*--------------------------------------------------------------------------------------+
|
|     $Source: Drainage/PublicAPI/drainage.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#ifndef TERRAINMODEL_DRAINAGE_H
#define TERRAINMODEL_DRAINAGE_H
#include <TerrainModel\Core\bcDTMClass.h>
#include <TerrainModel\Core\IDTM.h>
#include <TerrainModel\Core\dtmdefs.h>
#include <TerrainModel\Core\dtm2dfns.h>

#include <iterator>

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

struct DtmPondDesignCriteria
    {
    DtmPondDesignCriteria(DTMPondDesignMethod designMethod, DPoint3dCP pointsP, int numPoints, double sideSlope, double freeBoard, DTMPondTarget pondTarget, double target) :
        points(&pointsP[0], &pointsP[numPoints-1]) , sideSlope(sideSlope), freeBoard(freeBoard), pondTarget(pondTarget), designMethod(designMethod), target(target)
        {
        }

    DtmPondDesignCriteria(DTMPondDesignMethod designMethod, const bvector<DPoint3d>& points, double sideSlope, double freeBoard, DTMPondTarget pondTarget, double targetElevation, double targetVolume) :
        points(points), sideSlope(sideSlope), freeBoard(freeBoard), pondTarget(pondTarget), designMethod(designMethod), target(target)
        {
        }

    BENTLEYDTMDRAINAGE_EXPORT DTMPondResult  CreatePond(Bentley::TerrainModel::BcDTMPtr& pondDTM);
        
    bvector<DPoint3d> points;
    double sideSlope = 1; // pondSlope
    double freeBoard = 0;
    DTMPondTarget pondTarget = DTMPondTarget::Volume;
    DTMPondDesignMethod designMethod = DTMPondDesignMethod::BottomUp;
    double target= 100000;

    double pondElevation = 0;
    double pondVolume = 0;
    DTMPondResult result = DTMPondResult::TargetObtained;

    bool isBerm = 0;
    double bermSlope = 0;
    double bermWidth = 0;
    bool isCrown = false;
    double crownWidth = 0;
    double cornerStrokeTolerance = 0;
    bool isBermFillOnly = false;
    BcDTMP fillTinP = nullptr;
    double fillSlope = 0;
    };
#endif