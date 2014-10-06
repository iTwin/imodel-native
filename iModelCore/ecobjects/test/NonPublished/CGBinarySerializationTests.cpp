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
        BeXmlCGParser::TryParse(xml.c_str(), geoms, 0);

        bvector<byte> bytes;
        //    BeXmlCGWriter::WriteBytes(bytes, *(originalArc.get()));
        BeXmlCGWriter::WriteBytes(bytes, geoms[0]);

        }
    };

//TEST_F(CGBinarySerializationTests, WriterTest)
//    {
//    DEllipse3d ellipseData = DEllipse3d::From (0.0, 0.0, 0.0, 
//        1.0, 0.0, 0.0, 
//        0.0, 1.0, 0.0, 
//        0.0, Angle::TwoPi ());
//
//    ICurvePrimitivePtr originalArc = ICurvePrimitive::CreateArc (ellipseData);
//    ASSERT_EQ(true, originalArc->GetArcCP()->IsCircular());
//    //    BeXmlCGWriter::WriteBytes(bytes, *(originalArc.get()));
//
//    Roundtrip(Utf8String("<TransitionSpiral xmlns=\"http://www.bentley.com/schemas/Bentley.Geometry.Common.1.0\"><SpiralType>Clothoid</SpiralType><Placement><origin>0,0,0</origin><vectorZ>0,0,1</vectorZ><vectorX>1,0,0</vectorX></Placement><StartBearing>0</StartBearing><StartRadius>0</StartRadius><EndBearing>114.59155902616465</EndBearing><EndRadius>1</EndRadius><ActiveStartFraction>0</ActiveStartFraction><ActiveEndFraction>1</ActiveEndFraction><Geometry /></TransitionSpiral>"));
//    Roundtrip(Utf8String("<BsplineCurve xmlns=\"http://www.bentley.com/schemas/Bentley.Geometry.Common.1.0\"><Order>2</Order><Closed>false</Closed><ListOfControlPoint><xyz>1,0,0</xyz><xyz>0.86602540378443871,0.49999999999999994,0</xyz></ListOfControlPoint></BsplineCurve>"));
//    Roundtrip(Utf8String("<CircularCylinder xmlns=\"http://www.bentley.com/schemas/Bentley.Geometry.Common.1.0\"><placement><origin>0,0,0</origin><vectorZ>0,0,1</vectorZ><vectorX>1,0,0</vectorX></placement><height>1</height><radius>1</radius><capped>true</capped></CircularCylinder>"));
//    Roundtrip(Utf8String("<BsplineSurface xmlns=\"http://www.bentley.com/schemas/Bentley.Geometry.Common.1.0\"><OrderU>3</OrderU><ClosedU>false</ClosedU><NumUControlPoint>3</NumUControlPoint><OrderV>2</OrderV><ClosedV>false</ClosedV><NumVControlPoint>2</NumVControlPoint><ListOfControlPoint><xyz>0,0,0</xyz><xyz>1,0,0</xyz><xyz>2,0,0</xyz><xyz>0,1,0</xyz><xyz>1,1,0</xyz><xyz>3,1,0</xyz></ListOfControlPoint><ListOfWeight><Weight>1</Weight><Weight>1</Weight><Weight>1</Weight><Weight>1</Weight><Weight>1</Weight><Weight>1</Weight></ListOfWeight><ListOfKnotU><KnotU>0</KnotU><KnotU>0</KnotU><KnotU>0</KnotU><KnotU>1.5</KnotU><KnotU>1.5</KnotU><KnotU>1.5</KnotU></ListOfKnotU><ListOfKnotV><KnotV>1</KnotV><KnotV>1</KnotV><KnotV>2</KnotV><KnotV>2</KnotV></ListOfKnotV></BsplineSurface>"));
//    Roundtrip(Utf8String("<Polygon xmlns=\"http://www.bentley.com/schemas/Bentley.Geometry.Common.1.0\"><ListOfPoint><xyz>0.90096886790241915,0.43388373911755812,0</xyz><xyz>0.22252093395631445,0.97492791218182362,0</xyz><xyz>-0.62348980185873382,0.78183148246802958,0</xyz><xyz>-1,1.2246063538223773E-16,0</xyz><xyz>-0.62348980185873371,-0.78183148246802969,0</xyz><xyz>0.22252093395631509,-0.9749279121818234,0</xyz><xyz>0.900968867902419,-0.43388373911755834,0</xyz><xyz>0.90096886790241915,0.43388373911755812,0</xyz></ListOfPoint></Polygon>"));
//    Roundtrip(Utf8String("<CircularArc xmlns=\"http://www.bentley.com/schemas/Bentley.Geometry.Common.1.0\"><placement><origin>0,0,0</origin><vectorZ>0,0,1</vectorZ><vectorX>1,0,0</vectorX></placement><radius>1.125</radius><startAngle>0</startAngle><sweepAngle>-165</sweepAngle></CircularArc>"));
//    Roundtrip(Utf8String("<SurfacePatch xmlns=\"http://www.bentley.com/schemas/Bentley.Geometry.Common.1.0\"><ExteriorLoop><CurveChain><ListOfCurve><LineString><ListOfPoint><xyz>-5,0,0</xyz><xyz>5,0,0</xyz><xyz>5,3,0</xyz><xyz>-5,3,0</xyz><xyz>-5,0,0</xyz></ListOfPoint></LineString></ListOfCurve></CurveChain></ExteriorLoop></SurfacePatch>"));
//    Roundtrip(Utf8String("<EllipticDisk xmlns=\"http://www.bentley.com/schemas/Bentley.Geometry.Common.1.0\"><placement><origin>0,0,0</origin><vectorZ>0,0,1</vectorZ><vectorX>1,0,0</vectorX></placement><radiusA>1</radiusA><radiusB>1.5</radiusB></EllipticDisk>"));
//    Roundtrip(Utf8String("<CircularDisk xmlns=\"http://www.bentley.com/schemas/Bentley.Geometry.Common.1.0\"><placement><origin>0,0,0</origin><vectorZ>0,0,1</vectorZ><vectorX>1,0,0</vectorX></placement><radius>1</radius></CircularDisk>"));
//    Roundtrip(Utf8String("<LineSegment xmlns=\"http://www.bentley.com/schemas/Bentley.Geometry.Common.1.0\"><startPoint>0,0,0</startPoint><endPoint>1,0,0</endPoint></LineSegment>"));
//    }
//
//TEST (CGSerializationTests, DeserializeEllipticDisc)
//    {
//    Utf8String xml("<EllipticDisk xmlns=\"http://www.bentley.com/schemas/Bentley.Geometry.Common.1.0\"><placement><origin>0,0,0</origin><vectorZ>0,0,1</vectorZ><vectorX>1,0,0</vectorX></placement><radiusA>1</radiusA><radiusB>1.5</radiusB></EllipticDisk>");
//    bvector<IGeometryPtr> geoms;
//    BeXmlCGParser::TryParse(xml.c_str(), geoms, 0);
//
//    IGeometryPtr geometry = geoms[0];
//    ICurvePrimitivePtr curvePrimitive = geometry->GetAsICurvePrimitive ();
//    CurveVectorCR curveVector = *curvePrimitive->GetChildCurveVectorCP ();
//    DEllipse3d arc;
//    curveVector[0]->TryGetArc (arc);
//    ASSERT_TRUE(arc.IsFullEllipse ());
//    ASSERT_FALSE(arc.IsCircular ());
//    }

END_BENTLEY_ECOBJECT_NAMESPACE
