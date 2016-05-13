/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/NonPublished/DgnStyles_Test.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnHandlersTests.h"
#include <Bentley/BeTimeUtilities.h>
#include <DgnPlatform/ColorUtil.h>

#if defined (_MSC_VER)
#pragma warning (disable:4702)
#endif

USING_NAMESPACE_BENTLEY_SQLITE

//=======================================================================================
// @bsiclass                                                Algirdas.Mikoliunas   01/13
//=======================================================================================
struct TestStyleProperties
    {
    public:
        DgnStyleId tsId;
        LsComponentType tsType;
        Utf8String tsName;
        Utf8String tsDescription;

        void SetTestStyleProperties(LsComponentType type, Utf8String name, Utf8String description)
            {
            tsType = type;
            tsName = name;
            tsDescription = description;
            };
        void IsEqual (TestStyleProperties testStyle)
            {
            EXPECT_TRUE(testStyle.tsType == tsType ) << "Types don't match";
            EXPECT_STREQ(testStyle.tsName.c_str() , tsName.c_str() ) << "Names don't match";
            //EXPECT_STREQ (tsDescription.c_str(), testStyle.tsDescription.c_str()) << "Descriptions don't match";
            };
    };


/*---------------------------------------------------------------------------------**//**
* Test fixture for testing DgnStyles
* @bsimethod                                    Algirdas.Mikoliunas            03/2013
+---------------+---------------+---------------+---------------+---------------+------*/
struct DgnLineStyleTest : public ::testing::Test
    {
    public:
        ScopedDgnHost           m_host;
        DgnDbPtr      project;

        DgnLineStyleTest() { }

        void SetupProject (WCharCP projFile, Db::OpenMode mode);
    };

/*---------------------------------------------------------------------------------**//**
* Set up method that opens an existing .bim project file
* @bsimethod                                    Algirdas.Mikoliunas            03/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnLineStyleTest::SetupProject(WCharCP projFile, Db::OpenMode mode)
    {
    DgnDbTestDgnManager tdm (projFile, __FILE__, mode,true);
    project = tdm.GetDgnProjectP();
    ASSERT_TRUE( project != NULL);
    }

/*---------------------------------------------------------------------------------**//**
* Test for reading from line style table
* @bsimethod                                    Umar.Hayat                          03/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnLineStyleTest, ReadLineStyles)
    {
    SetupProject (L"SubStation_NoFence.i.ibim", Db::OpenMode::ReadWrite);
    
    //Get line styles
    LsCacheP cache = LsCache::GetDgnDbCache(*project);
    LsCacheStyleIterator iterLineStyles = cache->begin();
    LsCacheStyleIterator iterLineStyles_end   = cache->end();
    DgnModelPtr model = project->Models().GetModel(project->Models().QueryFirstModelId());

    //Iterate through each line style and make sure they have correct information
    TestStyleProperties testStyles[4], testStyle;
    testStyles[0].SetTestStyleProperties (LsComponentType::LineCode, "DGN Style 4",             "");
    testStyles[1].SetTestStyleProperties (LsComponentType::LineCode, "REFXA$0$TRAZO_Y_PUNTO",   "");
    testStyles[2].SetTestStyleProperties (LsComponentType::LineCode, "Continuous",              "");
    testStyles[3].SetTestStyleProperties (LsComponentType::LineCode, "DGN Style 5",             "");

    int i = 0;
    while (iterLineStyles != iterLineStyles_end)
        {
        TestStyleProperties testStyle;
        LsCacheStyleEntry const& entry = *iterLineStyles;
        LsDefinitionCP lsDef = entry.GetLineStyleCP();
        EXPECT_TRUE(nullptr != lsDef);
        LsComponentCP lsComponent = lsDef->GetComponentCP(model.get());
        EXPECT_TRUE(nullptr != lsComponent);
        if (lsComponent)
            {
            testStyle.SetTestStyleProperties(lsComponent->GetComponentType(), entry.GetStyleName(), lsComponent->GetDescription());
            testStyle.IsEqual(testStyles[i]);
            }
        ++iterLineStyles;
        ++i;
        }
    EXPECT_EQ(4, i) << "Count should be 4" ;
    }
/*---------------------------------------------------------------------------------**//**
* Test new style insert
* @bsimethod                                    Umar.Hayat                          03/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnLineStyleTest, InsertLineStyle)
    {
    SetupProject (L"SubStation_NoFence.i.ibim", Db::OpenMode::ReadWrite);

    //Get line styles
    DgnLineStyles& styleTable = project->Styles().LineStyles();
    LsCacheP cache = styleTable.GetLsCacheP();
    LsCacheStyleIterator iterLineStyles = cache->begin();
    DgnModelPtr model = project->Models().GetModel(project->Models().QueryFirstModelId());
    static Utf8CP STYLE_NAME = "ATestLineStyle";

    // Add new line style
    DgnStyleId newStyleId;
    LsComponentId componentId(LsComponentType::LineCode, 6);
    EXPECT_EQ(SUCCESS, styleTable.Insert(newStyleId, STYLE_NAME, componentId, 53, 0.0)) << "Insert line style return value should be SUCCESS";
    ASSERT_TRUE(newStyleId.IsValid());
    ASSERT_TRUE(DbResult::BE_SQLITE_OK == project->SaveChanges());
    }
/*---------------------------------------------------------------------------------**//**
* Test new style insert and query without reloading the cache
* @bsimethod                                    Umar.Hayat                          03/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnLineStyleTest, InsertAndQueryWithoutCacheReLoad)
    {
    SetupProject (L"SubStation_NoFence.i.ibim", Db::OpenMode::ReadWrite);

    //Get line styles
    DgnLineStyles& styleTable = project->Styles().LineStyles();
    LsCacheP cache = styleTable.GetLsCacheP();
    LsCacheStyleIterator iterLineStyles = cache->begin();
    DgnModelPtr model = project->Models().GetModel(project->Models().QueryFirstModelId());
    static Utf8CP STYLE_NAME = "ATestLineStyle";

    // Add new line style
    DgnStyleId newStyleId;
    LsComponentId componentId(LsComponentType::LineCode, 6);
    EXPECT_EQ(SUCCESS, styleTable.Insert(newStyleId, STYLE_NAME, componentId, 53, 0.0)) << "Insert line style return value should be SUCCESS";
    ASSERT_TRUE(newStyleId.IsValid());
    ASSERT_TRUE(DbResult::BE_SQLITE_OK == project->SaveChanges());

    LsDefinitionP  lsDef = cache->GetLineStyleP(newStyleId);
    ASSERT_TRUE(nullptr != lsDef);
    LsComponentCP lsComponent = lsDef->GetComponentCP(model.get());
    ASSERT_TRUE(nullptr != lsComponent);

    TestStyleProperties expectedTestStyle, testStyle;
    expectedTestStyle.SetTestStyleProperties (LsComponentType::LineCode, STYLE_NAME, "");
    testStyle.SetTestStyleProperties(lsComponent->GetComponentType(), lsDef->GetStyleName(), lsComponent->GetDescription());
    expectedTestStyle.IsEqual (testStyle);
    }
/*---------------------------------------------------------------------------------**//**
* Test new style insert
* @bsimethod                                    Umar.Hayat                          03/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnLineStyleTest, InsertDuplicateLineStyle)
    {
    SetupProject (L"3dMetricGeneral.ibim", Db::OpenMode::ReadWrite);

    //Get line styles
    DgnLineStyles& styleTable = project->Styles().LineStyles();
    LsCacheP cache = styleTable.GetLsCacheP();
    LsCacheStyleIterator iterLineStyles = cache->begin();
    static Utf8CP STYLE_NAME = "ATestLineStyle";

    // Add new line style
    DgnStyleId newStyleId;
    LsComponentId componentId(LsComponentType::LineCode, 6);
    EXPECT_EQ(SUCCESS, styleTable.Insert(newStyleId, STYLE_NAME, componentId, 53, 0.0)) << "Insert line style return value should be SUCCESS";
    ASSERT_TRUE(newStyleId.IsValid());


    EXPECT_TRUE(SUCCESS != styleTable.Insert(newStyleId, STYLE_NAME, componentId, 53, 0.0)) << "Insert duplicate should fail";

    EXPECT_TRUE(DbResult::BE_SQLITE_OK == project->SaveChanges());

    }

/*---------------------------------------------------------------------------------**//**
* Test update line style
* @bsimethod                                    Umar.Hayat                          03/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnLineStyleTest, UpdateLineStyle)
{
    SetupProject(L"3dMetricGeneral.ibim", Db::OpenMode::ReadWrite);

    //Get line styles
    DgnLineStyles& styleTable = project->Styles().LineStyles();
    LsCacheP cache = styleTable.GetLsCacheP();
    LsCacheStyleIterator iterLineStyles = cache->begin();
    DgnModelPtr model = project->Models().GetModel(project->Models().QueryFirstModelId());
    static Utf8CP STYLE_NAME = "ATestLineStyle";

    // Add new line style
    DgnStyleId newStyleId;
    LsComponentId componentId(LsComponentType::LineCode, 6);
    EXPECT_EQ(SUCCESS, styleTable.Insert(newStyleId, STYLE_NAME, componentId, 53, 0.0)) << "Insert line style return value should be SUCCESS";
    ASSERT_TRUE(newStyleId.IsValid());


    EXPECT_TRUE(SUCCESS != styleTable.Insert(newStyleId, STYLE_NAME, componentId, 53, 0.0)) << "Insert duplicate should fail";

    EXPECT_TRUE(DbResult::BE_SQLITE_OK == project->SaveChanges());

    // TODO: Update method is WIP, Need to update later

}

/*---------------------------------------------------------------------------------**//**
* Test update line style with existing name
* @bsimethod                                    Algirdas.Mikoliunas          01/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnLineStyleTest, UpdateLineStyleWithExistingName)
    {
    SetupProject (L"SubStation_NoFence.i.ibim", Db::OpenMode::ReadWrite);

    //Get line styles
    DgnLineStyles& styleTable = project->Styles().LineStyles();
    LsCacheP cache = styleTable.GetLsCacheP();
    LsCacheStyleIterator iterLineStyles = cache->begin();
    LsCacheStyleEntry const& entry = *iterLineStyles;
    static Utf8CP STYLE_NAME = "DGN Style 4";
    EXPECT_STREQ(STYLE_NAME, entry.GetStyleName());
    
    // This would normally trigger an assertion failure.
    LsComponentId componentId(LsComponentType::LineCode, 7);
    BeTest::SetFailOnAssert (false);
    BentleyStatus updateResult = styleTable.Update(entry.GetLineStyleP()->GetStyleId(), "Continuous", componentId, 53, 0.0);
    BeTest::SetFailOnAssert (true);

    EXPECT_NE(SUCCESS, updateResult) << "UpdateLineStyle with existing name must not be successfull";
    
    // Check if line style name changed
    LsCacheStyleIterator iterUpdated = cache->begin();
    LsCacheStyleEntry const& updatedEntry = *iterUpdated;
    EXPECT_STREQ(STYLE_NAME, updatedEntry.GetStyleName());
    }

/*---------------------------------------------------------------------------------**//**
* Test iterator entry get data
* @bsimethod                                    Umar.Hayat                          03/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnLineStyleTest, IteratorLineStyleElement)
    {
    SetupProject (L"SubStation_NoFence.i.ibim", Db::OpenMode::ReadWrite);

    int count = 0;
    for (LineStyleElement::Entry entry : LineStyleElement::MakeIterator(*project))
        {
        count++;
        }
    EXPECT_EQ(4, count);
    }
