/*--------------------------------------------------------------------------------------+
|
|     $Source: tests/NonPublished/CGBinarySerializationTests.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../ECObjectsTestPCH.h"
#include <GeomSerialization/GeomSerializationApi.h>
#include "../TestFixture/TestFixture.h"

using namespace BentleyApi::ECN;

BEGIN_BENTLEY_ECN_TEST_NAMESPACE

struct CGBinarySerializationTests : ECTestFixture
    {
    void Roundtrip(Utf8String xml, bool useStreamReader=false)
        {
        bvector<IGeometryPtr> geoms;
        bmap<OrderedIGeometryPtr, BeExtendedData> extendedData;
        ASSERT_TRUE(BeXmlCGStreamReader::TryParse(xml.c_str(), geoms, extendedData, 0)) << "Failed to deserialize string: " << xml.c_str();
        ASSERT_EQ(1, geoms.size()) << "Expected 1 geometry object returned for string deserialization but got " << geoms.size() << " for " << xml.c_str();

        bvector<Byte> bytes;
        //    BeXmlCGWriter::WriteBytes(bytes, *(originalArc.get()));
        BeXmlCGWriter::WriteBytes(bytes, *geoms[0], &extendedData);

        bvector<IGeometryPtr> geoms2;
        bmap<OrderedIGeometryPtr, BeExtendedData> extendedData2;
        ASSERT_TRUE(BeXmlCGStreamReader::TryParse(bytes.data(), (int) bytes.size(), geoms2, extendedData2, 0)) << "Failed binary deserialization of " << xml.c_str();
        ASSERT_EQ(1, geoms2.size()) << "Expected 1 geometry object returned for binary deserialization but got " << geoms2.size() << " for " << xml.c_str();
        ASSERT_TRUE(geoms[0]->IsSameStructureAndGeometry(*geoms2[0])) << "Did not get same structure for " << xml.c_str();
        }
    };

TEST_F(CGBinarySerializationTests, TestSpecific)
    {
    Roundtrip(Utf8String("<BsplineCurve xmlns=\"http://www.bentley.com/schemas/Bentley.Geometry.Common.1.0\"><Order>2</Order><Closed>false</Closed><ListOfControlPoint><xyz>1,0,0</xyz><xyz>0.86602540378443871,0.49999999999999994,0</xyz></ListOfControlPoint><ListOfKnot><Knot>0</Knot><Knot>0</Knot><Knot>1</Knot><Knot>1</Knot></ListOfKnot></BsplineCurve>"));
    }
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
    Roundtrip(Utf8String("<TorusPipe><placement><origin>0,0,0</origin><vectorZ>0,0,1</vectorZ><vectorX>1,0,0</vectorX></placement><radiusA>0</radiusA><radiusB>0</radiusB><startAngle>0</startAngle><sweepAngle>0</sweepAngle><bSolidFlag>false</bSolidFlag></TorusPipe>"));
    Roundtrip(Utf8String("<SkewedCone><placement><origin>0,0,0</origin><vectorZ>0,0,1</vectorZ><vectorX>1,0,0</vectorX></placement><centerB>0,0,0</centerB><radiusA>0</radiusA><radiusB>0</radiusB><bSolidFlag>false</bSolidFlag></SkewedCone>"));
    Roundtrip(Utf8String("<Block><placement><origin>0,0,0</origin><vectorZ>0,0,1</vectorZ><vectorX>1,0,0</vectorX></placement><cornerA>0,0,0</cornerA><cornerB>2,3,4</cornerB><bSolidFlag>false</bSolidFlag></Block>"));
    Roundtrip(Utf8String("<CircularCone><placement><origin>0,0,0</origin><vectorZ>0,0,1</vectorZ><vectorX>1,0,0</vectorX></placement><height>2</height><radiusA>3</radiusA><radiusB>4</radiusB><bSolidFlag>false</bSolidFlag></CircularCone>"));
    Roundtrip(Utf8String("<EllipticDisk xmlns=\"http://www.bentley.com/schemas/Bentley.Geometry.Common.1.0\"><placement><origin>0,0,0</origin><vectorZ>0,0,1</vectorZ><vectorX>1,0,0</vectorX></placement><radiusA>1</radiusA><radiusB>1.5</radiusB></EllipticDisk>"));
    Roundtrip(Utf8String("<CircularDisk xmlns=\"http://www.bentley.com/schemas/Bentley.Geometry.Common.1.0\"><placement><origin>0,0,0</origin><vectorZ>0,0,1</vectorZ><vectorX>1,0,0</vectorX></placement><radius>1</radius></CircularDisk>"));
    Roundtrip(Utf8String("<LineString><ListOfPoint><xyz>0,0,0</xyz><xyz>0,0,1</xyz><xyz>1,0,0</xyz></ListOfPoint></LineString>"));
    Roundtrip(Utf8String("<IndexedMesh><ListOfCoord><Coord>1,2,3</Coord><Coord>1,2,4</Coord><Coord>5,2,1</Coord></ListOfCoord><ListOfCoordIndex><id>1</id><id>2</id><id>3</id><id>0</id></ListOfCoordIndex><ListOfParam /><ListOfParamIndex /><ListOfNormal></ListOfNormal><ListOfNormalIndex /></IndexedMesh>"), true);
    Roundtrip(Utf8String("<BsplineCurve xmlns=\"http://www.bentley.com/schemas/Bentley.Geometry.Common.1.0\"><Order>2</Order><Closed>false</Closed><ListOfControlPoint><xyz>1,0,0</xyz><xyz>0.86602540378443871,0.49999999999999994,0</xyz></ListOfControlPoint><ListOfKnot><Knot>0</Knot><Knot>0</Knot><Knot>1</Knot><Knot>1</Knot></ListOfKnot></BsplineCurve>"));
    Roundtrip(Utf8String("<BsplineCurve xmlns=\"http://www.bentley.com/schemas/Bentley.Geometry.Common.1.0\"><Order>2</Order><Closed>false</Closed><ListOfControlPoint><xyz>1,0,0</xyz><xyz>0.86602540378443871,0.49999999999999994,0</xyz></ListOfControlPoint></BsplineCurve>"));
    Roundtrip(Utf8String("<CircularCylinder xmlns=\"http://www.bentley.com/schemas/Bentley.Geometry.Common.1.0\"><placement><origin>0,0,0</origin><vectorZ>0,0,1</vectorZ><vectorX>1,0,0</vectorX></placement><height>1</height><radius>1</radius><capped>true</capped></CircularCylinder>"));
    Roundtrip(Utf8String("<BsplineSurface xmlns=\"http://www.bentley.com/schemas/Bentley.Geometry.Common.1.0\"><OrderU>3</OrderU><ClosedU>false</ClosedU><NumUControlPoint>3</NumUControlPoint><OrderV>2</OrderV><ClosedV>false</ClosedV><NumVControlPoint>2</NumVControlPoint><ListOfControlPoint><xyz>0,0,0</xyz><xyz>1,0,0</xyz><xyz>2,0,0</xyz><xyz>0,1,0</xyz><xyz>1,1,0</xyz><xyz>3,1,0</xyz></ListOfControlPoint><ListOfWeight><Weight>1</Weight><Weight>1</Weight><Weight>1</Weight><Weight>1</Weight><Weight>1</Weight><Weight>1</Weight></ListOfWeight><ListOfKnotU><KnotU>0</KnotU><KnotU>0</KnotU><KnotU>0</KnotU><KnotU>1.5</KnotU><KnotU>1.5</KnotU><KnotU>1.5</KnotU></ListOfKnotU><ListOfKnotV><KnotV>1</KnotV><KnotV>1</KnotV><KnotV>2</KnotV><KnotV>2</KnotV></ListOfKnotV></BsplineSurface>"));
    Roundtrip(Utf8String("<Polygon xmlns=\"http://www.bentley.com/schemas/Bentley.Geometry.Common.1.0\"><ListOfPoint><xyz>0.90096886790241915,0.43388373911755812,0</xyz><xyz>0.22252093395631445,0.97492791218182362,0</xyz><xyz>-0.62348980185873382,0.78183148246802958,0</xyz><xyz>-1,1.2246063538223773E-16,0</xyz><xyz>-0.62348980185873371,-0.78183148246802969,0</xyz><xyz>0.22252093395631509,-0.9749279121818234,0</xyz><xyz>0.900968867902419,-0.43388373911755834,0</xyz><xyz>0.90096886790241915,0.43388373911755812,0</xyz></ListOfPoint></Polygon>"));
    Roundtrip(Utf8String("<SurfacePatch xmlns=\"http://www.bentley.com/schemas/Bentley.Geometry.Common.1.0\"><ExteriorLoop><CurveChain><ListOfCurve><LineString><ListOfPoint><xyz>-5,0,0</xyz><xyz>5,0,0</xyz><xyz>5,3,0</xyz><xyz>-5,3,0</xyz><xyz>-5,0,0</xyz></ListOfPoint></LineString></ListOfCurve></CurveChain></ExteriorLoop></SurfacePatch>"));
    }

TEST_F (CGBinarySerializationTests, DeserializeEllipticDisc)
    {
    Utf8String xml("<EllipticDisk xmlns=\"http://www.bentley.com/schemas/Bentley.Geometry.Common.1.0\"><placement><origin>0,0,0</origin><vectorZ>0,0,1</vectorZ><vectorX>1,0,0</vectorX></placement><radiusA>1</radiusA><radiusB>1.5</radiusB></EllipticDisk>");
    bvector<IGeometryPtr> geoms;
    bmap<OrderedIGeometryPtr, BeExtendedData> extendedData;
    BeXmlCGStreamReader::TryParse(xml.c_str(), geoms, extendedData, 0);

    IGeometryPtr geometry = geoms[0];
    ASSERT_TRUE(geoms.size () == 1);
    CurveVectorPtr curveVector = geometry->GetAsCurveVector ();
    ASSERT_TRUE(curveVector.IsValid ());
    DEllipse3d arc;
    curveVector->at(0)->TryGetArc (arc);
    ASSERT_TRUE(arc.IsFullEllipse ());
    ASSERT_FALSE(arc.IsCircular ());
    }

TEST_F (CGBinarySerializationTests, DeserializeExtendedData)
    {
    Utf8String xml("<ExtendedObject xmlns=\"http://www.bentley.com/schemas/Bentley.ECSerializable.1.0\"> \
                        <LineSegment xmlns=\"http://www.bentley.com/schemas/Bentley.Geometry.Common.1.0\"> \
                            <startPoint>0,0,0</startPoint> \
                            <endPoint>1,0,0</endPoint> \
                        </LineSegment> \
                        <ExtendedData> \
                            <TransientLookupCollection> \
                                <Entry key=\"length\" typeCode=\"Double\">3.14</Entry> \
                                <Entry key=\"name\" typeCode=\"String\">test string</Entry> \
                            </TransientLookupCollection> \
                        </ExtendedData> \
                    ExtendedObject>");

    Byte bytes[] = {0x40, 0x0E, 0x45, 0x78, 0x74, 0x65, 0x6E, 0x64, 0x65, 0x64, 0x4F, 0x62, 0x6A, 0x65, 0x63, 0x74, 0x08, 0x39, 0x68, 0x74,
                     0x74, 0x70, 0x3A, 0x2F, 0x2F, 0x77, 0x77, 0x77, 0x2E, 0x62, 0x65, 0x6E, 0x74, 0x6C, 0x65, 0x79, 0x2E, 0x63, 0x6F, 0x6D,
                     0x2F, 0x73, 0x63, 0x68, 0x65, 0x6D, 0x61, 0x73, 0x2F, 0x42, 0x65, 0x6E, 0x74, 0x6C, 0x65, 0x79, 0x2E, 0x45, 0x43, 0x53,
                     0x65, 0x72, 0x69, 0x61, 0x6C, 0x69, 0x7A, 0x61, 0x62, 0x6C, 0x65, 0x2E, 0x31, 0x2E, 0x30, 0x40, 0x0B, 0x4C, 0x69, 0x6E,
                     0x65, 0x53, 0x65, 0x67, 0x6D, 0x65, 0x6E, 0x74, 0x08, 0x3A, 0x68, 0x74, 0x74, 0x70, 0x3A, 0x2F, 0x2F, 0x77, 0x77, 0x77,
                     0x2E, 0x62, 0x65, 0x6E, 0x74, 0x6C, 0x65, 0x79, 0x2E, 0x63, 0x6F, 0x6D, 0x2F, 0x73, 0x63, 0x68, 0x65, 0x6D, 0x61, 0x73,
                     0x2F, 0x42, 0x65, 0x6E, 0x74, 0x6C, 0x65, 0x79, 0x2E, 0x47, 0x65, 0x6F, 0x6D, 0x65, 0x74, 0x72, 0x79, 0x2E, 0x43, 0x6F,
                     0x6D, 0x6D, 0x6F, 0x6E, 0x2E, 0x31, 0x2E, 0x30, 0x40, 0x0A, 0x73, 0x74, 0x61, 0x72, 0x74, 0x50, 0x6F, 0x69, 0x6E, 0x74,
                     0x99, 0x05, 0x30, 0x2C, 0x30, 0x2C, 0x30, 0x40, 0x08, 0x65, 0x6E, 0x64, 0x50, 0x6F, 0x69, 0x6E, 0x74, 0x99, 0x05, 0x31,
                     0x2C, 0x30, 0x2C, 0x30, 0x01, 0x40, 0x0C, 0x45, 0x78, 0x74, 0x65, 0x6E, 0x64, 0x65, 0x64, 0x44, 0x61, 0x74, 0x61, 0x40,
                     0x19, 0x54, 0x72, 0x61, 0x6E, 0x73, 0x69, 0x65, 0x6E, 0x74, 0x4C, 0x6F, 0x6F, 0x6B, 0x75, 0x70, 0x43, 0x6F, 0x6C, 0x6C,
                     0x65, 0x63, 0x74, 0x69, 0x6F, 0x6E, 0x40, 0x05, 0x45, 0x6E, 0x74, 0x72, 0x79, 0x04, 0x03, 0x6B, 0x65, 0x79, 0x98, 0x06,
                     0x6C, 0x65, 0x6E, 0x67, 0x74, 0x68, 0x04, 0x08, 0x74, 0x79, 0x70, 0x65, 0x43, 0x6F, 0x64, 0x65, 0x98, 0x06, 0x44, 0x6F,
                     0x75, 0x62, 0x6C, 0x65, 0x93, 0x1F, 0x85, 0xEB, 0x51, 0xB8, 0x1E, 0x09, 0x40, 0x40, 0x05, 0x45, 0x6E, 0x74, 0x72, 0x79,
                     0x04, 0x03, 0x6B, 0x65, 0x79, 0x98, 0x04, 0x6E, 0x61, 0x6D, 0x65, 0x04, 0x08, 0x74, 0x79, 0x70, 0x65, 0x43, 0x6F, 0x64,
                     0x65, 0x98, 0x06, 0x53, 0x74, 0x72, 0x69, 0x6E, 0x67, 0x99, 0x0B, 0x74, 0x65, 0x73, 0x74, 0x20, 0x73, 0x74, 0x72, 0x69,
                     0x6E, 0x67, 0x01, 0x01, 0x01};

    bvector<IGeometryPtr> geoms;
    bmap<OrderedIGeometryPtr, BeExtendedData> extendedData;
    ASSERT_TRUE(BeXmlCGStreamReader::TryParse(xml.c_str(), geoms, extendedData, 0)) << "Failed to deserialize string: " << xml.c_str();
    ASSERT_EQ(1, geoms.size()) << "Expected 1 geometry object returned for string deserialization but got " << geoms.size() << " for " << xml.c_str();
    bmap<OrderedIGeometryPtr, BeExtendedData>::const_iterator extendedDataIterator;

    for (extendedDataIterator = extendedData.begin(); extendedDataIterator != extendedData.end(); extendedDataIterator++)
        {
        OrderedIGeometryPtr geometryObj = extendedDataIterator->first;
        printf("Geometry type: %d\n", geometryObj.m_geometry->GetGeometryType());
        BeExtendedData entries = extendedDataIterator->second;
        for (auto const& entry: entries)
            {
            printf("%s: %s of type (%s)\n", entry.Key.c_str(), entry.Value.c_str(), entry.Type.c_str());
            }
        }

    bvector<IGeometryPtr> geoms2;
    bmap<OrderedIGeometryPtr, BeExtendedData> extendedData2;
    ASSERT_TRUE(BeXmlCGStreamReader::TryParse(bytes, 325, geoms2, extendedData2, 0)) << "Failed binary deserialization of extended data: " << xml.c_str();
    ASSERT_EQ(1, geoms2.size()) << "Expected 1 geometry object returned for binary deserialization but got " << geoms2.size() << " for " << xml.c_str();

    Utf8String beCgXml;
    BeXmlCGWriter::Write(beCgXml, *geoms[0], &extendedData);
    printf("%s\n", beCgXml.c_str());

    bvector<Byte> outputBytes;
    BeXmlCGWriter::WriteBytes(outputBytes, *geoms[0], &extendedData);

    }

TEST_F (CGBinarySerializationTests, DeserializeNestedExtendedData)
    {
    Utf8String xml("<CurveChain xmlns=\"http://www.bentley.com/schemas/Bentley.Geometry.Common.1.0\">\
                        <ListOfCurve>\
                            <ExtendedObject xmlns=\"http://www.bentley.com/schemas/Bentley.ECSerializable.1.0\">\
                                <LineSegment>\
                                    <startPoint>0,0,0</startPoint>\
                                    <endPoint>1,0,0</endPoint>\
                                </LineSegment>\
                                <ExtendedData>\
                                    <TransientLookupCollection>\
                                        <Entry key=\"length\" typeCode=\"Double\">3.14</Entry>\
                                        <Entry key=\"name\" typeCode=\"String\">test string</Entry>\
                                    </TransientLookupCollection>\
                                </ExtendedData>\
                            </ExtendedObject>\
                            <CircularArc>\
                                <placement>\
                                    <origin>1,2,0</origin>\
                                    <vectorZ>0,0,1</vectorZ>\
                                    <vectorX>1,0,0</vectorX>\
                                </placement>\
                                <radius>1</radius>\
                                <startAngle>-90</startAngle>\
                                <sweepAngle>180</sweepAngle>\
                            </CircularArc>\
                        </ListOfCurve>\
                </CurveChain>");
    bvector<IGeometryPtr> geoms;
    bmap<OrderedIGeometryPtr, BeExtendedData> extendedData;
    ASSERT_TRUE(BeXmlCGStreamReader::TryParse(xml.c_str(), geoms, extendedData, 0)) << "Failed to deserialize string: " << xml.c_str();
    ASSERT_EQ(1, geoms.size()) << "Expected 1 geometry object returned for string deserialization but got " << geoms.size() << " for " << xml.c_str();

    Utf8String beCgXml;
    BeXmlCGWriter::Write(beCgXml, *geoms[0], &extendedData);
    printf("%s\n", beCgXml.c_str());

    }

END_BENTLEY_ECN_TEST_NAMESPACE
