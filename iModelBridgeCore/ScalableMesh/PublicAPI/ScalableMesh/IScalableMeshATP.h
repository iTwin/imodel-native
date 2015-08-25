#pragma once
#include <Bentley\Bentley.h>
#include <Bentley\WString.h>
#ifdef SCALABLE_MESH_ATP
/*__PUBLISH_SECTION_START__*/

//values set by code when compiled with tests, to be read in ATPs
#define ATP_GROUNDDETECTION_SEEDS_NUMBER L"nOfGroundDetectionSeedTiles"
#define ATP_GROUNDDETECTION_ALL_LOOPS_NUMBER L"nOfGroundDetectionLoopsAllTiles"
#define ATP_GROUNDDETECTION_ALL_SEED_POINTS_NUMBER L"nOfGroundDetectionPointsAllTiles"
#define ATP_GROUNDDETECTION_CHOSEN_ACCELERATOR_TYPE L"chosenAccelerator"
#define ATP_GROUNDDETECTION_TIMINGS_PARAM_ESTIMATION L"nTimeToEstimateParams"
#define ATP_GROUNDDETECTION_TIMINGS_FILTER_GROUND L"nTimeToFilterGround"


//values set in ATP, to be used to configure code
#define ATP_GROUNDDETECTION_FORCE_USE_CPU L"useCpu"
#define ATP_GROUNDDETECTION_SHOULD_USE_MULTITHREAD L"useMultiThread"

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

/*
* This singleton class provides generic key/value storage for intermediate results to be used in ATPs.
*/
struct IScalableMeshATP
    {
    private:
        /*__PUBLISH_SECTION_END__*/
        static IScalableMeshATP* mInstance;
        bmap<WString, int64_t> dict_of_ints;
        bmap<WString, double> dict_of_doubles;


        IScalableMeshATP() {};
        IScalableMeshATP(const IScalableMeshATP&) {};
        IScalableMeshATP&  operator=   (const IScalableMeshATP&) {};
       /*__PUBLISH_SECTION_START__*/
    public:
        BENTLEYSTM_EXPORT static IScalableMeshATP*   GetInstance();
        BENTLEYSTM_EXPORT static StatusInt StoreInt(WString name, int64_t value);
        BENTLEYSTM_EXPORT static StatusInt StoreDouble(WString name, double value);
        BENTLEYSTM_EXPORT static StatusInt GetInt(WString name, int64_t& value);
        BENTLEYSTM_EXPORT static StatusInt GetDouble(WString name, double& value);


    };

END_BENTLEY_SCALABLEMESH_NAMESPACE
/*__PUBLISH_SECTION_END__*/

#endif