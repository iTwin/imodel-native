/*--------------------------------------------------------------------------------------+
|
|  $Source: src/BeXmlCGStreamReader.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "ECObjectsPch.h"

BEGIN_BENTLEY_ECOBJECT_NAMESPACE
typedef struct IPlacement const &IPlacementCR;
typedef struct IPlacement &IPlacementR;

// mappings from managed idioms to native
#define DVector3d DVec3d
#define String Utf8String

#define InputParamTypeFor_DPoint3d DPoint3dCR
#define InputParamTypeFor_DPoint2d DPoint2dCR
#define InputParamTypeFor_DVector3d DVec3dCR
#define InputParamTypeFor_IPlacement IPlacementCR

#define InputParamTypeFor_double double
#define InputParamTypeFor_Angle Angle
#define InputParamTypeFor_int int
#define InputParamTypeFor_bool bool
#define InputParamTypeFor_String Utf8StringCR

struct IPlacement
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
    
IPlacement ()
    {
    InitIdentity ();
    }
    
static IPlacement FromIdentity ()
    {
    IPlacement value;
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
};



#include "nativeCGFactoryH.h"



struct IGeometryCGFactory : ICGFactory
{
public:
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
InputParamTypeFor_IPlacement placement,
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


};


static DPoint3d s_default_DPoint3d = DPoint3d::From (0,0,0);
static DPoint2d s_default_DPoint2d = DPoint2d::From (0,0);
static DVec3d   s_default_DVector3d = DVec3d::From (0,0,0);
static double   s_default_double   = 0.0;
static int      s_default_int      = 0;
static bool     s_default_bool     = false;
static Angle     s_default_Angle    = Angle::FromRadians (0.0);
static IPlacement s_default_IPlacement = IPlacement::FromIdentity ();
static Utf8String s_default_String = Utf8String ();



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

static int s_defaultDebug = 10;
struct BeXmlCGStreamReaderImplementation
{
BeXmlReader &m_reader;
ICGFactory &m_factory;
int m_debug;
void Show (CharCP name)
    {
    printf ("%s (NodeType %d)\n", name, m_reader.GetCurrentNodeType ());
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


bool ReadEndElement ()
    {
    assert(m_reader.GetCurrentNodeType () == BeXmlReader::NODE_TYPE_EndElement);
    if (m_debug > 9)
        Show ("ReadEndElement");
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
    m_reader.Read ();
    m_reader.GetCurrentNodeName (m_currentElementName);
    // assert?  we have advanced to a sibling or to end of the 
    //  containing element.
    return true;
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
    if (!CurrentElementNameMatch (name))
        return false;
    m_reader.ReadTo (BeXmlReader::NODE_TYPE_Text);
    m_reader.GetCurrentNodeValue (m_currentValue8);
    bool stat = 3 == sscanf (&m_currentValue8[0],
              "%lf,%lf,%lf", &value.x, &value.y, &value.z);
    AdvanceAfterContentExtraction ();
    return stat;
    }

bool ReadTagString (CharCP name, Utf8StringR)
    {
    return false;
    }

bool ReadTagDPoint2d (CharCP name, DPoint2dR value)
    {
    if (!CurrentElementNameMatch (name))
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
    {
    if (!CurrentElementNameMatch (name))
        return false;
    m_reader.ReadTo (BeXmlReader::NODE_TYPE_Text);
    m_reader.GetCurrentNodeValue (m_currentValue8);
    bool stat = 3 == sscanf (&m_currentValue8[0],
              "%lf,%lf,%lf", &value.x, &value.y, &value.z);
    AdvanceAfterContentExtraction ();
    return stat;
    }    }

bool ReadTagbool(CharCP name, bool &value)
    {
    return false;
    }

bool ReadTagdouble(CharCP name, double &value)
    {
    {
    if (!CurrentElementNameMatch (name))
        return false;
    m_reader.ReadTo (BeXmlReader::NODE_TYPE_Text);
    m_reader.GetCurrentNodeValue (m_currentValue8);
    bool stat = 3 == sscanf (&m_currentValue8[0],
              "%lf", &value);
    AdvanceAfterContentExtraction ();
    return stat;
    }    }
    
bool ReadTagint(CharCP name, int &value)
    {
    {
    if (!CurrentElementNameMatch (name))
        return false;
    m_reader.ReadTo (BeXmlReader::NODE_TYPE_Text);
    m_reader.GetCurrentNodeValue (m_currentValue8);
    bool stat = 3 == sscanf (&m_currentValue8[0],
              "%d", &value);
    AdvanceAfterContentExtraction ();
    return stat;
    }    }

bool ReadTagIPlacement (CharCP name, IPlacement &value)
    {
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


    if (0 == BeStringUtilities::Stricmp (node->GetName (), "BsplineSurface")
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

public: bool TryParse (BeXmlNodeP node, ICurvePrimitivePtr &result)
    {
    IGeometryPtr geometry;
    if (ReadILineSegment (geometry))
        {
        result = geometry->GetAsICurvePrimitive ();
        return result.IsValid ();
        }
    else if (ReadICircularArc (geometry))
        {
        result = geometry->GetAsICurvePrimitive ();
        return result.IsValid ();
        }
    else if (ReadIEllipticArc (geometry))
        {
        result = geometry->GetAsICurvePrimitive ();
        return result.IsValid ();
        }
    else if (CurrentElementNameMatch ("SurfacePatch"))
        {
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
        }
    else if (CurrentElementNameMatch ("CurveChain"))
        {
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
        }
    else if (CurrentElementNameMatch ("Linestring"))
        {
        bvector<DPoint3d> points;
        if (GetPoints (node, "ListOfPoint", NULL, points))    // allow any tag name in the points !!!
            {
            result = ICurvePrimitive::CreateLineString (points);
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
            result = ICurvePrimitive::CreateChildCurveVector_SwapFromSource (*area);
            return true;
            }
        }
    else if (CurrentElementNameMatch ("CircularDisk"))
        {
        RotMatrix axes;
        DPoint3d origin;
        double radius;
        if (   FindChildPlacement (node, "placement", origin, axes)
            && FindChildDouble (node, "radius", radius)
            )
            {
            ICurvePrimitivePtr arc = ICurvePrimitive::CreateArc (
                        DEllipse3d::FromScaledRotMatrix (
                                origin, axes,
                                radius, radius,
                                0.0, Angle::TwoPi ()));
            CurveVectorPtr area = CurveVector::Create
                        (CurveVector::BOUNDARY_TYPE_Outer);
            area->push_back (arc);
            result = ICurvePrimitive::CreateChildCurveVector_SwapFromSource (*area);
            return true;
            }
        }
    else if (CurrentElementNameMatch ("EllipticDisk"))
        {
        RotMatrix axes;
        DPoint3d origin;
        double radiusA, radiusB;
        if (   FindChildPlacement (node, "placement", origin, axes)
            && FindChildDouble (node, "radiusA", radiusA)
            && FindChildDouble (node, "radiusB", radiusB)
            )
            {
            ICurvePrimitivePtr arc = ICurvePrimitive::CreateArc (
                        DEllipse3d::FromScaledRotMatrix (
                                origin, axes,
                                radiusA, radiusB,
                                0.0, Angle::TwoPi ()));
            CurveVectorPtr area = CurveVector::Create
                        (CurveVector::BOUNDARY_TYPE_Outer);
            area->push_back (arc);
            result = ICurvePrimitive::CreateChildCurveVector_SwapFromSource (*area);
            return true;
            }
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
            result = ICurvePrimitive::CreateBsplineCurve (*bcurve);
            return true;
            }
        }
    else if (CurrentElementNameMatch ("Coordinate"))
        {
        DPoint3d xyz;
        if (   FindChildDPoint3d (node, "xyz", xyz))
            {
            result = ICurvePrimitive::CreatePointString (&xyz, 1);
            return true;
            }        
        }     
    result = ICurvePrimitivePtr ();
    return false;
    }


public: bool TryParse (BeXmlNodeP node, ISolidPrimitivePtr &result)
    {
    if (CurrentElementNameMatch ("CircularCone"))
        {
        RotMatrix axes;
        DPoint3d centerA, centerB;
        double radiusA, radiusB, height;
        bool capped;
        FindChildBool (node, "bSolidFlag", "capped", capped);

        if (   FindChildPlacement (node, "placement", centerA, axes)
            && FindChildDouble (node, "radiusA", radiusA)
            && FindChildDouble (node, "radiusB", radiusB)
            && FindChildDouble (node, "height", height)
            )
            {
            DVec3d vectorX, vectorY, vectorZ;
            axes.GetColumns (vectorX, vectorY, vectorZ);
            centerB.SumOf (centerA, vectorZ, height);
            DgnConeDetail coneDetail (centerA, centerB,
                        axes, 
                        radiusA, radiusB,
                        capped);
            result = ISolidPrimitive::CreateDgnCone (coneDetail);
            return true;
            }
        }
    else if (CurrentElementNameMatch ("SkewedCone"))
        {
        RotMatrix axes;
        DPoint3d centerA, centerB;
        double radiusA, radiusB;
        bool capped;
        FindChildBool (node, "bSolidFlag", "capped", capped);
        if (   FindChildPlacement (node, "placement", centerA, axes)
            && FindChildDouble (node, "radiusA", radiusA)
            && FindChildDouble (node, "radiusB", radiusB)
            && FindChildDPoint3d (node, "centerB", centerB)
            )
            {
            DVec3d vectorX, vectorY, vectorZ;
            axes.GetColumns (vectorX, vectorY, vectorZ);
            DgnConeDetail coneDetail (centerA, centerB,
                        axes, 
                        radiusA, radiusB,
                        capped);
            result = ISolidPrimitive::CreateDgnCone (coneDetail);
            return true;
            }
        }
    else if (CurrentElementNameMatch ("Sphere"))
        {
        RotMatrix axes;
        DPoint3d center;
        double radius;
        if (   FindChildPlacement (node, "placement", center, axes)
            && FindChildDouble (node, "radius", radius)
            )
            {
            DgnSphereDetail detail (center, axes, radius);
            result = ISolidPrimitive::CreateDgnSphere (detail);
            return true;
            }
        }
    else if (   CurrentElementNameMatch ("Box") == 0
             || CurrentElementNameMatch ("Block") == 0
            )
        {
        RotMatrix axes;
        DPoint3d origin;
        DVec3d cornerA, cornerB;
        bool bSolid;
        if (   FindChildPlacement (node, "placement", origin, axes)
            && FindChildDPoint3d (node, "cornerA", cornerA)
            && FindChildDPoint3d (node, "cornerB", cornerB)
            && FindChildBool (node, "bSolidFlag", "capped", bSolid)
            )
            {
            DVec3d vectorX, vectorY, vectorZ;
            axes.GetColumns (vectorX, vectorY, vectorZ);
            DPoint3d baseOrigin = DPoint3d::FromProduct (origin, axes, cornerA.x, cornerA.y, cornerA.z);
            DPoint3d topOrigin  = DPoint3d::FromProduct (origin, axes, cornerA.x, cornerA.y, cornerB.z);
            double dx = cornerB.x - cornerA.x;
            double dy = cornerB.y - cornerA.y;
            DgnBoxDetail detail (baseOrigin, topOrigin, vectorX, vectorY, dx, dy, dx, dy, bSolid);
            result = ISolidPrimitive::CreateDgnBox (detail);
            return true;
            }
        }
    else if (CurrentElementNameMatch ("CircularCylinder"))
        {
        RotMatrix axes;
        DPoint3d centerA, centerB;
        double radius;
        double height;
        bool capped = false;
        FindChildBool (node, "bSolidFlag", "capped", capped);
        if (   FindChildPlacement (node, "placement", centerA, axes)
            && FindChildDouble (node, "radius", radius)
            && FindChildDouble (node, "height", height)
            )
            {
            DVec3d vectorX, vectorY, vectorZ;
            axes.GetColumns (vectorX, vectorY, vectorZ);
            centerB.SumOf (centerA, vectorZ, height);
            DgnConeDetail coneDetail (centerA, centerB,
                        axes, 
                        radius, radius,
                        capped);
            result = ISolidPrimitive::CreateDgnCone (coneDetail);
            return true;
            }
        }
    else if (CurrentElementNameMatch ("TorusPipe"))
        {
        RotMatrix axes;
        DPoint3d center;
        double radiusA, radiusB;
        double sweepDegrees = 360.0, startDegrees = 0.0;
        bool capped = false;
        FindChildBool (node, "bSolidFlag", "capped", capped);
        FindChildDouble (node, "startAngle", startDegrees);
        FindChildDouble (node, "sweepAngle", sweepDegrees);
        if (   FindChildPlacement (node, "placement", center, axes)
            && FindChildDouble (node, "radiusA", radiusA)
            && FindChildDouble (node, "radiusB", radiusB)
            )
            {
            DVec3d vectorX, vectorY, vectorZ, vector0;
            axes.GetColumns (vectorX, vectorY, vectorZ);
            vector0 = vectorX;
            double startRadians = Angle::DegreesToRadians (startDegrees);
            double sweepRadians = Angle::DegreesToRadians (sweepDegrees);
            if (startDegrees != 0.0)
                vector0.SumOf (vectorX, cos (startRadians), vectorY, sin(startRadians));
            DgnTorusPipeDetail detail (center,
                        vectorX, vectorY, 
                        radiusA, radiusB,
                        sweepRadians,
                        capped);
            result = ISolidPrimitive::CreateDgnTorusPipe (detail);
            return true;
            }
        }
    else if (CurrentElementNameMatch ("SurfaceBySweptCurve"))
        {
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
        }
    return false;
    }



public: bool TryParse (bvector<IGeometryPtr> &geometry, size_t maxDepth)
    {
    if (!ReadToElement ())
        return false;

    ICurvePrimitivePtr curvePrimitive;
    ISolidPrimitivePtr solidPrimitive;
    MSBsplineSurfacePtr surface;
    size_t count = 0;
    BeXmlNodeP node = NULL;
    if (TryParse (node, curvePrimitive))
        {
        geometry.push_back (IGeometry::Create (curvePrimitive));
        count = 1;
        }
    else if (TryParse (node, solidPrimitive))
        {
        geometry.push_back (IGeometry::Create (solidPrimitive));
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
bool BeXmlCGStreamReader::TryParse (Utf8CP beXmlCGString, bvector<IGeometryPtr> &geometry, size_t maxDepth)
    {
    BeXmlStatus status;
    IGeometryCGFactory factory;
    BeXmlReaderPtr reader = BeXmlReader::CreateAndReadFromString (status, beXmlCGString);
    BeXmlCGStreamReaderImplementation parser (*reader, factory);
    return parser.TryParse(geometry, maxDepth);
    }

END_BENTLEY_ECOBJECT_NAMESPACE