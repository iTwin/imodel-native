/*--------------------------------------------------------------------------------------+
|
|     $Source: TerrainModelNET/Caching.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <TerrainModel\TerrainModel.h>
#include <TerrainModel\Core\bcDTMClass.h>
#include <TerrainModel\Core\dtm2dfns.h>

struct DTMCachedFeature
    {
    DTMFeatureType    featureType;
    DTMUserTag        featureTag;
    DTMFeatureId      featureId;
    bvector<DPoint3d> points;

    DTMCachedFeature (DTMFeatureType featureType, DTMUserTag featureTag, DTMFeatureId featureId) :
        featureType (featureType), featureTag (featureTag), featureId (featureId)
        { }
    };

typedef std::function <int(bvector<DTMCachedFeature>& features, void *userP)> DTMBrowseFeatureCacheCallback;

/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
class DTMFeatureCache
    {
    private:
        BcDTMP m_dtm;
        size_t m_maxNumInCache;
        size_t m_numPoints;
        bvector<DTMCachedFeature> m_features;
        void *m_userP;
        DTMBrowseFeatureCacheCallback m_callBackFunctP;
    public:
        DTMFeatureCache (BcDTMP dtm, int maxNumInCache, DTMBrowseFeatureCacheCallback callBackFunctP, void* userP);

        ~DTMFeatureCache ();

        int FireCallback ();
        void EmptyCache ();
        int AddFeature (DTMFeatureType featureType, DTMUserTag featureTag, DTMFeatureId featureId, DPoint3dP tPoint, size_t nPoint);

        DTMStatusInt BrowseTinPointsVisibility (double eyeX, double eyeY, double eyeZ);

    DTMStatusInt BrowseTinLinesVisibility (double eyeX, double eyeY, double eyeZ);

    DTMStatusInt BrowseRadialViewSheds (double eyeX, double eyeY, double eyeZ, long viewShedOption, long numberRadials, double radialIncrement);

    DTMStatusInt BrowseRegionViewSheds (double eyeX, double eyeY, double eyeZ);

    DTMStatusInt BrowseDrainageFeatures (
        DTMFeatureType featureType,
        double      *minLowPointP,
        const DTMFenceParams& fence
        );

    DTMStatusInt BrowseFeatures
        (
        DTMFeatureType featureType,
        long        maxSpots
        );

    DTMStatusInt BrowseFeatures
        (
        DTMFeatureType featureType,
        const DTMFenceParams& fence,
        long        maxSpots
        );

    DTMStatusInt BrowseContours
        (
        double      interval,
        double      reg,
        double      zMin,
        double      zMax,
        bool        loadRange,
        DTMContourSmoothing smoothOption,
        double      smoothFactor,
        int         smoothDensity,
        const DTMFenceParams& fence,
        double*     contourValues,
        int         numContourValues,
        long        maxSlopeOption,
        double      maxSlopeValue,
        bool        highLowOption
        );

    DTMStatusInt AnalyzeElevation
        (
        DRange1d  *tInterval,
        int         nInterval,
        bool        polygonized,
        const DTMFenceParams& fence
        );
    DTMStatusInt AnalyzeSlope
        (
        DRange1d  *tInterval,
        int         nInterval,
        bool        polygonized,
        const DTMFenceParams& fence
        );
    DTMStatusInt AnalyzeAspect
        (
        DRange1d   *tInterval,
        int          nInterval,
        bool         polygonized,
        const DTMFenceParams& fence
        );
    };
