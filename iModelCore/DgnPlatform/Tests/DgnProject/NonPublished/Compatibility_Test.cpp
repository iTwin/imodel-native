/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/NonPublished/Compatibility_Test.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "DgnHandlersTests.h"
#include "CompatibilityHeaders.h"
#include <Bentley/BeTest.h>
#include <BeXml/BeXml.h>
#include <wchar.h>

USING_NAMESPACE_BENTLEY

#ifdef WIP_NOT_PORTABLE

struct TypeNamePair
{
    WString m_name;
    int32_t m_size;
    int32_t m_alignment;
    TypeNamePair() :m_size(-1), m_alignment(-1), m_name(L"UnIntializedName"){}
    TypeNamePair(int32_t size, WString name) :m_size(size), m_name(name){}
};
struct CompatibilityFixture : public ::testing::TestWithParam<struct TypeNamePair>
    {
     public:
         static BeXmlDomPtr m_dom;
         static xmlXPathContextPtr m_xmlXPathContext;
         static void SetUpTestCase()
            {
                printf("*** SetUpTestCase ***\n");
                printf("Reading config file from : %ls \n", DgnDbTestDgnManager::GetSeedFilePath(L"CompatibilityTestDataTypes.xml").c_str());
                BeXmlStatus status;
                m_dom = BeXmlDom::CreateAndReadFromFile(status, DgnDbTestDgnManager::GetSeedFilePath(L"CompatibilityTestDataTypes.xml").c_str());
                if (m_dom.IsValid())
                {
                    BeXmlNodeP          root = m_dom->GetRootElement();
                    if (NULL != root)
                        m_xmlXPathContext = m_dom->AcquireXPathContext(root);
                }
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
                //printf("node name %s \n", iterableNodeSet.front()->GetName());
                int32_t size = -1;
                iterableNodeSet.front()->GetAttributeInt32Value(size, "alignment");
                return size;
            }
            return -1;
        }

    };
BeXmlDomPtr CompatibilityFixture::m_dom;
xmlXPathContextPtr CompatibilityFixture::m_xmlXPathContext;

//---------------------------------------------------------------------------------------
// @bsimethod                                        Umar.Hayat                05/15
//---------------------------------------------------------------------------------------
TEST_P(CompatibilityFixture, SizeCheck)
    {    
    TypeNamePair param = GetParam();
    ASSERT_TRUE(m_dom.IsValid());
    //printf("size = %d\n", GetSizeOf(param.m_name));
    ASSERT_EQ(param.m_size, GetSizeOf(param.m_name)) << param.m_name.c_str() << " size is not matching";
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                        Umar.Hayat                05/15
//---------------------------------------------------------------------------------------
/*TEST_P(CompatibilityFixture, AlignmentCheck)
    {
    TypeNamePair param = GetParam();
    ASSERT_TRUE(m_dom.IsValid());
    //printf("size = %d\n", GetSizeOf(param.m_name));
    ASSERT_EQ(param.m_alignment, GetAlignOf(param.m_name)) << param.m_name.c_str() << " alignment is not matching";
    }
*/
INSTANTIATE_TEST_CASE_P(CompatibilityTests, CompatibilityFixture, ::testing::Values(
    TypeNamePair(sizeof(WString), L"WString"),
    TypeNamePair(sizeof(BeFileName), L"BeFileName"),
    TypeNamePair(sizeof(BeFile), L"BeFile"),
    TypeNamePair(sizeof(BeNumerical), L"BeNumerical"),
    TypeNamePair(sizeof(bvector<WString>), L"bvector"),
    TypeNamePair(sizeof(bset<WString, WString>), L"bset"),
    TypeNamePair(sizeof(Iota), L"Iota"),
    TypeNamePair(sizeof(bmap<WString, WString>), L"bmap"),
    TypeNamePair(sizeof(BeSharedMutex), L"BeSharedMutex"), 
    TypeNamePair(sizeof(BeVersion), L"BeVersion"),

    TypeNamePair(sizeof(DPoint2d), L"DPoint2d"),
    TypeNamePair(sizeof(DPoint3d), L"DPoint3d"),
    TypeNamePair(sizeof(DPoint4d), L"DPoint4d"),
    TypeNamePair(sizeof(DRange1d), L"DRange1d"),
    TypeNamePair(sizeof(DRange2d), L"DRange2d"),
    TypeNamePair(sizeof(DPlane3d), L"DPlane3d"),
    TypeNamePair(sizeof(DMatrix4d), L"DMatrix4d"),
    //TypeNamePair(sizeof(DMatrix3d), L"DMatrix3d "),
    TypeNamePair(sizeof(DMap4d), L"DMap4d")
));

#endif // WIP_NOT_PORTABLE