
/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include "GeomLibsTests.h"
#include "DataReader.h"
#include <windows.h>

#include <GeomSerialization/GeomSerializationApi.h>
#include <GeomSerialization/GeomLibsFlatBufferApi.h>

#include <G0503/Geom/GeomApi.h>
#include <G0503/GeomSerialization/BeXmlCommonGeometry.h>
#include <G0503/GeomSerialization/GeomLibsSerialization.h>
#include <G0503/GeomSerialization/GeomLibsFlatBufferApi.h>

namespace BentleyV = Bentley;
namespace BentleyGeomV = BentleyV::Geom;
namespace BentleyGeomSerializationV = BentleyV;

namespace BentleyG = BentleyG0503;
namespace BentleyGeomG = BentleyG0503::Geom;
namespace BentleyGeomSerializationG = BentleyApi;

BEGIN_GEOMLIBS_TESTS_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      06/03
+---------------+---------------+---------------+---------------+---------------+------*/
static void*    getDLLInstance ()
    {
    MEMORY_BASIC_INFORMATION    mbi;
    if (VirtualQuery ((void*)&getDLLInstance, &mbi, sizeof mbi))
        return mbi.AllocationBase;

    return 0;
    }

struct SerializationTests : ::testing::Test
    {
    private:
        static WString s_dllPath;
        static WString GetDllPath();

    protected:
        SerializationTests() {};

        void GraphiteToTopaz(Utf8CP xmlString)
            {
            // First, deserialize the xmlString into a Graphite IGeometryPtr
            bvector<BentleyG::IGeometryPtr> geoms;
            bmap<BentleyG::OrderedIGeometryPtr, BentleyG::BeExtendedData> extendedData;
            ASSERT_TRUE(BentleyG::BeXmlCGStreamReader::TryParse(xmlString, geoms, extendedData, 0)) << "Failed to deserialize string: " << xmlString;
            ASSERT_EQ(1, geoms.size()) << "Expected 1 geometry object returned for string deserialization but got " << geoms.size() << " for " << xmlString;

            // Now serialize using Graphite flatbuffer serialization
            bvector<Byte> geometryBlob;
            BentleyG::BentleyGeometryFlatBuffer::GeometryToBytes (*geoms[0], geometryBlob);

            // Now de-serialize using Topaz flatbuffer deserialization
            BentleyV::IGeometryPtr geom = BentleyV::BentleyGeometryFlatBuffer::BytesToGeometry (geometryBlob);
            ASSERT_TRUE(geom.IsValid());
            }
        static WString GetTestDataPath();

    };

WString SerializationTests::s_dllPath = L"";
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                08/2010
+---------------+---------------+---------------+---------------+---------------+------*/
WString SerializationTests::GetDllPath()
    {
    if (s_dllPath.empty())
        {
        HINSTANCE ecobjectsHInstance = (HINSTANCE) getDLLInstance();
        wchar_t strExePath [MAX_PATH];
        if (0 == (GetModuleFileNameW (ecobjectsHInstance, strExePath, MAX_PATH)))
            return L"";
            
        wchar_t executingDirectory[_MAX_DIR];
        wchar_t executingDrive[_MAX_DRIVE];
        _wsplitpath(strExePath, executingDrive, executingDirectory, NULL, NULL);
        wchar_t filepath[MAX_PATH];
        _wmakepath(filepath, executingDrive, executingDirectory, NULL, NULL);
        s_dllPath = filepath;
        }
    return s_dllPath;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Carole.MacDonald 02/10
+---------------+---------------+---------------+---------------+---------------+------*/
WString SerializationTests::GetTestDataPath()
    {
    WString testData(GetDllPath());
    testData.append(L"TestData\\");
    return testData;
    } 

TEST_F(SerializationTests, RoundtripCoordinate)
    {
    GraphiteToTopaz("<Coordinate xmlns=\"http://www.bentley.com/schemas/Bentley.Geometry.Common.1.0\"><xyz>0,0,0</xyz></Coordinate>");
    GraphiteToTopaz("<Coordinate xmlns=\"http://www.bentley.com/schemas/Bentley.Geometry.Common.1.0\"><xyz>1,1,1</xyz></Coordinate>");
    GraphiteToTopaz("<Coordinate xmlns=\"http://www.bentley.com/schemas/Bentley.Geometry.Common.1.0\"><xyz>2,3,4</xyz></Coordinate>");
    GraphiteToTopaz("<Coordinate xmlns=\"http://www.bentley.com/schemas/Bentley.Geometry.Common.1.0\"><xyz>127, 127, 127</xyz></Coordinate>");
    GraphiteToTopaz("<Coordinate xmlns=\"http://www.bentley.com/schemas/Bentley.Geometry.Common.1.0\"><xyz>-127,127,127</xyz></Coordinate>");
    GraphiteToTopaz("<Coordinate xmlns=\"http://www.bentley.com/schemas/Bentley.Geometry.Common.1.0\"><xyz>32767,32767,32767</xyz></Coordinate>");
    GraphiteToTopaz("<Coordinate xmlns=\"http://www.bentley.com/schemas/Bentley.Geometry.Common.1.0\"><xyz>2147483647,2147483647,2147483647</xyz></Coordinate>");
    GraphiteToTopaz("<Coordinate xmlns=\"http://www.bentley.com/schemas/Bentley.Geometry.Common.1.0\"><xyz>1.2624671484563434,1.2624671484563432,0.25</xyz></Coordinate>");
    GraphiteToTopaz("<Coordinate xmlns=\"http://www.bentley.com/schemas/Bentley.Geometry.Common.1.0\"><xyz>1.5741067580881295E-16,2.5707963267948966,0.5</xyz></Coordinate>");
    GraphiteToTopaz("<Coordinate xmlns=\"http://www.bentley.com/schemas/Bentley.Geometry.Common.1.0\"><xyz>-2.3731878829959347,2.3731878829959347,0.75</xyz></Coordinate>");
    }

TEST_F(SerializationTests, RoundtripAllGeometries)
    {
    BeFileListIterator  fileList (GetTestDataPath().c_str(), true);
    BeFileName          filePath;
    while (SUCCESS == fileList.GetNextFileName (filePath))
        {
        XmlFileDataReader dataReader(filePath);
        Utf8String description;
        Utf8String xml;
        int id = -1;
        while (dataReader.GetNextTest(description, xml, id))
            GraphiteToTopaz(xml.c_str());

        }
    }
END_GEOMLIBS_TESTS_NAMESPACE
