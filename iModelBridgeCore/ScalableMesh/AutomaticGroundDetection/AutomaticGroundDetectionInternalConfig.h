#pragma once
//#define USE_LARGER_QUERY_BUFFERS
typedef enum
    {
    USE_AVERAGE_AND_IQR,
    USE_RATIO_BETWEEN_PERCENTILES
    } SeedSelectionConditions;

struct TreeBuildingParams
    {
    static size_t numberOfPointsInVortexBuffer;
    static size_t numberOfPointsPerQuery;
    };

struct TINGrowingParams
    {
    static float acceptableHeightOfGroundPointsInPoints;
    static double maxTrustworthinessFactor;
    static double minTrustworthinessFactor;
    };

struct SeedExclusionParams
    {
    static double thresholdOfRansacInliersForExclusion;
    static double trustworthinessCorrectionFactorForInlierThreshold;
    static bool dontExcludeSeedsIfHighlyTrustworthy;
    static double trustworthinessThresholdForExclusion;
    };

struct SeedSelectionParams
    {
    static bool deeperTilesHaveFewerSeedPoints;
    static double relativeDepthThresholdToPickFewerSeedPoints;
    static double correctiveFactorForNumberOfSeedPoints;
    static int numberOfPointsInTilePerSeedPoint;
    static float trustworthinessThresholdForExtraSeeds;
    static SeedSelectionConditions extraSeedSelectionCondition;
    static float trustworthinessThresholdForWaivingExtraSeedCondition;
    static float acceptableRatioBetweenMedianAndMax;
    static float acceptableRatioBetweenMaxAndMin;
    };

struct TileProfilingParams
    {
    static size_t numberOfBinsPerAxisInTileHistogram;
    static float maxPercentileForOutlierDetectionInVariation;
    static float minPercentileForOutlierDetectionInVariation;
    static float minPercentileForOutlierDetectionInPointSet;
    static float maxPercentileForOutlierDetectionInPointSet;
    static double acceptableRatioBetweenNonOutlierVariations;
    static bool assumeNoLowOutliers;
    static bool adjustToleranceValue;
    };