/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "PerformanceTestFixture.h"
#include "DataReader.h"

#include <Bentley/BeTimeUtilities.h>
#include <GeomSerialization/GeomSerializationApi.h>

BEGIN_GEOMLIBS_TESTS_NAMESPACE

struct PerformanceSerializationTests   : PerformanceTestFixture
    {
    private:
        void TimeXmlText(IGeometryCR geomObj, Utf8CP testDescription, bmap<Utf8String, double>& results, Utf8String testcaseName, Utf8String testName)
            {
            Utf8PrintfString serializerName("%s - Serializing using XmlText", testDescription);
            StopWatch serializerTimer(serializerName.c_str(), false);
            bmap<OrderedIGeometryPtr, BeExtendedData> extendedData;
            Utf8String nativeXml;

            serializerTimer.Start();
            BeXmlCGWriter::Write (nativeXml, geomObj , &extendedData);
            serializerTimer.Stop();

            GEOMPERFORMANCELOG.infov("%s - %lf", serializerName.c_str(), serializerTimer.GetElapsedSeconds());
            results[serializerName] = serializerTimer.GetElapsedSeconds();
            LOGTODB(testcaseName.c_str(), testName.c_str(), serializerTimer.GetElapsedSeconds(), -1, serializerName.c_str());

            Utf8PrintfString deserializerName("%s - Deserializing using XmlText", testDescription);
            StopWatch deserializerTimer(serializerName.c_str(), false);
            bvector<IGeometryPtr> geoms;

            deserializerTimer.Start();
            bool success = BeXmlCGStreamReader::TryParse(nativeXml.c_str(), geoms, extendedData, 0);
            deserializerTimer.Stop();

            ASSERT_TRUE(success) << "Failed to deserialize string: " << nativeXml;
            ASSERT_EQ(1, geoms.size()) << "Expected 1 geometry object returned for string deserialization but got " << geoms.size() << " for " << nativeXml;
            GEOMPERFORMANCELOG.infov("%s - %lf", deserializerName.c_str(), deserializerTimer.GetElapsedSeconds());
            results[deserializerName] = deserializerTimer.GetElapsedSeconds();
            LOGTODB(testcaseName.c_str(), testName.c_str(), deserializerTimer.GetElapsedSeconds(), -1, deserializerName.c_str());
            }

        void TimeXmlBinary(IGeometryCR geomObj, Utf8CP testDescription, bmap<Utf8String, double>& results, Utf8String testcaseName, Utf8String testName)
            {
            Utf8PrintfString serializerName("%s - Serializing using XmlBinary", testDescription);
            StopWatch serializerTimer(serializerName.c_str(), false);
            bmap<OrderedIGeometryPtr, BeExtendedData> extendedData;

            bvector<Byte> bytes;
            serializerTimer.Start();
            BeXmlCGWriter::WriteBytes(bytes, geomObj, &extendedData);
            serializerTimer.Stop();

            GEOMPERFORMANCELOG.infov("%s - %lf", serializerName.c_str(), serializerTimer.GetElapsedSeconds());
            results[serializerName] = serializerTimer.GetElapsedSeconds();
            LOGTODB(testcaseName.c_str(), testName.c_str(), serializerTimer.GetElapsedSeconds(), -1, serializerName.c_str());

            Utf8PrintfString deserializerName("%s - Deserializing using XmlBinary", testDescription);
            StopWatch deserializerTimer(serializerName.c_str(), false);
            bvector<IGeometryPtr> geoms;

            deserializerTimer.Start();
            /*bool success = */BeXmlCGStreamReader::TryParse(bytes.data(), (int) bytes.size(), geoms, extendedData, 0);
            deserializerTimer.Stop();
            GEOMPERFORMANCELOG.infov("%s - %lf", deserializerName.c_str(), deserializerTimer.GetElapsedSeconds());
            results[deserializerName] = deserializerTimer.GetElapsedSeconds();
            LOGTODB(testcaseName.c_str(), testName.c_str(), deserializerTimer.GetElapsedSeconds(), -1, deserializerName.c_str());
            }

        void TimeFlatbuffer(IGeometryCR geomObj, Utf8CP testDescription, bmap<Utf8String, double>& results, Utf8String testcaseName, Utf8String testName)
            {
            Utf8PrintfString serializerName("%s - Serializing using Flatbuffers", testDescription);
            StopWatch serializerTimer(serializerName.c_str(), false);
            bmap<OrderedIGeometryPtr, BeExtendedData> extendedData;

            bvector<Byte> geometryBlob;
            serializerTimer.Start();
            BentleyGeometryFlatBuffer::GeometryToBytes (geomObj, geometryBlob);
            serializerTimer.Stop();

            GEOMPERFORMANCELOG.infov("%s - %lf", serializerName.c_str(), serializerTimer.GetElapsedSeconds());
            results[serializerName] = serializerTimer.GetElapsedSeconds();
            LOGTODB(testcaseName.c_str(), testName.c_str(), serializerTimer.GetElapsedSeconds(), -1, serializerName.c_str());

            Utf8PrintfString deserializerName("%s - Deserializing using Flatbuffers", testDescription);
            StopWatch deserializerTimer(serializerName.c_str(), false);

            deserializerTimer.Start();
            IGeometryPtr geom = BentleyGeometryFlatBuffer::BytesToGeometry (geometryBlob);
            deserializerTimer.Stop();
            ASSERT_TRUE(geom.IsValid());
            GEOMPERFORMANCELOG.infov("%s - %lf", deserializerName.c_str(), deserializerTimer.GetElapsedSeconds());
            results[deserializerName] = deserializerTimer.GetElapsedSeconds();
            LOGTODB(testcaseName.c_str(), testName.c_str(), deserializerTimer.GetElapsedSeconds(), -1, deserializerName.c_str());
            }

    protected:
        void Profile(Utf8CP xmlString, Utf8CP testDescription, Utf8String testcaseName, Utf8String testName)
            {
            // First, deserialize the xmlString into an IGeometryPtr
            bvector<IGeometryPtr> geoms;
            bmap<OrderedIGeometryPtr, BeExtendedData> extendedData;
            ASSERT_TRUE(BeXmlCGStreamReader::TryParse(xmlString, geoms, extendedData, 0)) << "Failed to deserialize string: " << xmlString;
            ASSERT_EQ(1, geoms.size()) << "Expected 1 geometry object returned for string deserialization but got " << geoms.size() << " for " << xmlString;

            bmap<Utf8String, double> results;

            // Now, serialize and de-serialize the IGeometryPtr using xmlText, xmlBinary, and flatbuffers
            TimeXmlText(*geoms[0], testDescription, results, testcaseName, testName);
            TimeXmlBinary(*geoms[0], testDescription, results, testcaseName, testName);
            TimeFlatbuffer(*geoms[0], testDescription, results, testcaseName, testName);
            LogResultsToFile(results);
            }

        void Profile(DataReader& dataReader, Utf8String testcaseName, Utf8String testName)
            {
            Utf8String description;
            Utf8String xml;
            int id = -1;
            while (dataReader.GetNextTest(description, xml, id))
                Profile(xml.c_str(), description.c_str(), testcaseName, testName);
            }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PerformanceSerializationTests, ProfileCoordinate)
    {
    Profile("<Coordinate xmlns=\"http://www.bentley.com/schemas/Bentley.Geometry.Common.1.0\"><xyz>0,0,0</xyz></Coordinate>", "Coordinate - All zeros", TEST_DETAILS);
    Profile("<Coordinate xmlns=\"http://www.bentley.com/schemas/Bentley.Geometry.Common.1.0\"><xyz>1,1,1</xyz></Coordinate>", "Coordinate - All ones", TEST_DETAILS);
    Profile("<Coordinate xmlns=\"http://www.bentley.com/schemas/Bentley.Geometry.Common.1.0\"><xyz>2,3,4</xyz></Coordinate>", "Coordinate - Small ints", TEST_DETAILS);
    Profile("<Coordinate xmlns=\"http://www.bentley.com/schemas/Bentley.Geometry.Common.1.0\"><xyz>127, 127, 127</xyz></Coordinate>", "Coordinate - int8", TEST_DETAILS);
    Profile("<Coordinate xmlns=\"http://www.bentley.com/schemas/Bentley.Geometry.Common.1.0\"><xyz>-127,127,127</xyz></Coordinate>", "Coordinate - -int8", TEST_DETAILS);
    Profile("<Coordinate xmlns=\"http://www.bentley.com/schemas/Bentley.Geometry.Common.1.0\"><xyz>32767,32767,32767</xyz></Coordinate>", "Coordinate - int16", TEST_DETAILS);
    Profile("<Coordinate xmlns=\"http://www.bentley.com/schemas/Bentley.Geometry.Common.1.0\"><xyz>2147483647,2147483647,2147483647</xyz></Coordinate>", "Coordinate - int32", TEST_DETAILS);
    Profile("<Coordinate xmlns=\"http://www.bentley.com/schemas/Bentley.Geometry.Common.1.0\"><xyz>1.2624671484563434,1.2624671484563432,0.25</xyz></Coordinate>", "Coordinate - double", TEST_DETAILS);
    Profile("<Coordinate xmlns=\"http://www.bentley.com/schemas/Bentley.Geometry.Common.1.0\"><xyz>1.5741067580881295E-16,2.5707963267948966,0.5</xyz></Coordinate>", "Coordinate - tiny double", TEST_DETAILS);
    Profile("<Coordinate xmlns=\"http://www.bentley.com/schemas/Bentley.Geometry.Common.1.0\"><xyz>-2.3731878829959347,2.3731878829959347,0.75</xyz></Coordinate>", "Coordinate - negative double", TEST_DETAILS);

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PerformanceSerializationTests, ProfileFromDb)
    {
    DbDataReader dataReader(L"geomTestData.ecdb");
    Profile(dataReader, TEST_DETAILS);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PerformanceSerializationTests, ProfileFromXmlFile)
    {
    XmlFileDataReader dataReader(L"xmlSamples\\ICoordinate.xml");
    Profile(dataReader, TEST_DETAILS);
    }

END_GEOMLIBS_TESTS_NAMESPACE