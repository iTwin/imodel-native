/*--------------------------------------------------------------------------------------+
|
|     $Source: Drainage/PublicAPI/drainage.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#ifndef TERRAINMODEL_DRAINAGE_H
#define TERRAINMODEL_DRAINAGE_H
#include <TerrainModel/Core/bcDTMClass.h>
#include <TerrainModel/Core/IDTM.h>
#include <TerrainModel/Core/DTMDefs.h>
#include <TerrainModel/Core/dtm2dfns.h>

#include <iterator>

#if defined (CREATE_STATIC_LIBRARIES) || defined (TERRAINMODEL_STATICLIB)
  #define BENTLEYDTMDRAINAGE_EXPORT
#elif defined (__BENTLEYTMDRAINAGE_BUILD__) || defined (__BENTLEYDTM_BUILD__)
  #define BENTLEYDTMDRAINAGE_EXPORT EXPORT_ATTRIBUTE
#else
  #define BENTLEYDTMDRAINAGE_EXPORT IMPORT_ATTRIBUTE
#endif

BEGIN_BENTLEY_TERRAINMODEL_NAMESPACE

class DTMDrainageTables ; 

enum class ZeroSlopeTraceOption
    {
    None = 0,
    TraceLastAngle = 1,
    Pond = 2,
    };

struct BcDTMDrainage
{
    private:
    BcDTMDrainage();

    public:
    BENTLEYDTMDRAINAGE_EXPORT static DTMStatusInt CreateDrainageTables(BENTLEYTERRAINMODEL_NAMESPACE_NAME::BcDTMP dtm, DTMDrainageTables*& dtmDrainageTablesPP);
    BENTLEYDTMDRAINAGE_EXPORT static DTMStatusInt TraceMaximumDescent(BENTLEYTERRAINMODEL_NAMESPACE_NAME::BcDTMP dtm, DTMDrainageTables* dtmDrainageTablesP, double minDepth, double x, double y, DTMFeatureCallback callBackFunctionP, void* userP) ;
    BENTLEYDTMDRAINAGE_EXPORT static DTMStatusInt TraceMaximumAscent(BENTLEYTERRAINMODEL_NAMESPACE_NAME::BcDTMP dtm, DTMDrainageTables* dtmDrainageTablesP, double minDepth, double x, double y, DTMFeatureCallback callBackFunctionP, void* userP) ;
    BENTLEYDTMDRAINAGE_EXPORT static DTMStatusInt DeterminePonds(BENTLEYTERRAINMODEL_NAMESPACE_NAME::BcDTMP dtm, DTMDrainageTables* dtmDrainageTablesP, DTMFeatureCallback callBackFunctionP, void *userP) ;
    BENTLEYDTMDRAINAGE_EXPORT static DTMStatusInt CalculatePondForPoint(BENTLEYTERRAINMODEL_NAMESPACE_NAME::BcDTMP dtm, double x, double y, double minDepth, bool& pondCalculatedP, double& pondElevationP, double& pondDepthP, double& pondAreaP, double& pondVolumeP, BENTLEY_NAMESPACE_NAME::TerrainModel::DTMDynamicFeatureArray& pondFeatures);
    BENTLEYDTMDRAINAGE_EXPORT static DTMStatusInt TraceCatchmentForPoint(BENTLEYTERRAINMODEL_NAMESPACE_NAME::BcDTMP dtm, DPoint3d tracePoint, double maxPondDepth, bool& catchmentDetermined, DPoint3d& sumpPoint, bvector<DPoint3d>& catchmentPts);
    BENTLEYDTMDRAINAGE_EXPORT static DTMStatusInt CreateDepressionDtm(BENTLEYTERRAINMODEL_NAMESPACE_NAME::BcDTMP dtmP, BC_DTM_OBJ*& depressionDtmPP, DTMFeatureCallback callBackFunctionP, void *userP);
    BENTLEYDTMDRAINAGE_EXPORT static DTMStatusInt CreateAndCheckDrainageTables(BENTLEYTERRAINMODEL_NAMESPACE_NAME::BcDTMP dtmP) ;
    BENTLEYDTMDRAINAGE_EXPORT static DTMStatusInt ReturnLowPoints(BENTLEYTERRAINMODEL_NAMESPACE_NAME::BcDTMP dtmP,DTMFeatureCallback loadFunctionP,BENTLEYTERRAINMODEL_NAMESPACE_NAME::DTMFenceParamsCR fence, void* userP ) ;
    BENTLEYDTMDRAINAGE_EXPORT static DTMStatusInt ReturnNoneFalseLowPoints(BENTLEYTERRAINMODEL_NAMESPACE_NAME::BcDTMP dtmP,double falseLowDepth,DTMFeatureCallback loadFunctionP,BENTLEYTERRAINMODEL_NAMESPACE_NAME::DTMFenceParamsCR fence, void* userP ) ;
    BENTLEYDTMDRAINAGE_EXPORT static DTMStatusInt ReturnHighPoints(BENTLEYTERRAINMODEL_NAMESPACE_NAME::BcDTMP dtmP,DTMFeatureCallback loadFunctionP,BENTLEYTERRAINMODEL_NAMESPACE_NAME::DTMFenceParamsCR fence, void* userP ) ;
    BENTLEYDTMDRAINAGE_EXPORT static DTMStatusInt ReturnSumpLines(BENTLEYTERRAINMODEL_NAMESPACE_NAME::BcDTMP dtmP,DTMFeatureCallback loadFunctionP,BENTLEYTERRAINMODEL_NAMESPACE_NAME::DTMFenceParamsCR fence,void* userP) ;
    BENTLEYDTMDRAINAGE_EXPORT static DTMStatusInt ReturnZeroSlopeSumpLines(BENTLEYTERRAINMODEL_NAMESPACE_NAME::BcDTMP dtmP,DTMFeatureCallback loadFunctionP,BENTLEYTERRAINMODEL_NAMESPACE_NAME::DTMFenceParamsCR fence,void* userP) ;
    BENTLEYDTMDRAINAGE_EXPORT static DTMStatusInt ReturnRidgeLines(BENTLEYTERRAINMODEL_NAMESPACE_NAME::BcDTMP dtmP,DTMFeatureCallback loadFunctionP,BENTLEYTERRAINMODEL_NAMESPACE_NAME::DTMFenceParamsCR fence,void* userP) ;
    BENTLEYDTMDRAINAGE_EXPORT static DTMStatusInt ReturnZeroSlopePolygons(BENTLEYTERRAINMODEL_NAMESPACE_NAME::BcDTMP dtmP,DTMFeatureCallback loadFunctionP,BENTLEYTERRAINMODEL_NAMESPACE_NAME::DTMFenceParamsCR fence,void* userP) ;
    BENTLEYDTMDRAINAGE_EXPORT static DTMStatusInt CreateRefinedDrainageDtm(BENTLEYTERRAINMODEL_NAMESPACE_NAME::BcDTMP dtmP,BENTLEYTERRAINMODEL_NAMESPACE_NAME::DTMFenceParamsCR fence, BENTLEY_NAMESPACE_NAME::TerrainModel::BcDTMPtr* refinedDtmPP) ;
    BENTLEYDTMDRAINAGE_EXPORT static DTMStatusInt ReturnCatchments(BENTLEYTERRAINMODEL_NAMESPACE_NAME::BcDTMP dtmP,class DTMDrainageTables* drainageTablesP,double falseLowDepth,bool refineOption, DTMFeatureCallback loadFunctionP,BENTLEYTERRAINMODEL_NAMESPACE_NAME::DTMFenceParamsCR  fence,void* userP) ;
};

struct DtmPondDesignCriteria
    {
	DtmPondDesignCriteria(DTMPondDesignMethod designMethod, DPoint3dCP pointsP, int numPoints, double sideSlope, double freeBoard, DTMPondTarget pondTarget, double target) :
		points(&pointsP[0], &pointsP[numPoints]), sideSlope(sideSlope), freeBoard(freeBoard), pondTarget(pondTarget), designMethod(designMethod), target(target)
		{
		}

    DtmPondDesignCriteria(DTMPondDesignMethod designMethod, DPoint3dCP pointsP, int numPoints, double sideSlope, double freeBoard, DTMPondTarget pondTarget, double target, BENTLEYTERRAINMODEL_NAMESPACE_NAME::BcDTMP fillTinP) :
        points(&pointsP[0], &pointsP[numPoints]) , sideSlope(sideSlope), freeBoard(freeBoard), pondTarget(pondTarget), designMethod(designMethod), target(target), fillTinP(fillTinP)
        {
        }

    DtmPondDesignCriteria(DTMPondDesignMethod designMethod, DPoint3dCP pointsP, double sideSlope, double freeBoard, DTMPondTarget pondTarget, double targetElevation, double target, BENTLEYTERRAINMODEL_NAMESPACE_NAME::BcDTMP fillTinP) :
        points(), sideSlope(sideSlope), freeBoard(freeBoard), pondTarget(pondTarget), designMethod(designMethod), target(target), fillTinP(fillTinP)
        {
        }

	DtmPondDesignCriteria(DTMPondDesignMethod designMethod, DPoint3dCP pointsP, int numPoints, double sideSlope, double freeBoard, DTMPondTarget pondTarget, double targetElevation, double target,
		bool isBerm, double bermSlope, double bermWidth, bool isCrown, double crownWidth, double cornerStrokeTolerance, bool isBermFillOnly, BENTLEYTERRAINMODEL_NAMESPACE_NAME::BcDTMP fillTinP)  :
 		points(&pointsP[0], &pointsP[numPoints]), sideSlope(sideSlope), freeBoard(freeBoard), pondTarget(pondTarget), designMethod(designMethod), target(target), isBerm(isBerm),
		bermSlope(bermSlope), bermWidth(bermWidth), isCrown(isCrown), crownWidth(crownWidth), cornerStrokeTolerance(cornerStrokeTolerance), isBermFillOnly(isBermFillOnly), fillTinP(fillTinP)
		{
		}

	DtmPondDesignCriteria()
		{
		}

    BENTLEYDTMDRAINAGE_EXPORT DTMPondResult  CreatePond(BENTLEY_NAMESPACE_NAME::TerrainModel::BcDTMPtr& pondDTM);
	BENTLEYDTMDRAINAGE_EXPORT DTMPondResult  CreatePond(double *outPondElevationP, double *outPondVolumeP, BENTLEY_NAMESPACE_NAME::TerrainModel::BcDTMPtr& pondDTM);

    bvector<DPoint3d> points;
    double sideSlope = 1; // pondSlope
    double freeBoard = 0;
    DTMPondTarget pondTarget = DTMPondTarget::Volume;
    DTMPondDesignMethod designMethod = DTMPondDesignMethod::BottomUp;
    double target = 100000;

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
    BENTLEYTERRAINMODEL_NAMESPACE_NAME::BcDTMP fillTinP = nullptr;
    double fillSlope = 0;
    };

END_BENTLEY_TERRAINMODEL_NAMESPACE

#endif
