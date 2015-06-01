/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/NonPublished/Compatibility_Test.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "DgnHandlersTests.h"
#include <Bentley/BeTest.h>
#include <BeXml/BeXml.h>
#include <wchar.h>

//#ifdef WIP_NOT_PORTABLE

struct TypeInfo
{
    WString m_name;
    int32_t m_size;
    int32_t m_alignment;
    TypeInfo() :m_size(-1), m_alignment(-1), m_name(L"UnIntializedName"){}
    TypeInfo(int32_t size, WString name) :m_size(size), m_name(name){}
    TypeInfo(int32_t size, int32_t align, WString name) :m_size(size), m_alignment(align), m_name(name){}
};

#include "BentleyHeaders.h"
#include "GeomHeaders.h"

USING_NAMESPACE_BENTLEY


struct Compatibility_Test : public ::testing::Test
    {
     public:
         static BeXmlDomPtr m_dom;
         static xmlXPathContextPtr m_xmlXPathContext;
         static void GetStructList(bvector<TypeInfo>& list);
         static bool IsConfigLoadError;
         static WString GetCompatibilityConfigFile();
         static void SetUpTestCase()
            {
                printf("*** SetUpTestCase ***\n");
                WString configFilePath = GetCompatibilityConfigFile();
                printf("Reading config file from : %ls \n", DgnDbTestDgnManager::GetSeedFilePath(configFilePath.c_str()).c_str());
                BeXmlStatus status;
                m_dom = BeXmlDom::CreateAndReadFromFile(status, DgnDbTestDgnManager::GetSeedFilePath(configFilePath.c_str()).c_str());
                if (BEXML_Success == status && m_dom.IsValid())
                {
                    BeXmlNodeP          root = m_dom->GetRootElement();
                    if (NULL != root)
                    {
                        m_xmlXPathContext = m_dom->AcquireXPathContext(root);
                        IsConfigLoadError = false;
                    }
                    else
                        printf("ERROR: Can't find root node., ERROR CODE (BeXmlStatus) = %d \n",(int)status);
                }
                else
                    printf("ERROR: Unable to read/parse xml document, ERROR CODE (BeXmlStatus) = %d\n", (int)status);
            }
        //---------------------------------------------------------------------------------------
        // @bsimethod                                        Umar.Hayat                02/13
        //---------------------------------------------------------------------------------------
        int32_t GetSizeOf(WString typeName)
        {
            BeXmlDom::IterableNodeSet iterableNodeSet;
            WString expressionW = L"//struct[@name='%ls']";
            expressionW.Sprintf(expressionW.c_str(),typeName.c_str());
            char *expression = new char[expressionW.size() + 1];
            expression = expressionW.ConvertToLocaleChars(expression);
            m_dom->SelectNodes(iterableNodeSet, expression, m_xmlXPathContext);
            //printf("size = %d\n", iterableNodeSet.size());
            if (iterableNodeSet.size()) 
            {
                //printf("node name %ls \n", (*(iterableNodeSet.begin()++))->GetName());
                //printf("node name %s \n", iterableNodeSet.front()->GetName());
                int32_t size = -1;
                iterableNodeSet.front()->GetAttributeInt32Value(size, "size");
                return size;
            }
            return -1;
        }
        //---------------------------------------------------------------------------------------
        // @bsimethod                                        Umar.Hayat                02/13
        //---------------------------------------------------------------------------------------
        int32_t GetAlignOf(WString typeName) 
        {
            BeXmlDom::IterableNodeSet iterableNodeSet;
            WString expressionW = L"//struct[@name='%ls']";
            expressionW.Sprintf(expressionW.c_str(), typeName.c_str());
            char *expression = new char[expressionW.size() + 1];
            expression = expressionW.ConvertToLocaleChars(expression);
            m_dom->SelectNodes(iterableNodeSet, expression, m_xmlXPathContext);
            //printf("size = %d\n", iterableNodeSet.size());
            if (iterableNodeSet.size())
            {
                //printf("node name %ls \n", (*(iterableNodeSet.begin()++))->GetName());
                int32_t size = -1;
                iterableNodeSet.front()->GetAttributeInt32Value(size, "alignment");
                return size;
            }
            return -1;
        }

    };
BeXmlDomPtr Compatibility_Test::m_dom;
xmlXPathContextPtr Compatibility_Test::m_xmlXPathContext;
bool Compatibility_Test::IsConfigLoadError = true;
//---------------------------------------------------------------------------------------
// @bsimethod                                        Umar.Hayat                05/15
//---------------------------------------------------------------------------------------
WString Compatibility_Test::GetCompatibilityConfigFile()
    {
#if defined (_X86_)
        return WString(L"CompatibilityTestDataTypes_x86.xml");
#else
        return WString(L"CompatibilityTestDataTypes.xml");
#endif
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                        Umar.Hayat                05/15
//---------------------------------------------------------------------------------------
TEST_F(Compatibility_Test, SizeCheck)
    {
    ASSERT_FALSE(IsConfigLoadError) << "Configuration file not loaded properly";
    bvector<TypeInfo> structList; 
    GetBentleyStructList(structList);
    GetGeomStructList(structList);
    for (TypeInfo param : structList)
        {
        //printf("size = %d\n", GetSizeOf(param.m_name));
        EXPECT_EQ(GetSizeOf(param.m_name), param.m_size ) << param.m_name.c_str() << " size is not matching";
        }
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                        Umar.Hayat                05/15
//---------------------------------------------------------------------------------------
TEST_F(Compatibility_Test, AlignCheck) 
{
    ASSERT_FALSE(IsConfigLoadError) << "Configuration file not loaded properly";
    bvector<TypeInfo> structList;
    GetBentleyStructList(structList);
    GetGeomStructList(structList);
    for (TypeInfo param : structList)
    {
    //printf("size = %d\n", GetSizeOf(param.m_name));
    EXPECT_EQ(GetAlignOf(param.m_name), param.m_alignment) << param.m_name.c_str() << " alignment is not matching";
    }
}

//#endif // WIP_NOT_PORTABLE
