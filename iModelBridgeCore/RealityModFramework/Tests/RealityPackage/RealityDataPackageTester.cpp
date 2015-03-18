//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Tests/RealityPackage/RealityDataPackageTester.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------


#include <Bentley/BeTest.h>
#include <RealityPackage/RealityDataPackage.h>

USING_BENTLEY_NAMESPACE_REALITYPACKAGE

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
class PackageTestFixture : public testing::Test 
{
public:
    //----------------------------------------------------------------------------------------
    // @bsimethod                                                   Mathieu.Marchand  3/2015
    //----------------------------------------------------------------------------------------
    BeFileName BuildOutputFilename(WCharCP filename)
        {
        BeFileName outputFilePath;
        BeTest::GetHost().GetOutputRoot (outputFilePath);
        outputFilePath.AppendToPath(filename);

        return outputFilePath;
        }
};

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
TEST_F (PackageTestFixture, CreateAndRead)
    {
    BeFileName outfilename  = BuildOutputFilename(L"CreateAndRead.xml");
    // Create a new package
    RealityDataPackagePtr pPackage = RealityDataPackage::Create(L"MyName");
    ASSERT_TRUE(pPackage.IsValid());
    ASSERT_STREQ(L"MyName", pPackage->GetName().c_str()); 

    pPackage->SetDescription(L"SomeDescription");
    ASSERT_STREQ(L"SomeDescription", pPackage->GetDescription().c_str()); 

    pPackage->SetCopyright(L"(c) 2015 Bentley Systems, Incorporated. All rights reserved.");
    ASSERT_STREQ(L"(c) 2015 Bentley Systems, Incorporated. All rights reserved.", pPackage->GetCopyright().c_str()); 

    pPackage->SetPackageId(L"123asd789avbdlk");
    ASSERT_STREQ(L"123asd789avbdlk", pPackage->GetPackageId().c_str()); 

    DPoint2d polygon[4] = {{1.0252, 53.04}, {441.024452, 4453.0444}, {44, 444.54}, {4541.066252, 5773.0666664}};
    BoundingPolygonPtr pBoundingPolygon = BoundingPolygon::Create(polygon, 4);
    pPackage->SetBoundingPolygon(*pBoundingPolygon);
    ASSERT_EQ(4+1/*closure point*/, pPackage->GetBoundingPolygon().GetPointCount()); 
    ASSERT_DOUBLE_EQ(polygon[2].x, pPackage->GetBoundingPolygon().GetPointCP()[2].x);
    ASSERT_DOUBLE_EQ(polygon[3].y, pPackage->GetBoundingPolygon().GetPointCP()[3].y);

    pPackage->GetImageryGroupR().push_back(RealityDataSource::Create(L"./imagery/satellite.jpeg", L"image/jpeg"));
    pPackage->GetImageryGroupR().push_back(RealityDataSource::Create(L"http://107.20.228.18/ArcGIS/services/Wetlands/MapServer/WMSServer?", L"wms"));

    pPackage->GetTerrainGroupR().push_back(RealityDataSource::Create(L"./terrain/satellite.tif", L"image/tif"));
    pPackage->GetTerrainGroupR().push_back(RealityDataSource::Create(L"./terrain/canada.dtm", L"dtm"));
    pPackage->GetTerrainGroupR().push_back(RealityDataSource::Create(L"./terrain/canada.pod", L"pod"));

    pPackage->GetModelGroupR().push_back(RealityDataSource::Create(L"./model/shape.shp", L"shapefile"));

    pPackage->GetPinnedGroupR().push_back(RealityDataSource::Create(L"./pinned/beach.avi", L"video/avi"));
    
    ASSERT_EQ(RealityPackageStatus::Success, pPackage->Write(outfilename));
    
    //&&MM needwork.
    // Schema validation
//         {
//         BeXmlStatus status = BEXML_Success;
//         BeXmlDomPtr pDom= BeXmlDom::CreateAndReadFromFile (status, outfilename.c_str(), NULL);
// 
//         status = pDom->SchemaValidate(L"C:/Users/Mathieu.Marchand/Desktop/PackageFile/package.xsd");
// 
//         WString errorString;
//         BeXmlDom::GetLastErrorString (errorString);
// 
//         ASSERT_STREQ(L"", errorString.c_str());
//         }
    

    // Validate what we have created.
    WString parseError;
    RealityPackageStatus readStatus = RealityPackageStatus::UnknownError;
    RealityDataPackagePtr pReadPackage = RealityDataPackage::CreateFromFile(readStatus, outfilename, &parseError);
    ASSERT_TRUE(pReadPackage.IsValid());
    ASSERT_EQ(RealityPackageStatus::Success, readStatus);
    ASSERT_STREQ(L"", parseError.c_str()); // we use _STREQ to display the parse error if we have one.


    ASSERT_STREQ(pPackage->GetName().c_str(), pReadPackage->GetName().c_str());
    ASSERT_STREQ(pPackage->GetDescription().c_str(), pReadPackage->GetDescription().c_str());
    ASSERT_STREQ(pPackage->GetCopyright().c_str(), pReadPackage->GetCopyright().c_str());
    ASSERT_STREQ(pPackage->GetPackageId().c_str(), pReadPackage->GetPackageId().c_str());
    ASSERT_TRUE(pPackage->GetCreationDate().Equals(pReadPackage->GetCreationDate()));

    //&&MM iterate over pts and test with an epsilon.
    ASSERT_EQ(pPackage->GetBoundingPolygon().GetPointCount(), pReadPackage->GetBoundingPolygon().GetPointCount());

    //&&MM validate the group content. how? it would be nice to do it field by field because it gives a more
    // explicit error but this is tedious
    ASSERT_EQ(pPackage->GetImageryGroup().size(), pReadPackage->GetImageryGroupR().size());
    ASSERT_EQ(pPackage->GetModelGroup().size(), pReadPackage->GetModelGroup().size());
    ASSERT_EQ(pPackage->GetPinnedGroup().size(), pReadPackage->GetPinnedGroup().size());
    ASSERT_EQ(pPackage->GetTerrainGroup().size(), pReadPackage->GetTerrainGroup().size());
    }

#if 0 //&&MM todo validate xml file.
/*
============================================================================
Name        : xmlvalidation.c
============================================================================
*/
#define LIBXML_SCHEMAS_ENABLED
#include <libxml/xmlschemastypes.h>

int main()
    {
    xmlDocPtr doc;
    xmlSchemaPtr schema = NULL;
    xmlSchemaParserCtxtPtr ctxt;
    char *XMLFileName = "test.xml";
    char *XSDFileName = "test.xsd";

    xmlLineNumbersDefault(1);

    ctxt = xmlSchemaNewParserCtxt(XSDFileName);

    xmlSchemaSetParserErrors(ctxt, (xmlSchemaValidityErrorFunc)fprintf, (xmlSchemaValidityWarningFunc)fprintf, stderr);
    schema = xmlSchemaParse(ctxt);
    xmlSchemaFreeParserCtxt(ctxt);
    //xmlSchemaDump(stdout, schema); //To print schema dump

    doc = xmlReadFile(XMLFileName, NULL, 0);

    if (doc == NULL)
        {
        fprintf(stderr, "Could not parse %s\n", XMLFileName);
        }
    else
        {
        xmlSchemaValidCtxtPtr ctxt;
        int ret;

        ctxt = xmlSchemaNewValidCtxt(schema);
        xmlSchemaSetValidErrors(ctxt, (xmlSchemaValidityErrorFunc)fprintf, (xmlSchemaValidityWarningFunc)fprintf, stderr);
        ret = xmlSchemaValidateDoc(ctxt, doc);
        if (ret == 0)
            {
            printf("%s validates\n", XMLFileName);
            }
        else if (ret > 0)
            {
            printf("%s fails to validate\n", XMLFileName);
            }
        else
            {
            printf("%s validation generated an internal error\n", XMLFileName);
            }
        xmlSchemaFreeValidCtxt(ctxt);
        xmlFreeDoc(doc);
        }

    // free the resource
    if (schema != NULL)
        xmlSchemaFree(schema);

    xmlSchemaCleanupTypes();
    xmlCleanupParser();
    xmlMemoryDump();

    return(0);
    }
#endif