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
TEST_F (PackageTestFixture, CreateNew)
    {
    RealityDataPackagePtr pPackage = RealityDataPackage::Create(L"MyName");
    ASSERT_TRUE(pPackage.IsValid());
    ASSERT_STREQ(L"MyName", pPackage->GetName().c_str()); 


    ASSERT_EQ(RealityDataPackageStatus::Success, pPackage->Write(BuildOutputFilename(L"CreateNew.xml")));
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

xmlSchemaSetParserErrors(ctxt, (xmlSchemaValidityErrorFunc) fprintf, (xmlSchemaValidityWarningFunc) fprintf, stderr);
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
xmlSchemaSetValidErrors(ctxt, (xmlSchemaValidityErrorFunc) fprintf, (xmlSchemaValidityWarningFunc) fprintf, stderr);
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
if(schema != NULL)
xmlSchemaFree(schema);

xmlSchemaCleanupTypes();
xmlCleanupParser();
xmlMemoryDump();

return(0);
}
#endif