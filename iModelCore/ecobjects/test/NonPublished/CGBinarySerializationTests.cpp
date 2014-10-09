/*--------------------------------------------------------------------------------------+
|
|     $Source: test/NonPublished/CGBinarySerializationTests.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsTestPCH.h"
#include "TestFixture.h"

#include "../../src/MSXmlBinary/MSXmlBinaryWriter.h"

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

struct CGBinarySerializationTests : ECTestFixture
    {
    void Roundtrip(Utf8String xml)
        {
        bvector<IGeometryPtr> geoms;
        ASSERT_TRUE(BeXmlCGParser::TryParse(xml.c_str(), geoms, 0)) << "Failed to deserialize string: " << xml.c_str();
        ASSERT_EQ(1, geoms.size()) << "Expected 1 geometry object returned for string deserialization but got " << geoms.size() << " for " << xml.c_str();

        bvector<byte> bytes;
        //    BeXmlCGWriter::WriteBytes(bytes, *(originalArc.get()));
        BeXmlCGWriter::WriteBytes(bytes, geoms[0]);

        bvector<IGeometryPtr> geoms2;
        byte *buffer = new byte[bytes.size()];
        for (int i = 0; i < (int) bytes.size(); i++)
            buffer[i] = bytes[i];
        ASSERT_TRUE(BeXmlCGStreamReader::TryParse(buffer, (int) bytes.size(), geoms2, 0)) << "Failed binary deserialization of " << xml.c_str();
        ASSERT_EQ(1, geoms2.size()) << "Expected 1 geometry object returned for binary deserialization but got " << geoms.size() << " for " << xml.c_str();
        ASSERT_TRUE(geoms[0]->IsSameStructureAndGeometry(*geoms2[0])) << "Did not get same structure for " << xml.c_str();
        }
    };

TEST_F(CGBinarySerializationTests, WriterTest)
    {
    DEllipse3d ellipseData = DEllipse3d::From (0.0, 0.0, 0.0, 
        1.0, 0.0, 0.0, 
        0.0, 1.0, 0.0, 
        0.0, Angle::TwoPi ());

    ICurvePrimitivePtr originalArc = ICurvePrimitive::CreateArc (ellipseData);
    ASSERT_EQ(true, originalArc->GetArcCP()->IsCircular());
    //    BeXmlCGWriter::WriteBytes(bytes, *(originalArc.get()));

    Roundtrip(Utf8String("<LineSegment xmlns=\"http://www.bentley.com/schemas/Bentley.Geometry.Common.1.0\"><startPoint>0,0,0</startPoint><endPoint>1,0,0</endPoint></LineSegment>"));
    Roundtrip(Utf8String("<CircularArc xmlns=\"http://www.bentley.com/schemas/Bentley.Geometry.Common.1.0\"><placement><origin>0,0,0</origin><vectorZ>0,0,1</vectorZ><vectorX>1,0,0</vectorX></placement><radius>1.125</radius><startAngle>0</startAngle><sweepAngle>-165</sweepAngle></CircularArc>"));
    Roundtrip(Utf8String("<EllipticArc xmlns=\"http://www.bentley.com/schemas/Bentley.Geometry.Common.1.0\"><placement><origin>1,2,3</origin><vectorZ>0,0,1</vectorZ><vectorX>1,0,0</vectorX></placement><radiusA>10</radiusA><radiusB>15</radiusB><startAngle>20</startAngle><sweepAngle>30</sweepAngle></EllipticArc>"));
    Roundtrip(Utf8String("<Sphere><placement><origin>0,0,0</origin><vectorZ>0,0,1</vectorZ><vectorX>1,0,0</vectorX></placement><radius>10</radius></Sphere>"));
    Roundtrip(Utf8String("<TorusPipe<placement><origin>0,0,0</origin><vectorZ>0,0,1</vectorZ><vectorX>1,0,0</vectorX></placement><radiusA>0</radiusA><radiusB>0</radiusB><startAngle>0</startAngle><sweepAngle>0</sweepAngle><bSolidFlag>false</bSolidFlag></TorusPipe>"));
    Roundtrip(Utf8String("<SkewedCone><placement><origin>0,0,0</origin><vectorZ>0,0,1</vectorZ><vectorX>1,0,0</vectorX></placement><centerB>0,0,0</centerB><radiusA>0</radiusA><radiusB>0</radiusB><bSolidFlag>false</bSolidFlag></SkewedCone>"));
    Roundtrip(Utf8String("<Block><placement><origin>0,0,0</origin><vectorZ>0,0,1</vectorZ><vectorX>1,0,0</vectorX></placement><cornerA>0,0,0</cornerA><cornerB>2,3,4<cornerB><bSolidFlag>false</bSolidFlag></Block>"));
    Roundtrip(Utf8String("<CircularCone><placement><origin>0,0,0</origin><vectorZ>0,0,1</vectorZ><vectorX>1,0,0</vectorX></placement><height>2</height><radiusA>3</radiusA><radiusB>4</radiusB><bSolidFlag>false</bSolidFlag></CircularCone>"));
    Roundtrip(Utf8String("<EllipticDisk xmlns=\"http://www.bentley.com/schemas/Bentley.Geometry.Common.1.0\"><placement><origin>0,0,0</origin><vectorZ>0,0,1</vectorZ><vectorX>1,0,0</vectorX></placement><radiusA>1</radiusA><radiusB>1.5</radiusB></EllipticDisk>"));
    Roundtrip(Utf8String("<CircularDisk xmlns=\"http://www.bentley.com/schemas/Bentley.Geometry.Common.1.0\"><placement><origin>0,0,0</origin><vectorZ>0,0,1</vectorZ><vectorX>1,0,0</vectorX></placement><radius>1</radius></CircularDisk>"));
    Roundtrip(Utf8String("<LineString><ListOfPoint><xyz>0,0,0</xyz><xyz>0,0,1</xyz><xyz>1,0,0</xyz></ListOfPoint></LineString>"));
    Roundtrip(Utf8String("<IndexedMesh><ListOfCoord><Coord>1,2,3</Coord><Coord>1,2,4</Coord><Coord>5,2,1</Coord></ListOfCoord><ListOfCoordIndex><id>1</id><id>2</id><id>3</id><id>0</id></ListOfCoordIndex></IndexedMesh>"));
    //Roundtrip(Utf8String("<BsplineCurve xmlns=\"http://www.bentley.com/schemas/Bentley.Geometry.Common.1.0\"><Order>2</Order><Closed>false</Closed><ListOfControlPoint><xyz>1,0,0</xyz><xyz>0.86602540378443871,0.49999999999999994,0</xyz></ListOfControlPoint><ListOfKnot><Knot>0</Knot><Knot>0</Knot><Knot>1</Knot><Knot>1</Knot></ListOfKnot></BsplineCurve>"));
    //Roundtrip(Utf8String("<BsplineCurve xmlns=\"http://www.bentley.com/schemas/Bentley.Geometry.Common.1.0\"><Order>2</Order><Closed>false</Closed><ListOfControlPoint><xyz>1,0,0</xyz><xyz>0.86602540378443871,0.49999999999999994,0</xyz></ListOfControlPoint></BsplineCurve>"));
    //Roundtrip(Utf8String("<CircularCylinder xmlns=\"http://www.bentley.com/schemas/Bentley.Geometry.Common.1.0\"><placement><origin>0,0,0</origin><vectorZ>0,0,1</vectorZ><vectorX>1,0,0</vectorX></placement><height>1</height><radius>1</radius><capped>true</capped></CircularCylinder>"));
    //Roundtrip(Utf8String("<BsplineSurface xmlns=\"http://www.bentley.com/schemas/Bentley.Geometry.Common.1.0\"><OrderU>3</OrderU><ClosedU>false</ClosedU><NumUControlPoint>3</NumUControlPoint><OrderV>2</OrderV><ClosedV>false</ClosedV><NumVControlPoint>2</NumVControlPoint><ListOfControlPoint><xyz>0,0,0</xyz><xyz>1,0,0</xyz><xyz>2,0,0</xyz><xyz>0,1,0</xyz><xyz>1,1,0</xyz><xyz>3,1,0</xyz></ListOfControlPoint><ListOfWeight><Weight>1</Weight><Weight>1</Weight><Weight>1</Weight><Weight>1</Weight><Weight>1</Weight><Weight>1</Weight></ListOfWeight><ListOfKnotU><KnotU>0</KnotU><KnotU>0</KnotU><KnotU>0</KnotU><KnotU>1.5</KnotU><KnotU>1.5</KnotU><KnotU>1.5</KnotU></ListOfKnotU><ListOfKnotV><KnotV>1</KnotV><KnotV>1</KnotV><KnotV>2</KnotV><KnotV>2</KnotV></ListOfKnotV></BsplineSurface>"));
    //Roundtrip(Utf8String("<Polygon xmlns=\"http://www.bentley.com/schemas/Bentley.Geometry.Common.1.0\"><ListOfPoint><xyz>0.90096886790241915,0.43388373911755812,0</xyz><xyz>0.22252093395631445,0.97492791218182362,0</xyz><xyz>-0.62348980185873382,0.78183148246802958,0</xyz><xyz>-1,1.2246063538223773E-16,0</xyz><xyz>-0.62348980185873371,-0.78183148246802969,0</xyz><xyz>0.22252093395631509,-0.9749279121818234,0</xyz><xyz>0.900968867902419,-0.43388373911755834,0</xyz><xyz>0.90096886790241915,0.43388373911755812,0</xyz></ListOfPoint></Polygon>"));
    //Roundtrip(Utf8String("<SurfacePatch xmlns=\"http://www.bentley.com/schemas/Bentley.Geometry.Common.1.0\"><ExteriorLoop><CurveChain><ListOfCurve><LineString><ListOfPoint><xyz>-5,0,0</xyz><xyz>5,0,0</xyz><xyz>5,3,0</xyz><xyz>-5,3,0</xyz><xyz>-5,0,0</xyz></ListOfPoint></LineString></ListOfCurve></CurveChain></ExteriorLoop></SurfacePatch>"));
    }

TEST (CGSerializationTests, DeserializeEllipticDisc)
    {
    Utf8String xml("<EllipticDisk xmlns=\"http://www.bentley.com/schemas/Bentley.Geometry.Common.1.0\"><placement><origin>0,0,0</origin><vectorZ>0,0,1</vectorZ><vectorX>1,0,0</vectorX></placement><radiusA>1</radiusA><radiusB>1.5</radiusB></EllipticDisk>");
    bvector<IGeometryPtr> geoms;
    BeXmlCGParser::TryParse(xml.c_str(), geoms, 0);

    IGeometryPtr geometry = geoms[0];
    ICurvePrimitivePtr curvePrimitive = geometry->GetAsICurvePrimitive ();
    CurveVectorCR curveVector = *curvePrimitive->GetChildCurveVectorCP ();
    DEllipse3d arc;
    curveVector[0]->TryGetArc (arc);
    ASSERT_TRUE(arc.IsFullEllipse ());
    ASSERT_FALSE(arc.IsCircular ());
    }

END_BENTLEY_ECOBJECT_NAMESPACE
