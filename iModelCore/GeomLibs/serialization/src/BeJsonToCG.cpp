/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include "serializationPCH.h"
#include <BeJsonCpp/BeJsonUtilities.h>

// need mutex for std::call_once
#include <mutex>

BEGIN_BENTLEY_GEOMETRY_NAMESPACE
typedef struct PlacementOriginZX const &PlacementOriginZXCR;
typedef struct PlacementOriginZX &PlacementOriginZXR;

// mappings from managed idioms to native
#define DVector3d DVec3d
#define String Utf8String


typedef int LoopType;

template<typename T, typename TBlocked>
static void CopyToBlockedVector (bvector<T> const &source, TBlocked &dest)
    {
    dest.clear ();
    dest.reserve (source.size ());
    if (source.size () > 0)
        dest.SetActive (true);
    for (T const &x : source)
        dest.push_back (x);
    }

struct PlacementOriginZX
{
DPoint3d m_origin;
DVec3d   m_vectorZ;
DVec3d   m_vectorX;

void InitIdentity ()
    {    
    m_origin = DPoint3d::From (0,0,0);
    m_vectorX = DVec3d::From (1,0,0);
    m_vectorZ = DVec3d::From (0,0,1);
    }
    
void InitOriginVectorZVectorX (DPoint3dCR origin, DVec3dCR vectorZ, DVec3dCR vectorX)
    {    
    m_origin = origin;
    m_vectorZ = vectorZ;
    m_vectorX = vectorX;
    }


PlacementOriginZX ()
    {
    InitIdentity ();
    }
    
static PlacementOriginZX FromIdentity ()
    {
    PlacementOriginZX value;
    return value;
    }

// compute unit vectors. package as DEllipse3d.
DEllipse3d AsDEllipse3d
(
double radius0  = 1.0,
double radius90 = 1.0,
double startRadians = 0.0,
double sweepRadians = msGeomConst_2pi
) const
    {
    DVec3d unitY = DVec3d::FromNormalizedCrossProduct (m_vectorZ, m_vectorX);
    DVec3d unitX = DVec3d::FromNormalizedCrossProduct (unitY, m_vectorZ);
    DEllipse3d ellipse;
    ellipse.center = m_origin;
    ellipse.vector0.Scale (unitX, radius0);
    ellipse.vector90.Scale (unitY, radius90);
    ellipse.start = startRadians;
    ellipse.sweep = sweepRadians;
    return ellipse;
    }

bool GetFrame (DPoint3dR origin, RotMatrixR axes) const
    {
    DVec3d vectorY;
    vectorY.CrossProduct (m_vectorZ, m_vectorX);
    axes = RotMatrix::FromColumnVectors (m_vectorX, vectorY, m_vectorZ);
    bool stat = axes.SquareAndNormalizeColumns (axes, 2, 0);
    origin = m_origin;
    return stat;
    }

bool GetFrame (DPoint3dR origin, DVec3dR xAxis, DVec3dR yAxis, DVec3dR zAxis) const
    {
    RotMatrix axes;
    bool stat = GetFrame (origin, axes);
    axes.GetColumns (xAxis, yAxis, zAxis);
    return stat;
    }
};

#include "nativeCGClasses.h"

#include "CGNativeFactoryImplementations.h"

template <typename TSource>
struct BeDirectReader
{
ICGFactory &m_factory;

BeDirectReader (ICGFactory &factory) : m_factory (factory){}

GEOMAPI_VIRTUAL bool IsCGType (TSource const &source, Utf8CP name) = 0;
GEOMAPI_VIRTUAL bool ReadTagDPoint3d (TSource const &source, CharCP name, DPoint3dR value) = 0;
GEOMAPI_VIRTUAL bool ReadTagTransform (TSource const &source, CharCP name, TransformR value) = 0;
GEOMAPI_VIRTUAL bool ReadTagPlacementOriginZX (Json::Value const &source, CharCP name, PlacementOriginZX &value) = 0;
GEOMAPI_VIRTUAL bool ReadTagdouble (TSource const &source, CharCP name, double &value) = 0;
GEOMAPI_VIRTUAL bool ReadTagint (TSource const &source, CharCP name, int &value) = 0;
GEOMAPI_VIRTUAL bool ReadTagAngle (TSource const &source, CharCP name, Angle &value) = 0;
GEOMAPI_VIRTUAL bool ReadTagString (TSource const &source, CharCP name, Utf8StringR)  = 0;
GEOMAPI_VIRTUAL bool ReadTagDPoint2d (TSource const &source, CharCP name, DPoint2dR value)  = 0;
GEOMAPI_VIRTUAL bool ReadTagDVector3d(TSource const &source, CharCP name, DVec3dR value) = 0;
GEOMAPI_VIRTUAL bool ReadTagbool(TSource const &source, CharCP name, bool &value) = 0;
GEOMAPI_VIRTUAL bool ReadListOfDPoint3d (TSource const &source, CharCP listName, CharCP shortListName, bvector<DPoint3d> &values) = 0;
GEOMAPI_VIRTUAL bool ReadListOfint (TSource const &source, CharCP listName, CharCP shortListName, bvector<int> &values) = 0;
GEOMAPI_VIRTUAL bool ReadListOfdouble (TSource const &source, CharCP listName, CharCP shortListName, bvector<double> &values) = 0;
GEOMAPI_VIRTUAL bool ReadListOfDPoint2d (TSource const &source, CharCP listName, CharCP shortListName, bvector<DPoint2d> &values) = 0;
GEOMAPI_VIRTUAL bool ReadListOfDVector3d (TSource const &source, CharCP listName, CharCP shortListName, bvector<DVector3d> &values) = 0;

//=======================================================================================
GEOMAPI_VIRTUAL bool ReadListOfISurfacePatch (TSource const &source, CharCP listName, CharCP shortListName, bvector<IGeometryPtr> &values)  = 0;
GEOMAPI_VIRTUAL bool ReadListOf_AnyCurveVector (TSource const &source, CharCP listName, CharCP shortListName, bvector<CurveVectorPtr> &values)  = 0;
GEOMAPI_VIRTUAL bool ReadListOf_AnyICurvePrimitive (TSource const &source, CharCP listName, CharCP shortListName, bvector<ICurvePrimitivePtr> &values)  = 0;
GEOMAPI_VIRTUAL bool ReadListOf_AnyICurveChain (TSource const &source, CharCP listName, CharCP shortListName, bvector<CurveVectorPtr> &values)  = 0;
GEOMAPI_VIRTUAL bool ReadListOfICurve (TSource const &source, CharCP listName, CharCP shortListName, bvector<IGeometryPtr> &values)  = 0;
GEOMAPI_VIRTUAL bool ReadListOfICurveChain (TSource const &source, CharCP listName, CharCP shortListName, bvector<IGeometryPtr> &values)  = 0;
GEOMAPI_VIRTUAL bool ReadListOfISolid (TSource const &source, CharCP listName, CharCP shortListName, bvector<IGeometryPtr> &values)  = 0;
GEOMAPI_VIRTUAL bool ReadListOfISurface (TSource const &source, CharCP listName, CharCP shortListName, bvector<IGeometryPtr> &values)  = 0;
GEOMAPI_VIRTUAL bool ReadListOfIPoint (TSource const &source, CharCP listName, CharCP shortListName, bvector<IGeometryPtr> &values)  = 0;
GEOMAPI_VIRTUAL bool ReadListOfIGeometry (TSource const &source, CharCP listName, CharCP shortListName, bvector<IGeometryPtr> &values)  = 0;
GEOMAPI_VIRTUAL bool ReadListOfISinglePoint (TSource const &source, CharCP listName, CharCP shortListName, bvector<IGeometryPtr> &values)  = 0;

GEOMAPI_VIRTUAL bool ReadTag_AnySurface (TSource const &source, CharCP name, IGeometryPtr &value)  = 0;
GEOMAPI_VIRTUAL bool ReadTag_AnyCurve (TSource const &source, CharCP name, IGeometryPtr &value)  = 0;
GEOMAPI_VIRTUAL bool ReadTag_AnyParametricSurface (TSource const &source, CharCP name, IGeometryPtr &value)  = 0;
GEOMAPI_VIRTUAL bool ReadTag_AnyCurveChain (TSource const &source, CharCP name, IGeometryPtr &value)  = 0;
GEOMAPI_VIRTUAL bool ReadTag_AnyICurvePrimitive (TSource const &source, ICurvePrimitivePtr &value)  = 0;
GEOMAPI_VIRTUAL bool ReadTag_AnyCurveVector (TSource const &source, CharCP name, CurveVectorPtr &value)  = 0;
GEOMAPI_VIRTUAL bool ReadTag_AnyICurvePrimitive (TSource const &source, CharCP name, ICurvePrimitivePtr &value)  = 0;

GEOMAPI_VIRTUAL bool ReadTagLoopType (TSource const &source, CharCP name, LoopType value) = 0;

GEOMAPI_VIRTUAL bool ReadTag_AnyGeometry (Json::Value const &source, IGeometryPtr &value) = 0;
GEOMAPI_VIRTUAL bool ReadTag_AnyGeometry (Json::Value const &source, CharCP name, IGeometryPtr &value) = 0;
#include "nativeCGDirectReaderH.h"
};

// hmph . .  isArray() on null value returns true. . .
static bool IsNonNullArray (Json::Value const &value)
    {
    return !value.isNull () && value.isArray ();
    }
// look up both names (short first!!!) and return
static Json::Value const &TryFindArray (Json::Value const &source, Utf8CP longName, Utf8CP shortName)
    {
    Json::Value const &target1 = source[shortName];
    if (IsNonNullArray (target1))
        return target1;
    Json::Value const &target2 = source[longName];
    if (IsNonNullArray (target2))
        return target2;
    return Json::Value::GetNull();
    }

// Case insensetive property lookup ...
bool FindProperty (Json::Value const &source, CharCP targetName, Json::Value &value)
    {
    for (Json::Value::iterator iter = source.begin(); iter != source.end(); iter++)
        {
        Utf8CP childName = iter.memberName();
        if (0 == BeStringUtilities::Stricmp (targetName, childName))
            {
            value = *iter;
            return true;
            }
        }
    return false;
    }


struct BeJsonToCGReaderImplementation : BeDirectReader<Json::Value>
{
Transform m_placement;

typedef bool (BeDirectReader<Json::Value>::*ParseMethod)(Json::Value const &, IGeometryPtr &);
typedef bmap <Utf8String, ParseMethod> ParseDictionary;

static ParseDictionary s_parseTable;

static void InitParseTable_go ()
    {
    if (s_parseTable.empty ())
        {
        s_parseTable[Utf8String("LineSegment")] = &BeJsonToCGReaderImplementation::ReadILineSegment;
        s_parseTable[Utf8String("CircularArc")] = &BeJsonToCGReaderImplementation::ReadICircularArc;
        s_parseTable[Utf8String("DgnBox")] = &BeJsonToCGReaderImplementation::ReadIDgnBox;
        s_parseTable[Utf8String("DgnSphere")] = &BeJsonToCGReaderImplementation::ReadIDgnSphere;
        s_parseTable[Utf8String("DgnCone")] = &BeJsonToCGReaderImplementation::ReadIDgnCone;
        s_parseTable[Utf8String("DgnTorusPipe")] = &BeJsonToCGReaderImplementation::ReadIDgnTorusPipe;
        s_parseTable[Utf8String("Block")] = &BeJsonToCGReaderImplementation::ReadIBlock;
        s_parseTable[Utf8String("CircularCone")] = &BeJsonToCGReaderImplementation::ReadICircularCone;
        s_parseTable[Utf8String("CircularCylinder")] = &BeJsonToCGReaderImplementation::ReadICircularCylinder;
        s_parseTable[Utf8String("CircularDisk")] = &BeJsonToCGReaderImplementation::ReadICircularDisk;
        s_parseTable[Utf8String("Coordinate")] = &BeJsonToCGReaderImplementation::ReadICoordinate;
        s_parseTable[Utf8String("EllipticArc")] = &BeJsonToCGReaderImplementation::ReadIEllipticArc;
        s_parseTable[Utf8String("EllipticDisk")] = &BeJsonToCGReaderImplementation::ReadIEllipticDisk;
        s_parseTable[Utf8String("SingleLineText")] = &BeJsonToCGReaderImplementation::ReadISingleLineText;
        s_parseTable[Utf8String("SkewedCone")] = &BeJsonToCGReaderImplementation::ReadISkewedCone;
        s_parseTable[Utf8String("Sphere")] = &BeJsonToCGReaderImplementation::ReadISphere;
        s_parseTable[Utf8String("TorusPipe")] = &BeJsonToCGReaderImplementation::ReadITorusPipe;
        s_parseTable[Utf8String("Vector")] = &BeJsonToCGReaderImplementation::ReadIVector;
        s_parseTable[Utf8String("IndexedMesh")] = &BeJsonToCGReaderImplementation::ReadIIndexedMesh;
        s_parseTable[Utf8String("AdjacentSurfacePatches")] = &BeJsonToCGReaderImplementation::ReadIAdjacentSurfacePatches;
        s_parseTable[Utf8String("BsplineCurve")] = &BeJsonToCGReaderImplementation::ReadIBsplineCurve;
        s_parseTable[Utf8String("BsplineSurface")] = &BeJsonToCGReaderImplementation::ReadIBsplineSurface;
        s_parseTable[Utf8String("CurveChain")] = &BeJsonToCGReaderImplementation::ReadICurveChain;
        s_parseTable[Utf8String("CurveGroup")] = &BeJsonToCGReaderImplementation::ReadICurveGroup;
        s_parseTable[Utf8String("CurveReference")] = &BeJsonToCGReaderImplementation::ReadICurveReference;
        s_parseTable[Utf8String("Group")] = &BeJsonToCGReaderImplementation::ReadIGroup;
        s_parseTable[Utf8String("InterpolatingCurve")] = &BeJsonToCGReaderImplementation::ReadIInterpolatingCurve;
        s_parseTable[Utf8String("LineString")] = &BeJsonToCGReaderImplementation::ReadILineString;
        s_parseTable[Utf8String("Operation")] = &BeJsonToCGReaderImplementation::ReadIOperation;
        s_parseTable[Utf8String("ParametricSurfacePatch")] = &BeJsonToCGReaderImplementation::ReadIParametricSurfacePatch;
        s_parseTable[Utf8String("PointChain")] = &BeJsonToCGReaderImplementation::ReadIPointChain;
        s_parseTable[Utf8String("PointGroup")] = &BeJsonToCGReaderImplementation::ReadIPointGroup;
        s_parseTable[Utf8String("Polygon")] = &BeJsonToCGReaderImplementation::ReadIPolygon;
        s_parseTable[Utf8String("PrimitiveCurveReference")] = &BeJsonToCGReaderImplementation::ReadIPrimitiveCurveReference;
        s_parseTable[Utf8String("SharedGroupDef")] = &BeJsonToCGReaderImplementation::ReadISharedGroupDef;
        s_parseTable[Utf8String("SharedGroupInstance")] = &BeJsonToCGReaderImplementation::ReadISharedGroupInstance;
        s_parseTable[Utf8String("ShelledSolid")] = &BeJsonToCGReaderImplementation::ReadIShelledSolid;
        s_parseTable[Utf8String("SolidBySweptSurface")] = &BeJsonToCGReaderImplementation::ReadISolidBySweptSurface;
        s_parseTable[Utf8String("SolidByRuledSweep")] = &BeJsonToCGReaderImplementation::ReadISolidByRuledSweep;
        s_parseTable[Utf8String("SurfaceByRuledSweep")] = &BeJsonToCGReaderImplementation::ReadISurfaceByRuledSweep;
        s_parseTable[Utf8String("SolidGroup")] = &BeJsonToCGReaderImplementation::ReadISolidGroup;
        s_parseTable[Utf8String("Spiral")] = &BeJsonToCGReaderImplementation::ReadISpiral;
        s_parseTable[Utf8String("SurfaceBySweptCurve")] = &BeJsonToCGReaderImplementation::ReadISurfaceBySweptCurve;
        s_parseTable[Utf8String("SurfaceGroup")] = &BeJsonToCGReaderImplementation::ReadISurfaceGroup;
        s_parseTable[Utf8String("SurfacePatch")] = &BeJsonToCGReaderImplementation::ReadISurfacePatch;
        s_parseTable[Utf8String("DgnCurveVector")] = &BeJsonToCGReaderImplementation::ReadIDgnCurveVector;
        s_parseTable[Utf8String("TransformedGeometry")] = &BeJsonToCGReaderImplementation::ReadITransformedGeometry;
        s_parseTable[Utf8String("DgnExtrusion")] = &BeJsonToCGReaderImplementation::ReadIDgnExtrusion;
        s_parseTable[Utf8String("DgnRotationalSweep")] = &BeJsonToCGReaderImplementation::ReadIDgnRotationalSweep;
        s_parseTable[Utf8String("DgnRuledSweep")] = &BeJsonToCGReaderImplementation::ReadIDgnRuledSweep;
        s_parseTable[Utf8String("TransitionSpiral")] = &BeJsonToCGReaderImplementation::ReadITransitionSpiral;
        s_parseTable[Utf8String("PartialCurve")] = &BeJsonToCGReaderImplementation::ReadIPartialCurve;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Earlin.Lutz                     03/17
+---------------+---------------+---------------+---------------+---------------+------*/
static void InitParseTable ()
    {
    static std::once_flag s_ignoreListOnceFlag;
    std::call_once(s_ignoreListOnceFlag, InitParseTable_go);
    }

Utf8String m_parseSearchString;
ParseMethod FindParseMethod (Utf8CP name)
    {
    m_parseSearchString.AssignOrClear (name);
    return s_parseTable[m_parseSearchString];
    }
int m_debug;

static Utf8String s_jsonKey_type;
static int s_defaultDebug;
BeJsonToCGReaderImplementation (ICGFactory &factory)
    : BeDirectReader (factory), m_debug (s_defaultDebug)
    {
    m_placement = Transform::FromIdentity ();
    InitParseTable ();
    }
#ifdef abc


    // This is assigned by IsCGValue ...
    Utf8String m_currentCGType;
    // For reuse by all readers.
    WString m_currentValueW;
    Utf8String m_currentValue8;


bool CurrentElementNameMatch (CharCP name)
    {
    return 0 == m_currentElementName.CompareToI (name);
    }

bool CurrentElementNameMatch (CharCP nameA, CharCP nameB)
    {
    return 0 == m_currentElementName.CompareToI (nameA)
        || 0 == m_currentElementName.CompareToI (nameB);
    }
#endif
// ASSUME input is confirmed as an array.
bool TryGetDoubleAtArrayPosition (Json::Value const &source, int i, double &value)
    {
    if (i >= 0 && source.size () > (size_t)i && source[i].isNumeric ())
        {
        value = source[i].asDouble ();
        return true;
        }
    return false;        
    }

bool ReadDPoint3d (Json::Value const &source, DPoint3dR value)
    {
    if (IsNonNullArray (source)
        && source.size () == 3
        )
        {
        Json::Value const &xValue = source[0];
        Json::Value const &yValue = source[1];
        Json::Value const &zValue = source[2];
        if (xValue.isNumeric () && yValue.isNumeric () && zValue.isNumeric ())
            {
            value.Init (xValue.asDouble (), yValue.asDouble (), zValue.asDouble ());
            return true;
            }
        }    
    return false;
    }

bool ReadDVector3d (Json::Value const &source, DVec3dR value)
    {
    if (IsNonNullArray (source)
        && source.size () == 3
        )
        {
        Json::Value const &xValue = source[0];
        Json::Value const &yValue = source[1];
        Json::Value const &zValue = source[2];
        if (xValue.isNumeric () && yValue.isNumeric () && zValue.isNumeric ())
            {
            value.Init (xValue.asDouble (), yValue.asDouble (), zValue.asDouble ());
            return true;
            }
        }    
    return false;
    }


bool ReadDPoint2d (Json::Value const &source, DPoint2dR value)
    {
    if (IsNonNullArray (source)
        && source.size () == 2
        )
        {
        Json::Value const &xValue = source[0];
        Json::Value const &yValue = source[1];
        if (xValue.isNumeric () && yValue.isNumeric ())
            {
            value.Init (xValue.asDouble (), yValue.asDouble ());
            return true;
            }
        }    
    return false;
    }

bool ReadTagDPoint3d (Json::Value const &source, CharCP name, DPoint3dR value) override
    {
    Json::Value target;
    return   FindProperty (source, name, target)
          && ReadDPoint3d (target, value);
    }

bool ReadTagDPoint2d (Json::Value const &source, CharCP name, DPoint2dR value) override
    {
    Json::Value target;
    return   FindProperty (source, name, target)
          && ReadDPoint2d (target, value);
    }

bool ReadTagDVector3d(Json::Value const &source, CharCP name, DVec3dR value) override
    {
    Json::Value target;
    return   FindProperty (source, name, target)
          && ReadDVector3d (target, value);
    }



bool ReadTagTransform (Json::Value const &source, CharCP name, TransformR value) override
    {
    Json::Value target;
    if (FindProperty (source, name, target)
        &&IsNonNullArray (target)
        && target.size () == 12
        )
        {
        return TryGetDoubleAtArrayPosition (target,  0, value.form3d[0][0])
            && TryGetDoubleAtArrayPosition (target,  1, value.form3d[0][1])
            && TryGetDoubleAtArrayPosition (target,  2, value.form3d[0][2])
            && TryGetDoubleAtArrayPosition (target,  3, value.form3d[0][3])
            && TryGetDoubleAtArrayPosition (target,  4, value.form3d[1][0])
            && TryGetDoubleAtArrayPosition (target,  5, value.form3d[1][1])
            && TryGetDoubleAtArrayPosition (target,  6, value.form3d[1][2])
            && TryGetDoubleAtArrayPosition (target,  7, value.form3d[1][3])
            && TryGetDoubleAtArrayPosition (target,  8, value.form3d[2][0])
            && TryGetDoubleAtArrayPosition (target,  9, value.form3d[2][1])
            && TryGetDoubleAtArrayPosition (target, 10, value.form3d[2][2])
            && TryGetDoubleAtArrayPosition (target, 11, value.form3d[2][3]);
        }
    return false;
    }

bool ReadTagLoopType (Json::Value const &source, CharCP name, LoopType value) override
    {
    return false;
    }

bool ReadListOfDPoint3d (Json::Value const &source, CharCP listName, CharCP shortListName, bvector<DPoint3d> &values) override
    {
    Json::Value const target = TryFindArray (source, listName, shortListName);
    if (IsNonNullArray (target))
        {
        int n = target.size ();
        DPoint3d xyz;
        for (int i = 0; i < n; i++)
            {
            Json::Value const &entry = target[i];
            if (ReadDPoint3d (entry, xyz))
                values.push_back (xyz);
            else
                return false;
            }
        return true;
        }
      return false;
    }

bool ReadListOfint (Json::Value const &source, CharCP listName, CharCP shortListName, bvector<int> &values) override
    {
    Json::Value const target = TryFindArray (source, listName, shortListName);
    if (IsNonNullArray (target))
        {
        int n = target.size ();
        for (int i = 0; i < n; i++)
            {
            Json::Value const &entry = target[i];
            
            if (entry.isInt ())
                values.push_back (entry.asInt ());
            else
                return false;
            }
        return true;
        }
    return false;
    }

bool ReadListOfdouble (Json::Value const &source, CharCP listName, CharCP shortListName, bvector<double> &values) override
    {
    Json::Value const target = TryFindArray (source, listName, shortListName);
    if (IsNonNullArray (target))
        {
        int n = target.size ();
        for (int i = 0; i < n; i++)
            {
            Json::Value const &entry = target[i];
            
            if (entry.isNumeric ())
                values.push_back (entry.asDouble ());
            else
                return false;
            }
        return true;
        }
    return false;
    }

bool ReadListOfDPoint2d (Json::Value const &source, CharCP listName, CharCP shortListName, bvector<DPoint2d> &values) override 
    {
    Json::Value const target = TryFindArray (source, listName, shortListName);
    if (IsNonNullArray (target))
        {
        int n = target.size ();
        DPoint2d xy;
        for (int i = 0; i < n; i++)
            {
            Json::Value const &entry = target[i];
            if (ReadDPoint2d (entry, xy))
                values.push_back (xy);
            else
                return false;
            }
        return true;
        }
    return false;
    }


//=======================================================================================
bool ReadListOfDVector3d (Json::Value const &source, CharCP listName, CharCP shortListName, bvector<DVector3d> &values) override
    {
    Json::Value const target = TryFindArray (source, listName, shortListName);
    if (IsNonNullArray (target))
        {
        int n = target.size ();
        DVector3d xyz;
        for (int i = 0; i < n; i++)
            {
            Json::Value const &entry = target[i];
            if (ReadDVector3d (entry, xyz))
                values.push_back (xyz);
            else
                return false;
            }
        return true;
        }
    return false;
    }


//=======================================================================================
bool ReadListOfISurfacePatch (Json::Value const &source, CharCP listName, CharCP shortListName, bvector<IGeometryPtr> &values) override
    {
    BeAssert(false);
    return false;
    }

bool ReadListOf_AnyCurveVector (Json::Value const &source, CharCP listName, CharCP shortListName, bvector<CurveVectorPtr> &values) override
    {
    Json::Value const target = TryFindArray (source, listName, shortListName);
    if (IsNonNullArray (target))
        {
        int n = target.size ();
        for (int i = 0; i < n; i++)
            {
            CurveVectorPtr childGeometry;
            if (!ReadTag_AnyCurveVector (target[i], nullptr, childGeometry))
                return false;
            values.push_back (childGeometry);
            }
        return true;
        }
    return false;
    }


bool ReadListOf_AnyICurvePrimitive (Json::Value const &source, CharCP listName, CharCP shortListName, bvector<ICurvePrimitivePtr> &values) override
    {
    Json::Value const target = TryFindArray (source, listName, shortListName);
    if (IsNonNullArray (target))
        {
        int n = target.size ();
        for (int i = 0; i < n; i++)
            {
            ICurvePrimitivePtr childGeometry;
            if (!ReadTag_AnyICurvePrimitive (target[i], childGeometry))
                return false;
            values.push_back (childGeometry);
            }
        return true;
        }
    return false;
    }

bool ReadListOf_AnyICurveChain (Json::Value const &source, CharCP listName, CharCP shortListName, bvector<CurveVectorPtr> &values) override
    {
    return ReadListOf_AnyCurveVector (source, listName, shortListName, values);
    }

bool ReadListOfICurve (Json::Value const &source, CharCP listName, CharCP shortListName, bvector<IGeometryPtr> &values) override
    {
    BeAssert(false);
    return false;
    }

bool ReadListOfICurveChain (Json::Value const &source, CharCP listName, CharCP shortListName, bvector<IGeometryPtr> &values) override
    {
    Json::Value const target = TryFindArray (source, listName, shortListName);
    if (IsNonNullArray (target))
        {
        int n = target.size ();
        for (int i = 0; i < n; i++)
            {
            IGeometryPtr childGeometry;
            // ignore the component name !!!
            if (!ReadTag_AnyCurveChain (target[i], childGeometry))
                return false;
            values.push_back (childGeometry);
            }
        return true;
        }
    return false;
    }


bool ReadListOfISolid (Json::Value const &source, CharCP listName, CharCP shortListName, bvector<IGeometryPtr> &values) override
    {
    BeAssert(false);
    return false;
    }
bool ReadListOfISurface (Json::Value const &source, CharCP listName, CharCP shortListName, bvector<IGeometryPtr> &values) override
    {
    BeAssert(false);
    return false;
    }
bool ReadListOfIPoint (Json::Value const &source, CharCP listName, CharCP shortListName, bvector<IGeometryPtr> &values) override
    {
    BeAssert(false);
    return false;
    }





bool ReadListOfIGeometry (Json::Value const &source, CharCP listName, CharCP shortListName, bvector<IGeometryPtr> &values) override
    {
    Json::Value const target = TryFindArray (source, listName, shortListName);
    if (IsNonNullArray (target))
        {
        int n = target.size ();
        for (int i = 0; i < n; i++)
            {
            IGeometryPtr childGeometry;
            if (!ReadTag_AnyGeometry (target[i], nullptr, childGeometry))
                return false;
            values.push_back (childGeometry);
            }
        return true;
        }
    return false;
    }






bool ReadListOfISinglePoint (Json::Value const &source, CharCP listName, CharCP shortListName, bvector<IGeometryPtr> &values) override
    {
    BeAssert(false);
    return false;
    }





//=======================================================================================
bool ReadTagString (Json::Value const &source, CharCP name, Utf8StringR value) override
    {
    Json::Value target;
    if (FindProperty (source, name, target)
       && target.isString ()
       )
        {
        value = target.asString ();
        return true;
        }
    return false;
    }


bool ReadTagbool(Json::Value const &source, CharCP name, bool &value) override
    {
    Json::Value target;
    value = false;
    if (FindProperty (source, name, target)
       && target.isBool ()
       )
        {
        value = target.asBool ();
        return true;
        }
    return false;
    }

bool ReadTagdouble(Json::Value const &source, CharCP name, double &value) override
    {
    Json::Value target;
    value = 0.0;
    if (FindProperty (source, name, target)
       && target.isNumeric ())
        {
        value = target.asDouble ();
        return true;
        }
    return false;
    }
    
bool ReadTagint(Json::Value const &source, CharCP name, int &value) override
    {
    Json::Value target;
    value = 0;
    if (FindProperty (source, name, target)
       && target.isInt ())
        {
        value = target.asInt ();
        return true;
        }
    return false;
    }
    
bool ReadTagPlacementOriginZX (Json::Value const &source, CharCP name, PlacementOriginZX &value) override
    {
    Json::Value target;
    if (FindProperty (source, name, target)
       && target.isObject ())
        {
        DPoint3d origin;
        DVec3d vectorX, vectorZ;
        bool bCenter = ReadTagDPoint3d (target, "origin", origin);
        bool bVectorZ = ReadTagDPoint3d (target, "vectorZ", vectorZ);
        bool bVectorX = ReadTagDPoint3d (target, "vectorX", vectorX);
        if (bCenter && bVectorX && bVectorZ)
            {
            value.InitOriginVectorZVectorX (origin, vectorZ, vectorX);
            return true;
            }
        return true;
        }
    return false;
    }

bool ReadTagAngle (Json::Value const &source, CharCP name, Angle &value) override
    {
    double degrees;
    if (ReadTagdouble (source, name, degrees))
        {
        value = Angle::FromDegrees (degrees);
        return true;
        }
     return false;
    }
#ifdef abc
//<ExtendedObject xmlns="http://www.bentley.com/schemas/Bentley.ECSerializable.1.0">
//    <Coordinate xmlns="http://www.bentley.com/schemas/Bentley.Geometry.Common.1.0">
//        <xyz>11.1,22.2,33.3</xyz>
//    </Coordinate>
//    <ExtendedData>
//        <TransientLookupCollection>
//            <Entry key="color" typeCode="Int32">32</Entry>
//        </TransientLookupCollection>
//    </ExtendedData>
//</ExtendedObject>

bool ReadExtendedData(BeExtendedData& extendedDataEntries)
    {
    ReadToChildOrEnd();

    // Should be 'TransientLookupCollection'
    if (!CurrentElementNameMatch("TransientLookupCollection"))
        return false;

    ReadToChildOrEnd();
    // Next, iterate over each entry
    for (;IsStartElement ();)
        {
        BeExtendedDataEntry entry;
        Utf8String keyName("key");
        m_reader.ReadToNextAttribute (&keyName, &entry.Key);

        Utf8String typeCode("typeCode");
        m_reader.ReadToNextAttribute (&typeCode, &entry.Type);

        m_reader.Read();

        m_reader.MoveToContent();
        m_reader.ReadContentAsString(entry.Value);
        m_reader.MoveToContent();
        extendedDataEntries.push_back(entry);
        // m_reader.ReadEndElement()
            {
            m_reader.ReadToEndOfElement();
            m_reader.Read();
            }
        m_reader.MoveToContent();
        }
    ReadEndElement(); // Entry
    ReadEndElement(); // TransientLookupCollection
    return false;
    }

bmap<OrderedIGeometryPtr, BeExtendedData> m_extendedData;
void ReadExtendedObject(bvector<IGeometryPtr> &geometry)
    {
    if (!CurrentElementNameMatch("ExtendedObject"))
        return;

    ReadToChildOrEnd();

    ParseMethod parseMethod = s_parseTable[m_currentElementName];
    IGeometryPtr result;
    if (parseMethod != nullptr)
        {
        if (!(this->*parseMethod)(result))
            return;
        geometry.push_back (result);
        }

    if (!CurrentElementNameMatch("ExtendedData"))
        return;

    BeExtendedData dataEntries;
    ReadExtendedData(dataEntries);
    m_extendedData[result] = dataEntries;
    m_reader.ReadToEndOfElement();
    ReadEndElement();
    }

bool ReadExtendedObject(IGeometryPtr &geometry)
    {
    if (!CurrentElementNameMatch("ExtendedObject"))
        return false;

    ReadToChildOrEnd();

    ParseMethod parseMethod = s_parseTable[m_currentElementName];
    if (parseMethod != nullptr)
        {
        if (!(this->*parseMethod)(geometry))
            return false;
        }

    if (!CurrentElementNameMatch("ExtendedData"))
        return false;

    BeExtendedData dataEntries;
    ReadExtendedData(dataEntries);
    m_extendedData[geometry] = dataEntries;
    ReadEndElement();
    return true;
    }
#endif

bool ReadTag_AnyGeometry (Json::Value const &source, IGeometryPtr &value) override
    {
    return ReadTag_AnyIGeometry (source, value);
    }

bool ReadTag_AnyGeometry (Json::Value const &source, CharCP name, IGeometryPtr &value) override
    {
    if (nullptr == name)
        return ReadTag_AnyIGeometry (source, value);
    Json::Value target;
    return   FindProperty (source, name, target)
          && ReadTag_AnyIGeometry (target, value);
    }

static bool IsAnySurface (IGeometryCR value)
    {
    IGeometry::GeometryType type = value.GetGeometryType ();
    bool stat = false;
    if (type == IGeometry::GeometryType::BsplineSurface)
        stat = true;
    else if (type == IGeometry::GeometryType::SolidPrimitive)
        stat = true;
    else if (type == IGeometry::GeometryType::CurveVector)
        stat = value.GetAsCurveVector ()->IsAnyRegionType ();
    return stat;
    }

static bool IsParametricSurface (IGeometryCR value)
    {
    IGeometry::GeometryType type = value.GetGeometryType ();
    bool stat = false;
    if (type == IGeometry::GeometryType::BsplineSurface)
        stat = true;
    return stat;
    }

static bool IsAnyCurve (IGeometryCR value)
    {
    IGeometry::GeometryType type = value.GetGeometryType ();
    bool stat = false;
    if (type == IGeometry::GeometryType::CurvePrimitive)
        stat = true;
    if (type == IGeometry::GeometryType::CurveVector)
        stat = true;
    return stat;
    }

static bool IsAnyCurvePrimitive (IGeometryCR value)
    {
    IGeometry::GeometryType type = value.GetGeometryType ();
    bool stat = false;
    if (type == IGeometry::GeometryType::CurvePrimitive)
        stat = true;
    return stat;
    }

static bool IsAnyCurveChain (IGeometryCR value)
    {
    IGeometry::GeometryType type = value.GetGeometryType ();
    bool stat = false;
    if (type == IGeometry::GeometryType::CurveVector)
        {
        CurveVectorPtr cv = value.GetAsCurveVector ();
        stat = cv->IsOpenPath ()
            || cv->IsClosedPath ();
        stat = true;
        }
    return stat;
    }

bool ReadTag_AnySurface (Json::Value const &source, CharCP name, IGeometryPtr &value) override
    {
    return ReadTag_AnyGeometry (source, name,value)
        && IsAnySurface (*value);
    }

bool ReadTag_AnyCurve (Json::Value const &source, CharCP name, IGeometryPtr &value) override
    {
    return ReadTag_AnyGeometry (source, name,value)
        && IsAnyCurve (*value);
    }

bool ReadTag_AnyParametricSurface (Json::Value const &source, CharCP name, IGeometryPtr &value) override
    {
    return ReadTag_AnyGeometry (source, name,value)
        && IsParametricSurface (*value);
    }

bool ReadTag_AnyCurveChain (Json::Value const &source, CharCP name, IGeometryPtr &value) override
    {
    return ReadTag_AnyGeometry (source, name,value)
        && IsAnyCurveChain (*value);
    }

bool ReadTag_AnyCurveChain (Json::Value const &source, IGeometryPtr &value)
    {
    return ReadTag_AnyGeometry (source, value)
        && IsAnyCurveChain (*value);
    }


bool ReadTag_AnyICurvePrimitive (Json::Value const &source, ICurvePrimitivePtr &value) override
    {
    IGeometryPtr geometry;
    if (ReadTag_AnyGeometry (source,geometry))
        {
        value = geometry->GetAsICurvePrimitive ();
        if (value.IsValid ())
            return true;
        CurveVectorPtr cv = geometry->GetAsCurveVector ();
        if (cv.IsValid ())
            {
            value = ICurvePrimitive::CreateChildCurveVector (cv);
            return true;
            }
        }

    return false;
    }

bool ReadTag_AnyCurveVector (Json::Value const &source, CharCP name, CurveVectorPtr &value) override
    {
    IGeometryPtr geometry;
    if (ReadTag_AnyGeometry (source, name,geometry))
        {
        value = geometry->GetAsCurveVector ();
        if (value.IsValid ())
            return true;
        ICurvePrimitivePtr primitive = geometry->GetAsICurvePrimitive ();
        if (primitive.IsValid ())
            {
            value = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);
            value->Add (primitive);
            return true;
            }
        }

    return false;
    }

bool ReadTag_AnyICurvePrimitive (Json::Value const &source, CharCP name, ICurvePrimitivePtr &value) override
    {
    return false;
    }

#ifdef CompactCurveVectors

// ---- hand crafted reader for compact ParityRegion
struct CGDgnExtrusionDetail
{
CurveVectorPtr baseGeometry;
};

struct CGDgnExtrusionFlags
{
CGDgnExtrusionFlags ()
    {
    extrusionVector_defined = false;
    capped_defined = false;
    baseGeometry_defined = false;    
    
    }

bool extrusionVector_defined;
bool capped_defined;
bool baseGeometry_defined;  

int NumUndefined ()
    {
    int numUndefined = 0;

    if (!extrusionVector_defined)
        numUndefined++;
    if (!capped_defined)
        numUndefined++;
    if (!baseGeometry_defined)
        numUndefined++;  
    return numUndefined;
    }
};




// 
   "Type" : "ParityRegion",
   "loops" : [[prim,prim],[prim,prim]]
bool ReadParityRegion(Json::Value const &value, IGeometryPtr &result)
    {
    result = nullptr;
    if (IsCGType (value, "ParityRegion"))
        {
        CGParityRegionDetail detail;
        CGParityRegionFlags flags;

        flags.placement_defined = ReadTagNestedLoops (value, "loops", detail.placement);
        result = m_factory.Create (detail);//ParityRegion(detail.placement,detail.cornerA,detail.cornerB,detail.bSolidFlag);
        return true;
        }
    return false;
    }
#endif
#ifdef abc
bool HasNamedValue (Json::Value const &value, Utf8CP name)
    {
    if (value.isObject ())
        {
        for (Json::Value::iterator iter = value.begin(); iter != value.end(); iter++)
            {
            Utf8CP childName = iter.memberName();
            if (0 == stricmp (name, childName))
                return true;
            }
        }
    return false;
    }
#endif  
Utf8CP HasNamedString (Json::Value const &value, Utf8StringCR name)
    {
    if (value.isObject ())
        {
        for (Json::Value::iterator iter = value.begin(); iter != value.end(); iter++)
            {
            Utf8CP childName = iter.memberName();
            if (0 == name.CompareToI (childName))
                {
                Json::Value &childValue = *iter;
                if (childValue.isString ())
                    return childValue.asCString ();
                }
            }
        }
    return nullptr;
    }

bool IsCGType (Json::Value const &value, Utf8CP targetTypeName) override
    {
#ifdef CGTypeAppearsAsTypeProperty
    if (value.isObject ())
        {
        for (Json::Value::iterator iter = value.begin(); iter != value.end(); iter++)
            {
            Utf8CP childName = iter.memberName();
            if (0 == s_jsonKey_type.CompareToI (childName))
                {
                Json::Value &childValue = *iter;
                return 0 == BeStringUtilities::Stricmp (targetTypeName, childValue.asCString ());
                }
            }
        }
    return false;
#else
    return true;  // We have to trust that the router went to the right parser
#endif
    }
    
bool ReadTag_AnyIGeometry (Json::Value const &value, IGeometryPtr &result)
    {
    // expect a property name to be in the parse table . .
    // Return immediately when one is found . . 
    if (value.isObject ())
        {
        for (Json::Value::iterator iter = value.begin(); iter != value.end(); iter++)
            {
            ParseMethod parseMethod = FindParseMethod (iter.memberName());
            if (parseMethod != nullptr)
                {
                Json::Value &memberValue = *iter;
                return (this->*parseMethod) (memberValue, result);
                }
            }
        }
    return false;
    }
public: bool ReadTag_Action (JsonValueCR source)
    {
    DVec3d vector;
    if (ReadTagDVector3d(source, "moveOrigin", vector))
        {
        Transform shift = Transform::From (vector);
        m_placement= shift * m_placement;
        return true;
        }
    else if (ReadTagDVector3d(source, "setOrigin", vector))
        {
        m_placement = Transform::From (vector);
        return true;
        }
    return false;
    }
public: IGeometryPtr ApplyState (IGeometryPtr g)
    {
    if (!m_placement.IsIdentity ())
        g->TryTransformInPlace (m_placement);
    return g;
    }
public: bool TryParse (JsonValueCR source, bvector<IGeometryPtr> &geometry, bmap<OrderedIGeometryPtr, BeExtendedData> &extendedData)
    {
    IGeometryPtr result;
    if (source.isObject ())
        {
        if (ReadTag_AnyIGeometry (source, result))
            {
            if (m_factory.m_groupMembers.size () > 0)
                {
                for (auto &member : m_factory.m_groupMembers)
                    geometry.push_back (ApplyState (member));
                m_factory.m_groupMembers.clear ();
                }
            else
                geometry.push_back (result);
            }
        else if (ReadTag_Action (source))
            {
            }
        }
    else if (source.isArray ())
        {
        int n = source.size ();
        for (int i = 0; i < n; i++)
            {
            TryParse (source[i], geometry, extendedData);
            }
        }
    return geometry.size () > 0;
    }    
};

    
BeJsonToCGReaderImplementation::ParseDictionary BeJsonToCGReaderImplementation::s_parseTable;


Utf8String BeJsonToCGReaderImplementation::s_jsonKey_type ("Type");
int BeJsonToCGReaderImplementation::s_defaultDebug = 10;

bool BentleyGeometryJson::TryJsonValueToGeometry
(
JsonValueCR source,
bvector<IGeometryPtr> &geometry
)
    {
    if (IModelJson::TryIModelJsonValueToGeometry (source, geometry))
        return true;
    IGeometryCGFactory factory;
    BeJsonToCGReaderImplementation parser (factory);
    BeExtendedDataGeometryMap extendedData;
    return parser.TryParse(source, geometry, extendedData);
    }


bool BentleyGeometryJson::TryJsonStringToGeometry
(
Utf8StringCR string,
bvector<IGeometryPtr> &geometry
)
    {
    geometry.clear ();
    Json::Value value;
    Json::Reader::Parse (string, value, false);
    if (value.isNull ())
        return false;
    return TryJsonValueToGeometry (value, geometry);
    }


static size_t DumpIndent (size_t n)
    {
    GEOMAPI_PRINTF ("\n");
    for (size_t i = 0; i < n; i++)
        GEOMAPI_PRINTF ("  ");
    return 2 * n;
    }
static bool IsBreak (char c)
    {
    return c == ',' || c == '[' || c == ']';
    }
static size_t CharWeight (char c)
    {
    if (c == '[' || c == ']')
        return 15;
    return 1;
    }
// Look at contents starting at i0.
// skip over numbers, decimals, comma, and plus/minus signs.
// Return first index other than those.
static size_t  LookAheadOverNumbers (Utf8StringCR string, size_t i0)
    {
    auto n = string.size ();
    for (;i0 < n; i0++)
        {
        char c = string[i0];
        if (c >= '0' && c <= '9')
            continue;
        if (c == ',')
            continue;
        if (c == '.')
            continue;
        break;
        }
    return i0;
    }
void BentleyGeometryJson::DumpJson (Utf8StringCR string)
    {
    size_t columnLimit = 70;
    size_t columnLimit2 = 90;
    size_t blockPad = 10;
    size_t indent = 0;
    size_t column = 0;
    size_t n = string.size ();
    for (size_t i0 = 0; i0 < n; i0++)
        {
        char c = string[i0];
        if (c == '{')
            {
            indent++;
            column = DumpIndent (indent);
            GEOMAPI_PRINTF ("%c", c);
            }
        else if (c == '}')
            {
            column = DumpIndent (indent);
            indent--;
            GEOMAPI_PRINTF ("%c", c);
            }
        else if (c == '[')
            {
            size_t i1 = LookAheadOverNumbers (string, i0 + 1);
            if (i1 > i0 && string[i1] == ']')
                {
                // absorb trailing comma . . 
                if (i1 + 1 < n && string[i1 + 1] == ',')
                    i1++;
                for (;i0 <= i1; i0++)
                    {
                    char c2 = string[i0];
                    GEOMAPI_PRINTF ("%c", c2);
                    column++;
                    if (c2 == ',' && column > columnLimit2)
                        column = DumpIndent (indent);
                    }
                if (column + blockPad >= columnLimit)
                    column = DumpIndent (indent);
                i0 = i1;
                //GEOMAPI_PRINTF ("!");
                // This is the body of an array of comma-separated numbers, and they all fit on a (longish) line . . .  fast dump.
                // the entire block was dumped, including the final bracket.
                // i0 sits on the bracket.
                }
            else
                {
                // strong left bracket ..
                indent++;
                column = DumpIndent (indent);
                GEOMAPI_PRINTF ("%c", c);
                column = DumpIndent (indent);
                }
            }
        else if (c == ']')
            {
            // string right bracket . . 
            column = DumpIndent (indent);
            indent--;
            GEOMAPI_PRINTF ("%c", c);
            }
        else
            {
            GEOMAPI_PRINTF("%c", c);
            column += CharWeight (c);
            }
        if (column > columnLimit && IsBreak(c))
            {
            column = DumpIndent (indent);
            }
        }
    }
END_BENTLEY_GEOMETRY_NAMESPACE
