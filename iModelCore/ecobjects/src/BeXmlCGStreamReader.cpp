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


#include "nativeCGFactoryH.h"



struct IGeometryCGFactory : ICGFactory
{
public:

// ===================================================================================
/// <summary>
/// factory base class placeholder to create a Coordinate from explicit args.
virtual IGeometryPtr CreateCoordinate
(
InputParamTypeFor_DPoint3d xyz
) override
    {
    return IGeometry::Create(ICurvePrimitive::CreatePointString (&xyz, 1));
    }

// ===================================================================================

virtual IGeometryPtr CreateLineSegment
(
InputParamTypeFor_DPoint3d startPoint,
InputParamTypeFor_DPoint3d endPoint
) override
    {
    ICurvePrimitivePtr cp = ICurvePrimitive::CreateLine (DSegment3d::From (startPoint, endPoint));
    return IGeometry::Create (cp);
    }

/// <summary>
/// factory base class placeholder to create a EllipticArc from explicit args.
virtual IGeometryPtr CreateEllipticArc
(
InputParamTypeFor_PlacementOriginZX placement,
InputParamTypeFor_double radiusA,
InputParamTypeFor_double radiusB,
InputParamTypeFor_Angle startAngle,
InputParamTypeFor_Angle sweepAngle
) override
    {
    DEllipse3d ellipse = placement.AsDEllipse3d
        (
        radiusA,
        radiusB,
        startAngle.Radians (),
        sweepAngle.Radians ()
        );
    ICurvePrimitivePtr cp = ICurvePrimitive::CreateArc (ellipse);
    return IGeometry::Create (cp);
    }
// ===================================================================================

/// <summary>
/// factory base class placeholder to create a EllipticDisk from explicit args.
virtual IGeometryPtr CreateEllipticDisk
(
InputParamTypeFor_PlacementOriginZX placement,
InputParamTypeFor_double radiusA,
InputParamTypeFor_double radiusB
)
    {
    DPoint3d origin;
    RotMatrix axes;
    ICurvePrimitivePtr arc = ICurvePrimitive::CreateArc (
                DEllipse3d::FromScaledRotMatrix (
                        origin, axes,
                        radiusA, radiusB,
                        0.0, Angle::TwoPi ()));
    CurveVectorPtr area = CurveVector::Create
                (CurveVector::BOUNDARY_TYPE_Outer);
    area->push_back (arc);
    return IGeometry::Create(ICurvePrimitive::CreateChildCurveVector_SwapFromSource (*area));
    }

// ===================================================================================

/// <summary>
/// factory base class placeholder to create a EllipticArc from explicit args.
virtual IGeometryPtr CreateCircularArc
(
InputParamTypeFor_PlacementOriginZX placement,
InputParamTypeFor_double radius,
InputParamTypeFor_Angle startAngle,
InputParamTypeFor_Angle sweepAngle
) override
    {
    DEllipse3d ellipse = placement.AsDEllipse3d
        (
        radius,
        radius,
        startAngle.Radians (),
        sweepAngle.Radians ()
        );
    ICurvePrimitivePtr cp = ICurvePrimitive::CreateArc (ellipse);
    return IGeometry::Create (cp);
    }

// ===================================================================================
/// <summary>
/// factory base class placeholder to create a CircularDisk from explicit args.
virtual IGeometryPtr CreateCircularDisk
(
InputParamTypeFor_PlacementOriginZX placement,
InputParamTypeFor_double radius
)
    {
    DPoint3d origin;
    RotMatrix axes;
    ICurvePrimitivePtr arc = ICurvePrimitive::CreateArc (
                DEllipse3d::FromScaledRotMatrix (
                        origin, axes,
                        radius, radius,
                        0.0, Angle::TwoPi ()));
    CurveVectorPtr area = CurveVector::Create
                (CurveVector::BOUNDARY_TYPE_Outer);
    area->push_back (arc);
    return IGeometry::Create(ICurvePrimitive::CreateChildCurveVector_SwapFromSource (*area));
    }

// ===================================================================================
/// <summary>
/// factory base class placeholder to create a SkewedCone from explicit args.
virtual IGeometryPtr CreateSkewedCone
(
InputParamTypeFor_PlacementOriginZX placement,
InputParamTypeFor_DPoint3d centerB,
InputParamTypeFor_double radiusA,
InputParamTypeFor_double radiusB,
InputParamTypeFor_bool capped
) override
    {
    RotMatrix axes;
    DPoint3d centerA;
    placement.GetFrame (centerA, axes);
    DgnConeDetail coneDetail (centerA, centerB,
                axes, 
                radiusA, radiusB,
                capped);
    ISolidPrimitivePtr sp = ISolidPrimitive::CreateDgnCone (coneDetail);
    return IGeometry::Create (sp);
    }

// ===================================================================================

/// <summary>
/// factory base class placeholder to create a CircularCone from explicit args.
virtual IGeometryPtr CreateCircularCone
(
InputParamTypeFor_PlacementOriginZX placement,
InputParamTypeFor_double height,
InputParamTypeFor_double radiusA,
InputParamTypeFor_double radiusB,
InputParamTypeFor_bool capped
) override
    {
    RotMatrix axes;
    DVec3d vectorZ;
    DPoint3d centerA;
    placement.GetFrame (centerA, axes);
    axes.GetColumn (vectorZ, 2);
    DPoint3d centerB = DPoint3d::FromSumOf (centerA, vectorZ, height);
    DgnConeDetail coneDetail (centerA, centerB,
                axes, 
                radiusA, radiusB,
                capped);
    ISolidPrimitivePtr sp = ISolidPrimitive::CreateDgnCone (coneDetail);
    return IGeometry::Create (sp);
    }

// ===================================================================================

/// <summary>
/// factory base class placeholder to create a CircularCylinder from explicit args.
virtual IGeometryPtr CreateCircularCylinder
(
InputParamTypeFor_PlacementOriginZX placement,
InputParamTypeFor_double height,
InputParamTypeFor_double radius,
InputParamTypeFor_bool capped
) override
    {
    RotMatrix axes;
    DVec3d vectorZ;
    DPoint3d centerA;
    placement.GetFrame (centerA, axes);
    axes.GetColumn (vectorZ, 2);
    DPoint3d centerB = DPoint3d::FromSumOf (centerA, vectorZ, height);
    DgnConeDetail coneDetail (centerA, centerB,
                axes, 
                radius, radius,
                capped);
    ISolidPrimitivePtr sp = ISolidPrimitive::CreateDgnCone (coneDetail);
    return IGeometry::Create (sp);
    }

// ===================================================================================

/// <summary>
/// factory base class placeholder to create a Block from explicit args.
virtual IGeometryPtr CreateBlock
(
InputParamTypeFor_PlacementOriginZX placement,
InputParamTypeFor_DPoint3d cornerA,
InputParamTypeFor_DPoint3d cornerB,
InputParamTypeFor_bool capped
) override
    {
    DPoint3d origin;
    RotMatrix axes;
    DVec3d vectorX, vectorY, vectorZ;
    placement.GetFrame (origin, axes);
    axes.GetColumns (vectorX, vectorY, vectorZ);
    DPoint3d baseOrigin = DPoint3d::FromProduct (origin, axes, cornerA.x, cornerA.y, cornerA.z);
    DPoint3d topOrigin  = DPoint3d::FromProduct (origin, axes, cornerA.x, cornerA.y, cornerB.z);
    double dx = cornerB.x - cornerA.x;
    double dy = cornerB.y - cornerA.y;
    DgnBoxDetail detail (baseOrigin, topOrigin, vectorX, vectorY, dx, dy, dx, dy, capped);
    ISolidPrimitivePtr sp = ISolidPrimitive::CreateDgnBox (detail);
    return IGeometry::Create (sp);
    }

// ===================================================================================

/// <summary>
/// factory base class placeholder to create a Sphere from explicit args.
virtual IGeometryPtr CreateSphere
(
InputParamTypeFor_PlacementOriginZX placement,
InputParamTypeFor_double radius
) override
    {
    DPoint3d center;
    RotMatrix axes;
    placement.GetFrame (center, axes);
    DgnSphereDetail detail (center, axes, radius);
    return IGeometry::Create(ISolidPrimitive::CreateDgnSphere (detail));
    }

// ===================================================================================

/// <summary>
/// factory base class placeholder to create a TorusPipe from explicit args.
virtual IGeometryPtr CreateTorusPipe
(
InputParamTypeFor_PlacementOriginZX placement,
InputParamTypeFor_double radiusA,
InputParamTypeFor_double radiusB,
InputParamTypeFor_Angle startAngle,
InputParamTypeFor_Angle sweepAngle,
InputParamTypeFor_bool capped
) override
    {
    DPoint3d center;
    RotMatrix axes;
    placement.GetFrame (center, axes);
    DVec3d vectorX, vectorY, vectorZ;
    axes.GetColumns (vectorX, vectorY, vectorZ);
    DVec3d vector0 = vectorX;
    DVec3d vector90 = vectorY;
    double startRadians = startAngle.Radians ();
    double sweepRadians = sweepAngle.Radians ();
    if (startRadians != 0.0)
        {
        double c = cos (startRadians);
        double s = sin (startRadians);
        vector0.SumOf  (vectorX,  c, vectorY, s);
        vector90.SumOf (vectorX, -s, vectorY, c);
        }
    DgnTorusPipeDetail detail (center,
                vector0, vector90, 
                radiusA, radiusB,
                sweepRadians,
                capped);
    return IGeometry::Create(ISolidPrimitive::CreateDgnTorusPipe (detail));
    }

// ===================================================================================
virtual IGeometryPtr CreateLineString
(
bvector<DPoint3d> const &points
) override
    {
    return IGeometry::Create(ICurvePrimitive::CreateLineString(points));
    }
#define CGFactory_CreateLineString
// ===================================================================================

/// <summary>
/// factory base class placeholder to create a IndexedMesh from explicit args.
virtual IGeometryPtr CreateIndexedMesh
(
bvector<DPoint3d> const &CoordArray,
bvector<int> const &CoordIndexArray,
bvector<DPoint2d> const &ParamArray,
bvector<int> const &ParamIndexArray,
bvector<DVector3d> const &NormalArray,
bvector<int> const &NormalIndexArray,
bvector<DVector3d> const &ColorArray,
bvector<int> const &ColorIndexArray
) override
    {
    PolyfaceHeaderPtr polyface = PolyfaceHeader::CreateVariableSizeIndexed ();

    CopyToBlockedVector (CoordArray, polyface->Point ());
    CopyToBlockedVector (CoordIndexArray, polyface->PointIndex ());

    CopyToBlockedVector (ParamArray, polyface->Param ());
    CopyToBlockedVector (ParamIndexArray, polyface->ParamIndex ());

    CopyToBlockedVector (NormalArray, polyface->Normal ());
    CopyToBlockedVector (NormalIndexArray, polyface->NormalIndex ());

    // Colors have to be reformatted ...
    if (ColorArray.size () > 0)
        {
        bvector<RgbFactor> dest = polyface->DoubleColor ();
        dest.reserve (ColorArray.size ());
        for (DVec3d const &data : ColorArray)
            {
            dest.push_back (RgbFactor::From (data));
            }
        }
    CopyToBlockedVector (ColorIndexArray, polyface->ColorIndex ());

    return IGeometry::Create (polyface);
    }
};


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
BeXmlReader &m_reader;
ICGFactory &m_factory;
int m_debug;
void Show (CharCP name)
    {
    BeXmlReader::NodeType nodeType = m_reader.GetCurrentNodeType ();
    printf ("%s (%s)", name, GetBeXmlNodeTypeString (nodeType));
    Utf8String elementName;
    if (nodeType == BeXmlReader::NODE_TYPE_Element
        || nodeType == BeXmlReader::NODE_TYPE_EndElement)
        {
        m_reader.GetCurrentNodeName (elementName);
        printf ("%s", elementName);
        }
    printf ("\n");
    }
BeXmlCGStreamReaderImplementation::BeXmlCGStreamReaderImplementation (BeXmlReader &reader, ICGFactory &factory)
    : m_reader(reader), m_factory(factory), m_debug (s_defaultDebug)
    {
    }

bool TagMatch (CharCP name)
    {
    if (m_debug > 9)
        Show ("TagMatch");
    return false;
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
    return 0 == m_currentElementName.CompareTo (name);
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
bool ReadListOfIPrimitiveCurve (CharCP listName, CharCP componentName, bvector<IGeometryPtr> &values)
    {
    return false;
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
    if (0 == m_currentValue8.CompareTo ("true"))
        {
        value = true;
        stat = true;
        }
    if (0 == m_currentValue8.CompareTo ("false"))
        {
        value = false;
        stat = true;
        }
    if (0 == m_currentValue8.CompareTo ("1"))
        {
        value = true;
        stat = true;
        }
    if (0 == m_currentValue8.CompareTo ("0"))
        {
        value = false;
        stat = true;
        }
    AdvanceAfterContentExtraction ();        
    return false;
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


BeXmlNodeP FindChild (BeXmlNodeP parent, CharCP name)
    {
    for (BeXmlNodeP child = parent->GetFirstChild (BEXMLNODE_Any); NULL != child;
                child = child->GetNextSibling (BEXMLNODE_Any))
        {
        if (0 == BeStringUtilities::Stricmp (child->GetName (), name))
            return child;
        }
    return NULL;
    }

bool GetDPoint3d (BeXmlNodeP element, DPoint3dR xyz)
    {
    BeXmlNodeP text;
    bvector<double> doubles;
    if (   NULL != element
        && NULL != (text = element->GetFirstChild (BEXMLNODE_Any))
        && BEXML_Success == text->GetContentDoubleValues (doubles)
        && doubles.size () == 3)
        {
        xyz.x = doubles[0];
        xyz.y = doubles[1];
        xyz.z = doubles[2];
        return true;
        }
    return false;
    }

bool GetDouble (BeXmlNodeP element, double &value)
    {
    BeXmlNodeP text;
    return NULL != element
        && NULL != (text = element->GetFirstChild (BEXMLNODE_Any))
        && BEXML_Success == text->GetContentDoubleValue (value);
    }


bool GetBool (BeXmlNodeP element, bool &value)
    {
    BeXmlNodeP text;
    return NULL != element
        && NULL != (text = element->GetFirstChild (BEXMLNODE_Any))
        && BEXML_Success == text->GetContentBooleanValue (value);
    }

bool FindChildBool (BeXmlNodeP parent, CharCP name, bool &value) {return GetBool (FindChild (parent, name), value);}
bool FindChildBool (BeXmlNodeP parent, CharCP nameA, CharCP nameB, bool &value)
    {
    return GetBool (FindChild (parent, nameA), value)
        || GetBool (FindChild (parent, nameB), value);
    }

bool FindChildDPoint3d (CharCP name, DPoint3dR xyz){return GetDPoint3d (FindChild (nullptr, name), xyz);}
bool FindChildDPoint3d (BeXmlNodeP parent, CharCP name, DPoint3dR xyz){return GetDPoint3d (FindChild (parent, name), xyz);}
bool FindChildDouble (BeXmlNodeP parent, CharCP name, double &value) {return GetDouble (FindChild (parent, name), value);}

bool FindChildInt (BeXmlNodeP parent, CharCP name, int &value)
    {
    BeXmlNodeP child = FindChild (parent, name);
    if (NULL != child
        && BEXML_Success == child->GetContentInt32Value (value))
        {
        return true;
        }
    return false;
    }

bool FindChildPlacement (BeXmlNodeP parent, CharCP name, DPoint3dR origin, RotMatrixR axes)
    {
    BeXmlNodeP child = FindChild (parent, name);
    DVec3d vectorX, vectorZ;
    if (NULL != child
        && FindChildDPoint3d (child, "origin", origin)
        && FindChildDPoint3d (child, "vectorZ", vectorZ)    // DVec3d has DPoint3d base class !!!
        && FindChildDPoint3d (child, "vectorX", vectorX)
        )
        {
        DVec3d vectorY;
        vectorY.CrossProduct (vectorZ, vectorX);
        axes = RotMatrix::FromColumnVectors (vectorX, vectorY, vectorZ);
        axes.SquareAndNormalizeColumns (axes, 2, 0);
        return true;
        }
    return false;
    }

bool GetPoints (BeXmlNodeP parent, CharCP listName, CharCP pointName, bvector<DPoint3d> &points)
    {
    BeXmlNodeP listNode = FindChild (parent, listName);
    if (NULL == listNode)
        return false;
    DPoint3d xyz;
    for (BeXmlNodeP child = listNode->GetFirstChild (BEXMLNODE_Element); NULL != child;
                child = child->GetNextSibling (BEXMLNODE_Element))
        {
        if ((NULL == pointName
             || 0 == BeStringUtilities::Stricmp (child->GetName (), pointName))
            && GetDPoint3d (child, xyz))
            points.push_back (xyz);
        }
    return true;
    }

bool GetDoubles (BeXmlNodeP parent, CharCP listName, CharCP pointName, bvector<double> &values)
    {
    BeXmlNodeP listNode = FindChild (parent, listName);
    if (NULL == listNode)
        return false;
    double value;
    for (BeXmlNodeP child = listNode->GetFirstChild (BEXMLNODE_Any); NULL != child;
                child = child->GetNextSibling (BEXMLNODE_Any))
        {
        if (0 == BeStringUtilities::Stricmp (child->GetName (), pointName)
            && GetDouble (child, value))
            values.push_back (value);
        }
    return true;
    }



public: bool TryParse (BeXmlNodeP node, MSBsplineSurfacePtr &result)
    {
    int orderU = 0;
    int orderV = 0;
    bool closedU = false;
    bool closedV = false;
    int numPolesU = 0;
    int numPolesV = 0;

    bvector<DPoint3d> points;
    bvector<double>   knotsU;
    bvector<double>   knotsV;
    bvector<double>   weights;


    if (CurrentElementNameMatch ("BsplineSurface")
        && FindChildInt (node, "orderU", orderU)
        && FindChildInt (node, "numUControlPoint", numPolesU)
        && FindChildInt (node, "orderV", orderV)
        && FindChildInt (node, "numVControlPoint", numPolesV)
        && GetPoints (node, "ListOfControlPoint", NULL, points)
        )
        {
        FindChildBool (node, "closedU", closedU);   // optional !!
        FindChildBool (node, "closedV", closedV);   // optional !!
        GetDoubles (node, "ListOfKnotU", "knotU", knotsU);
        GetDoubles (node, "ListOfKnotV", "knotV", knotsV);
        GetDoubles (node, "ListOfWeight", "weight", weights);
        MSBsplineSurfacePtr surface = MSBsplineSurface::CreatePtr ();
        if (SUCCESS == surface->Populate (points,
                weights.size () > 0 ? &weights : NULL,
                knotsU.size () > 0 ? &knotsU : NULL, orderU, numPolesU, closedU,
                knotsV.size () > 0 ? &knotsV : NULL, orderV, numPolesV, closedV,
                true
                ))
            {
            result = surface;
            return true;
            }
        return true;
        }
    return false;
    }

public: bool TryParseCurvePrimitive (BeXmlNodeP node, IGeometryPtr &result)
    {
    IGeometryPtr geometry;
    if (ReadILineSegment (result))
        {
        return result.IsValid ();
        }
    else if (ReadICircularArc (result))
        {
        return result.IsValid ();
        }
    else if (ReadIEllipticArc (result))
        {
        return result.IsValid ();
        }
    else if (ReadILineString (result))
        {
        return result.IsValid ();
        }        
    else if (CurrentElementNameMatch ("SurfacePatch"))
        {
#ifdef abc
        BeXmlNodeP exteriorLoop = FindChild (node, "ExteriorLoop");
        BeXmlNodeP holeLoops     = FindChild (node, "ListOfHoleLoop");
        if (NULL != exteriorLoop)
            {
            ICurvePrimitivePtr exteriorChild;
            CurveVectorPtr loops = CurveVector::Create (CurveVector::BOUNDARY_TYPE_ParityRegion);
            if (TryParse (exteriorLoop->GetFirstChild (BEXMLNODE_Element), exteriorChild))
                {
                loops->push_back (exteriorChild);
                if (NULL != holeLoops)
                    {
                    for (BeXmlNodeP child = holeLoops->GetFirstChild (BEXMLNODE_Element); NULL != child;
                                child = child->GetNextSibling (BEXMLNODE_Element))
                        {
                        ICurvePrimitivePtr hole;
                        if (TryParse (child, hole))
                            {
                            loops->push_back (hole);
                            size_t index = loops->size () - 1;
                            CurveVector::BoundaryType boundaryType;
                            if (loops->GetChildBoundaryType (index, boundaryType))
                                loops->SetChildBoundaryType (index, CurveVector::BOUNDARY_TYPE_Inner);
                            }
                        }
                    }
                result = ICurvePrimitive::CreateChildCurveVector_SwapFromSource (*loops);
                // hmm.. how to verify/announce validity (closed curve as child)???
                return true;
                }
            }
#endif
        }
    else if (CurrentElementNameMatch ("CurveChain"))
        {
#ifdef abc
        BeXmlNodeP curveList = FindChild (node, "ListOfCurve");
        CurveVectorPtr curves = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Outer);
        for (BeXmlNodeP child = curveList->GetFirstChild (BEXMLNODE_Element); NULL != child;
                    child = child->GetNextSibling (BEXMLNODE_Element))
            {
            ICurvePrimitivePtr segment;
            TryParse (child, segment);
            curves->push_back (segment);
            }
        result = ICurvePrimitive::CreateChildCurveVector_SwapFromSource (*curves);
        return true;
#endif
        }
    else if (CurrentElementNameMatch ("Linestring"))
        {
        bvector<DPoint3d> points;
        if (GetPoints (node, "ListOfPoint", NULL, points))    // allow any tag name in the points !!!
            {
            result = IGeometry::Create(ICurvePrimitive::CreateLineString (points));
            return true;
            }
        }
    else if (CurrentElementNameMatch ("Polygon"))
        {
        bvector<DPoint3d> points;
        if (GetPoints (node, "ListOfPoint", NULL, points))    // allow any tag name in the points !!!
            {
            ICurvePrimitivePtr linestring = ICurvePrimitive::CreateLineString (points);
            CurveVectorPtr area = CurveVector::Create
                        (CurveVector::BOUNDARY_TYPE_Outer);
            area->push_back (linestring);
            result = IGeometry::Create(ICurvePrimitive::CreateChildCurveVector_SwapFromSource (*area));
            return true;
            }
        }
    else if (ReadICircularDisk(result))
        {
        return result.IsValid ();
        }
    else if (ReadIEllipticDisk(result))
        {
        return result.IsValid ();
        }
    else if (CurrentElementNameMatch ("BsplineCurve"))
        {
        int order;
        bool closed = false;
        bvector<DPoint3d> points;
        bvector<double>   knots;
        bvector<double>   weights;
        if (FindChildInt (node, "order", order)
            && GetPoints (node, "ListOfControlPoint", NULL, points)) // "ControlPoint" ???
            {
            MSBsplineCurvePtr bcurve = MSBsplineCurve::CreatePtr ();
            GetDoubles (node, "ListOfKnot", "knot", knots);
            GetDoubles (node, "ListOfWeight", "weight", weights);
            FindChildBool (node, "closed", closed);
            bcurve->Populate (points,
                            weights.size () > 0 ? &weights : NULL,
                            knots.size () > 0 ? &knots : NULL,
                            order, closed, true);
            result = IGeometry::Create(ICurvePrimitive::CreateBsplineCurve (*bcurve));
            return true;
            }
        }
    else if (ReadICoordinate (result))
        {
        return result.IsValid ();
        }
    result = nullptr;
    return false;
    }


public: bool TryParseSolidPrimitive (BeXmlNodeP node, IGeometryPtr &result)
    {
    if (ReadICircularCylinder (result))
        {
        return result.IsValid ();
        }
    else if (ReadICircularCone (result))
        {
        return result.IsValid ();
        }
    else if (ReadISkewedCone (result))
        {
        return result.IsValid ();
        }
    else if (ReadISphere (result))
        {
        return result.IsValid ();
        }
    else if (ReadIBlock (result))   // ???? Should allow "Box"??
        {
        return result.IsValid ();
        }
    else if (ReadITorusPipe (result))
        {
        return result.IsValid ();
        }
    else if (ReadIIndexedMesh (result))
        {
        return result.IsValid ();
        }
    else if (CurrentElementNameMatch ("SurfaceBySweptCurve"))
        {
#ifdef abc
        BeXmlNodeP baseGeometryNode = FindChild (node, "BaseGeometry");
        BeXmlNodeP railCurveNode    = FindChild (node, "RailCurve");
        ICurvePrimitivePtr baseGeometry;
        ICurvePrimitivePtr railCurve;
        if (   NULL != baseGeometryNode
            && NULL != railCurveNode
            && TryParse (baseGeometryNode->GetFirstChild (), baseGeometry)
            && TryParse (railCurveNode->GetFirstChild (), railCurve)
           )
            {
            DSegment3d segment;
            DEllipse3d arc;
            if (railCurve->TryGetLine (segment))
                {
                DVec3d vector = DVec3d::FromStartEnd (segment.point[0], segment.point[1]);
                result = ISolidPrimitive::CreateDgnExtrusion (DgnExtrusionDetail (
                        CurveVectorOf (baseGeometry, CurveVector::BOUNDARY_TYPE_Open),
                                vector, false));
                return true;
                }
            else if (railCurve->TryGetArc (arc))
                {
                DVec3d normal = DVec3d::FromNormalizedCrossProduct (arc.vector0, arc.vector90);
                result = ISolidPrimitive::CreateDgnRotationalSweep (DgnRotationalSweepDetail (
                        CurveVectorOf (baseGeometry, CurveVector::BOUNDARY_TYPE_Open),
                                arc.center, normal, arc.sweep, false));
                return true;
                }
            }
#endif
        }
    return false;
    }



public: bool TryParse (bvector<IGeometryPtr> &geometry, size_t maxDepth)
    {
    if (!ReadToElement ())
        return false;

    IGeometryPtr g;
    MSBsplineSurfacePtr surface;
    size_t count = 0;
    BeXmlNodeP node = NULL;
    // This sequencing is just for clarity -- no actual type requirements.
    if (TryParseCurvePrimitive (node, g))
        {
        geometry.push_back (g);
        count = 1;
        }
    else if (TryParseSolidPrimitive (node, g))
        {
        geometry.push_back (g);
        count = 1;
        }
    else if (TryParse (node, surface))
        {
        geometry.push_back (IGeometry::Create (surface));
        count = 1;
        }
    else if (maxDepth == 0)
        {
        }
    else
        {
#if seachChildren        
        for (BeXmlNodeP child = node->GetFirstChild (BEXMLNODE_Element); NULL != child;
                    child = child->GetNextSibling (BEXMLNODE_Element))
            {
            count += TryParse (child, geometry, maxDepth - 1);
            }
#endif            
        }
    return count > 0;
    }
};
#ifdef abc
bool BeXmlCGStreamReaderImplementation__TryParse (Utf8CP beXmlCGString, ICurvePrimitivePtr &result)
    {
    BeXmlCGStreamReaderImplementation parser;
    size_t stringByteCount = strlen (beXmlCGString) * sizeof(Utf8Char);
    BeXmlStatus xmlStatus;
    BeXmlDomPtr pXmlDom(BeXmlDom::CreateAndReadFromString (xmlStatus, beXmlCGString, stringByteCount));

    BeXmlNodeP child(pXmlDom->GetRootElement());
    return parser.TryParse(child, result);
    }

bool BeXmlCGStreamReaderImplementation__TryParse (Utf8CP beXmlCGString, ISolidPrimitivePtr &result)
    {
    BeXmlCGStreamReaderImplementation parser;
    size_t stringByteCount = strlen (beXmlCGString) * sizeof(Utf8Char);
    BeXmlStatus xmlStatus;
    BeXmlDomPtr pXmlDom(BeXmlDom::CreateAndReadFromString (xmlStatus, beXmlCGString, stringByteCount));

    BeXmlNodeP child(pXmlDom->GetRootElement());
    return parser.TryParse(child, result);
    }

bool BeXmlCGStreamReaderImplementation__TryParse (Utf8CP beXmlCGString, MSBsplineSurfacePtr &result)
    {
    BeXmlCGStreamReaderImplementation parser;
    size_t stringByteCount = strlen (beXmlCGString) * sizeof(Utf8Char);
    BeXmlStatus xmlStatus;
    BeXmlDomPtr pXmlDom(BeXmlDom::CreateAndReadFromString (xmlStatus, beXmlCGString, stringByteCount));

    BeXmlNodeP child(pXmlDom->GetRootElement());
    return parser.TryParse(child, result);
    }
#endif

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

END_BENTLEY_ECOBJECT_NAMESPACE