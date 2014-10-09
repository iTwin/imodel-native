/*--------------------------------------------------------------------------------------+
|
|  $Source: src/BeXmlCGStreamReader.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "ECObjectsPch.h"

BEGIN_BENTLEY_ECOBJECT_NAMESPACE
typedef struct PlacementOriginZX const &PlacementOriginZXCR;
typedef struct PlacementOriginZX &PlacementOriginZXR;

// mappings from managed idioms to native
#define DVector3d DVec3d
#define String Utf8String

#define InputParamTypeFor_DPoint3d DPoint3dCR
#define InputParamTypeFor_Transform TransformCR

#define InputParamTypeFor_DPoint2d DPoint2dCR
#define InputParamTypeFor_DVector3d DVec3dCR
#define InputParamTypeFor_PlacementOriginZX PlacementOriginZXCR

#define InputParamTypeFor_double double
#define InputParamTypeFor_Angle Angle
#define InputParamTypeFor_int int
#define InputParamTypeFor_bool bool
#define InputParamTypeFor_String Utf8StringCR
#define InputParamTypeFor_LoopType int

#define InputParamTypeFor_IPoint IGeometryPtr
#define InputParamTypeFor_ISinglePoint IGeometryPtr
#define InputParamTypeFor_IPrimitiveCurve IGeometryPtr
#define InputParamTypeFor_ICurve IGeometryPtr
#define InputParamTypeFor_ICurveChain IGeometryPtr
#define InputParamTypeFor_ISurface IGeometryPtr
#define InputParamTypeFor_ISurfacePatch IGeometryPtr
#define InputParamTypeFor_IParametricSurface IGeometryPtr
#define InputParamTypeFor_ISweepable IGeometryPtr
#define InputParamTypeFor_ISolid IGeometryPtr
#define InputParamTypeFor_IGeometry IGeometryPtr

#define ReaderTypeFor_ICurve IGeometryPtr
#define ReaderTypeFor_ICurveChain IGeometryPtr
#define ReaderTypeFor_IGeometry IGeometryPtr
#define ReaderTypeFor_IParametricSurface IGeometryPtr
#define ReaderTypeFor_IPrimitiveCurve IGeometryPtr
#define ReaderTypeFor_ISurface IGeometryPtr
#define ReaderTypeFor_ISweepable IGeometryPtr

#define s_default_ICurve nullptr
#define s_default_ICurveChain nullptr
#define s_default_IGeometry nullptr
#define s_default_IParametricSurface nullptr
#define s_default_IPrimitiveCurve nullptr
#define s_default_ISurface nullptr
#define s_default_ISweepable nullptr

#define s_default_LoopType 0
typedef int LoopType;

template<typename T, typename TBlocked>
static void CopyToBlockedVector (bvector<T> const &source, TBlocked &dest)
    {
    dest.clear ();
    dest.reserve (source.size ());
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


#include "CGNativeFactoryImplementations.h"


static DPoint3d s_default_DPoint3d = DPoint3d::From (0,0,0);
static DPoint2d s_default_DPoint2d = DPoint2d::From (0,0);
static DVec3d   s_default_DVector3d = DVec3d::From (0,0,0);
static double   s_default_double   = 0.0;
static int      s_default_int      = 0;
static bool     s_default_bool     = false;
static Angle     s_default_Angle    = Angle::FromRadians (0.0);
static Utf8String s_default_String = Utf8String ();
static PlacementOriginZX s_default_PlacementOriginZX = PlacementOriginZX::FromIdentity ();
static Transform s_default_Transform = Transform::FromIdentity ();

#ifdef abc
// If primitve has a child curve vector, just extract it.
// Otherwise put the primitive in a new curve vector.
static CurveVectorPtr CurveVectorOf (ICurvePrimitivePtr primitive, CurveVector::BoundaryType btype)
    {
    if (NULL == primitive.get())
        return CurveVectorPtr ();
    CurveVectorCP child = primitive->GetChildCurveVectorCP ();
    if (NULL != child)
        {
        return child->Clone ();
        }
    CurveVectorPtr vector = CurveVector::Create (btype);
    vector->push_back (primitive);
    return vector;
    }
#endif
static int s_defaultDebug = 0;

static char const * s_nodeTypeNames[] =
    {
    "None",
    "Element",
    "Attribute",
    "Text",
    "CDATA",
    "EntityReference",
    "Entity",
    "ProcessingInstruction",
    "Comment",
    "Document",
    "DocumentType",
    "DocumentFragment",
    "Notation",
    "{w}", //"Whitespace",
    "{W}", //"SignificantWhitespace",
    "EndElement",
    "EndEntity",
    "XmlDeclaration"
    };

static char const *GetBeXmlNodeTypeString (BeXmlReader::NodeType nodeType)
    {
    if (nodeType < BeXmlReader::NODE_TYPE_XmlDeclaration)
        return s_nodeTypeNames[(Int32)nodeType];
    return "(Unknown BeXmlReader::NodeType)";
    }

struct BeXmlCGStreamReaderImplementation
{
typedef bool (BeXmlCGStreamReaderImplementation::*ParseMethod)(IGeometryPtr &);
typedef bmap <Utf8String, ParseMethod> ParseDictionary;

static ParseDictionary s_parseTable;

static void InitParseTable ()
    {
    if (s_parseTable.empty ())
        {
        s_parseTable[Utf8String("LineSegment")] = &ReadILineSegment;
        s_parseTable[Utf8String("CircularArc")] = &ReadICircularArc;
        s_parseTable[Utf8String("DgnBox")] = &ReadIDgnBox;
        s_parseTable[Utf8String("DgnSphere")] = &ReadIDgnSphere;
        s_parseTable[Utf8String("DgnCone")] = &ReadIDgnCone;
        s_parseTable[Utf8String("DgnTorusPipe")] = &ReadIDgnTorusPipe;
        s_parseTable[Utf8String("Block")] = &ReadIBlock;
        s_parseTable[Utf8String("CircularCone")] = &ReadICircularCone;
        s_parseTable[Utf8String("CircularCylinder")] = &ReadICircularCylinder;
        s_parseTable[Utf8String("CircularDisk")] = &ReadICircularDisk;
        s_parseTable[Utf8String("Coordinate")] = &ReadICoordinate;
        s_parseTable[Utf8String("EllipticArc")] = &ReadIEllipticArc;
        s_parseTable[Utf8String("EllipticDisk")] = &ReadIEllipticDisk;
        s_parseTable[Utf8String("SingleLineText")] = &ReadISingleLineText;
        s_parseTable[Utf8String("SkewedCone")] = &ReadISkewedCone;
        s_parseTable[Utf8String("Sphere")] = &ReadISphere;
        s_parseTable[Utf8String("TorusPipe")] = &ReadITorusPipe;
        s_parseTable[Utf8String("Vector")] = &ReadIVector;
        s_parseTable[Utf8String("IndexedMesh")] = &ReadIIndexedMesh;
        s_parseTable[Utf8String("AdjacentSurfacePatches")] = &ReadIAdjacentSurfacePatches;
        s_parseTable[Utf8String("BsplineCurve")] = &ReadIBsplineCurve;
        s_parseTable[Utf8String("BsplineSurface")] = &ReadIBsplineSurface;
        s_parseTable[Utf8String("CurveChain")] = &ReadICurveChain;
        s_parseTable[Utf8String("CurveGroup")] = &ReadICurveGroup;
        s_parseTable[Utf8String("CurveReference")] = &ReadICurveReference;
        s_parseTable[Utf8String("Group")] = &ReadIGroup;
        s_parseTable[Utf8String("InterpolatingCurve")] = &ReadIInterpolatingCurve;
        s_parseTable[Utf8String("LineString")] = &ReadILineString;
        s_parseTable[Utf8String("Operation")] = &ReadIOperation;
        s_parseTable[Utf8String("ParametricSurfacePatch")] = &ReadIParametricSurfacePatch;
        s_parseTable[Utf8String("PointChain")] = &ReadIPointChain;
        s_parseTable[Utf8String("PointGroup")] = &ReadIPointGroup;
        s_parseTable[Utf8String("Polygon")] = &ReadIPolygon;
        s_parseTable[Utf8String("PrimitiveCurveReference")] = &ReadIPrimitiveCurveReference;
        s_parseTable[Utf8String("SharedGroupDef")] = &ReadISharedGroupDef;
        s_parseTable[Utf8String("SharedGroupInstance")] = &ReadISharedGroupInstance;
        s_parseTable[Utf8String("ShelledSolid")] = &ReadIShelledSolid;
        s_parseTable[Utf8String("SolidBySweptSurface")] = &ReadISolidBySweptSurface;
        s_parseTable[Utf8String("SolidByRuledSweep")] = &ReadISolidByRuledSweep;
        s_parseTable[Utf8String("SurfaceByRuledSweep")] = &ReadISurfaceByRuledSweep;
        s_parseTable[Utf8String("SolidGroup")] = &ReadISolidGroup;
        s_parseTable[Utf8String("Spiral")] = &ReadISpiral;
        s_parseTable[Utf8String("SurfaceBySweptCurve")] = &ReadISurfaceBySweptCurve;
        s_parseTable[Utf8String("SurfaceGroup")] = &ReadISurfaceGroup;
        s_parseTable[Utf8String("SurfacePatch")] = &ReadISurfacePatch;
        s_parseTable[Utf8String("TransformedGeometry")] = &ReadITransformedGeometry;
        s_parseTable[Utf8String("DgnExtrusion")] = &ReadIDgnExtrusion;
        s_parseTable[Utf8String("DgnRotationalSweep")] = &ReadIDgnRotationalSweep;
        s_parseTable[Utf8String("DgnRuledSweep")] = &ReadIDgnRuledSweep;
        s_parseTable[Utf8String("TransitionSpiral")] = &ReadITransitionSpiral;
        }
    }

BeXmlReader &m_reader;
ICGFactory &m_factory;
int m_debug;
void Show (CharCP name)
    {
    BeXmlReader::NodeType nodeType = m_reader.GetCurrentNodeType ();
    printf ("%s (%s)", name, GetBeXmlNodeTypeString (nodeType));
    Utf8String s;
    if (nodeType == BeXmlReader::NODE_TYPE_Element
        || nodeType == BeXmlReader::NODE_TYPE_EndElement)
        {
        m_reader.GetCurrentNodeName (s);
        printf ("%s", s);
        }
    else if (nodeType == BeXmlReader::NODE_TYPE_Text)
        {
        m_reader.GetCurrentNodeValue (s);
        printf ("{%s}", s);
        }

    printf ("\n");
    }
BeXmlCGStreamReaderImplementation::BeXmlCGStreamReaderImplementation (BeXmlReader &reader, ICGFactory &factory)
    : m_reader(reader), m_factory(factory), m_debug (s_defaultDebug)
    {
    InitParseTable ();
    ReadToElement ();
    }

// On input: XML reader at element start.
//           XML reader at first child element.
bool ReadToChild ()
    {
    if (m_debug > 9)
        Show ("ReadToChild");
    BeXmlReader::ReadResult status = m_reader.ReadTo (BeXmlReader::NODE_TYPE_Element);
    if (status != BeXmlReader::READ_RESULT_Success)
        return false;
    m_reader.GetCurrentNodeName (m_currentElementName);
    return true;
    }

void ReadOverWhiteSpace ()
    {
    BeXmlReader::NodeType nodeType;
    for (;;)
        {
        m_reader.Read ();
        nodeType = m_reader.GetCurrentNodeType ();
        if (nodeType == BeXmlReader::NODE_TYPE_SignificantWhitespace)
            continue;
        if (nodeType == BeXmlReader::NODE_TYPE_Whitespace)
            continue;
        break;
        }
    if (nodeType == BeXmlReader::NODE_TYPE_Element)
        m_reader.GetCurrentNodeName (m_currentElementName);
    }

bool ReadEndElement ()
    {
    assert(m_reader.GetCurrentNodeType () == BeXmlReader::NODE_TYPE_EndElement);
    if (m_debug > 9)
        Show ("ReadEndElement");
    ReadOverWhiteSpace ();
    return false;
    }
bool IsStartElement ()
    {
    if (m_debug > 9)
        Show ("IsStartElement");
    return m_reader.GetCurrentNodeType () == BeXmlReader::NODE_TYPE_Element;
    }
bool SkipUnexpectedTag ()
    {
    if (m_debug > 9)
        Show ("SkipUnexpectedTag");
    return false;
    }

    // This is assigned by ReadToElement ...
    Utf8String m_currentElementName;
    // For reuse by all readers.
    WString m_currentValueW;
    Utf8String m_currentValue8;

bool AdvanceAfterContentExtraction ()
    {
    m_reader.ReadTo (BeXmlReader::NODE_TYPE_EndElement);
    ReadOverWhiteSpace ();
    BeXmlReader::NodeType nodeType = m_reader.GetCurrentNodeType ();
    if (nodeType == BeXmlReader::NODE_TYPE_Element)
        {
        m_reader.GetCurrentNodeName (m_currentElementName);
        return true;
        }
    else if (nodeType == BeXmlReader::NODE_TYPE_EndElement)
        {
        return true;
        }
    return false;
    }

bool ReadToElement ()
    {
    BeXmlReader::ReadResult status = m_reader.ReadTo (BeXmlReader::NODE_TYPE_Element);
    if (status != BeXmlReader::READ_RESULT_Success)
        return false;
    m_reader.GetCurrentNodeName (m_currentElementName);
    return true;
    }

bool CurrentElementNameMatch (CharCP name)
    {
    return 0 == m_currentElementName.CompareToI (name);
    }


bool ReadTagDPoint3d (CharCP name, DPoint3dR value)
    {
    if (name != nullptr && !CurrentElementNameMatch (name))
        return false;
    m_reader.ReadTo (BeXmlReader::NODE_TYPE_Text);
    m_reader.GetCurrentNodeValue (m_currentValue8);
    bool stat = 3 == sscanf (&m_currentValue8[0],
              "%lf,%lf,%lf", &value.x, &value.y, &value.z);
    AdvanceAfterContentExtraction ();
    return stat;
    }

bool ReadTagTransform (CharCP name, TransformR value)
    {
    if (!CurrentElementNameMatch (name))
        return false;
    m_reader.ReadTo (BeXmlReader::NODE_TYPE_Text);
    m_reader.GetCurrentNodeValue (m_currentValue8);
    bool stat = 9 == sscanf (&m_currentValue8[0],
              "%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf",
              &value.form3d[0][0], &value.form3d[0][1], &value.form3d[0][2], &value.form3d[0][3],
              &value.form3d[1][0], &value.form3d[1][1], &value.form3d[1][2], &value.form3d[1][3],
              &value.form3d[2][0], &value.form3d[1][1], &value.form3d[2][2], &value.form3d[2][3]);
    AdvanceAfterContentExtraction ();
    return stat;
    }
    
bool ReadTagLoopType (CharCP name, LoopType value)
    {
    if (!CurrentElementNameMatch (name))
        return false;
    m_reader.ReadTo (BeXmlReader::NODE_TYPE_Text);
    m_reader.GetCurrentNodeValue (m_currentValue8);
    // TODO: find the magic strings
    bool stat = true;
    value = 0;
    AdvanceAfterContentExtraction ();
    return stat;
    }


bool ReadListOfDPoint3d (CharCP listName, CharCP componentName, bvector<DPoint3d> &values)
    {
    if (!CurrentElementNameMatch (listName))
        return false;
    ReadToChild ();
    DPoint3d xyz;
    while (IsStartElement ()
            && ReadTagDPoint3d (nullptr, xyz))
        {
        values.push_back (xyz);
        }
    // Get out of the primary element ..
    ReadEndElement ();
    return true;
    }

bool ReadListOfint (CharCP listName, CharCP componentName, bvector<int> &values)
    {
    if (!CurrentElementNameMatch (listName))
        return false;
    ReadToChild ();
    int xyz;
    while (IsStartElement ()
            && ReadTagint (nullptr, xyz))
        {
        values.push_back (xyz);
        }
    // Get out of the primary element ..
    ReadEndElement ();
    return true;
    }

bool ReadListOfdouble (CharCP listName, CharCP componentName, bvector<double> &values)
    {
    if (!CurrentElementNameMatch (listName))
        return false;
    ReadToChild ();
    double xyz;
    while (IsStartElement ()
            && ReadTagdouble (nullptr, xyz))
        {
        values.push_back (xyz);
        }
    // Get out of the primary element ..
    ReadEndElement ();
    return true;
    }

bool ReadListOfDPoint2d (CharCP listName, CharCP componentName, bvector<DPoint2d> &values)
    {
    if (!CurrentElementNameMatch (listName))
        return false;
    ReadToChild ();
    DPoint2d xyz;
    while (IsStartElement ()
            && ReadTagDPoint2d (nullptr, xyz))
        {
        values.push_back (xyz);
        }
    // Get out of the primary element ..
    ReadEndElement ();
    return true;
    }


//=======================================================================================
bool ReadListOfDVector3d (CharCP listName, CharCP componentName, bvector<DVector3d> &values)
    {
    if (!CurrentElementNameMatch (listName))
        return false;
    ReadToChild ();
    DVector3d xyz;
    while (IsStartElement ()
            && ReadTagDVector3d (nullptr, xyz))
        {
        values.push_back (xyz);
        }
    // Get out of the primary element ..
    ReadEndElement ();
    return true;
    }


//=======================================================================================
bool ReadListOfISurfacePatch (CharCP listName, CharCP componentName, bvector<IGeometryPtr> &values)
    {
    return false;
    }
bool ReadListOfISweepable (CharCP listName, CharCP componentName, bvector<IGeometryPtr> &values)
    {
    return false;
    }

bool ReadListOfIPrimitiveCurve (CharCP listName, CharCP componentName, bvector<ICurvePrimitivePtr> &values)
    {
    if (!CurrentElementNameMatch (listName))
        return false;
    ReadToChild ();
    ICurvePrimitivePtr cp;
    while (IsStartElement ()
            && ReadTag_AnyCurvePrimitive (cp)
            && cp.IsValid ())
        {
        values.push_back (cp);
        }
    // Get out of the primary element ..
    ReadEndElement ();
    return true;
    }

bool ReadListOfICurve (CharCP listName, CharCP componentName, bvector<IGeometryPtr> &values)
    {
    return false;
    }
bool ReadListOfICurveChain (CharCP listName, CharCP componentName, bvector<IGeometryPtr> &values)
    {
    return false;
    }
bool ReadListOfISolid (CharCP listName, CharCP componentName, bvector<IGeometryPtr> &values)
    {
    return false;
    }
bool ReadListOfISurface (CharCP listName, CharCP componentName, bvector<IGeometryPtr> &values)
    {
    return false;
    }
bool ReadListOfIPoint (CharCP listName, CharCP componentName, bvector<IGeometryPtr> &values)
    {
    return false;
    }





bool ReadListOfIGeometry (CharCP listName, CharCP componentName, bvector<IGeometryPtr> &values)
    {
    return false;
    }





bool ReadListOfISinglePoint (CharCP listName, CharCP componentName, bvector<IGeometryPtr> &values)
    {
    return false;
    }






//=======================================================================================
bool ReadTagString (CharCP name, Utf8StringR)
    {
    return false;
    }

bool ReadTagDPoint2d (CharCP name, DPoint2dR value)
    {
    if (nullptr != name  && !CurrentElementNameMatch (name))
        return false;
    m_reader.ReadTo (BeXmlReader::NODE_TYPE_Text);
    m_reader.GetCurrentNodeValue (m_currentValue8);
    bool stat = 2 == sscanf (&m_currentValue8[0],
              "%lf,%lf", &value.x, &value.y);
    AdvanceAfterContentExtraction ();
    return stat;
    }

bool ReadTagDVector3d(CharCP name, DVec3dR value)
    {
    if (nullptr != name  && !CurrentElementNameMatch (name))
        return false;
    m_reader.ReadTo (BeXmlReader::NODE_TYPE_Text);
    m_reader.GetCurrentNodeValue (m_currentValue8);
    bool stat = 3 == sscanf (&m_currentValue8[0],
              "%lf,%lf,%lf", &value.x, &value.y, &value.z);
    AdvanceAfterContentExtraction ();
    return stat;
    }

bool ReadTagbool(CharCP name, bool &value)
    {
    if (nullptr != name  && !CurrentElementNameMatch (name))
        {
        // allow bSolidFlag as synonym for capped ...
        if (0 != stricmp (name, "capped")
            || !CurrentElementNameMatch ("bSolidFlag")
            )
            return false;
        }
    m_reader.ReadTo (BeXmlReader::NODE_TYPE_Text);
    m_reader.GetCurrentNodeValue (m_currentValue8);
    bool stat = false;
    if (0 == m_currentValue8.CompareToI ("true"))
        {
        value = true;
        stat = true;
        }
    if (0 == m_currentValue8.CompareToI ("false"))
        {
        value = false;
        stat = true;
        }
    if (0 == m_currentValue8.CompareToI ("1"))
        {
        value = true;
        stat = true;
        }
    if (0 == m_currentValue8.CompareToI ("0"))
        {
        value = false;
        stat = true;
        }
    AdvanceAfterContentExtraction ();        
    return stat;
    }

bool ReadTagdouble(CharCP name, double &value)
    {
    {
    if (nullptr != name  && !CurrentElementNameMatch (name))
        return false;
    m_reader.ReadTo (BeXmlReader::NODE_TYPE_Text);
    m_reader.GetCurrentNodeValue (m_currentValue8);
    bool stat = 1 == sscanf (&m_currentValue8[0],
              "%lf", &value);
    AdvanceAfterContentExtraction ();
    return stat;
    }    }
    
bool ReadTagint(CharCP name, int &value)
    {
    {
    if (nullptr != name  && !CurrentElementNameMatch (name))
        return false;
    m_reader.ReadTo (BeXmlReader::NODE_TYPE_Text);
    m_reader.GetCurrentNodeValue (m_currentValue8);
    bool stat = 1 == sscanf (&m_currentValue8[0],
              "%d", &value);
    AdvanceAfterContentExtraction ();
    return stat;
    }    }

bool ReadTagPlacementOriginZX (CharCP name, PlacementOriginZX &value)
    {
    if (CurrentElementNameMatch ("placement")
        && ReadToChild ())
        {
        // Start with the system default for each field ....
        DPoint3d origin = s_default_DPoint3d;
        DVec3d vectorZ = s_default_DVector3d;
        DVec3d vectorX = s_default_DVector3d;

        for (;IsStartElement ();)
            {
            if (ReadTagDPoint3d ("origin", origin))
                continue;

            if (ReadTagDPoint3d ("vectorZ", vectorZ))
                continue;

            if (ReadTagDPoint3d ("vectorX", vectorX))
                continue;

            if (!SkipUnexpectedTag ())
                return false;
            }
        // Get out of the primary element ..
        ReadEndElement ();
        value.InitOriginVectorZVectorX (origin, vectorZ, vectorX);
        return true;
        }
    return false;
    }

bool ReadTagAngle (CharCP name, Angle &value)
    {
    double degrees;
    if (ReadTagdouble (name, degrees))
        {
        value = Angle::FromDegrees (degrees);
        return true;
        }
    return false;
    }

#include "nativeCGReaderH.h"

bool ReadTag_AnyGeometry (IGeometryPtr &value)
    {
    ParseMethod parseMethod = s_parseTable[m_currentElementName];
    if (parseMethod != nullptr)
        {
        if ((this->*parseMethod)(value))
            {
            return true;
            }
        }
    return false;
    }

bool ReadTag_AnyCurvePrimitive (ICurvePrimitivePtr &value)
    {
    IGeometryPtr geometry;
    if (ReadTag_AnyGeometry (geometry)
        && geometry.IsValid ())
        {
        value = geometry->GetAsICurvePrimitive ();
        return value.IsValid ();
        }
    return false;
    }




public: bool TryParse (bvector<IGeometryPtr> &geometry, size_t maxDepth)
    {
    size_t count = 0;

    for (;IsStartElement ();)
        {
        ParseMethod parseMethod = s_parseTable[m_currentElementName];
        if (parseMethod == nullptr)
            {
            if (maxDepth > 0)
                {
                ReadToElement ();
                if (!TryParse (geometry, maxDepth - 1))
                  break;
                AdvanceAfterContentExtraction ();
                }
            break;
            }
        else
            {
            IGeometryPtr result;
            if ((this->*parseMethod)(result))
                {
                geometry.push_back (result);
                count++;
                }
            else
                {
                break;
                }
            }
        }
    return count > 0;
    }
};


void ShowAllNodeTypes (Utf8CP beXmlCGString)
    {
    BeXmlStatus status;
    BeXmlReaderPtr reader = BeXmlReader::CreateAndReadFromString (status, beXmlCGString);
    printf ("<XMLNodes>\n");
    int depth = 0;
    for (;BeXmlReader::READ_RESULT_Success == reader->Read ();)
        {
        BeXmlReader::NodeType nodeType = reader->GetCurrentNodeType();
        if (nodeType == BeXmlReader::NODE_TYPE_EndElement)
            depth--;
        if (nodeType == BeXmlReader::NODE_TYPE_Whitespace
            || nodeType == BeXmlReader::NODE_TYPE_SignificantWhitespace
            || nodeType == BeXmlReader::NODE_TYPE_Text
            )
            {
            // (inline)
            }
        else
            {
            printf ("\n");
            for (int i = 0; i < depth; i++)
                printf ("  ");
            }
        printf (" %s", GetBeXmlNodeTypeString (nodeType));
        if (nodeType == BeXmlReader::NODE_TYPE_Element)
            depth++;
        }
    printf ("\n</XMLNodes>");
    }

bool BeXmlCGStreamReader::TryParse (Utf8CP beXmlCGString, bvector<IGeometryPtr> &geometry, size_t maxDepth)
    {
    static int s_preview = 0;
    if (s_preview)
        ShowAllNodeTypes (beXmlCGString);
    BeXmlStatus status;
    IGeometryCGFactory factory;
    BeXmlReaderPtr reader = BeXmlReader::CreateAndReadFromString (status, beXmlCGString);
    BeXmlCGStreamReaderImplementation parser (*reader, factory);
    return parser.TryParse(geometry, maxDepth);
    }

BeXmlCGStreamReaderImplementation::ParseDictionary BeXmlCGStreamReaderImplementation::s_parseTable;

END_BENTLEY_ECOBJECT_NAMESPACE