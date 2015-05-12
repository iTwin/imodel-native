/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Published/DgnModel_Test.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnHandlersTests.h"

USING_NAMESPACE_BENTLEY_SQLITE

//----------------------------------------------------------------------------------------
// @bsiclass                                                    Julija.Suboc     07/2013
//----------------------------------------------------------------------------------------
struct DgnModelTests : public testing::Test
    {
     public:
        ScopedDgnHost m_autoDgnHost;
        DgnDbPtr m_dgndb;    
        DgnModelP m_modelP;
        //---------------------------------------------------------------------------------------
        // @bsimethod                                                   Julija Suboc     07/13
        // Prepares test data file
        //---------------------------------------------------------------------------------------
        void SetUp()
            {
            DgnDbTestDgnManager tdm(L"XGraphicsElements.idgndb", __FILE__, Db::OPEN_ReadWrite);
            m_dgndb = tdm.GetDgnProjectP();
           
            }
         //---------------------------------------------------------------------------------------
        // @bsimethod                                                   Julija Suboc     07/13
        //---------------------------------------------------------------------------------------
        void LoadModel(Utf8CP name)
            {
            DgnModels& modelTable =  m_dgndb->Models();
            DgnModelId id = modelTable.QueryModelId(name);
            m_modelP =  modelTable.GetModel(id);
            if (m_modelP)
                m_modelP->FillModel();
            }
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Julija Suboc     08/13
//---------------------------------------------------------------------------------------
TEST_F(DgnModelTests, GetGraphicElements)
    {
    LoadModel("Splines");
    uint32_t graphicElementCount = m_modelP->CountElements();
    ASSERT_NE(graphicElementCount, 0);
    ASSERT_TRUE(graphicElementCount > 0)<<"Please provide model with graphics elements, otherwise this test case makes no sense";
    int count = 0;
    for (auto const& elm : *m_modelP)
        {
        EXPECT_TRUE(&elm.second->GetDgnModel() == m_modelP);
        ++count;
        }
    EXPECT_EQ(graphicElementCount, count);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Julija Suboc     08/13
//---------------------------------------------------------------------------------------
TEST_F(DgnModelTests, GetName)
    {
    LoadModel("Splines");
    Utf8String name(m_modelP->GetModelName());
    EXPECT_TRUE(name.CompareTo("Splines")==0);
    Utf8String newName("New Long model name Longer than expectedNew Long model name Longer"
        " than expectedNew Long model name Longer than expectedNew Long model name Longer than expectedNew Long model");
    DgnModelStatus status;
    DgnModels& modelTable =  m_dgndb->Models();
    modelTable.CreateNewModelFromSeed(&status, newName.c_str(), m_modelP->GetModelId());
    EXPECT_TRUE(status == DGNMODEL_STATUS_Success)<<"Failed to create model";
    DgnModelId id = modelTable.QueryModelId(newName.c_str());
    ASSERT_TRUE(id.IsValid());
    m_modelP =  modelTable.GetModel (id);
    Utf8String nameToVerify(m_modelP->GetModelName());
    EXPECT_TRUE(newName.CompareTo(nameToVerify.c_str())==0);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Julija Suboc     07/13
//---------------------------------------------------------------------------------------
TEST_F(DgnModelTests, EmptyList)
    {
    LoadModel("Splines");
    m_modelP->Empty();
    ASSERT_EQ(0, m_modelP->CountElements())<<"Failed to empty element list in model";
    LoadModel("Splines");
    ASSERT_EQ(0, m_modelP->CountElements())<<"Failed to empty element list in model";
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Julija Suboc     07/13
//---------------------------------------------------------------------------------------
TEST_F(DgnModelTests, GetRange)
    {
    DgnDbTestDgnManager tdm(L"ModelRangeTest.idgndb", __FILE__, Db::OPEN_ReadWrite);
    m_dgndb = tdm.GetDgnProjectP();
    LoadModel("RangeTest");

    AxisAlignedBox3d range = m_modelP->QueryModelRange();
    EXPECT_TRUE(range.IsValid());
    DPoint3d low; low.Init(-1.4011580427821895, 0.11538461538461531, -0.00050000000000000001);
    DPoint3d high; high.Init(-0.59795039550813156, 0.60280769230769227, 0.00050000000000000001);
    AxisAlignedBox3d box(low,high);

    EXPECT_TRUE(box.IsEqual(range,.00000001));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Julija Suboc     07/13
//---------------------------------------------------------------------------------------
TEST_F(DgnModelTests, GetRangeOfEmptyModel)
    {
    DgnDbTestDgnManager tdm(L"3dMetricGeneral.idgndb", __FILE__, Db::OPEN_ReadWrite);
    m_dgndb = tdm.GetDgnProjectP();
    LoadModel("Default");

    AxisAlignedBox3d thirdRange = m_modelP->QueryModelRange();
    EXPECT_FALSE(thirdRange.IsValid());
    }
