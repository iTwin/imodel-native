/*--------------------------------------------------------------------------------------+
|
|   $Source: PublicAPI/TerrainModel/TerrainModel.h $
|
| $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

#if !defined(__midl) /* During a MIDL compile, there's nothing herein that we care about. */

//__PUBLISH_SECTION_START__
#include <Bentley/Bentley.h>
#include <Bentley/RefCounted.h>
#include <Geom/GeomApi.h>
//__PUBLISH_SECTION_END__

#if defined(__cplusplus) || defined(DOCUMENTATION_GENERATOR)

//__PUBLISH_SECTION_START__
#define BEGIN_BENTLEY_TERRAINMODEL_NAMESPACE    BEGIN_BENTLEY_NAMESPACE namespace TerrainModel {
#define END_BENTLEY_TERRAINMODEL_NAMESPACE     }}
#define USING_NAMESPACE_BENTLEY_TERRAINMODEL    using namespace Bentley::TerrainModel;

#define BEGIN_BENTLEY_MRDTM_NAMESPACE    BEGIN_BENTLEY_NAMESPACE namespace MrDTM {
#define END_BENTLEY_MRDTM_NAMESPACE     }}
#define USING_NAMESPACE_BENTLEY_MRDTM    using namespace Bentley::MrDTM;

#ifndef BEGIN_BENTLEY_MRDTM_IMPORT_NAMESPACE
#define BEGIN_BENTLEY_MRDTM_IMPORT_NAMESPACE BEGIN_BENTLEY_MRDTM_NAMESPACE namespace Import {
#define END_BENTLEY_MRDTM_IMPORT_NAMESPACE  END_BENTLEY_MRDTM_NAMESPACE}
#define USING_NAMESPACE_BENTLEY_MRDTM_IMPORT using namespace Bentley::MrDTM::Import;
#endif //!BEGIN_BENTLEY_MRDTM_IMPORT_NAMESPACE

#define TERRAINMODEL_TYPEDEFS(_name_) \
    BEGIN_BENTLEY_TERRAINMODEL_NAMESPACE struct _name_; END_BENTLEY_TERRAINMODEL_NAMESPACE \
    ADD_BENTLEY_TYPEDEFS (Bentley::TerrainModel, _name_)

#define TERRAINMODEL_ENUM(t,tEnum) \
    BEGIN_BENTLEY_TERRAINMODEL_NAMESPACE enum t; END_BENTLEY_TERRAINMODEL_NAMESPACE \
    typedef enum  Bentley :TerrainModel::##t  tEnum;

//__PUBLISH_SECTION_END__
#else
#define BEGIN_BENTLEY_TERRAINMODEL_NAMESPACE
#define END_BENTLEY_TERRAINMODEL_NAMESPACE

#define BEGIN_BENTLEY_MRDTM_NAMESPACE
#define END_BENTLEY_MRDTM_NAMESPACE
#define USING_NAMESPACE_BENTLEY_MRDTM   

#define BEGIN_BENTLEY_MRDTM_IMPORT_NAMESPACE 
#define END_BENTLEY_MRDTM_IMPORT_NAMESPACE  
#define USING_NAMESPACE_BENTLEY_MRDTM_IMPORT 

#define TERRAINMODEL_TYPEDEFS(_name_)   typedef struct _name_ * _name_##P; typedef struct _name_ const* _name_##CP; typedef struct _name_ & _name_##R; typedef struct _name_ const& _name_##CR;
#define TERRAINMODEL_ENUM(t,tEnum)   typedef enum  t tEnum;

#endif

//__PUBLISH_SECTION_START__
TERRAINMODEL_TYPEDEFS (BcDTM)
TERRAINMODEL_TYPEDEFS (DTMFenceParams)
TERRAINMODEL_TYPEDEFS (DTMContourParams)
//__PUBLISH_SECTION_END__
//TERRAINMODEL_TYPEDEFS (BcDTMEdges)
//TERRAINMODEL_TYPEDEFS (BcDTMMeshFace)
//TERRAINMODEL_TYPEDEFS (BcDTMMesh)
TERRAINMODEL_TYPEDEFS (IBcDtmStream)
TERRAINMODEL_TYPEDEFS (BcDTMDrapedLine)
TERRAINMODEL_TYPEDEFS (BcDTMDrapedLinePoint)
TERRAINMODEL_TYPEDEFS (BcDTMFeatureEnumerator)
TERRAINMODEL_TYPEDEFS (DTMFeatureEnumerator)
TERRAINMODEL_TYPEDEFS (DTMMeshEnumerator)

TERRAINMODEL_TYPEDEFS (BcDTMFeature)
TERRAINMODEL_TYPEDEFS (BcDTMLinearFeature)
TERRAINMODEL_TYPEDEFS (BcDTMComplexLinearFeature)
TERRAINMODEL_TYPEDEFS (BcDTMSpot)

//ADD_BENTLEY_TYPEDEFS1 (Bentley::TerrainModel, BcDTM, BcDTM, struct)
//ADD_BENTLEY_TYPEDEFS1 (Bentley::TerrainModel, IBcDTMFeatureEnumerator, BcDTMFeatureEnumerator, struct)
//ADD_BENTLEY_TYPEDEFS1 (Bentley::TerrainModel, BcDTMDrapedLine, BcDTMDrapedLine, struct)
//ADD_BENTLEY_TYPEDEFS1 (Bentley::TerrainModel, BcDTMFeature, BcDTMFeature, struct)

TERRAINMODEL_TYPEDEFS (TMTransformHelper)

//__PUBLISH_SECTION_START__
BEGIN_BENTLEY_TERRAINMODEL_NAMESPACE

typedef RefCountedPtr<BcDTM> BcDTMPtr;
//__PUBLISH_SECTION_END__
//typedef RefCountedPtr<BcDTMEdges> BcDTMEdgesPtr;
//typedef RefCountedPtr<BcDTMMeshFace> BcDTMMeshFacePtr;
//typedef RefCountedPtr<BcDTMMesh> BcDTMMeshPtr;
typedef RefCountedPtr<IBcDtmStream> BcDtmStreamPtr;
typedef RefCountedPtr<BcDTMDrapedLine> BcDTMDrapedLinePtr;
typedef RefCountedPtr<BcDTMDrapedLinePoint> BcDTMDrapedLinePointPtr;
typedef RefCountedPtr<BcDTMFeatureEnumerator> BcDTMFeatureEnumeratorPtr;
typedef RefCountedPtr<DTMFeatureEnumerator> DTMFeatureEnumeratorPtr;
typedef RefCountedPtr<DTMMeshEnumerator> DTMMeshEnumeratorPtr;

typedef RefCountedPtr<BcDTMFeature> BcDTMFeaturePtr;
typedef RefCountedPtr<BcDTMLinearFeature> BcDTMLinearFeaturePtr;
typedef RefCountedPtr<BcDTMComplexLinearFeature> BcDTMComplexLinearFeaturePtr;
typedef RefCountedPtr<BcDTMSpot> BcDTMSpotPtr;

struct DtmString : bvector<DPoint3d>
    {
    DtmString ()
        {
        }
    DtmString (DPoint3dCP pt, int numPts)
        {
        resize (numPts);
        memcpy (data (), pt, numPts * sizeof(DPoint3d));
        }
    };
typedef bvector<DtmString> DtmVectorString;

//__PUBLISH_SECTION_START__

END_BENTLEY_TERRAINMODEL_NAMESPACE

TERRAINMODEL_TYPEDEFS (IDTM)
//__PUBLISH_SECTION_END__
TERRAINMODEL_TYPEDEFS (IDTMDraping)
TERRAINMODEL_TYPEDEFS (IDTMDrapedLine)
TERRAINMODEL_TYPEDEFS (IDTMDrapedLinePoint)
TERRAINMODEL_TYPEDEFS (IDTMDrainageFeature)
TERRAINMODEL_TYPEDEFS (IDTMDrainage)
TERRAINMODEL_TYPEDEFS (IDTMContouring)
TERRAINMODEL_TYPEDEFS (TMTransformHelper)
//__PUBLISH_SECTION_START__

BEGIN_BENTLEY_TERRAINMODEL_NAMESPACE

typedef RefCountedPtr<IDTM>         DTMPtr;
//__PUBLISH_SECTION_END__
typedef RefCountedPtr<IDTMDraping>     DTMDrapingPtr;
typedef RefCountedPtr<IDTMDrapedLine>    DTMDrapedLinePtr;
typedef RefCountedPtr<IDTMDrapedLinePoint> DTMDrapedLinePointPtr;
typedef RefCountedPtr<IDTMDrainageFeature> DTMDrainageFeaturePtr;

typedef RefCountedPtr<TMTransformHelper> TMTransformHelperPtr;
//__PUBLISH_SECTION_START__

END_BENTLEY_TERRAINMODEL_NAMESPACE

#if (_MSC_VER == 1800)
#define thread_local __declspec(thread)
#endif

//__PUBLISH_SECTION_END__

#define BENTLEYDTM_Private static
#if defined (CREATE_STATIC_LIBRARIES) || defined (TERRAINMODEL_STATICLIB)
#define BENTLEYDTM_Public
#define BENTLEYDTM_EXPORT 
#elif defined (__BENTLEYDTM_BUILD__)
#define BENTLEYDTM_Public EXPORT_ATTRIBUTE
#define BENTLEYDTM_EXPORT EXPORT_ATTRIBUTE
#else
#define BENTLEYDTM_Public IMPORT_ATTRIBUTE
#define BENTLEYDTM_EXPORT IMPORT_ATTRIBUTE
#endif

//__PUBLISH_SECTION_START__
#ifndef BENTLEYDTM_EXPORT
#define BENTLEYDTM_EXPORT IMPORT_ATTRIBUTE
#endif

// public enums this should be put into it's own header file.
// moved from dtmdefs.h
// ToDo Replace in Code.
typedef Int64 DTMUserTag;
typedef Int64 DTMFeatureId;

//__PUBLISH_SECTION_END__

enum class DTMFenceOption : unsigned char
    {
    None = 0,
    Inside = 1,
    Overlap = 2,
    Outside = 3
    };

enum class DTMFenceType : unsigned char
    {
    None = 0,
    Block = 1,
    Shape = 2
    };

//__PUBLISH_SECTION_START__
/*
** DTM Feature Definitions
*/
enum class DTMFeatureType : UInt32
    {
    None = 0,
    RandomSpots = 0,
    GroupSpots = 1,
    Spots = 2,
    DatPoint = 2, // Backwards Compatiability With XM
    TriangleIndex = 3,
    TriangleInfo = 4,
    FeatureSpot = 5,
    FlowArrow = 6,
    TinPoint = 15,
    TinLine = 16,
    DtmPoint = 17,
    Breakline = 10,
    SoftBreakline = 11,
    DrapeLine = 12,
    GraphicBreak = 13,
    ContourLine = 14,
    Void = 20,
    VoidLine = 30,
    Island = 21,
    Hole = 22,
    HoleLine = 32,
    BreakVoid = 25,
    DrapeVoid = 26,
    Hull = 23,
    HullLine = 24,
    TinHull = 27,
    DrapeHull = 28,
    Triangle = 101,
    TriangleEdge = 102,
    Lattice = 103,
    LatticeEdge = 104,
    LatticeXLine = 105,
    LatticeYLine = 106,
    LatticePoint = 107,
    Contour = 110,
    ZeroSlopeLine = 112,
    ZeroSlopePolygon = 113,
    ZeroSlopeTriangle = 114,
    ISOLine = 200,
    ISOCell = 201,
    Theme = 210,
    SlopeToe = 220,
    LowPoint = 400,
    HighPoint = 401,
    SumpLine = 402,
    RidgeLine = 403,
    DescentTrace = 404,
    AscentTrace = 405,
    Catchment = 406,
    CrossLine = 407,
    LowPointPond = 408,
    PondIsland = 409,
    ClkFlowLine = 410,
    FlowLine = 411,
    VisiblePoint = 415,
    InvisiblePoint = 416,
    VisibleLine = 420,
    InvisibleLine = 421,
    SlopeLine = 422,
    Polygon = 430,
    Region = 431,
    GradeSlope = 440,
    //PolyMeshCounts = 500, // Not Used
    //PolyMeshVertices = 501, // Not Used
    //PolyMeshFaces = 502, // Not Used
    TriangleMesh = 503,
    //LatticeMesh = 504, // Not Used
    CheckStop = 505,
    TriangleShadeMesh = 506,
    InroadsRectangle = 507,
    TriangleHillShadeMesh = 508,  // Should this juse be TriangleShadeMesh?
    };

//__PUBLISH_SECTION_END__

enum class DTMCleanupFlags : short
    {
    None = 0,
    Changes = 1 << 0,
    VoidsAndIslands = 1 << 1,
    All = Changes | VoidsAndIslands,
    };

ENUM_IS_FLAGS (DTMCleanupFlags)

//__PUBLISH_SECTION_START__
enum class DTMState : UInt32
    {
    Data = 0,
    PointsSorted = 1,
    DuplicatesRemoved = 2,
    Tin = 3,
    TinError = 4,
    };
//__PUBLISH_SECTION_END__


enum class DTMDrapedLineCode : char
    {
    External = 0,
    Tin = 1,
    Breakline = 2,
    BetweenBreaklines = 3,
    Void = 4,
    InVoid = 4,
    OnPoint = 5,
    Edge = 6,
    };

// Constants that define the position of a draped point with regards to the DTMFeatureState::Tin
enum class DTMDrapePointCode
    {
    External = 0,
    Tin = 1,
    Void = 2,
    PointOrLine = 3,
    };

/*
** DTM Null Values
*/
// RobC 17-Dec-1010 Modified DTM_NULL_PNT and DTM_NULL_PTR 
// For 64 Bit . Both These Constants Have to Be A Minimum Of
// 6 Times Larger Than the Maximum Number Of Points That Can Be Triangulated
// The New Values Have Been set To Allow A Maximum Of 350 M Triangulation Points
//0x7ffffffe
//0x7ffffffd
enum DTMFeatureIdConst : DTMFeatureId
    {
    DTM_NULL_FEATURE_ID = -9898989898
    };

enum DTMUserTagConst : DTMUserTag
    {
    DTM_NULL_USER_TAG = -9898989898
    };

//__PUBLISH_SECTION_START__
enum DTMStatusInt
    {
    DTM_SUCCESS = 0,
    DTM_ERROR = 1
    };
//__PUBLISH_SECTION_END__

enum class DTMContourSmoothing : short
    {
    None = 0,
    Vertex = 1,
    Spline = 2,
    SplineWithoutOverLapDetection = 3,
    };

// Forward Create structures will need to use the TERRAINMODEL_TYPEDEFS
struct BC_DTM_OBJ;
struct BC_DTM_OBJ_EXTENDED;
struct DTM_DUPLICATE_POINT_ERROR;
struct DTM_CROSSING_FEATURE_ERROR;

//=======================================================================================
//! A unique (for this session) key to identify this AppData type. Create a static instance
//! of this class to identify each subclass of AppData.
//! @bsiclass                                                     Keith.Bentley   10/07
//=======================================================================================
struct          DTMAppDataKey
    {
    private:
        DTMAppDataKey (DTMAppDataKey const&);                  // illegal
        DTMAppDataKey const& operator= (DTMAppDataKey const&); // illegal
    public:
        DTMAppDataKey () {}
    };

//__PUBLISH_SECTION_END__
//=======================================================================================
//! @bsiclass                                                     Keith.Bentley   10/07
//=======================================================================================
template <typename APPDATA, typename KEY, typename HOST> struct DTMAppDataList
    {
    struct  AppDataEntry
        {
        KEY const*   m_key;
        APPDATA*     m_obj;

        AppDataEntry (APPDATA* entry, KEY const& key) : m_key (&key) { m_obj = entry; }
        void ChangeValue (APPDATA* obj, HOST host) { APPDATA* was = m_obj; m_obj = obj; if (was) was->_OnCleanup (host); }
        void Clear (HOST host) { ChangeValue (NULL, host); m_key = 0; }
        };

    typedef bvector<AppDataEntry> T_List;
    bool    m_locked;
    T_List  m_list;

    DTMAppDataList () { m_locked = false; }

    /*---------------------------------------------------------------------------------****
    * @bsimethod                                    Keith.Bentley                   10/07
    +---------------+---------------+---------------+---------------+---------------+------*/
    APPDATA* FindAppData (KEY const& key) const
        {
        for (AppDataEntry entry : m_list)
            {
            if (&key == entry.m_key)
                return entry.m_obj;
            }

        return  nullptr;
        }

    /*---------------------------------------------------------------------------------****
    //! It is NOT legal to call AddAppData from within a callback.
    * @bsimethod                                    Keith.Bentley                   10/07
                                                                                          +---------------+---------------+---------------+---------------+---------------+------*/
    StatusInt  AddAppData (KEY const& key, APPDATA* obj, HOST host)
        {
        if (m_locked)
            {
            BeAssert (0);
            return ERROR;
            }

        for (AppDataEntry entry : m_list)
            {
            if (&key == entry.m_key)
                {
                entry.ChangeValue (obj, host);
                return  SUCCESS;
                }
            }

        m_list.push_back (AppDataEntry (obj, key));
        return  SUCCESS;
        }

    /*---------------------------------------------------------------------------------****
                                                                                          //! It IS legal to call DropAppData from within a callback.
                                                                                          * @bsimethod                                    Keith.Bentley                   10/07
                                                                                          +---------------+---------------+---------------+---------------+---------------+------*/
    StatusInt DropAppData (KEY const& key, HOST host)
        {
        for (AppDataEntry entry : m_list)
            {
            if (&key == entry.m_key)
                {
                entry.Clear (host);        // doesn't get removed until next traversal
                return  SUCCESS;
                }
            }
        return  ERROR;
        }

    /*---------------------------------------------------------------------------------****
                                                                                          * @bsimethod                                    Keith.Bentley                   10/07
                                                                                          +---------------+---------------+---------------+---------------+---------------+------*/
    template <typename CALLER> void CallAllDroppable (CALLER const& caller, HOST host)
        {
        m_locked = true;
        for (typename T_List::iterator entry = m_list.begin (); entry != m_list.end ();)
            {
            if (NULL == entry->m_obj)      // was previously dropped
                {
                entry = m_list.erase (entry);
                }
            else if (caller.CallHandler (*entry->m_obj))
                {
                entry->ChangeValue (NULL, host);
                entry = m_list.erase (entry);
                }
            else
                ++entry;
            }
        m_locked = false;
        }

    /*---------------------------------------------------------------------------------****
                                                                                          * @bsimethod                                    Keith.Bentley                   10/07
                                                                                          +---------------+---------------+---------------+---------------+---------------+------*/
    template <typename CALLER> void CallAll (CALLER const& caller)
        {
        m_locked = true;
        for (typename T_List::iterator entry = m_list.begin (); entry != m_list.end ();)
            {
            if (NULL == entry->m_obj)   // was previously dropped
                {
                entry = m_list.erase (entry);
                }
            else
                {
                caller.CallHandler (*entry->m_obj);
                ++entry;
                }
            }
        m_locked = false;
        }
    };


// Callbacks
typedef std::function <int(DTMFeatureType dtmFeatureType, DTMUserTag userTag, DTMFeatureId featureId, DPoint3d *points, size_t numPoints, void* userArg)> DTMFeatureCallback;
//typedef int(*DTMFeatureCallback)(DTMFeatureType dtmFeatureType, DTMUserTag userTag, DTMFeatureId featureId, DPoint3d *points, size_t numPoints, void* userArg);
typedef std::function <void(int newPointNumber, DPoint3dCR pt, double& elevation, bool onEdge, const int existingPointsNumbers[])> DTMInsertPointCallback;
typedef std::function <int(DPoint3dP points, size_t numPoints, void * userArg)> DTMTransformPointsCallback;
typedef std::function <int(DTMFeatureType featureType, DPoint3d& point, void *userP)> DTMBrowseSinglePointFeatureCallback;
typedef std::function <int(bool major, DPoint3d& point1, DPoint3d& point2, void *userP)> DTMBrowseSlopeIndicatorCallback;
typedef std::function <int(double x, double y, DTM_DUPLICATE_POINT_ERROR *dupErrorsP, long numDupErrors, void *userP)> DTMDuplicatePointsCallback;
typedef std::function <int(DTM_CROSSING_FEATURE_ERROR& crossError, void *userP)> DTMCrossingFeaturesCallback;
typedef std::function <int(DTMFeatureType featureType, int numTriangles, int numMeshPoints, DPoint3d *meshPointsP, int numMeshFaces, long *meshFacesP, void *userP)> DTMTriangleMeshCallback;
typedef std::function <int(DTMFeatureType featureType, int numTriangles, int numMeshPoints, DPoint3d* meshPointsP, DPoint3d*, int numMeshFaces, long* meshFacesP, void* userP)> DTMTriangleShadeMeshCallback;
typedef std::function <int(DTMFeatureType featureType, int numTriangles, int numMeshPoints, DPoint3d* meshPointsP, long* meshReflectance, int numMeshFaces, long* meshFacesP, void* userP)> DTMTriangleHillShadeMeshCallback;

#endif /* !defined(__midl) */


