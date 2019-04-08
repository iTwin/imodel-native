#include "ScalableMeshPCH.h"
#include "AutomaticGroundDetectionInternalConfig.h"


#ifdef USE_LARGER_QUERY_BUFFERS
size_t TreeBuildingParams::numberOfPointsInVortexBuffer = 100001;
size_t TreeBuildingParams::numberOfPointsPerQuery = 100000;  

float TINGrowingParams::acceptableHeightOfGroundPointsInPoints = 49.143326f;//20.0f; 
double TINGrowingParams::maxTrustworthinessFactor = 10.738185;// 5.0;
double TINGrowingParams::minTrustworthinessFactor = 0.781344;// 0.5;

double SeedExclusionParams::thresholdOfRansacInliersForExclusion = 0.069004;// 0.25;
double SeedExclusionParams::trustworthinessCorrectionFactorForInlierThreshold = 0.335618; // 0.12;
bool SeedExclusionParams::dontExcludeSeedsIfHighlyTrustworthy = true; // true;
double SeedExclusionParams::trustworthinessThresholdForExclusion = 11.748763; // 10.0;

bool SeedSelectionParams::deeperTilesHaveFewerSeedPoints = false;
double SeedSelectionParams::relativeDepthThresholdToPickFewerSeedPoints = 0.820665; // 0.75;
double SeedSelectionParams::correctiveFactorForNumberOfSeedPoints = 13.667249; // 10.0;
int SeedSelectionParams::numberOfPointsInTilePerSeedPoint = 302; // 150;
float SeedSelectionParams::trustworthinessThresholdForExtraSeeds = 0.409707; // 0.0;
SeedSelectionConditions SeedSelectionParams::extraSeedSelectionCondition = SeedSelectionConditions::USE_RATIO_BETWEEN_PERCENTILES; 
float SeedSelectionParams::trustworthinessThresholdForWaivingExtraSeedCondition = 4.873829f;// 4.0f;
float SeedSelectionParams::acceptableRatioBetweenMedianAndMax = 0.167834;// 0.3;
float SeedSelectionParams::acceptableRatioBetweenMaxAndMin = 0.217107;// 0.1;

size_t TileProfilingParams::numberOfBinsPerAxisInTileHistogram = 15;// 16; //5
float TileProfilingParams::maxPercentileForOutlierDetectionInVariation = 0.748443;// 0.75f;
float TileProfilingParams::minPercentileForOutlierDetectionInVariation = 0.118562; // 0.25f;
float TileProfilingParams::minPercentileForOutlierDetectionInPointSet = 0.088492; // 0.01f;
float TileProfilingParams::maxPercentileForOutlierDetectionInPointSet = 0.917614; // 0.85f;
double TileProfilingParams::acceptableRatioBetweenNonOutlierVariations = 0.036084; // 0.01;
bool TileProfilingParams::assumeNoLowOutliers = false; // false;//true
bool TileProfilingParams::adjustToleranceValue = false; // true; //false;
#else
size_t TreeBuildingParams::numberOfPointsInVortexBuffer = 20000;
size_t TreeBuildingParams::numberOfPointsPerQuery = 10000;

float TINGrowingParams::acceptableHeightOfGroundPointsInPoints = 10.0f;
double TINGrowingParams::maxTrustworthinessFactor = 10.0;
double TINGrowingParams::minTrustworthinessFactor = 0.5;

double SeedExclusionParams::thresholdOfRansacInliersForExclusion = 0.25;
double SeedExclusionParams::trustworthinessCorrectionFactorForInlierThreshold = 0.12;
bool SeedExclusionParams::dontExcludeSeedsIfHighlyTrustworthy = false;
double SeedExclusionParams::trustworthinessThresholdForExclusion = 10.0;

bool SeedSelectionParams::deeperTilesHaveFewerSeedPoints = true;
double SeedSelectionParams::relativeDepthThresholdToPickFewerSeedPoints = 0.75;
double SeedSelectionParams::correctiveFactorForNumberOfSeedPoints = 10.0;
int SeedSelectionParams::numberOfPointsInTilePerSeedPoint = 200;
float SeedSelectionParams::trustworthinessThresholdForExtraSeeds = 2.0;
SeedSelectionConditions SeedSelectionParams::extraSeedSelectionCondition = SeedSelectionConditions::USE_AVERAGE_AND_IQR;
float SeedSelectionParams::trustworthinessThresholdForWaivingExtraSeedCondition = 10.0f;
float SeedSelectionParams::acceptableRatioBetweenMedianAndMax = 0.3;
float SeedSelectionParams::acceptableRatioBetweenMaxAndMin = 0.1;

size_t TileProfilingParams::numberOfBinsPerAxisInTileHistogram = 5;
float TileProfilingParams::maxPercentileForOutlierDetectionInVariation = 0.75f;
float TileProfilingParams::minPercentileForOutlierDetectionInVariation = 0.25f;
float TileProfilingParams::minPercentileForOutlierDetectionInPointSet = 0.01f;
float TileProfilingParams::maxPercentileForOutlierDetectionInPointSet = 0.85f;
double TileProfilingParams::acceptableRatioBetweenNonOutlierVariations = 0.01;
bool TileProfilingParams::assumeNoLowOutliers = true;
bool TileProfilingParams::adjustToleranceValue = false;
#endif