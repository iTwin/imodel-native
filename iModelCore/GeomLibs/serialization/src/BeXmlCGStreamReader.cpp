/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include "serializationPCH.h"
#include "MSXmlBinary/MSXmlBinaryReader.h"
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
        return s_nodeTypeNames[(int32_t)nodeType];
    return "(Unknown BeXmlReader::NodeType)";
    }

struct BeXmlCGStreamReaderImplementation
{
typedef bool (BeXmlCGStreamReaderImplementation::*ParseMethod)(IGeometryPtr &);
typedef bmap <Utf8String, ParseMethod> ParseDictionary;

static ParseDictionary s_parseTable;

static void InitParseTable_go ()
    {
    if (s_parseTable.empty ())
        {
        s_parseTable[Utf8String("LineSegment")] = &BeXmlCGStreamReaderImplementation::ReadILineSegment;
        s_parseTable[Utf8String("CircularArc")] = &BeXmlCGStreamReaderImplementation::ReadICircularArc;
        s_parseTable[Utf8String("DgnBox")] = &BeXmlCGStreamReaderImplementation::ReadIDgnBox;
        s_parseTable[Utf8String("DgnSphere")] = &BeXmlCGStreamReaderImplementation::ReadIDgnSphere;
        s_parseTable[Utf8String("DgnCone")] = &BeXmlCGStreamReaderImplementation::ReadIDgnCone;
        s_parseTable[Utf8String("DgnTorusPipe")] = &BeXmlCGStreamReaderImplementation::ReadIDgnTorusPipe;
        s_parseTable[Utf8String("Block")] = &BeXmlCGStreamReaderImplementation::ReadIBlock;
        s_parseTable[Utf8String("CircularCone")] = &BeXmlCGStreamReaderImplementation::ReadICircularCone;
        s_parseTable[Utf8String("CircularCylinder")] = &BeXmlCGStreamReaderImplementation::ReadICircularCylinder;
        s_parseTable[Utf8String("CircularDisk")] = &BeXmlCGStreamReaderImplementation::ReadICircularDisk;
        s_parseTable[Utf8String("Coordinate")] = &BeXmlCGStreamReaderImplementation::ReadICoordinate;
        s_parseTable[Utf8String("EllipticArc")] = &BeXmlCGStreamReaderImplementation::ReadIEllipticArc;
        s_parseTable[Utf8String("EllipticDisk")] = &BeXmlCGStreamReaderImplementation::ReadIEllipticDisk;
        s_parseTable[Utf8String("SingleLineText")] = &BeXmlCGStreamReaderImplementation::ReadISingleLineText;
        s_parseTable[Utf8String("SkewedCone")] = &BeXmlCGStreamReaderImplementation::ReadISkewedCone;
        s_parseTable[Utf8String("Sphere")] = &BeXmlCGStreamReaderImplementation::ReadISphere;
        s_parseTable[Utf8String("TorusPipe")] = &BeXmlCGStreamReaderImplementation::ReadITorusPipe;
        s_parseTable[Utf8String("Vector")] = &BeXmlCGStreamReaderImplementation::ReadIVector;
        s_parseTable[Utf8String("IndexedMesh")] = &BeXmlCGStreamReaderImplementation::ReadIIndexedMesh;
        s_parseTable[Utf8String("AdjacentSurfacePatches")] = &BeXmlCGStreamReaderImplementation::ReadIAdjacentSurfacePatches;
        s_parseTable[Utf8String("BsplineCurve")] = &BeXmlCGStreamReaderImplementation::ReadIBsplineCurve;
        s_parseTable[Utf8String("BsplineSurface")] = &BeXmlCGStreamReaderImplementation::ReadIBsplineSurface;
        s_parseTable[Utf8String("CurveChain")] = &BeXmlCGStreamReaderImplementation::ReadICurveChain;
        s_parseTable[Utf8String("CurveGroup")] = &BeXmlCGStreamReaderImplementation::ReadICurveGroup;
        s_parseTable[Utf8String("CurveReference")] = &BeXmlCGStreamReaderImplementation::ReadICurveReference;
        s_parseTable[Utf8String("Group")] = &BeXmlCGStreamReaderImplementation::ReadIGroup;
        s_parseTable[Utf8String("InterpolatingCurve")] = &BeXmlCGStreamReaderImplementation::ReadIInterpolatingCurve;
        s_parseTable[Utf8String("LineString")] = &BeXmlCGStreamReaderImplementation::ReadILineString;
        s_parseTable[Utf8String("Operation")] = &BeXmlCGStreamReaderImplementation::ReadIOperation;
        s_parseTable[Utf8String("ParametricSurfacePatch")] = &BeXmlCGStreamReaderImplementation::ReadIParametricSurfacePatch;
        s_parseTable[Utf8String("PointChain")] = &BeXmlCGStreamReaderImplementation::ReadIPointChain;
        s_parseTable[Utf8String("PointGroup")] = &BeXmlCGStreamReaderImplementation::ReadIPointGroup;
        s_parseTable[Utf8String("Polygon")] = &BeXmlCGStreamReaderImplementation::ReadIPolygon;
        s_parseTable[Utf8String("PrimitiveCurveReference")] = &BeXmlCGStreamReaderImplementation::ReadIPrimitiveCurveReference;
        s_parseTable[Utf8String("SharedGroupDef")] = &BeXmlCGStreamReaderImplementation::ReadISharedGroupDef;
        s_parseTable[Utf8String("SharedGroupInstance")] = &BeXmlCGStreamReaderImplementation::ReadISharedGroupInstance;
        s_parseTable[Utf8String("ShelledSolid")] = &BeXmlCGStreamReaderImplementation::ReadIShelledSolid;
        s_parseTable[Utf8String("SolidBySweptSurface")] = &BeXmlCGStreamReaderImplementation::ReadISolidBySweptSurface;
        s_parseTable[Utf8String("SolidByRuledSweep")] = &BeXmlCGStreamReaderImplementation::ReadISolidByRuledSweep;
        s_parseTable[Utf8String("SurfaceByRuledSweep")] = &BeXmlCGStreamReaderImplementation::ReadISurfaceByRuledSweep;
        s_parseTable[Utf8String("SolidGroup")] = &BeXmlCGStreamReaderImplementation::ReadISolidGroup;
        s_parseTable[Utf8String("Spiral")] = &BeXmlCGStreamReaderImplementation::ReadISpiral;
        s_parseTable[Utf8String("SurfaceBySweptCurve")] = &BeXmlCGStreamReaderImplementation::ReadISurfaceBySweptCurve;
        s_parseTable[Utf8String("SurfaceGroup")] = &BeXmlCGStreamReaderImplementation::ReadISurfaceGroup;
        s_parseTable[Utf8String("SurfacePatch")] = &BeXmlCGStreamReaderImplementation::ReadISurfacePatch;
        s_parseTable[Utf8String("DgnCurveVector")] = &BeXmlCGStreamReaderImplementation::ReadIDgnCurveVector;
        s_parseTable[Utf8String("TransformedGeometry")] = &BeXmlCGStreamReaderImplementation::ReadITransformedGeometry;
        s_parseTable[Utf8String("DgnExtrusion")] = &BeXmlCGStreamReaderImplementation::ReadIDgnExtrusion;
        s_parseTable[Utf8String("DgnRotationalSweep")] = &BeXmlCGStreamReaderImplementation::ReadIDgnRotationalSweep;
        s_parseTable[Utf8String("DgnRuledSweep")] = &BeXmlCGStreamReaderImplementation::ReadIDgnRuledSweep;
        s_parseTable[Utf8String("TransitionSpiral")] = &BeXmlCGStreamReaderImplementation::ReadITransitionSpiral;
        s_parseTable[Utf8String("PartialCurve")] = &BeXmlCGStreamReaderImplementation::ReadIPartialCurve;

        s_parseTable[Utf8String("Geometry")] = &BeXmlCGStreamReaderImplementation::ReadIGeometry;

        }
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Earlin.Lutz         03/17
+---------------+---------------+---------------+---------------+---------------+------*/
static void InitParseTable ()
    {
    static std::once_flag s_ignoreListOnceFlag;
    std::call_once(s_ignoreListOnceFlag, InitParseTable_go);
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
        printf ("%s", s.c_str());
        }
    else if (nodeType == BeXmlReader::NODE_TYPE_Text)
        {
        m_reader.GetCurrentNodeValue (s);
        printf ("{%s}", s.c_str());
        }
    if (m_reader.IsEmptyElement ())
        printf (" (EmptyElement !!!)\n");
    printf ("\n");
    }
BeXmlCGStreamReaderImplementation (IBeXmlReader &reader, ICGFactory &factory)
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

bool IsEmptyElement ()
    {
    if (m_debug > 9)
        Show ("IsEmptyElement");
    return m_reader.IsEmptyElement ();
    }

bool IsEndElement ()
    {
    if (m_debug > 9)
        Show ("IsEndElement");
    return m_reader.GetCurrentNodeType () == BeXmlReader::NODE_TYPE_EndElement;
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


bool ReadListOfDPoint3d (CharCP listName, CharCP shortListName, bvector<DPoint3d> &values)
    {
    if (!CurrentElementNameMatch (listName))
        return false;
    if (IsEmptyElement ())
        {
        ReadOverWhiteSpace ();
        return true;
        }

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

bool ReadListOfint (CharCP listName, CharCP shortListName, bvector<int> &values)
    {
    if (!CurrentElementNameMatch (listName))
        return false;
    if (IsEmptyElement ())
        {
        ReadOverWhiteSpace ();
        return true;
        }

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

bool ReadListOfdouble (CharCP listName, CharCP shortListName, bvector<double> &values)
    {
    if (!CurrentElementNameMatch (listName))
        return false;
    if (IsEmptyElement ())
        {
        ReadOverWhiteSpace ();
        return true;
        }

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

bool ReadListOfDPoint2d (CharCP listName, CharCP shortListName, bvector<DPoint2d> &values)
    {
    if (!CurrentElementNameMatch (listName))
        return false;
    if (IsEmptyElement ())
        {
        ReadOverWhiteSpace ();
        return true;
        }

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
bool ReadListOfDVector3d (CharCP listName, CharCP shortListName, bvector<DVector3d> &values)
    {
    if (!CurrentElementNameMatch (listName))
        return false;
    if (IsEmptyElement ())
        {
        ReadOverWhiteSpace ();
        return true;
        }
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
bool ReadListOfISurfacePatch (CharCP listName, CharCP shortListName, bvector<IGeometryPtr> &values)
    {
    BeAssert(false);
    return false;
    }

bool ReadListOf_AnyCurveVector (CharCP listName, CharCP shortListName, bvector<CurveVectorPtr> &values)
    {
    if (!CurrentElementNameMatch (listName))
        return false;
    if (IsEmptyElement ())
        {
        ReadOverWhiteSpace ();
        return true;
        }

    ReadToChildOrEnd ();
    CurveVectorPtr cp;
    while (IsStartElement ()
            && ReadTag_AnyCurveVector (cp)
            && cp.IsValid ())
        {
        values.push_back (cp);
        }
    // Get out of the primary element ..
    ReadEndElement ();
    return true;
    }

bool ReadListOf_AnyICurvePrimitive (CharCP listName, CharCP shortListName, bvector<ICurvePrimitivePtr> &values)
    {
    if (!CurrentElementNameMatch (listName))
        return false;
    if (IsEmptyElement ())
        {
        ReadOverWhiteSpace ();
        return true;
        }

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

bool ReadListOf_AnyICurveChain (CharCP listName, CharCP shortListName, bvector<CurveVectorPtr> &values)
    {
    return ReadListOf_AnyCurveVector (listName, shortListName, values);
    }

bool ReadListOfICurve (CharCP listName, CharCP shortListName, bvector<IGeometryPtr> &values)
    {
    BeAssert(false);
    return false;
    }
bool ReadListOfICurveChain (CharCP listName, CharCP shortListName, bvector<IGeometryPtr> &values)
    {
    if (!CurrentElementNameMatch (listName))
        return false;
    if (IsEmptyElement ())
        {
        ReadOverWhiteSpace ();
        return true;
        }

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

bool ReadListOfISolid (CharCP listName, CharCP shortListName, bvector<IGeometryPtr> &values)
    {
    BeAssert(false);
    return false;
    }
bool ReadListOfISurface (CharCP listName, CharCP shortListName, bvector<IGeometryPtr> &values)
    {
    BeAssert(false);
    return false;
    }
bool ReadListOfIPoint (CharCP listName, CharCP shortListName, bvector<IGeometryPtr> &values)
    {
    BeAssert(false);
    return false;
    }





bool ReadListOfIGeometry (CharCP listName, CharCP shortListName, bvector<IGeometryPtr> &values)
    {
    if (!CurrentElementNameMatch (listName))
        return false;
    if (IsEmptyElement ())
        {
        ReadOverWhiteSpace ();
        return true;
        }
				
    ReadToChildOrEnd ();
    IGeometryPtr member;
    while (IsStartElement ()
            && ReadTag_AnyGeometry (member))
        {
        values.push_back (member);
        }
    // Get out of the primary element ..
    ReadEndElement ();
    return true;
    }


bool ReadListOfISinglePoint (CharCP listName, CharCP shortListName, bvector<IGeometryPtr> &values)
    {
    if (!CurrentElementNameMatch (listName))
        return false;
    if (IsEmptyElement ())
        {
        ReadOverWhiteSpace ();
        return true;
        }
				
    ReadToChildOrEnd ();
    IGeometryPtr member;
    while (IsStartElement ()
            && ReadTag_AnyGeometry (member))
        {
        values.push_back (member);
        }
    // Get out of the primary element ..
    ReadEndElement ();
    return true;
    }


// This method is entered into the parse table .. reads content if <Geometry>...</Geometry> (or <Geometry/>
// Unlike most readers, this can return nullptr as its value (for empty <Geometry/>
bool ReadIGeometry (IGeometryPtr &value)
    {
    value = nullptr;
    if (IsEndElement ())
        {
        return true;
        }
    if (CurrentElementNameMatch ("Geometry")
        && ReadToChildOrEnd ())
        {
        if (IsStartElement ())
            {
            if (ReadTag_AnyGeometry (value))
                {
                // Get out of the primary element ..
                ReadEndElement ();
                return true;
                }
            }
        else
            {
            // end element?
            return true;
            }
        }
    return false;
    }



//=======================================================================================
bool ReadTagString (CharCP name, Utf8StringR value)
    {
    if (nullptr != name  && !CurrentElementNameMatch (name))
        return false;
    m_reader.ReadTo (BeXmlReader::NODE_TYPE_Text);
    m_reader.GetCurrentNodeValue (value);
    AdvanceAfterContentExtraction ();
    return true;
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
        if (0 == BeStringUtilities::Stricmp (name, "capped") && CurrentElementNameMatch ("bSolidFlag"))
            {
            // accept substitute.
            }
        else if (0 == BeStringUtilities::Stricmp (name, "bSolidFlag") && CurrentElementNameMatch ("capped"))
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


bool ReadTag_AnyGeometry (IGeometryPtr &value)
    {
    if (CurrentElementNameMatch("ExtendedObject"))
        return ReadExtendedObject(value);

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
        if (ReadToChildOrEnd ())
            {
            ParseMethod parseMethod = s_parseTable[m_currentElementName];
            if (parseMethod != nullptr)
                {
                if ((this->*parseMethod)(value))
                    stat = true;
                }
            ReadEndElement ();
            }
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
        ICurvePrimitivePtr cp = geometry->GetAsICurvePrimitive ();
        if (cp.IsValid ())
            {
            value = cp;
            return true;
            }
        CurveVectorPtr cv = geometry->GetAsCurveVector ();
        if (cv.IsValid ())
            {
            value = ICurvePrimitive::CreateChildCurveVector (cv);
            return true;
            }
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

bool ReadTag_AnyCurveVector (CurveVectorPtr &value)
    {
    IGeometryPtr geometry;
    bool stat = false;
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


public: bool TryParse (bvector<IGeometryPtr> &geometry, bmap<OrderedIGeometryPtr, BeExtendedData> &extendedData, size_t maxDepth)
    {
    size_t count = 0;
    if (CurrentElementNameMatch("ExtendedObject"))
        {
        ReadExtendedObject(geometry);
        }
    else 
        {
        for (;IsStartElement ();)
            {
            ParseMethod parseMethod = s_parseTable[m_currentElementName];
            if (parseMethod == nullptr)
                {
                if (maxDepth > 0)
                    {
                    ReadToElement ();
                    if (!TryParse (geometry, extendedData, maxDepth - 1))
                      break;
                    AdvanceAfterContentExtraction ();
                    }
                break;
                }
            else
                {
                IGeometryPtr result;
                if ((this->*parseMethod)(result) && result.IsValid())
                    {
                    geometry.push_back (result);
                    count++;
                    }
                }
            }
        }

    bmap<OrderedIGeometryPtr, BeExtendedData>::const_iterator extendedDataIterator;
    bmap<OrderedIGeometryPtr, BeExtendedData>::const_iterator extendedDataIterator2;

    for (extendedDataIterator = m_extendedData.begin(); extendedDataIterator != m_extendedData.end(); extendedDataIterator++)
        {
        OrderedIGeometryPtr geometryObj = extendedDataIterator->first;
        extendedDataIterator2 = extendedData.find(geometryObj);
        if (extendedDataIterator2 != extendedData.end())
            continue;
        BeExtendedData entries = extendedDataIterator->second;
        extendedData[geometryObj] = entries;
        }

    return geometry.size() > 0;
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
            printf (" %s", s.c_str());
            }
        else if (nodeType == BeXmlReader::NODE_TYPE_Text)
            {
            reader->GetCurrentNodeValue (s);
            printf (" %s", s.c_str());
            }
        if (nodeType == BeXmlReader::NODE_TYPE_Element)
            depth++;
        }
    printf ("\n</XMLNodes>");
    }

bool BeXmlCGStreamReader::TryParse (Utf8CP beXmlCGString, bvector<IGeometryPtr> &geometry, bmap<OrderedIGeometryPtr, BeExtendedData> &extendedData, size_t maxDepth)
    {
    static int s_preview = 0;
    if (s_preview)
        ShowAllNodeTypes (beXmlCGString);
    BeXmlStatus status;
    IGeometryCGFactory factory;
    BeXmlReaderPtr reader = BeXmlReader::CreateAndReadFromString (status, beXmlCGString);
    BeXmlCGStreamReaderImplementation parser (*reader, factory);
    return parser.TryParse(geometry, extendedData, maxDepth);
    }

bool BeXmlCGStreamReader::TryParse (Byte* buffer, int bufferLength, bvector<IGeometryPtr> &geometry, bmap<OrderedIGeometryPtr, BeExtendedData> &extendedData, size_t maxDepth)
    {
    IGeometryCGFactory factory;
    MSXmlBinaryReader reader(buffer, bufferLength);
    BeXmlCGStreamReaderImplementation parser (reader, factory);
    return parser.TryParse(geometry, extendedData, maxDepth);
    }
    
BeXmlCGStreamReaderImplementation::ParseDictionary BeXmlCGStreamReaderImplementation::s_parseTable;

END_BENTLEY_GEOMETRY_NAMESPACE
