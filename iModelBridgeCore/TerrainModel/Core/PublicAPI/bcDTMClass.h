/*--------------------------------------------------------------------------------------+
|
|     $Source: Core/PublicAPI/bcDTMClass.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

//__PUBLISH_SECTION_START__
#ifndef __bcDTMClassH__
#define __bcDTMClassH__

#include <Bentley\Bentley.h>
#include <Bentley\RefCounted.h>
#include <TerrainModel\TerrainModel.h>
#include <TerrainModel/Core/IDTM.h>
#include <Geom/GeomApi.h>

//__PUBLISH_SECTION_END__
#ifdef __BENTLEYDTM_BUILD__
#define dc_zero 0
#define dc_1 1
#endif

/*----------------------------------------------------------------------+
| Include BCivil general header files                                   |
+----------------------------------------------------------------------*/
#include <string>
#include <list>
#include "bcDTM.h"
#include "bcdtmstream.h"

//__PUBLISH_SECTION_START__
BEGIN_BENTLEY_TERRAINMODEL_NAMESPACE

#pragma warning(push)
#pragma warning(disable:4263)
#pragma warning(disable:4264)
//__PUBLISH_SECTION_END__

/*----------------------------------------------------------------------+
| This template class must be exported, so we instanciate and export it |
+----------------------------------------------------------------------*/

/*----------------------------------------------------------------------+
| CONSTANT definitions                                                  |
+----------------------------------------------------------------------*/

/*----------------------------------------------------------------------+
| CLASS definitions                                                     |
+----------------------------------------------------------------------*/

typedef Bentley::TerrainModel::DTMFenceParams DTMFenceParams;


/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
struct DTMDynamicFeature
    {
    DTMFeatureType featureType;
    bvector<DPoint3d> featurePts;
    };

typedef bvector<DTMDynamicFeature> DTMDynamicFeatureArray;

/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
class DTMFeatureBuffer
    {
    public:
        struct DtmFeature
            {
            DTMFeatureType featureType;
            DTMUserTag   featureTag ;
            DTMFeatureId featureId;
            bvector<DPoint3d> featurePts;
            };
    private:
        bvector<DtmFeature> dtmFeatures;
    public:
        DTMFeatureBuffer ()
            {
            } ;

        ~DTMFeatureBuffer()
            {
            EmptyDtmFeatureBuffer() ;
            } ;

        BENTLEYDTM_EXPORT void EmptyDtmFeatureBuffer();
        BENTLEYDTM_EXPORT int  AddDtmFeatureToBuffer(DTMFeatureType featureType, DTMUserTag featureTag, DTMFeatureId featureId, DPoint3dP featurePtsP, int nPoint);
        BENTLEYDTM_EXPORT int  GetDtmDynamicFeaturesArray (DTMDynamicFeatureArray& array, TMTransformHelper* transformHelper);
    };

//__PUBLISH_SECTION_START__
struct DTMFeatureStatisticsInfo
    {
    Int64 numPoints;
    long numTinLines;
    long numTriangles;
    long numDtmFeatures;
    long numBreaks;
    long numContourLines;
    long numVoids;
    long numIslands;
    long numHoles;
    long numGroupSpots;
    bool hasHull;
    };

struct BcDTMVolumeAreaResult
    {
    double cutVolume;
    double fillVolume;
    double balanceVolume;
    double cutArea;
    double fillArea;
    double totalArea;
    };
/**
* @memo     class which defines a Digital Terrain Model handle
* @doc      class which defines a Digital Terrain Model handle
* @author   Sylvain Pucci --  15/01/02 -- sylvain.pucci@bentley.com
* @see
*/
struct BcDTM : Bentley::RefCounted<Bentley::TerrainModel::IDTM>
//__PUBLISH_SECTION_END__
    , Bentley::TerrainModel::IDTMDraping, Bentley::TerrainModel::IDTMDrainage, Bentley::TerrainModel::IDTMContouring
//__PUBLISH_SECTION_START__
    {
//__PUBLISH_SECTION_END__
//__PUBLISH_CLASS_VIRTUAL__
    private:
        BC_DTM_OBJ* _dtmHandleP;

        TMTransformHelperPtr _dtmTransformHelper;
        bool _readonly;
        bool _ownTinHandle;
        bool _ownTinHandleMembers;
//__PUBLISH_SECTION_START__
    public:
        static BENTLEYDTM_EXPORT BcDTMPtr Create ();
        static BENTLEYDTM_EXPORT BcDTMPtr Create (int iniPoint, int incPoint);
//__PUBLISH_SECTION_END__

        static BENTLEYDTM_EXPORT BcDTMPtr CreateFromDtmHandle (BC_DTM_OBJ& dtmHandleP);
        static BENTLEYDTM_EXPORT BcDTMPtr CreateFromDtmHandles
            (
            BC_DTM_OBJ* headerP,
            void *featureArraysP,
            void *pointArraysP,
            void *nodeArraysP,
            void *fListArraysP,
            void *cListArraysP
            );

        static BENTLEYDTM_EXPORT BcDTMPtr CreateFromMemoryBlock (const char* memoryBlockP, unsigned long memoryBlockSize) ;

        static BENTLEYDTM_EXPORT BcDTMPtr CreateFromStream (IBcDtmStreamR stream);

        //__PUBLISH_SECTION_START__
        static BENTLEYDTM_EXPORT BcDTMPtr CreateFromTinFile (WCharCP fileNameP);
        //__PUBLISH_SECTION_END__

        static BENTLEYDTM_EXPORT BcDTMPtr CreateFromTinFile (WCharCP fileNameP, double filePosition);

        static BENTLEYDTM_EXPORT BcDTMPtr CreateFromTinFile (int fileHandle, long filePosition);

        static BENTLEYDTM_EXPORT BcDTMPtr CreateFromGeopakDatFile (WCharCP fileNameP);

        static BENTLEYDTM_EXPORT BcDTMPtr CreateFromGeopakDatFile (WCharCP fileNameP, double p2pTol, double p2lTol, long edgeOption, double maxSide);

        static BENTLEYDTM_EXPORT BcDTMPtr CreateFromXyzFile (WCharCP fileNameP);

        static BENTLEYDTM_EXPORT BcDTMPtr CreateFromXyzFile (WCharCP fileNameP, double p2pTol, double p2lTol, long edgeOption, double maxSide) ;

        static BENTLEYDTM_EXPORT BcDTMPtr DesignPondToTargetVolumeOrElevation (long* pondFlag, double* pondElevation, double* pondVolume, DPoint3dP points, long numPoints, long perimeterOrInvert,
            long targetVolumeOrElevation, double targetVolume, double targetElevation, double sideSlope, double freeBoard);

    protected:
        BcDTM ();
        BcDTM (int initPoint, int incPoint);
        BcDTM (BC_DTM_OBJ& dtmHandleP, TMTransformHelperP transformHelper = nullptr, bool readonly = false);
        BcDTM(BC_DTM_OBJ* headerP, void *featureArraysP, void *pointArraysP, void *nodeArraysP, void *fListArraysP, void *cListArraysP);
        virtual ~BcDTM (void);

        // IDTM Implementation
        virtual Int64 _GetPointCount () override;
        virtual DTMStatusInt _GetRange (DRange3d& range) override;
        virtual BcDTMP _GetBcDTM () override;
        virtual Bentley::TerrainModel::IDTMDraping* _GetDTMDraping () override;
        virtual Bentley::TerrainModel::IDTMContouring* _GetDTMContouring () override;
        virtual Bentley::TerrainModel::IDTMDrainage* _GetDTMDrainage () override;
        virtual DTMStatusInt _GetBoundary (Bentley::TerrainModel::DTMPointArray& ret) override;
        virtual DTMStatusInt _CalculateSlopeArea (double& flatArea, double& slopeArea, const DPoint3d pts[], int numPoints) override;
        virtual DTMStatusInt _GetTransformDTM (Bentley::TerrainModel::DTMPtr& transformedDTM, TransformCR transformation) override;
        virtual bool _GetTransformation (TransformR transformation) override;
        // End IDTM Implementation

        // IDTMDraping Implementation
        virtual DTMStatusInt _DrapePoint
            (
            double      *elevation,
            double      *slope,
            double      *aspect,
            DPoint3d    triangle[3],
            int&         drapedType,
            const DPoint3d  &point
            ) override;
        virtual DTMStatusInt _DrapeLinear (Bentley::TerrainModel::DTMDrapedLinePtr& ret, DPoint3dCP pts, int numPoints) override;

        // End IDTMDraping Implementation

        // IDTMDrainage Implementation
        virtual DTMStatusInt _GetDescentTrace (Bentley::TerrainModel::DTMDrainageFeaturePtr& ret, DPoint3dCR pt, double maxPondDepth) override;
        virtual DTMStatusInt _GetAscentTrace (Bentley::TerrainModel::DTMDrainageFeaturePtr& ret, DPoint3dCR pt, double maxPondDepth) override;
        virtual DTMStatusInt _TraceCatchmentForPoint (Bentley::TerrainModel::DTMDrainageFeaturePtr& ret, DPoint3dCR pt, double maxPondDepth) override;
        // End IDTMDrainage Implementation

        // IDTMContouring Implementation
        virtual DTMStatusInt _ContourAtPoint (Bentley::TerrainModel::DTMPointArray& ret, DPoint3dCR pt, double contourInterval, DTMContourSmoothing smoothOption, double smoothFactor, int smoothDensity, DTMFenceParamsCR fence) override;
        virtual DTMStatusInt _ContourAtPoint (Bentley::TerrainModel::DTMPointArray& ret, DPoint3dCR pt, double contourInterval, DTMContourSmoothing smoothOption, double smoothFactor, int smoothDensity) override;
        // End IDTMContouring Implementation
        // Concrete functions
    public:
        BENTLEYDTM_EXPORT  DTMStatusInt GetMemoryUsed (size_t& memoryUsed);
        //__PUBLISH_SECTION_START__
        BENTLEYDTM_EXPORT  DTMState GetDTMState ();

        BENTLEYDTM_EXPORT  DTMStatusInt SetTriangulationParameters (double pointTol, double lineTol, long edgeOption, double maxSide);
        BENTLEYDTM_EXPORT  DTMStatusInt GetTriangulationParameters (double& pointTol, double& lineTol, long& edgeOption, double& maxSide);
        BENTLEYDTM_EXPORT  DTMStatusInt Triangulate ();
        BENTLEYDTM_EXPORT  DTMStatusInt CheckTriangulation ();
        //__PUBLISH_SECTION_END__

        BENTLEYDTM_EXPORT  DTMStatusInt TinFilterPoints (long filterOption, int reinsertOption, double zTolerance, long* numPointsBefore, long* numPointsAfter, double* filterReduction);
        BENTLEYDTM_EXPORT  DTMStatusInt TileFilterPoints (long minTilePts, long maxTileDivide, double tileLength, double zTolerance, long* numPointsBefore, long* numPointsAfter, double* filterReduction);

        BENTLEYDTM_EXPORT  DTMStatusInt TinFilterSinglePointPointFeatures (long filterOption, int reinsertOption, double zTolerance, long* numPointsBefore, long* numPointsAfter, double* filterReduction);
        BENTLEYDTM_EXPORT  DTMStatusInt TileFilterSinglePointPointFeatures (long minTilePts, long maxTileDivide, double tileLength, double zTolerance, long* numPointsBefore, long* numPointsAfter, double* filterReduction);

        BENTLEYDTM_EXPORT  DTMStatusInt EditorSelectDtmTinFeature (DTMFeatureType dtmFeatureType, DPoint3d pt, bool& featureFoundP, bvector<DPoint3d>& featurePts);
        BENTLEYDTM_EXPORT  DTMStatusInt EditorDeleteDtmTinFeature (long* result);

        BENTLEYDTM_EXPORT  DTMStatusInt BrowseFeaturesWithTinErrors (void* userP, DTMFeatureCallback callback);
        BENTLEYDTM_EXPORT  DTMStatusInt BrowseFeaturesWithUserTag (DTMUserTag userTag, void* userP, DTMFeatureCallback callback);
        BENTLEYDTM_EXPORT  DTMStatusInt BrowseFeaturesWithFeatureId (DTMFeatureId featureId, void* userP, DTMFeatureCallback callback);

        BENTLEYDTM_EXPORT  DTMStatusInt BrowseDuplicatePoints (void* userP, DTMDuplicatePointsCallback callback);
        BENTLEYDTM_EXPORT  DTMStatusInt BrowseCrossingFeatures (const DTMFeatureType featureList[], int numFeaturesList, void* userP, DTMCrossingFeaturesCallback callback);
        // BOTH OF THE METHODS BELOW CAN RETURN MORE THAN ONE INSTANCE OF A FEATURE - FOR EXAMPLE
        // A MERGE OPERATION CAN BREAK A SINGLE GUID FEATURE INTO 2 PIECES EACH RETAINING THEIR
        // ORIGINAL GUIDS
        // SO CHANGE THE NAME TO getFeatures and return an array
        BENTLEYDTM_EXPORT  DTMStatusInt GetFeatureById (BcDTMFeaturePtr& featurePP, DTMFeatureId identP);

        BENTLEYDTM_EXPORT  DTMStatusInt GetFeatureByUserTag (BcDTMFeaturePtr& featurePP, DTMUserTag userTag);

        BENTLEYDTM_EXPORT  DTMStatusInt GetFeatureEnumerator (BcDTMFeatureEnumeratorPtr& enumPP);
        BENTLEYDTM_EXPORT  DTMStatusInt ClipByPointString
            (
            BcDTMPtr&               clippedPP,
            DPoint3dCP              points,
            int                     nbPt,
            DTMClipOption           clippingMethod
            );

        BENTLEYDTM_EXPORT DTMStatusInt CreatePointStockPile (DPoint3d headCoordinates, double stockPileSlope, bool mergeOption, double& stockPileVolume, BcDTMPtr *stockPileTmPP, BcDTMPtr *mergedTmPP);
        BENTLEYDTM_EXPORT DTMStatusInt CreateAlignmentStockPile (DPoint3dCP headCoordinates, long numHeadCoordinates, double stockPileSlope, bool mergeOption, double& stockPileVolume, BcDTMPtr *stockPileTmPP, BcDTMPtr *mergedTmPP);

        //__PUBLISH_SECTION_START__
        BENTLEYDTM_EXPORT  DTMStatusInt DrapePoint
            (
            int&        drapedTypeP,
            DPoint3dR    pointP
            ) ;
        BENTLEYDTM_EXPORT  DTMStatusInt DrapePoint
            (
            double      *elevation,
            double      *slope,
            double      *aspect,
            DPoint3d    triangle[3],
            int&         drapedType,
            const DPoint3d  &point
            );


        //__PUBLISH_SECTION_END__

        BENTLEYDTM_EXPORT  DTMStatusInt SaveAsGeopakTinFile
            (
            WCharCP fileNameP
            );

        BENTLEYDTM_EXPORT  DTMStatusInt PopulateFromGeopakTinFile
            (
            WCharCP fileNameP
            );

        //__PUBLISH_SECTION_START__
        BENTLEYDTM_EXPORT  DTMStatusInt Save
            (
            WCharCP fileNameP
            );
        //__PUBLISH_SECTION_END__

        BENTLEYDTM_EXPORT  DTMStatusInt SaveAsGeopakDat
            (
            WCharCP fileNameP
            );

        BENTLEYDTM_EXPORT  DTMStatusInt SaveAsGeopakAsciiDat
            (
            WCharCP fileNameP,
            int numDecPts
            );

        BENTLEYDTM_EXPORT  DTMStatusInt SaveToStream
            (
            IBcDtmStreamR stream
            );

        BENTLEYDTM_EXPORT  DTMStatusInt CalculateSlopeArea
            (
            double          &areaP,
            DPoint3dCP      points,
            int             nbPt
            );
        BENTLEYDTM_EXPORT  DTMStatusInt CalculateSlopeArea
            (
            double          &areaP
            );
        BENTLEYDTM_EXPORT  DTMStatusInt CalculateSlopeArea
            (
            double          &flatAreaP,
            double          &slopeAreaP,
            DPoint3dCP      points,
            int             numPoints
            );
        //__PUBLISH_SECTION_START__
        BENTLEYDTM_EXPORT  DTMStatusInt Merge (BcDTMR toMergeP);
        //__PUBLISH_SECTION_END__
        BENTLEYDTM_EXPORT  DTMStatusInt MergeAdjacent (BcDTMR toMergeP);

        BENTLEYDTM_EXPORT  DTMStatusInt InterpolateDtmFeatureType
            (
            DTMFeatureType dtmFeatureType,                    // ==> Dtm Feature Type To Be Interpolated
            double  snapTolerance,                    // ==> Snap tolerance for interpolation purposes
            BcDTMR spotsP,                           // ==> 3D Points To Interpolate From
            BcDTMR intDtmP,                          // ==> DTM to Store Interpolated Features
            long   *numDtmFeaturesP,                  // <== Number Of Dtm Features
            long   *numDtmFeaturesInterpolatedP       // <== Number Of Dtm Features Interpolated
            ) ;

        BENTLEYDTM_EXPORT  DTMStatusInt RemoveNoneFeatureHullLines ();

        BENTLEYDTM_EXPORT  DTMStatusInt CalculatePond (double x, double y, bool& pondCalculatedP, double& pondElevationP, double& pondDepthP, double& pondAreaP, double& pondVolumeP, DTMDynamicFeatureArray& ponds);

        BENTLEYDTM_EXPORT  DTMStatusInt CalculatePond (double x, double y, double falseLowDepth, bool& pondCalculatedP, double& pondElevationP, double& pondDepthP, double& pondAreaP, double& pondVolumeP, DTMDynamicFeatureArray& ponds);

        BENTLEYDTM_EXPORT  DTMStatusInt TraceCatchmentForPoint (double x, double y, double maxPondDepth, bool& catchmentDetermined, DPoint3d& sumpPoint, bvector<DPoint3d>& catchmentPts);

        BENTLEYDTM_EXPORT  DTMStatusInt PointVisibility (bool& pointVisibleP, double eyeX, double eyeY, double eyeZ, double pntX, double pntY, double pntZ);

        BENTLEYDTM_EXPORT  DTMStatusInt LineVisibility (bool& lineVisibleP, double eyeX, double eyeY, double eyeZ, double X1, double Y1, double Z1, double X2, double Y2, double Z2, DTMDynamicFeatureArray& visibilityFeatures);

        BENTLEYDTM_EXPORT  DTMStatusInt BrowseTinPointsVisibility (double eyeX, double eyeY, double eyeZ, void *userP, DTMFeatureCallback callBackFunctP);

        BENTLEYDTM_EXPORT  DTMStatusInt BrowseTinLinesVisibility (double eyeX, double eyeY, double eyeZ, void *userP, DTMFeatureCallback callBackFunctP);

        BENTLEYDTM_EXPORT  DTMStatusInt BrowseRadialViewSheds (double eyeX, double eyeY, double eyeZ, long viewShedOption, long numberRadials, double radialIncrement, void *userP, DTMFeatureCallback callBackFunctP);

        BENTLEYDTM_EXPORT  DTMStatusInt BrowseRegionViewSheds (double eyeX, double eyeY, double eyeZ, void *userP, DTMFeatureCallback callBackFunctP);

        BENTLEYDTM_EXPORT  DTMStatusInt ClipToTinHull (DTMClipOption clipOption, DPoint3dCP featurePtsP, int numFeaturePts, bvector<DtmString>& clipSections);

        //__PUBLISH_SECTION_START__
        BENTLEYDTM_EXPORT  DTMStatusInt Append (BcDTMR appendDtmP);
        //__PUBLISH_SECTION_END__

        BENTLEYDTM_EXPORT  DTMStatusInt AppendWithUserTag (BcDTMR appendDtmP, DTMUserTag userTag);

        //__PUBLISH_SECTION_START__
        BENTLEYDTM_EXPORT  BcDTMPtr Clone ();
        //__PUBLISH_SECTION_END__

        BENTLEYDTM_EXPORT  BcDTMPtr Delta (BcDTMR toDeltaP, DPoint3dCP points, int numPoints);

        BENTLEYDTM_EXPORT  BcDTMPtr DeltaElevation (double elevation, DPoint3dCP points, int numPoints);

        BENTLEYDTM_EXPORT  DTMStatusInt OffsetDeltaElevation (double offset);

        BENTLEYDTM_EXPORT  BC_DTM_OBJ* GetTinHandle() const;

        //__PUBLISH_SECTION_START__
        BENTLEYDTM_EXPORT  DTMStatusInt DrapeLinearPoints
            (
            BcDTMDrapedLinePtr& drapedLinePP,
            DPoint3dCP          pointsP,
            const double*       distTabP,
            int                 nbPt
            );
        //__PUBLISH_SECTION_END__

        BENTLEYDTM_EXPORT  DTMStatusInt DrapeLinearPoints(bvector<DTMDrapePoint>& drapedPts, DPoint3dCP pointsP, int nbPt, bool getFeatures);


        BENTLEYDTM_EXPORT  DTMStatusInt ShotVector
            (
            double        *endSlopeP,
            double        *endAspectP,
            DPoint3d    endTriangle[3],
            int            *endDrapedTypeP,
            DPoint3d    *endPtP,
            DPoint3d    *startPtP,
            double      direction,
            double      slope
            );

        BENTLEYDTM_EXPORT  DTMStatusInt ComputePlanarPrismoidalVolume
            (
            BcDTMVolumeAreaResult& result,
            double              elevation,
            DPoint3dCP          points,
            int                 nbPt,
            VOLRANGETAB*        rangeTableP,
            int                 numRanges
            );

        BENTLEYDTM_EXPORT  DTMStatusInt CalculateCutFillVolume
            (
            BcDTMVolumeAreaResult& result,
            BcDTMR              otherDtmP,
            DPoint3dCP          points,
            int                 nbPt,
            VOLRANGETAB*        rangeTableP,
            int                 numRanges
            );

        BENTLEYDTM_EXPORT  DTMStatusInt CalculatePrismoidalVolumeToElevation
            (
            BcDTMVolumeAreaResult& result,
            DtmVectorString     *volumePolygonsP,
            double              elevation,
            DPoint3dCP          points,
            int                 nbPt,
            VOLRANGETAB*        rangeTableP,
            int                 numRanges
            ) ;

        BENTLEYDTM_EXPORT  DTMStatusInt CalculatePrismoidalVolumeToSurface
            (
            BcDTMVolumeAreaResult& result,
            DtmVectorString     *volumePolygonsP,
            BcDTMR              otherDtmP,
            DPoint3dCP          points,
            int                 nbPt,
            VOLRANGETAB*        rangeTableP,
            int                 numRanges
            ) ;

        BENTLEYDTM_EXPORT  DTMStatusInt CalculateGridVolumeToElevation
            (
            BcDTMVolumeAreaResult& result,
            long                &numCellsUsedP,
            double              &cellAreaP,
            DtmVectorString     *volumePolygonsP,
            long                numLatticePoints,
            double              elevation,
            DPoint3dCP          points,
            int                 nbPt,
            VOLRANGETAB*        rangeTableP,
            int                 numRanges
            ) ;

        BENTLEYDTM_EXPORT  DTMStatusInt CalculateGridVolumeToSurface
            (
            BcDTMVolumeAreaResult& result,
            long                &numCellsUsedP,
            double              &cellAreaP,
            DtmVectorString     *volumePolygonsP,
            BcDTMR              otherDtmP,
            long                numLatticePoints,
            DPoint3dCP          points,
            int                 nbPt,
            VOLRANGETAB*        rangeTableP,
            int                 numRanges
            ) ;

        BENTLEYDTM_EXPORT  DTMStatusInt CalculatePrismoidalVolumeBalanceToSurface
            (
            double              &fromAreaP,
            double              &toAreaP,
            double              &balanceP,
            DtmVectorString     *volumePolygonsP,
            BcDTMR              otherDtmP,
            DPoint3dCP          points,
            int                 nbPt
            ) ;

        //__PUBLISH_SECTION_START__
        BENTLEYDTM_EXPORT  DTMStatusInt CalculateFeatureStatistics (DTMFeatureStatisticsInfo& info) const;
        BENTLEYDTM_EXPORT  int GetTrianglesCount () const;
        //__PUBLISH_SECTION_END__

        BENTLEYDTM_EXPORT  DTMStatusInt BrowseSlopeIndicator
            (
            BcDTMR dtmP,
            double  majorInterval,
            double  minorInterval,
            void *userP,
            DTMBrowseSlopeIndicatorCallback callBackFunctP
            );

        BENTLEYDTM_EXPORT    DTMStatusInt BrowseDrainageFeatures
            (
            DTMFeatureType featureType,
            double        *minLowPointP,
            const DTMFenceParams& fence,
            void         *userP,
            DTMFeatureCallback callBackFunctP
            );


        BENTLEYDTM_EXPORT DTMStatusInt CalculateCatchments
            (
            bool        refineOption,
            double      falseLowDepth,
            const DTMFenceParams& fence,
            void        *userP,
            DTMFeatureCallback callBackFunctP
            ) ;


        BENTLEYDTM_EXPORT   DTMStatusInt BrowseTriangleMesh
            (
            long        maxTriangles ,
            const DTMFenceParams& fence,
            void         *userP,
            DTMTriangleMeshCallback callBackFunctP
            ) ;

        BENTLEYDTM_EXPORT     DTMStatusInt BrowseFeatures
            (
            DTMFeatureType featureType,
            long        maxSpots,
            void         *userP,
            DTMFeatureCallback callBackFunctP
            );

        BENTLEYDTM_EXPORT      DTMStatusInt BrowseFeatures
            (
            DTMFeatureType featureType,
            const DTMFenceParams& fence,
            long        maxSpots,
            void         *userP,
            DTMFeatureCallback callBackFunctP
            );


        BENTLEYDTM_EXPORT      DTMStatusInt BrowsePonds
            (
            void         *userP,
            DTMFeatureCallback callBackFunctP
            );

        BENTLEYDTM_EXPORT  DTMStatusInt BrowseSinglePointFeatures
            (
            DTMFeatureType featureType,
            double      *minDepthP,
            const DTMFenceParams& fence,
            long        *nPointP,
            void        *userP,
            DTMBrowseSinglePointFeatureCallback callBackFunctP
            );
        BENTLEYDTM_EXPORT  DTMStatusInt BrowseContours
            (
            DTMContourParamsCR contourParams,
            const DTMFenceParams& fence, 
            void        *userArgP,
            DTMFeatureCallback callBackFunctP
            );

        BENTLEYDTM_EXPORT  DTMStatusInt ContourAtPoint
            (
            double      x,
            double      y,
            double      contourInterval,
            DTMContourSmoothing smoothOption,
            double      smoothFactor,
            int         smoothDensity,
            const DTMFenceParams& fence,
            void        *userArgP,
            DTMFeatureCallback callBackFunctP
            );


        BENTLEYDTM_EXPORT  DTMStatusInt GetAscentTrace
            (
            double         minDepth,
            double         pX,
            double         pY,
            void         *userP,
            DTMFeatureCallback callBackFunctP
            );
        BENTLEYDTM_EXPORT  DTMStatusInt GetDescentTrace
            (
            double         minDepth,
            double         pX,
            double         pY,
            void         *userP,
            DTMFeatureCallback callBackFunctP
            );
        BENTLEYDTM_EXPORT  DTMStatusInt AnalyzeElevation
            (
            DRange1d     *tInterval,
            int         nInterval,
            bool        polygonized,
            const DTMFenceParams& fence,
            void         *userP,
            DTMFeatureCallback callBackFunctP
            );
        BENTLEYDTM_EXPORT  DTMStatusInt AnalyzeSlope
            (
            DRange1d     *tInterval,
            int         nInterval,
            bool        polygonized,
            const DTMFenceParams& fence,
            void         *userP,
            DTMFeatureCallback callBackFunctP
            );
        BENTLEYDTM_EXPORT  DTMStatusInt AnalyzeAspect
            (
            DRange1d     *tInterval,
            int         nInterval,
            bool        polygonized,
            const DTMFenceParams& fence,
            void         *userP,
            DTMFeatureCallback callBackFunctP
            );
        
        BENTLEYDTM_EXPORT  DTMStatusInt TracePath
            (
            double        pointX,
            double        pointY,
            double        slope,
            double        dist,
            void         *userP,
            DTMFeatureCallback callBackFunctP
            );
        BENTLEYDTM_EXPORT  DTMStatusInt TraceLine
            (
            double        pointX,
            double        pointY,
            double        slope,
            double        dist,
            void         *userP,
            DTMFeatureCallback callBackFunctP
            );
        BENTLEYDTM_EXPORT  void GetHandles
            (
            void **headerPP,
            int *nHeaderP,
            void ***featureArraysPP,
            int  *nFeatureArraysP,
            int  *featureArraysSize,
            int  *lastFeatureArraysSize,
            void ***pointArraysPP,
            int  *nPointArraysP,
            int  *pointArraysSize,
            int  *lastPointArraysSize,
            void ***nodeArraysPP,
            int  *nNodeArraysP,
            int  *nodeArraysSize,
            int  *lastNodeArraysSize,
            void ***cListArraysPP,
            int  *nCListArraysP,
            int  *cListArraysSize,
            int  *lastCListArraysSize,
            void ***fListArraysPP,
            int  *nFListArraysP,
            int  *fListArraySize,
            int  *lastFListArraySize
            );
        //BENTLEYDTM_EXPORT  BcDTMMeshPtr GetMesh
        //    (
        //    long firstCall,
        //    long maxMeshSize,
        //    DPoint3dCP fencePtsP,
        //    int numFencePts
        //    );
        //BENTLEYDTM_EXPORT  BcDTMEdgesPtr GetEdges
        //    (
        //    DPoint3dCP fencePtsP,
        //    int numFencePts
        //    );
        //BENTLEYDTM_EXPORT  void GetMesh
        //    (
        //    DPoint3dCP fencePtsP,
        //    int numFencePts,
        //    DPoint3dP* pointsPP,
        //    long *numIndices,
        //    long** triangleIndexPP
        //    );

        BENTLEYDTM_EXPORT  DTMStatusInt ConvertUnits
            (
            double xyFactor,
            double zFactor
            );

        BENTLEYDTM_EXPORT  DTMStatusInt TransformUsingCallback
            (
            DTMTransformPointsCallback callback,
            void* userP
            );

        //__PUBLISH_SECTION_START__
        BENTLEYDTM_EXPORT  DTMStatusInt Transform (TransformCR trsfMatP);
        //__PUBLISH_SECTION_END__

        BENTLEYDTM_EXPORT  DTMStatusInt PurgeDTM (unsigned int flags);

        BENTLEYDTM_EXPORT  DTMStatusInt GetPoint (long index, DPoint3d& pt);

        BENTLEYDTM_EXPORT  DTMStatusInt Clip (DPoint3dCP clipPolygonPtsP, int numClipPolygonPts, DTMClipOption clippingMethod);
        BENTLEYDTM_EXPORT  DTMStatusInt CopyToMemoryBlock (char **memoryBlockPP, unsigned long *memoryBlockSizeP);

        BENTLEYDTM_EXPORT  DTMStatusInt ConvertToDataState (void);

        BENTLEYDTM_EXPORT  DTMStatusInt SetPointMemoryAllocationParameters (int iniPointAllocation, int incPointAllocation);

        BENTLEYDTM_EXPORT  DTMStatusInt ReplaceFeaturePoints (DTMFeatureId dtmFeatureId, DPoint3d *pointsP, int numPoints);

        BENTLEYDTM_EXPORT  DTMStatusInt BulkDeleteFeaturesByUserTag (DTMUserTag *userTagP, int numUserTag);

        BENTLEYDTM_EXPORT  DTMStatusInt BulkDeleteFeaturesByFeatureId (DTMFeatureId *featureIdP, int numFeatureId);

        BENTLEYDTM_EXPORT  DTMStatusInt SetCleanUpOptions (DTMCleanupFlags option);

        BENTLEYDTM_EXPORT  DTMStatusInt GetCleanUpOptions (DTMCleanupFlags& option);

        BENTLEYDTM_EXPORT  DTMStatusInt SetMemoryAccess (DTMAccessMode accessMode);

        BENTLEYDTM_EXPORT DTMStatusInt GetReadOnlyDTM (Bentley::TerrainModel::BcDTMPtr& readonlyDTM);
        //__PUBLISH_SECTION_START__
        BENTLEYDTM_EXPORT DTMStatusInt GetLastModifiedTime (Int64& lastModifiedTime);

        BENTLEYDTM_EXPORT DTMStatusInt AddPoint (DPoint3dCR point);
        BENTLEYDTM_EXPORT DTMStatusInt AddPoints (DPoint3dCP pointsP, int nPoint);
        BENTLEYDTM_EXPORT DTMStatusInt AddPoints (const bvector<DPoint3d>& points)
            {
            return AddPoints (points.data (), (int)points.size ());
            }

        //__PUBLISH_SECTION_END__

        BENTLEYDTM_EXPORT DTMStatusInt AddPointFeature (DPoint3dCR point, DTMFeatureId* featureIdP);
        BENTLEYDTM_EXPORT DTMStatusInt AddPointFeature (DPoint3dCR point, DTMUserTag userTag, DTMFeatureId* idP);
        BENTLEYDTM_EXPORT DTMStatusInt AddPointFeature (DPoint3dCP ptsP, int numPts, DTMFeatureId* featureIdP);
        BENTLEYDTM_EXPORT DTMStatusInt AddPointFeature (DPoint3dCP ptsP, int numPts, DTMUserTag userTag, DTMFeatureId* idP);

        BENTLEYDTM_EXPORT DTMStatusInt AddLinearFeature (DTMFeatureType dtmFeatureType, DPoint3dCP ptsP, int numPts, DTMFeatureId* featureIdP);

        BENTLEYDTM_EXPORT DTMStatusInt AddLinearFeature (DTMFeatureType dtmFeatureType, DPoint3dCP ptsP, int numPts, DTMUserTag  userTag, DTMFeatureId* featureIdP);
        BENTLEYDTM_EXPORT DTMStatusInt DeleteFeatureById (DTMFeatureId  guID);
        BENTLEYDTM_EXPORT DTMStatusInt DeleteFeaturesByUserTag (DTMUserTag  userTag);
        BENTLEYDTM_EXPORT DTMStatusInt DeleteFeaturesByType (DTMFeatureType dtmfeat);
        BENTLEYDTM_EXPORT DTMStatusInt JoinFeatures (DTMFeatureType dtmFeatureType, int* nFeatures, int* nJoinedFeatures, double tol);
        BENTLEYDTM_EXPORT DTMStatusInt RemoveHull ();
        BENTLEYDTM_EXPORT bool IntersectVector (DPoint3dR interscetionPoint, DPoint3dCR startPt, DPoint3dCR endPoint);

        BENTLEYDTM_EXPORT TMTransformHelperP GetTransformHelper();


        BENTLEYDTM_EXPORT DTMStatusInt Clean ();
        BENTLEYDTM_EXPORT DTMStatusInt AddFeatureWithMultipleSegments (DTMFeatureType dtmFeatureType, const DtmVectorString& features, DTMUserTag   userTag, DTMFeatureId *featureIdP);
        BENTLEYDTM_EXPORT DTMStatusInt ReplaceFeatureWithMultipleSegments (const DtmVectorString& features, DTMFeatureId featureId);
        BENTLEYDTM_EXPORT DTMStatusInt BulkDeleteByFeatureType (DTMFeatureType dtmFeatureType);
        BENTLEYDTM_EXPORT bool GetProjectedPointOnDTM (DPoint3dR pointOnDTM, DMatrix4dCR w2vMap, DPoint3dCR testPoint);
        BENTLEYDTM_EXPORT DTMStatusInt FilterPoints (long numPointsToRemove, double percentageToRemove, long& pointsBefore, long& pointsAfter);

//__PUBLISH_SECTION_START__
    };

//__PUBLISH_SECTION_END__
#if !defined (ANDROID)
#define DG_INLINE inline
#else
#define DG_INLINE
#endif

    DG_INLINE BC_DTM_OBJ* BcDTM::GetTinHandle() const
        {
        return _dtmHandleP;
        };

    DG_INLINE TMTransformHelperP BcDTM::GetTransformHelper()
        {
        return _dtmTransformHelper.get();
        }

/**
* @memo     class which defines a Digital Terrain Model draped line point handle
* @doc      class which defines a Digital Terrain Model draped line point handle
* @author   Sylvain Pucci --  15/01/02 -- sylvain.pucci@bentley.com
* @see
*/
struct BcDTMDrapedLinePoint : RefCounted<Bentley::TerrainModel::IDTMDrapedLinePoint>
    {
    private:
        BcDTMDrapedLinePoint (DPoint3d pt, double distance, DTMDrapedLineCode code) :
            _pt (pt), _distance (distance), _code (code)
            {
            _breakLine = false;
            };

    protected:
        // IDTMDrapedLinePoint
        virtual DTMDrapedLineCode _GetCode () const override;
        virtual DTMStatusInt    _GetPointCoordinates (DPoint3d& coordP) const override;
        virtual double          _GetDistanceAlong () const override { return GetDistance (); };

        // BcDTMDrapedLinePoint
        virtual int             _GetUserTagCount () const { return (int)_features.size (); };
        virtual int             _GetFeatureIdCount () const { return (int)_features.size (); };

        virtual DTMFeatureType  _GetFeatureTypeAtIndex (int index) const { return _features[index].dtmFeatureType; };
        virtual int             _GetFeatureIndexAtIndex (int index) const { return _features[index].dtmFeatureIndex; };
        virtual DTMUserTag      _GetUserTagAtIndex (int index) const { return _features[index].dtmUserTag; };
        virtual DTMFeatureId    _GetFeatureIdAtIndex (int index) const { return _features[index].dtmFeatureId; };
        virtual int             _GetFeaturePriorPointAtIndex (int index) const { return _features[index].priorFeaturePoint; };
        virtual int             _GetFeatureNextPointAtIndex (int index) const { return _features[index].nextFeaturePoint; };

    public:
        //BENTLEYDTM_EXPORT static BcDTMDrapedLinePoint* Create ()
        //    {
        //    return new BcDTMDrapedLinePoint ();
        //    }
        BENTLEYDTM_EXPORT static BcDTMDrapedLinePoint* Create (DPoint3d pt, double distance, DTMDrapedLineCode code)
            {
            return new BcDTMDrapedLinePoint (pt, distance, code);
            };

        inline DTMDrapedLineCode GetCode () const { return _code; }
        inline DPoint3d     GetPoint () const { return _pt; };
        inline DTMFeatureId    GetBcDTMFeatureIdByIndex (int index) const { return _features[index].dtmFeatureId; };
        DTMUserTag        GetBcDTMUserTag () const;
        DTMFeatureId           GetBcDTMFeatureId () const;
        inline int          GetFeatureCount () const { return (int)_features.size (); };
        inline double       GetDistance () const { return _distance; };
#ifdef ToDo // Vancouver
        void                AddFeature (DTMFeatureType feature, DTMUserTag userTag, DTMFeatureId id);
#endif
        void                AddFeature (int dtmFeatureIndex, DTMFeatureType dtmFeatureType, DTMUserTag dtmFeatureUserTag, DTMFeatureId  dtmFeatureId, int priorFeaturePoint, int nextFeaturePoint);

        void SetPoint (DPoint3dCR value) { _pt = value; }
        void SetDistance (double value) { _distance = value; }
    public:
        BENTLEYDTM_EXPORT int GetUserTagCount () const;
        BENTLEYDTM_EXPORT int GetFeatureIdCount () const;

        BENTLEYDTM_EXPORT int GetFeatureIndexAtIndex (int index) const ;
        BENTLEYDTM_EXPORT DTMFeatureType GetFeatureTypeAtIndex (int index) const ;
        BENTLEYDTM_EXPORT DTMUserTag GetUserTagAtIndex (int index) const;
        BENTLEYDTM_EXPORT DTMFeatureId GetFeatureIdAtIndex (int index) const;
        BENTLEYDTM_EXPORT int GetFeaturePriorPointAtIndex (int index) const ;
        BENTLEYDTM_EXPORT int GetFeatureNextPointAtIndex (int index) const ;

    private:
        DPoint3d            _pt;
        double              _distance;
        DTMDrapedLineCode   _code;
        bool             _breakLine;
        struct PtFeature
            {
            //ULong            feature;
            //DTMUserTag     userTag;
            //DTMFeatureId      guId;
            int              dtmFeatureIndex;    // Index To Feature In DTM Feature Table
            DTMFeatureType   dtmFeatureType;     // DTM Feature Type
            DTMUserTag     dtmUserTag;         // DTM User Tag
            DTMFeatureId   dtmFeatureId;        // DTM Feature Id
            int              priorFeaturePoint;  // Index To DTM Feature Point Prior To Drape Point
            int              nextFeaturePoint;   // Index To DTM Feature Point After Drape Point
            };
        bvector<PtFeature>   _features;

    };

/**
* @memo     class which defines a Digital Terrain Model draped line handle
* @doc      class which defines a Digital Terrain Model draped line handle
* @author   Sylvain Pucci --  15/01/02 -- sylvain.pucci@bentley.com
* @see
*/
struct BcDTMDrapedLine : public RefCounted<Bentley::TerrainModel::IDTMDrapedLine>
    {
    // => Type of defintion for a draped line
   // ===================================== 
    enum class DTMDrapedLineFlag
        {
        // Void and outside parts
        Void = 0,
        // In triangulation parts
        Tin = 1,
        // Breakline points only
        Breakline = 2,
        // Specific points only
        SpecificPoints = 3,
        };

    public:
        static BcDTMDrapedLine* Create (bvector<RefCountedPtr<BcDTMDrapedLinePoint>> &drapedPoints)
            {
            return new BcDTMDrapedLine (drapedPoints);
            }
        static BcDTMDrapedLine* Create (bvector<RefCountedPtr<BcDTMDrapedLinePoint>> &drapedPoints, IRefCounted* linearElP)
            {
            return new BcDTMDrapedLine (drapedPoints, linearElP);
            }
    private:
        BcDTMDrapedLine (bvector<RefCountedPtr<BcDTMDrapedLinePoint>> &drapedPoints);
        BcDTMDrapedLine (bvector<RefCountedPtr<BcDTMDrapedLinePoint>> &drapedPoints, IRefCounted* linearElP);
        virtual ~BcDTMDrapedLine ();
        bvector<RefCountedPtr<BcDTMDrapedLinePoint>>& GetDrapedPoints ()
            {
            return _drapedPoints;
            }
    protected:

        // IDTMDrapedLine
        virtual DTMStatusInt _GetPointByIndex (Bentley::TerrainModel::DTMDrapedLinePointPtr& ret, unsigned int index) const override;
        virtual DTMStatusInt _GetPointByIndex (DPoint3dR ptP, double *distanceP, DTMDrapedLineCode *codeP, unsigned int index) const override;
        virtual unsigned int _GetPointCount () const override;

        // BcDTMDrapedLine
        virtual Bentley::TerrainModel::IDTMDrapedLine* _GetIDTMDrapedLine ()
            {
            return this;
            }
        virtual IRefCounted* _GetIRefCounted ()
            {
            return _linearElP.get ();
            }

        virtual void _SetIRefCounted (IRefCounted* obj)
            {
            _linearElP = obj;
            }

        virtual DTMStatusInt _GetProfileSections
            (
            DPoint3d            **xyzPointsPP,
            DPoint3d            **profilePointsP,
            DTMDrapedLineCode   **tabCodeP,
            int                 **profileEndIndexP,
            int                 *nSectionP,
            DTMDrapedLineFlag   flagPt,
            double              abcArray[],
            int                 nbAbc
            );

        virtual DTMStatusInt _GetBreakLinePoints
            (
            bool       selNoFeature,
            /* => if TRUE gets alla point, otherwise  */
            /*     gets only points with feature      */
            DPoint3dP     xyTabP[],     /* <= or NULL                             */
            DPoint3dP     szTabP[],     /* <= or NULL                             */
            DTMUserTag *userTagTabP[],    // <= Feature point table
            DTMFeatureId    *guidTabP[],       // <= Feature point table
            int          *nPtP          /* <= number of point                     */
            );

        virtual DTMStatusInt _GetPointByIndex
            (
            BcDTMDrapedLinePointPtr&  drapedPointPP,
            int                     index
            );

        virtual DTMStatusInt _GetPointByIndex
            (
            DPoint3d                *ptP,
            double                  *distanceP,
            DTMDrapedLineCode       *codeP,
            int                     index
            );

        virtual bool _IsPartiallyOnDTM ();

    public:
        BENTLEYDTM_EXPORT Bentley::TerrainModel::IDTMDrapedLine* GetIDTMDrapedLine();
        BENTLEYDTM_EXPORT IRefCounted* GetIRefCounted ();
        BENTLEYDTM_EXPORT void SetIRefCounted (IRefCounted* obj);

        BENTLEYDTM_EXPORT DTMStatusInt GetProfileSections
            (
            DPoint3d            **xyzPointsPP,
            DPoint3d            **profilePointsP,
            DTMDrapedLineCode   **tabCodeP,
            int                 **profileEndIndexP,
            int                 *nSectionP,
            DTMDrapedLineFlag   flagPt,
            double              abcArray[],
            int                 nbAbc
            );

        BENTLEYDTM_EXPORT DTMStatusInt GetProfileSections
            (
            DPoint3d            **profilePointsP,
            DTMDrapedLineCode   **tabCodeP,
            int                 **profileEndIndexP,
            int                 *nSectionP,
            DTMDrapedLineFlag   flagPt,
            double              abcArray[],
            int                 nbAbc
            );

        BENTLEYDTM_EXPORT DTMStatusInt GetBreakLinePoints
            (
            bool      selNoFeature,
            /* => if TRUE gets alla point, otherwise  */
            /*     gets only points with feature      */
            DPoint3dP     xyTabP[],     /* <= or NULL                             */
            DPoint3dP     szTabP[],     /* <= or NULL                             */
            DTMUserTag *userTagTabP[],    // <= Feature point table
            DTMFeatureId    *guidTabP[],       // <= Feature point table
            int          *nPtP          /* <= number of point                     */
            );

        BENTLEYDTM_EXPORT DTMStatusInt GetPointByIndex
            (
            BcDTMDrapedLinePointPtr&  drapedPointPP,
            int                     index
            );

        BENTLEYDTM_EXPORT DTMStatusInt GetPointByIndex
            (
            DPoint3d                *ptP,
            double                  *distanceP,
            DTMDrapedLineCode       *codeP,
            int                     index
            );

        BENTLEYDTM_EXPORT int GetPointCount
            (
            );

        BENTLEYDTM_EXPORT bool IsPartiallyOnDTM ();

    private:
        bvector<RefCountedPtr<BcDTMDrapedLinePoint>>     _drapedPoints;
        Bentley::RefCountedPtr<Bentley::IRefCounted> _linearElP;
        int                     _containsVoid;

        DTMStatusInt AddPointInTable (int index, bvector<RefCountedPtr<BcDTMDrapedLinePoint>>& selPoint);
        DTMStatusInt AddPointInTableByZInterpolation (int index, double abcissa, bvector<RefCountedPtr<BcDTMDrapedLinePoint>>& selPoint);
    };

/**
* @memo     Pure virtual class which defines a Digital Terrain Feature
* @doc      Pure virtual class which defines a Digital Terrain Feature
* @author   Sylvain Pucci --  15/01/02 -- sylvain.pucci@bentley.com
* @see
*/
struct BcDTMFeature: RefCountedBase
    {
    protected: BcDTMFeature (DTMFeatureType featureType, DTMUserTag userTag, DTMFeatureId featureId) :
               m_ident (featureId), m_featureType (featureType), m_userTag (userTag)
        {
        }

    public:
        BENTLEYDTM_EXPORT const DTMFeatureId& GetIdent ()
            {
            return m_ident;
            }
        BENTLEYDTM_EXPORT const DTMUserTag GetUserTag ()
            {
            return m_userTag;
            }
        BENTLEYDTM_EXPORT DTMFeatureType GetFeatureType ()
            {
            return m_featureType;
            }
        virtual BcDTMLinearFeature* AsLinear () = 0;
        virtual BcDTMComplexLinearFeature* AsComplexLinear () = 0;
        virtual BcDTMSpot* AsSpot () = 0;

    //protected:
    //    virtual int GetDefinitionPoints (DPoint3dP& pointsPP, int& nbPt);

    //    virtual void ComputeTangentsField (DPoint3dP  secStartP, double* secAngP) const
    //        {
    //        }

    private:
        DTMFeatureId    m_ident;
        DTMFeatureType  m_featureType;
        DTMUserTag      m_userTag;
    };

/**
* @memo     Pure virtual class which defines a Digital Terrain Linear Feature
* @doc      Pure virtual class which defines a Digital Terrain Linear Feature
* @author   Sylvain Pucci --  15/01/02 -- sylvain.pucci@bentley.com
* @see
*/
struct BcDTMLinearFeature: BcDTMFeature
    {
    friend BcDTM;
    friend BcDTMFeatureEnumerator;
    protected: BcDTMLinearFeature (DTMFeatureType feature, DTMUserTag userTagP, DTMFeatureId featureIdP, DPoint3dCP point, int nbPt);
    public: static BcDTMLinearFeaturePtr Create (DTMFeatureType feature, DTMUserTag userTagP, DTMFeatureId guidP, DPoint3dCP point, int nbPt);
    virtual BcDTMLinearFeature* AsLinear () { return this; };
    virtual BcDTMComplexLinearFeature* AsComplexLinear () { return nullptr; };
    virtual BcDTMSpot* AsSpot () { return nullptr; };
    BENTLEYDTM_EXPORT int GetDefinitionPoints (DPoint3dP& pointsPP, int& nbPt);
    BENTLEYDTM_EXPORT DtmString& GetLinearEl ()
        {
        return m_linearElP;
        }

    private:
        DtmString       m_linearElP;
    };

/**
* @memo     Pure virtual class which defines a Digital Terrain Linear Feature
* @doc      Pure virtual class which defines a Digital Terrain Linear Feature
* @author   Sylvain Pucci --  15/01/02 -- sylvain.pucci@bentley.com
* @see
*/
struct BcDTMComplexLinearFeature : BcDTMFeature
    {
    friend BcDTM;
    friend BcDTMFeatureEnumerator;

    private: BcDTMComplexLinearFeature (DTMFeatureType featureType, DTMUserTag userTagP, DTMFeatureId featureIdP, DPoint3dCP point, int nbPt);
    private: BcDTMComplexLinearFeature (DTMFeatureType featureType, DTMUserTag userTagP, DTMFeatureId featureIdP, const DtmString* pLinearElement, int nbLinearElement);
    public: static BcDTMComplexLinearFeaturePtr Create (DTMFeatureType featureType, DTMUserTag userTagP, DTMFeatureId featureId, DPoint3dCP point, int nbPt);
    public: static BcDTMComplexLinearFeaturePtr Create (DTMFeatureType featureType, DTMUserTag userTagP, DTMFeatureId featureId, const DtmString* pLinearElement, int nbLinearElement);

        virtual BcDTMLinearFeature* AsLinear () { return nullptr; };
        virtual BcDTMComplexLinearFeature* AsComplexLinear () { return this; };
        virtual BcDTMSpot* AsSpot () { return nullptr; };

    protected: int AppendElement (const DtmString& elmtP);
    public:
    BENTLEYDTM_EXPORT int GetComponentCount () const;
    BENTLEYDTM_EXPORT int GetComponentByIndex (DtmString& elem, int index) const;
    BENTLEYDTM_EXPORT int GetDefinitionPoints (DPoint3dP& pointsPP, int& nbPt, int index);

    private:
        DtmVectorString m_elmList;
    };

/**
* @memo     Pure virtual class which defines a Digital Terrain Spot Feature
* @doc      Pure virtual class which defines a Digital Terrain Spot Feature
* @author   Sylvain Pucci --  15/01/02 -- sylvain.pucci@bentley.com
* @see
*/
struct BcDTMSpot : BcDTMFeature
    {
    public:
        friend BcDTM;
        friend BcDTMFeatureEnumerator;
        static BcDTMSpotPtr Create (DTMUserTag userTagP, DTMFeatureId identP, DTMFeatureType featureTypeP, DPoint3dCP pointP, int nPt);
    protected:BcDTMSpot (DTMUserTag userTagP, DTMFeatureId identP, DTMFeatureType featureTypeP, DPoint3dCP pointP, int nPt);
    public:

    BENTLEYDTM_EXPORT int GetPoints (DPoint3dP& pointP, int& nPtP);
    BENTLEYDTM_EXPORT int GetPointsCount () { return (int)m_points.size (); }

    protected:
        int AppendPoints (DPoint3dCP pointsP, int nPt);

        virtual BcDTMSpot* AsSpot () { return this; };
        virtual BcDTMLinearFeature* AsLinear () { return nullptr; };
        virtual BcDTMComplexLinearFeature* AsComplexLinear () { return nullptr; };
    private:
        DtmString m_points;
    };

/**
* @memo     Pure virtual class which defines a Digital Terrain Feature
* @doc      Pure virtual class which defines a Digital Terrain Feature
* @author   Sylvain Pucci --  15/01/02 -- sylvain.pucci@bentley.com
* @see
*/
struct BcDTMFeatureEnumerator : Bentley::RefCountedBase
    {
    struct FeatureDescr
        {
        DTMFeatureType    type;
        DTMUserTag            featureUserTag;
        DTMFeatureId               featureId;
        bvector<DtmString>      pointArrays;
        };

    public: static BcDTMFeatureEnumerator* Create (BcDTMP dtmP) { return new BcDTMFeatureEnumerator (dtmP); }
    public: static BcDTMFeatureEnumerator* Create (BcDTMFeatureEnumerator &rhs) { return new BcDTMFeatureEnumerator (rhs); }

    private: BcDTMFeatureEnumerator (BcDTMP dtmP);
    private: BcDTMFeatureEnumerator (BcDTMFeatureEnumerator &rhs);
             ~BcDTMFeatureEnumerator ();

    public:
        virtual bool MoveNext ();
        virtual int Reset ();
        virtual BcDTMFeature* Current ();

        virtual BcDTMFeatureEnumeratorPtr Clone ();
        virtual int SetRange (DPoint3d *minRangeP, DPoint3d *maxRangeP);
        virtual int RemoveRange ();
        virtual int IncludeFeatureType (DTMFeatureType featureType);
        virtual int ExcludeAllFeatureTypes ();
        virtual int SetFeatureTypes (DTMFeatureType *featureTypesP, int nFeature);

    private:
        bvector<FeatureDescr>::const_iterator   m_it;
        int                                    m_state;
        bool                                   m_isMinRange;
        bool                                   m_isMaxRange;
        DPoint3d                               m_minRange;
        DPoint3d                               m_maxRange;
        bvector<DTMFeatureType>                 m_featuresTypes;

        // DTMFeatureState::Tin features enumeration
        BcDTMPtr                               m_dtmP;

        BC_DTM_OBJ*                            m_dtmHandleP;

        // Position indicators
        long                                   m_Position1;
        long                                   m_Position2;
        long                                   m_Position3;
        BC_DTM_SCAN_CONTEXT*                   m_DtmScanContext;
        bvector<DTMFeatureType>::const_iterator m_CurrentFeatureTypeIterator;
        bool                                   m_IsAtEnd;
        FeatureDescr                           m_CurrentFeature;
        // Private functions
        void            _Initialize ();
        bool            _MoveNextTin ();
        int             _MoveNextFeatureTin ();
        BcDTMFeaturePtr _CurrentFeature ();
        int             _GetFenceFromRange (DPoint3dP& fenceP, int& nPoint);
        int             _ResetScanContextForTin (DTMFeatureType newfeatureType, bool newScan);
    };


#pragma warning(pop)

//__PUBLISH_SECTION_START__
END_BENTLEY_TERRAINMODEL_NAMESPACE

#endif    /* #ifndef __bcDTMClassH__ */
