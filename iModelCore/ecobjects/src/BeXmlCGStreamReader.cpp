/*--------------------------------------------------------------------------------------+
|
|  $Source: src/BeXmlCGStreamReader.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "ECObjectsPch.h"
#include "MSXmlBinary\MSXmlBinaryReader.h"

BEGIN_BENTLEY_ECOBJECT_NAMESPACE
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
        s_parseTable[Utf8String("ExtendedObject")] = &ReadExtendedObject;
        s_parseTable[Utf8String("ExtendedData")] = &ReadExtendedData;
        }
    }

IBeXmlReader &m_reader;
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
BeXmlCGStreamReaderImplementation::BeXmlCGStreamReaderImplementation (IBeXmlReader &reader, ICGFactory &factory)
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

// On input: XML reader at element start.
//           XML reader at first child element, or immediate end element.
bool ReadToChildOrEnd ()
    {
    if (m_debug > 9)
        Show ("ReadToChildOrEnd");
    ReadOverWhiteSpace ();
    auto nodeType = m_reader.GetCurrentNodeType ();
    return nodeType == BeXmlReader::NODE_TYPE_Element
        || nodeType == BeXmlReader::NODE_TYPE_EndElement;
    }


bool ReadToText ()
    {
    if (m_debug > 9)
        Show ("ReadToText");
    BeXmlReader::ReadResult status = m_reader.ReadTo (BeXmlReader::NODE_TYPE_Text);
    if (status != BeXmlReader::READ_RESULT_Success)
        return false;
    m_reader.GetCurrentNodeValue (m_currentValue8);
    if (m_debug > 9)
        Show ("Text");
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
    auto nodeType = m_reader.GetCurrentNodeType ();

    BeAssert(nodeType == BeXmlReader::NODE_TYPE_EndElement || nodeType == BeXmlReader::NODE_TYPE_None);
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
    AdvanceAfterContentExtraction ();
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

bool CurrentElementNameMatch (CharCP nameA, CharCP nameB)
    {
    return 0 == m_currentElementName.CompareToI (nameA)
        || 0 == m_currentElementName.CompareToI (nameB);
    }

bool ReadTagDPoint3d (CharCP name, DPoint3dR value)
    {
    if (name != nullptr && !CurrentElementNameMatch (name))
        return false;
    bool stat = false;
    if (ReadToText ())
        {
        stat = 3 == sscanf (&m_currentValue8[0],
                  "%lf,%lf,%lf", &value.x, &value.y, &value.z);
        AdvanceAfterContentExtraction ();
        }
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
    ReadToChildOrEnd ();
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
    ReadToChildOrEnd ();
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
    ReadToChildOrEnd ();
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
    ReadToChildOrEnd ();
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
    ReadToChildOrEnd ();
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
    BeAssert(false);
    return false;
    }

bool ReadListOf_AnyCurveVector (CharCP listName, CharCP componentName, bvector<CurveVectorPtr> &values)
    {
    if (!CurrentElementNameMatch (listName))
        return false;
    ReadToChildOrEnd ();
    CurveVectorPtr cp;
    while (IsStartElement ()
            && ReadTag_AnyCurveVector (componentName, cp)
            && cp.IsValid ())
        {
        values.push_back (cp);
        }
    // Get out of the primary element ..
    ReadEndElement ();
    return true;
    }


bool ReadListOf_AnyGeometry (CharCP listName, CharCP componentName, bvector<IGeometryPtr> &values)
    {
    if (!CurrentElementNameMatch (listName))
        return false;
    ReadToChildOrEnd ();
    IGeometryPtr cp;
    while (IsStartElement ()
            && ReadTag_AnyGeometry (componentName, cp)
            && cp.IsValid ())
        {
        values.push_back (cp);
        }
    // Get out of the primary element ..
    ReadEndElement ();
    return true;
    }


bool ReadListOf_AnyICurvePrimitive (CharCP listName, CharCP componentName, bvector<ICurvePrimitivePtr> &values)
    {
    if (!CurrentElementNameMatch (listName))
        return false;
    ReadToChildOrEnd ();
    ICurvePrimitivePtr cp;
    while (IsStartElement ()
            && ReadTag_AnyICurvePrimitive (cp)    // ignore the component name !!!
            && cp.IsValid ())
        {
        values.push_back (cp);
        }
    // Get out of the primary element ..
    ReadEndElement ();
    return true;
    }

bool ReadListOf_AnyICurveChain (CharCP listName, CharCP componentName, bvector<CurveVectorPtr> &values)
    {
    return ReadListOf_AnyCurveVector (listName, componentName, values);
    }

bool ReadListOfICurve (CharCP listName, CharCP componentName, bvector<IGeometryPtr> &values)
    {
    BeAssert(false);
    return false;
    }
bool ReadListOfICurveChain (CharCP listName, CharCP componentName, bvector<IGeometryPtr> &values)
    {
    if (!CurrentElementNameMatch (listName))
        return false;
    ReadToChildOrEnd ();
    IGeometryPtr chain;
    while (IsStartElement ()
            && ReadICurveChain (chain))
        {
        CurveVectorPtr cv = chain->GetAsCurveVector ();
        if (cv.IsValid ())
          values.push_back (IGeometry::Create (cv));
        }
    // Get out of the primary element ..
    ReadEndElement ();
    return true;
    }

bool ReadListOfISolid (CharCP listName, CharCP componentName, bvector<IGeometryPtr> &values)
    {
    BeAssert(false);
    return false;
    }
bool ReadListOfISurface (CharCP listName, CharCP componentName, bvector<IGeometryPtr> &values)
    {
    BeAssert(false);
    return false;
    }
bool ReadListOfIPoint (CharCP listName, CharCP componentName, bvector<IGeometryPtr> &values)
    {
    BeAssert(false);
    return false;
    }





bool ReadListOfIGeometry (CharCP listName, CharCP componentName, bvector<IGeometryPtr> &values)
    {
    BeAssert(false);
    return false;
    }





bool ReadListOfISinglePoint (CharCP listName, CharCP componentName, bvector<IGeometryPtr> &values)
    {
    BeAssert(false);
    return false;
    }






//=======================================================================================
bool ReadTagString (CharCP name, Utf8StringR)
    {
    BeAssert(false);
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
        if (0 == stricmp (name, "capped") && CurrentElementNameMatch ("bSolidFlag"))
            {
            // accept substitute.
            }
        else if (0 == stricmp (name, "bSolidFlag") && CurrentElementNameMatch ("capped"))
            {
            // accept substitute.
            }
        else
            return false;
        }
    bool stat = false;
    if (ReadToText ())
        {
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
        }
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
        && ReadToChildOrEnd ())
        {
        // Start with the system default for each field ....
        DPoint3d origin = DPoint3d::FromZero ();
        DVec3d vectorZ = DVec3d::From (0,0,0);
        DVec3d vectorX = DVec3d::From (0,0,0);

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
bool ReadExtendedObject(IGeometryPtr &result)
    {
    result = nullptr;
    if (CurrentElementNameMatch("ExtendedObject"))
        ReadToChildOrEnd();

    return false;
    }

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

bool ReadExtendedData(IGeometryPtr &result)
    {
    if (!CurrentElementNameMatch("ExtendedData"))
        return false;

    ReadToChildOrEnd();

    // Should be 'TransientLookupCollection'
    if (!CurrentElementNameMatch("TransientLookupCollection"))
        return false;
        
    ReadToChildOrEnd();
    // Next, iterate over each entry
    for (;IsStartElement ();)
        {
        Utf8String keyValue;
        Utf8String keyName("key");
        m_reader.ReadToNextAttribute (&keyName, &keyValue);

        Utf8String typeValue;
        Utf8String typeCode("typeCode");
        m_reader.ReadToNextAttribute (&typeCode, &typeValue);
        
        Utf8String content;
        m_reader.Read();

        m_reader.MoveToContent();
        m_reader.ReadContentAsString(content);
        m_reader.MoveToContent();

        // m_reader.ReadEndElement()
            {
            m_reader.ReadToEndOfElement();
            m_reader.Read();
            }
        m_reader.MoveToContent();
        }

    return false;
    }


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

bool ReadTag_AnyGeometry (CharCP name, IGeometryPtr &value)
    {
    IGeometryPtr geometry;
    bool stat = false;
    if (CurrentElementNameMatch (name))
        {
        ReadToElement ();
        ParseMethod parseMethod = s_parseTable[m_currentElementName];
        if (parseMethod != nullptr)
            {
            if ((this->*parseMethod)(value))
                stat = true;
            }
        ReadEndElement ();
        }
    return stat;
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

bool ReadTag_AnySurface (CharCP name, IGeometryPtr &value)
    {
    return ReadTag_AnyGeometry (name,value)
        && IsAnySurface (*value);
    }

bool ReadTag_AnyCurve (CharCP name, IGeometryPtr &value)
    {
    return ReadTag_AnyGeometry (name,value)
        && IsAnyCurve (*value);
    }

bool ReadTag_AnyParametricSurface (CharCP name, IGeometryPtr &value)
    {
    return ReadTag_AnyGeometry (name,value)
        && IsParametricSurface (*value);
    }

bool ReadTag_AnyCurveChain (CharCP name, IGeometryPtr &value)
    {
    return ReadTag_AnyGeometry (name,value)
        && IsAnyCurveChain (*value);
    }

bool ReadTag_AnyICurvePrimitive (ICurvePrimitivePtr &value)
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

bool ReadTag_AnyCurveVector (CharCP name, CurveVectorPtr &value)
    {
    IGeometryPtr geometry;
    bool stat = false;
    if (CurrentElementNameMatch (name)
        && ReadToChildOrEnd ())
        {
        if (ReadTag_AnyGeometry (geometry))
            {
            value = geometry->GetAsCurveVector ();
            if (value.IsValid ())
                stat = true;
            else
                {
                ICurvePrimitivePtr cp = geometry->GetAsICurvePrimitive ();
                if (cp.IsValid ())
                    {
                    stat = true;
                    value = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);
                    value->push_back (cp);
                    }
                }
            ReadEndElement ();
            }
        }
    return stat;
    }

bool ReadTag_AnyICurvePrimitive (CharCP name, ICurvePrimitivePtr &value)
    {
    IGeometryPtr geometry;
    bool stat = false;
    if (CurrentElementNameMatch (name)
        && ReadToChildOrEnd ())
        {
        if (ReadTag_AnyGeometry (geometry))
            {
            value = geometry->GetAsICurvePrimitive ();
            stat = value.IsValid ();
            ReadEndElement ();
            }
        }
    return stat;
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
    Utf8String s;
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
            {
            reader->GetCurrentNodeName (s);
            printf (" %s", s);
            }
        else if (nodeType == BeXmlReader::NODE_TYPE_Text)
            {
            reader->GetCurrentNodeValue (s);
            printf (" %s", s);
            }
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

bool BeXmlCGStreamReader::TryParse (byte* buffer, int bufferLength, bvector<IGeometryPtr> &geometry, size_t maxDepth)
    {
    IGeometryCGFactory factory;
    MSXmlBinaryReader reader(buffer, bufferLength);
    BeXmlCGStreamReaderImplementation parser (reader, factory);
    return parser.TryParse(geometry, maxDepth);
    }
    
BeXmlCGStreamReaderImplementation::ParseDictionary BeXmlCGStreamReaderImplementation::s_parseTable;

END_BENTLEY_ECOBJECT_NAMESPACE