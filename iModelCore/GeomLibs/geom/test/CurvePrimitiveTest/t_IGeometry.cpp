//
//
#include "testHarness.h"
#include <GeomSerialization/GeomSerializationApi.h>

USING_NAMESPACE_BENTLEY_GEOMETRY_INTERNAL

#define TestFlatBuffers


#if defined (_WIN32) && !defined(BENTLEY_WINRT)
static bool s_emitGeometry = false;
#endif

bool CheckUpCastMatch (IGeometryPtr g)
    {
    ICurvePrimitivePtr curvePrim;
    CurveVectorPtr curves;
    MSBsplineCurvePtr bcurve;
    MSBsplineSurfacePtr bsurf;
    ISolidPrimitivePtr solid;
    PolyfaceHeaderPtr mesh;
    bool stat = false;
    if (g->GetGeometryType () == IGeometry::GeometryType::CurvePrimitive)
        stat = g->GetAsICurvePrimitive ().IsValid ();

    if (g->GetGeometryType () == IGeometry::GeometryType::CurveVector)
        stat = g->GetAsCurveVector ().IsValid ();

    if (g->GetGeometryType () == IGeometry::GeometryType::SolidPrimitive)
        stat = g->GetAsISolidPrimitive ().IsValid ();

    if (g->GetGeometryType () == IGeometry::GeometryType::BsplineSurface)
        stat = g->GetAsMSBsplineSurface ().IsValid ();

    if (g->GetGeometryType () == IGeometry::GeometryType::Polyface)
        stat = g->GetAsPolyfaceHeader ().IsValid ();
    
    return stat;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(IGeoemtry,HelloWorld)
    {
    ICurvePrimitivePtr line = ICurvePrimitive::CreateLine (DSegment3d::From (1,2,3, 4,2,9));
    CheckUpCastMatch (IGeometry::Create(line));
    
    }    

#if defined (_WIN32) && !defined(BENTLEY_WINRT)
// serializer=0 for flatbuffer, 1 for json string
void DoRoundTrip (IGeometryPtr g0, bool emitGeometry, int serializerSelect)
    {
    bvector<Byte> buffer0;
    IGeometryPtr g1 = nullptr;
    bool checkPolyfaceQueryCarrier = false;
    if (serializerSelect == 1)
        {
        Utf8String jsonString;
        if (Check::True (BentleyGeometryJson::TryGeometryToJsonString (jsonString, *g0)))
            {
            bvector<IGeometryPtr> allGeometry;
            if (Check::True (BentleyGeometryJson::TryJsonStringToGeometry (jsonString, allGeometry))
                && Check::True (allGeometry.size () == 1)
                )
                {
                g1 = allGeometry[0];
                }
            }
        }
    else
        {
        BentleyGeometryFlatBuffer::GeometryToBytes (*g0, buffer0);
        g1 = BentleyGeometryFlatBuffer::BytesToGeometry (buffer0);
        checkPolyfaceQueryCarrier = true;
        }
    PolyfaceHeaderPtr polyface = g0->GetAsPolyfaceHeader ();
    if (checkPolyfaceQueryCarrier && polyface.IsValid () && buffer0.size () > 8)
        {

        PolyfaceQueryCarrier carrier0 (0, false, 0, 0, nullptr, nullptr);        
        Check::True (BentleyGeometryFlatBuffer::BytesToPolyfaceQueryCarrier (&buffer0[0], carrier0));

        bvector<Byte> buffer1;
        BentleyGeometryFlatBuffer::GeometryToBytes (*polyface, buffer1);

        PolyfaceQueryCarrier carrier1 (0, false, 0, 0, nullptr, nullptr);        
        Check::True (BentleyGeometryFlatBuffer::BytesToPolyfaceQueryCarrier (&buffer1[0], carrier1));
        }

    if (Check::True (g1.IsValid (), 
            serializerSelect == 1 ? "JsonString RoundTrip" :"FlatBuffer Roundtrip"))
        {
        Check::True (g0->IsSameStructureAndGeometry (*g1), "BGFB IsSameStructureAndGeoemtry");
        }
    
    }

void DoRoundTrip (IGeometryPtr g0, bool emitGeometry)
    {
    DoRoundTrip (g0, emitGeometry, 0);
    static int s_checkJson = 0;
    // Known json problems:
    // 1) bsurf -- implied outer loop changes to explicit.  Stroking generates extra points.
    // 2) partial curve not supported
    // 3) inteprolating curve???
    if (s_checkJson)
        DoRoundTrip (g0, emitGeometry, 1);
    }

void DoRoundTripMeshedSolid (IGeometryPtr g0, bool emitGeometry)
    {
    auto solid = g0->GetAsISolidPrimitive ();
    if (solid.IsValid ())
        {
        IFacetOptionsPtr options = IFacetOptions::Create ();
        options->SetParamsRequired (true);
        IPolyfaceConstructionPtr builder = IPolyfaceConstruction::Create (*options);
        if (Check::True (builder->AddSolidPrimitive (*solid), "DoRoundTrip"))
            {
            auto gMesh = IGeometry::Create (builder->GetClientMeshPtr ());
            DoRoundTrip (gMesh, emitGeometry);
            }
        }
    }
  
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(BGFB,HelloWorld)
    {
    ICurvePrimitivePtr cp0 = ICurvePrimitive::CreateLine (DSegment3d::From (1,2,3,4,3,2));
    IGeometryPtr g0 = IGeometry::Create (cp0);
    DoRoundTrip (g0, s_emitGeometry);

    ICurvePrimitivePtr cp1 = ICurvePrimitive::CreateLineString (NULL, 0);
    for (double x = 0; x <= 4.0; x += 1.0)
        cp1->TryAddLineStringPoint (DPoint3d::From (x,0,2 * x));
    IGeometryPtr g1 = IGeometry::Create (cp1);
    DoRoundTrip (g1, s_emitGeometry);

    ICurvePrimitivePtr cp2 = ICurvePrimitive::CreateLineString (NULL, 0);
    for (double x = 0; x <= 104.0; x += 1.0)
        cp2->TryAddLineStringPoint (DPoint3d::From (x,0,2 * x));
    IGeometryPtr g2 = IGeometry::Create (cp2);
    DoRoundTrip (g2, s_emitGeometry);
    
    DEllipse3d arc;
    arc.InitFromXYMajorMinor (1,2,3, 1,4, Angle::DegreesToRadians (45),
            Angle::DegreesToRadians (10.0), Angle::DegreesToRadians (270.0));

    ICurvePrimitivePtr cp3A = ICurvePrimitive::CreateArc (arc);
    IGeometryPtr g3A = IGeometry::Create (cp3A);
    DoRoundTrip (g3A, s_emitGeometry);

    MSBsplineCurve bcurve;
    bcurve.InitFromDEllipse3d (arc);
    ICurvePrimitivePtr cp3 = ICurvePrimitive::CreateBsplineCurveSwapFromSource (bcurve);
    DoRoundTrip (IGeometry::Create (cp3), s_emitGeometry);



    CurveVectorPtr cvNone = CurveVector::Create (CurveVector::BOUNDARY_TYPE_None);
    cvNone->push_back (cp0);
    cvNone->push_back (cp1);
    cvNone->push_back (cp2);
    cvNone->push_back (cp3);
    DoRoundTrip (IGeometry::Create (cvNone), s_emitGeometry);

    DgnConeDetail coneDetail (DPoint3d::From (1,2,2), DPoint3d::From (3,2,1), 1, 2, true);
    ISolidPrimitivePtr cone = ISolidPrimitive::CreateDgnCone (coneDetail);
    DoRoundTrip (IGeometry::Create (cone), s_emitGeometry);
    DoRoundTripMeshedSolid (IGeometry::Create (cone), s_emitGeometry);
    
    DgnBoxDetail boxDetail = DgnBoxDetail::InitFromCenterAndSize (DPoint3d::From (2,2,2), DPoint3d::From (3,4,5), false);
    ISolidPrimitivePtr Box = ISolidPrimitive::CreateDgnBox (boxDetail);
    DoRoundTrip (IGeometry::Create (Box), s_emitGeometry);
    DoRoundTripMeshedSolid (IGeometry::Create (Box), s_emitGeometry);

    DgnSphereDetail sphereDetail (DPoint3d::From (2,2,2), 6);
    ISolidPrimitivePtr sphere = ISolidPrimitive::CreateDgnSphere (sphereDetail);
    DoRoundTrip (IGeometry::Create (sphere), s_emitGeometry);
    DoRoundTripMeshedSolid (IGeometry::Create (sphere), s_emitGeometry);
    
    DgnTorusPipeDetail torusPipeDetail (
                DPoint3d::From (2,2,2),
                DVec3d::From (1,0,0),
                DVec3d::From (0,1,0),
                10,
                1,
                Angle::DegreesToRadians (45),
                true
                );
    ISolidPrimitivePtr torusPipe = ISolidPrimitive::CreateDgnTorusPipe (torusPipeDetail);
    DoRoundTrip (IGeometry::Create (torusPipe), s_emitGeometry);
    DoRoundTripMeshedSolid (IGeometry::Create (torusPipe), s_emitGeometry);

    CurveVectorPtr unitSquare = CurveVector::CreateRectangle (0.0, 0.0,    1.0,1.0,   0.0);
    DgnExtrusionDetail extrusionDetail (unitSquare, DVec3d::From (0,0,2), true);
    ISolidPrimitivePtr extrusion = ISolidPrimitive::CreateDgnExtrusion (extrusionDetail);
    DoRoundTrip (IGeometry::Create(unitSquare), s_emitGeometry);
    DoRoundTrip (IGeometry::Create(extrusion), s_emitGeometry);

    MSBsplineSurfacePtr surface = SimpleBilinearPatch (1, 1, 0.5);
    DoRoundTrip (IGeometry::Create (surface), s_emitGeometry);
    bvector<DPoint2d> uvTrim;

    uvTrim.push_back (DPoint2d::From (0,0));
    uvTrim.push_back (DPoint2d::From (0.25,0));
    uvTrim.push_back (DPoint2d::From (0.25,1));
    uvTrim.push_back (DPoint2d::From (0,1));
    uvTrim.push_back (DPoint2d::From (0,0));

    surface->AddTrimBoundary (uvTrim);
    DoRoundTrip (IGeometry::Create (surface), s_emitGeometry);

    PolyfaceHeaderPtr mesh = PolyfaceHeader::CreateVariableSizeIndexed ();
    mesh->Point ().push_back (DPoint3d::From (0,0,0));
    mesh->Point ().push_back (DPoint3d::From (2,0,0));
    mesh->Point ().push_back (DPoint3d::From (1,1,0));
    mesh->Point ().push_back (DPoint3d::From (0,2,0));
    mesh->PointIndex ().push_back (1);
    mesh->PointIndex ().push_back (2);
    mesh->PointIndex ().push_back (3);
    mesh->PointIndex ().push_back (0);

    mesh->PointIndex ().push_back (1);
    mesh->PointIndex ().push_back (3);
    mesh->PointIndex ().push_back (4);
    mesh->PointIndex ().push_back (0);
    
    DoRoundTrip (IGeometry::Create (mesh), s_emitGeometry);    

    {
    ICurvePrimitivePtr cp0 = ICurvePrimitive::CreateLine (DSegment3d::From (1,2,3,4,3,2));
    ICurvePrimitivePtr cp1 = ICurvePrimitive::CreatePartialCurve (cp0.get (), 0.2, 0.4);
    IGeometryPtr g0 = IGeometry::Create (cp1);
    DoRoundTrip (g0, s_emitGeometry, 0);
    DoRoundTrip (g0, s_emitGeometry, 1);
    }


    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(FlatBuffer,Catenary1)
    {
    auto catenary = ICurvePrimitive::CreateCatenary (2.0,
                DPoint3dDVec3dDVec3d::FromXYPlane (), 0.0, 3.0);
    DoRoundTrip (IGeometry::Create (catenary), s_emitGeometry);

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Flatbuffer, InterpolationCurve)
    {
    MSInterpolationCurve curve;
    bvector<DPoint3d> iPoints;
    iPoints.push_back (DPoint3d::From (0,0,0));
    iPoints.push_back (DPoint3d::From (1,1,0));
    iPoints.push_back (DPoint3d::From (2,1,0));
    iPoints.push_back (DPoint3d::From (3,0,0));
    DPoint3d tangents[2];
    tangents[0] = DPoint3d::From (1,0,0);
    tangents[1] = DPoint3d::From (0,-1,0);
    curve.InitFromPointsAndEndTangents (iPoints, false, 0.0, tangents, false, false, false, false);
    ICurvePrimitivePtr ic0 = ICurvePrimitive::CreateInterpolationCurveSwapFromSource (curve);
    DoRoundTrip (IGeometry::Create(ic0), s_emitGeometry);

    }    
 
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Flatbuffer, TransitionSpiral)
    {
    Transform frame = Transform::From (
          RotMatrix::FromVectorAndRotationAngle (DVec3d::From (0.3, 0.2, 0.8), 0.3),
          DPoint3d::From (1,2,4)
          );

    ICurvePrimitivePtr ic0 = ICurvePrimitive::CreateSpiralBearingCurvatureBearingCurvature
        (
        DSpiral2dBase::TransitionType_Clothoid,
        0.2, 0.0,
        0.4, 0.01,
        frame, 
        0.2, 0.9
        );
    DoRoundTrip (IGeometry::Create(ic0), s_emitGeometry);

    }    

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Flatbuffer, GeometryGroup)
    {
    auto cp0 = ICurvePrimitive::CreateLine (DSegment3d::From (1,2,3,4,5,6));
    auto cp1 = ICurvePrimitive::CreateArc (DEllipse3d::From (0,1,2,  1,0,0,  0,2,0, 0.0, Angle::TwoPi ()));
    bvector<IGeometryPtr> array0, array1;
    array0.push_back (IGeometry::Create (cp0));
    array0.push_back (IGeometry::Create (cp1));
    bvector<Byte> buffer;
    BentleyGeometryFlatBuffer::GeometryToBytes (array0, buffer);
    BentleyGeometryFlatBuffer::BytesToVectorOfGeometry (buffer, array1);
    if (Check::Size (array0.size (), array1.size ()))
        {
        for (size_t i = 0; i < array0.size (); i++)
            {
            Check::True (array0[i]->IsSameStructureAndGeometry (*array1[i]));
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(FlatBuffer,Signature)
    {
    auto cp0 = IGeometry::Create (ICurvePrimitive::CreateLine (DSegment3d::From (1,2,3,4,5,6)));
    bvector<Byte> buffer0;
    BentleyGeometryFlatBuffer::GeometryToBytes (*cp0, buffer0);
    Check::True (BentleyGeometryFlatBuffer::IsFlatBufferFormat (buffer0), "Confirm FB signature");
    buffer0[3] = '$';
    Check::False (BentleyGeometryFlatBuffer::IsFlatBufferFormat (buffer0), "Confirm invalid FB signature");
    }

IGeometryPtr upgradeToGeometry (ICurvePrimitiveR cpref)
    {
    ICurvePrimitivePtr cpPtr = &cpref;
    return IGeometry::Create (cpPtr);
    }

ICurvePrimitivePtr upgradeC (ICurvePrimitiveCR cpref)
    {
    ICurvePrimitivePtr cpPtr = const_cast <ICurvePrimitiveP>(&cpref);
    return cpPtr;
    }

ICurvePrimitivePtr upgrade (ICurvePrimitiveR cpref)
    {
    ICurvePrimitivePtr cpPtr = &cpref;
    return cpPtr;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (IcurvePrimCasts, Test0)
    {
    ICurvePrimitivePtr linePtr = ICurvePrimitive::CreateLine (DSegment3d::From (1,2,3,4,5,6));
    IGeometryPtr gptrA = IGeometry::Create (upgrade (*linePtr));
    IGeometryPtr gptrB = IGeometry::Create (ICurvePrimitivePtr (linePtr.get ()));
    IGeometryPtr gptrC = upgradeToGeometry (*linePtr);
    }

#ifdef abc
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(OrderedIGeometryPtr, Test1)
    {
    bmap<OrderedIGeometryPtr, size_t> myMap;
    ICurvePrimitivePtr line1 = ICurvePrimitive::CreateLine (DSegment3d::From (1,2,3,4,5,6));
    ICurvePrimitivePtr line2 = ICurvePrimitive::CreateLine (DSegment3d::From (10,11,12,13,14,15));
    ICurvePrimitivePtr line3 = ICurvePrimitive::CreateLine (DSegment3d::From (101,111,121,131,141,151));

    IGeometryPtr g1 = IGeometry::Create (line1);
    OrderedIGeometryPtr o1 = OrderedIGeometryPtr (g1);

    myMap [o1] = 1;
    myMap[OrderedIGeometryPtr(*line2)] = 2;
    printf (" demap [o1] = %d\n", myMap[o1]);
    printf (" dmap [second ref to line1] = %d\n", myMap[OrderedIGeometryPtr (*line1)]);
    printf (" dmap [bare use of line2] = %d\n", myMap[OrderedIGeometryPtr(*line2)]);
    printf (" demap [other] = %d\n", myMap[OrderedIGeometryPtr (line3)]);
    }
#endif

#endif
#ifdef TestQuickGeometryFormat
struct IQuickGeometry
{
virtual void StartObject (char const *name) = 0;
virtual void EndObject () = 0;
virtual void AddProperty (char const *name, int value) = 0;

virtual void AddProperty (char const *name, size_t value)
    {
    AddProperty (name, (int)value);
    }

virtual void AnnounceDoubleIndex ()
    {
    AddProperty ("d", m_doubles.size ());
    }

Utf8String m_string;
bvector<double> m_doubles;
bvector<int> m_ints;
public:
void AddToString (char const *s)
    {
    for (int i = 0; s[i] != 0; i++)
      m_string.push_back (s[i]);
    }



void AddXYZ (DPoint3dCR xyz)
    {
    m_doubles.push_back (xyz.x);
    m_doubles.push_back (xyz.y);
    m_doubles.push_back (xyz.z);
    }

void AddDouble (double a)
    {
    m_doubles.push_back (a);
    }


// count is POINTS, not doubles . .  .
void AddXYZ (char const *name, bvector<DPoint3d> const &xyz)
    {
    AddProperty (name, xyz.size ());
    for (DPoint3d point : xyz)
        {
        m_doubles.push_back (point.x);
        m_doubles.push_back (point.y);
        m_doubles.push_back (point.z);
        }
    }



void AddRadians (double radians)
    {
    m_doubles.push_back (radians);
    }


bool AddGeometry (IGeometry const &geometry)
    {
    switch (geometry.GetGeometryType ())
        {
        case IGeometry::GeometryType::CurvePrimitive:
            return CurvePrimitiveToData (*geometry.GetAsICurvePrimitive());
        }
    return false;
    }
private:
bool CurvePrimitiveToData (ICurvePrimitive const &cp)
    {
    switch (cp.GetCurvePrimitiveType ())
        {
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line:
            {
            DSegment3d segment;
            cp.TryGetLine (segment);
            StartObject ("LSG");
            AddXYZ (segment.point[0]);
            AddXYZ (segment.point[1]);
            EndObject ();
            return true;
            }

        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc:
            {
            DEllipse3d arc;
            cp.TryGetArc (arc);
            StartObject ("ARC");
            AddXYZ (arc.center);
            AddXYZ (arc.vector0);
            AddXYZ (arc.vector90);
            AddRadians (arc.start);
            AddRadians (arc.sweep);
            EndObject ();
            return true;
            }

        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString:
            {
            bvector<DPoint3d> const *points = cp.GetLineStringCP ();
            StartObject ("LST");
            AddXYZ ("n", *points);
            EndObject ();
            return true;
            }

        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_AkimaCurve:
            {
            bvector<DPoint3d> const *points = cp.GetAkimaCurveCP ();
            StartObject ("AKC");
            AddXYZ ("n", *points);
            EndObject ();
            return true;
            }

        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_BsplineCurve:
            {
            MSBsplineCurvePtr bcurve = cp.GetBsplineCurvePtr ();
            StartObject ("BCU");
            AddProperty ("order", bcurve->GetOrder ());
            size_t numPoles = bcurve->GetNumPoles ();
            size_t numKnots = bcurve->GetNumKnots ();
            AddProperty ("poles", numPoles);
            for (size_t i = 0; i < numPoles; i++)
                AddXYZ (bcurve->GetPole (i));
            AddProperty ("knots", numKnots);
            for (size_t i = 0; i < numKnots; i++)
                AddDouble (bcurve->GetKnot (i));
            if (bcurve->HasWeights ())
                {
                AddProperty ("weights", numPoles);
                for (size_t i = 0; i < numPoles; i++)
                    AddDouble (bcurve->GetWeight (i));
                }
            EndObject ();
            return true;
            }
        }
    return false;
    }
};

struct LispishQuickGeometry : IQuickGeometry
{

void StartObject (char const *name) override
    {
    m_string.push_back ('(');
    AddToString (name);
    AnnounceDoubleIndex ();
    }
void EndObject () override
    {
    m_string.push_back (')');
    }
void AddProperty (char const *name, int value) override
    {
    char s[128];
    sprintf (s, " :%s:%d", name, value);
    AddToString (s);
    }

void AddProperty (char const *name, size_t value) override
    {
    char s[128];
    sprintf (s, " (%s %d)", name, (int)value);
    AddToString (s);
    }
};

struct XmlQuickGeometry : IQuickGeometry
{
void StartObject (char const *name) override
    {
    AddToString ("<");
    AddToString (name);
    AnnounceDoubleIndex ();
    }
void EndObject () override
    {
    AddToString ("/>");
    }
void AddProperty (char const *name, int value) override
    {
    char s[128];
    sprintf (s, " %s=\"%d\"", name, value);
    AddToString (s);
    }

};


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(SampleGeometryCreator,Test0)
    {
    bvector<IGeometryPtr> geometry;
    SampleGeometryCreator::AddSizeSampler (geometry);


    for (IGeometryPtr &g : geometry)
        {
        bvector<Byte> buffer;
        BentleyGeometryFlatBuffer::GeometryToBytes (*g, buffer);
        if (Check::True (buffer.size () > 8))
            {
            IGeometryPtr g1 = BentleyGeometryFlatBuffer::BytesToGeometry (buffer);
            Check::True (g->IsSameStructureAndGeometry (*g1));
            }
        //LispishQuickGeometry qg;
        XmlQuickGeometry qg;
        qg.AddGeometry (*g);
        size_t qgSize = qg.m_string.size () + 8 * qg.m_doubles.size () + 4 * qg.m_ints.size ();
;
        printf ("(quickG (%3d + 8*%4d+4*%5d=%5d)) (FB %5d) (diff %5d) %s\n",
                (int)qg.m_string.size (),
                (int)qg.m_doubles.size (),
                (int)qg.m_ints.size (),
                (int)qgSize,
                (int)buffer.size (),
                ((int)qgSize - (int)buffer.size ()),
                &qg.m_string[0]
                );
        }
    }

#endif
void TestFBLineString (size_t numPoint)
    {
    CurveVectorPtr sticks = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Outer);
    bvector<DPoint3d> points;
    for (size_t i = 0; i < numPoint; i++)
        {
        points.push_back (DPoint3d::From ((double)i, (double)2*i, (double)i*i));
        }
    auto linestring = ICurvePrimitive::CreateLineString (points);
    auto g = IGeometry::Create (linestring);
    bvector<Byte> buffer;
    BentleyGeometryFlatBuffer::GeometryToBytes (*g, buffer);
    IGeometryPtr g1 = BentleyGeometryFlatBuffer::BytesToGeometry (buffer);
    Check::True (g->IsSameStructureAndGeometry (*g1));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(FlatBuffer, LongLineString)
    {
    TestFBLineString (10);
    TestFBLineString (100);
    TestFBLineString (1000);
    TestFBLineString (10000);
    }

GeometryNodePtr MakeNodeWithAllGeometry ()
    {
    GeometryNodePtr node = GeometryNode::Create ();
    SampleGeometryCreator::AddAllTypes (node->Geometry());
    return node;
    }
// Check that children accessed via AppendTransformedGeometry have same change to transform.
// This is only valid if the transform has only translate, principal scale, and principal exchange.
void CheckRangeTransformCommutes (
GeometryNodePtr const &rootNode,
GeometryNodePtr const &leafNode,
TransformCR transform)
    {
    auto flattened = rootNode->Flatten ();
    auto &transformedChildren = flattened->Geometry ();
    if (Check::Size (leafNode->Geometry ().size (), transformedChildren.size ()))
        {
        for (size_t i = 0; i < transformedChildren.size (); i++)
            {
            DRange3d range1, range2, range1a;
            auto leaf = leafNode->Geometry ()[i];
            leaf->TryGetRange (range1);
            transform.Multiply (range1a, range1);
            transformedChildren[i]->TryGetRange (range2);
            Check::Near (range1a, range2, "Range in geometry tree transforms");
            }
        }
    DRange3d rootRange;
    DRange3d flattenedRange;

    if (Check::True (rootNode->TryGetRange (rootRange), "recursive range")
        && Check::True (flattened->TryGetRange (flattenedRange), "flattened range"))
        {
        Check::Near (rootRange, flattenedRange, "Recursive tree range versus flattened range");
        }
    if (Check::True (rootNode->TryGetRange (rootRange, transform), "recursive range")
        && Check::True (flattened->TryGetRange (flattenedRange, transform), "flattened range"))
        {
        Check::Near (rootRange, flattenedRange, "Recursive tree range versus flattened range");
        }
    auto rootB = rootNode->Clone ();
    auto rootC = rootNode->Clone (transform);
    Check::True (rootNode->IsSameStructureAndGeometry (*rootB));
    Check::Bool (!transform.IsIdentity (), rootNode->IsSameStructureAndGeometry (*rootB));
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(GeometryNode,Test0)
    {
    auto m2 = RotMatrix::FromPrimaryAxisDirections (1, 1, 2, -1);
    auto t2 = Transform::FromMatrixAndFixedPoint (m2.Value (), DPoint3d::From (4,3,2));
    auto m3 = RotMatrix::FromPrimaryAxisDirections (1, 1, 0, -1);
    auto t3 = Transform::FromMatrixAndFixedPoint (m3.Value (), DPoint3d::From (-1,2, 4));

    GeometryNodePtr root   = GeometryNode::Create ();
    GeometryNodePtr child2 = GeometryNode::Create ();
    GeometryNodePtr child3 = MakeNodeWithAllGeometry ();
    root->AddMember (child2, t2);
    child2->AddMember (child3, t3);

    CheckRangeTransformCommutes (child2, child3, t3);
    CheckRangeTransformCommutes (root, child3, t2 * t3);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(FlatBuffer,CurvePrimitiveId)
    {
    uint32_t myIndex = 322;
    auto cp0 = ICurvePrimitive::CreateLine (DSegment3d::From (1,2,3,4,5,6));
    auto id = CurvePrimitiveId::Create (
            CurvePrimitiveId::Type::ConceptStationAlignmentIndex,
            (void*)&myIndex, sizeof (myIndex)
            );
    if (id.IsValid ())
        cp0->SetId (id.get ());
    auto g0 = IGeometry::Create (cp0);
    bvector<Byte> buffer;
    BentleyGeometryFlatBuffer::GeometryToBytes (*g0, buffer);
    IGeometryPtr g1 = BentleyGeometryFlatBuffer::BytesToGeometry (buffer);
    Check::True (g0->IsSameStructureAndGeometry (*g1));
    auto cp1 = g1->GetAsICurvePrimitive ();
    if (Check::True (cp1.IsValid ()))
        {
        auto id0 = cp0->GetId ();
        auto id1 = cp1->GetId ();
        if (Check::True (nullptr != id0, "cp0->GetId")
            && Check::True (nullptr != id1, "cp1->GetId")
            )
            {
            Check::Int ((int)id0->GetType (), (int)id1->GetType(),"Round trip id type");
            }
        }
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(FlatBuffer,CurvePrimitiveIdInSolidPrimitive)
    {
    for (auto attachId : bvector<bool> {false, true})
        {
        uint32_t myIndex = 322;
        auto cp0 = ICurvePrimitive::CreateLineString (bvector<DPoint3d>{
                DPoint3d::From (1,2,3),
                DPoint3d::From (4,2,5),
                DPoint3d::From (1,0,1)
                });
        if (attachId)
            {
            auto id = CurvePrimitiveId::Create (
                    CurvePrimitiveId::Type::ConceptStationAlignmentIndex,
                    (void*)&myIndex, sizeof (myIndex)
                    );
            if (id.IsValid ())
                {
                bvector<uint8_t> myBytes {1,2,3,0};
                id->Store (myBytes);
                cp0->SetId (id.get ());
                }
            }
        CurveVectorPtr cv = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);
        cv->push_back (cp0);
        DgnExtrusionDetail extrusionDetail (cv, DVec3d::From (0,0,2), true);
        auto sp0 = ISolidPrimitive::CreateDgnExtrusion (extrusionDetail);
        auto g0 = IGeometry::Create (sp0);
        bvector<Byte> buffer;
        BentleyGeometryFlatBuffer::GeometryToBytes (*g0, buffer);
        IGeometryPtr g1 = BentleyGeometryFlatBuffer::BytesToGeometry (buffer);
        Check::True (g0->IsSameStructureAndGeometry (*g1));
        }
    }