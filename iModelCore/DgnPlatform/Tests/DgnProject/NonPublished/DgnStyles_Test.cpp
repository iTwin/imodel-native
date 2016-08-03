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
#include "../TestFixture/DgnDbTestFixtures.h"

#if defined (_MSC_VER)
#pragma warning (disable:4702)
#endif

USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_RENDER

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
struct DgnLineStyleTest : public DgnDbTestFixture
    {
    };

/*---------------------------------------------------------------------------------**//**
* Test for reading from line style table
* @bsimethod                                    Umar.Hayat                          03/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnLineStyleTest, ReadLineStyles)
    {
    SetupWithPrePublishedFile (L"SubStation_NoFence.i.ibim", L"ReadLineStyles.ibim", Db::OpenMode::ReadWrite);
    DgnDbPtr      project = m_db;
    
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
    SetupWithPrePublishedFile (L"SubStation_NoFence.i.ibim", L"InsertLineStyle.ibim", Db::OpenMode::ReadWrite);
    DgnDbPtr      project = m_db;

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
    SetupWithPrePublishedFile (L"SubStation_NoFence.i.ibim", L"InsertAndQueryWithoutCacheReLoad.ibim", Db::OpenMode::ReadWrite);
    DgnDbPtr      project = m_db;

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
    SetupSeedProject();
    DgnDbPtr      project = m_db;

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
    SetupSeedProject();
    DgnDbPtr      project = m_db;

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
    SetupWithPrePublishedFile (L"SubStation_NoFence.i.ibim", L"UpdateLineStyleWithExistingName.ibim", Db::OpenMode::ReadWrite);
    DgnDbPtr      project = m_db;

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
    SetupWithPrePublishedFile (L"SubStation_NoFence.i.ibim", L"IteratorLineStyleElement.ibim", Db::OpenMode::ReadWrite);
    DgnDbPtr      project = m_db;

    int count = 0;
    for (LineStyleElement::Entry entry : LineStyleElement::MakeIterator(*project))
        {
        count++;
        }
    EXPECT_EQ(4, count);
    }
/*---------------------------------------------------------------------------------**//**
* Test InsertRasterComponentAsJson
* @bsimethod                                    Ridha.Malik                          07/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnLineStyleTest, InsertRasterComponentAsJson)
    {
    SetupSeedProject();
    DgnDbPtr      project = m_db;

    //Get line styles
    DgnLineStyles& styleTable = project->Styles().LineStyles();
    LsCacheP cache = styleTable.GetLsCacheP();
    LsCacheStyleIterator iterLineStyles = cache->begin();
    DgnModelPtr model = project->Models().GetModel(project->Models().QueryFirstModelId());
    static Utf8CP STYLE_NAME = "RidhaTestLineStyle";
    DgnStyleId newStyleId;

    Json::Value     jsonValue(Json::objectValue);
    uint32_t width = 100;
    uint32_t height = 200;

    ByteStream testImage(height * width * 3);
    Byte* p = testImage.GetDataP();
    for (uint8_t y = 0; y<height; ++y)
        {
        for (uint8_t x = 0; x<width; ++x)
            {
            *p++ = (y % 256); // R
            *p++ = (x % 256); // G
            *p++ = (0x33);  // B
            }
        }

    Image image(width, height, std::move(testImage), Image::Format::Rgb);

    jsonValue["x"] = image.GetWidth();
    jsonValue["y"] = image.GetHeight();
    jsonValue["flags"] = 12;
    jsonValue["trueWidth"] = 100;
    LsComponentId componentId(LsComponentType::RasterImage, 7);
    EXPECT_TRUE(BSISUCCESS == LsComponent::AddRasterComponentAsJson(componentId, *project, jsonValue, image.GetByteStream().GetData(), image.GetByteStream().GetSize()));
    EXPECT_TRUE(DbResult::BE_SQLITE_OK == project->SaveChanges());
    // Add new line style
    EXPECT_EQ(SUCCESS, styleTable.Insert(newStyleId, STYLE_NAME, componentId, 53, 0.0)) << "Insert line style return value should be SUCCESS";
    ASSERT_TRUE(newStyleId.IsValid());
    EXPECT_TRUE(DbResult::BE_SQLITE_OK == project->SaveChanges());
    ASSERT_TRUE(1 == LineStyleElement::QueryCount(*project));
    styleTable.ReloadMap();
    cache = styleTable.GetLsCacheP();
    LsDefinitionP  lsDef = cache->GetLineStyleP(newStyleId);
    ASSERT_TRUE(nullptr != lsDef);
    LsComponentCP lsComponent = lsDef->GetComponentCP(model.get());
    ASSERT_TRUE(nullptr != lsComponent);
    ASSERT_TRUE(LsComponentType::RasterImage == lsComponent->GetComponentType());

    LsIdNodeP nodeid = cache->SearchIdsForName(STYLE_NAME);
    LsDefinitionP checkComponentype = nodeid->GetValue();
    ASSERT_TRUE(checkComponentype->IsOfType(LsComponentType::RasterImage));
    checkComponentype = LsCache::FindInMap(*project, newStyleId);
    ASSERT_TRUE(checkComponentype->IsOfType(LsComponentType::RasterImage));
    LsRasterImageComponent *rasterimage = (LsRasterImageComponent *)lsComponent;
    ASSERT_TRUE(image.GetWidth() == rasterimage->GetWidth());
    ASSERT_TRUE(image.GetHeight() == rasterimage->GetHeight());
    ASSERT_TRUE(12 == rasterimage->GetFlags());
    Byte const* src1 = image.GetByteStream().GetData();
    Byte const* dst1=rasterimage->GetImage();
    for (uint8_t y = 0; y<height; ++y)
    {
        for (uint8_t x = 0; x<width; ++x)
        {
            ASSERT_TRUE(*src1++ == *dst1++);
            ASSERT_TRUE(*src1++ == *dst1++);
            ASSERT_TRUE(*src1++ == *dst1++);
        }
    }
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ridha.Malik                          08/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnLineStyleTest, InsertLineCodeComponentAsJson)
    {
    //SetupSeedProject();
    SetupWithPrePublishedFile(L"3dMetricGeneral.ibim", L"LineCodeComponentAsJson.ibim", Db::OpenMode::ReadWrite);
    DgnDbPtr      project = m_db;
    DgnLineStyles& styleTable = project->Styles().LineStyles();
    LsCacheP cache = styleTable.GetLsCacheP();
    LsCacheStyleIterator iterLineStyles = cache->begin();
    DgnModelPtr model = project->Models().GetModel(project->Models().QueryFirstModelId());
    static Utf8CP STYLE_NAME = "TestLineStyle";
    DgnStyleId newStyleId;
    LsComponentId componentId(LsComponentType::LineCode, 10);
    ASSERT_TRUE(componentId.IsValid());
    BeFileName outFileName= (BeFileName)cache->GetFileName();
    Json::Value     jsonValue(Json::objectValue);
    double phase = 0.0;
    uint32_t options = 0;
    uint32_t  maxIterate = 5;

    // Set phase, maxIterate, and options

    options |= LCOPT_ITERATION;

    options |= LCOPT_SEGMENT;
    options |= LsStrokePatternComponent::PhaseMode::PHASEMODE_Fixed;
    phase = 10;
  

    jsonValue["phase"] = phase;
    jsonValue["options"] = options;
    jsonValue["maxIter"] = maxIterate;
    Json::Value strokes(Json::arrayValue);
    for (int i = 0; i < 4; i++)
        {
        Json::Value  entry(Json::objectValue);
        entry["length"] = 10;
        entry["orgWidth"] = 5;
        entry["endWidth"] = 5;
        int32_t strokeMode = 0;
        strokeMode |= LCSTROKE_DASH;
        strokeMode |= LCSTROKE_RAY;
        strokeMode |= LCSTROKE_SCALE;
        entry["strokeMode"] = strokeMode;
        entry["widthMode"]= LCWIDTH_FULL;
        entry["capMode"] = LCCAP_CLOSED;
        strokes[i] = entry;
        }

    jsonValue["strokes"] = strokes;

    LsComponent::AddComponentAsJsonProperty(componentId, *project, LsComponentType::LineCode, jsonValue);
    EXPECT_TRUE(DbResult::BE_SQLITE_OK == project->SaveChanges());

    EXPECT_EQ(SUCCESS, styleTable.Insert(newStyleId, STYLE_NAME, componentId, 53, 0.0)) << "Insert line style return value should be SUCCESS";
    ASSERT_TRUE(newStyleId.IsValid());

    EXPECT_TRUE(DbResult::BE_SQLITE_OK == project->SaveChanges());
    uint32_t id = componentId.GetValue();
    uint32_t nextid = id + 1;
    BeSQLite::PropertySpec spec = LineStyleProperty::LineCode();
    LsComponent::GetNextComponentNumber(id,*project, spec);
    ASSERT_TRUE(nextid==id);
    
    CloseDb();
   
    BeSQLite::Db::OpenMode mode = BeSQLite::Db::OpenMode::ReadWrite;

    OpenDb(m_db, outFileName, mode);
   
    cache = m_db->Styles().LineStyles().GetLsCacheP();
    LsDefinitionP  lsDef = cache->GetLineStyleP(newStyleId);
    ASSERT_TRUE(nullptr != lsDef);
    LsComponentCP lsComponent = lsDef->GetComponentCP(model.get());
    ASSERT_TRUE(nullptr != lsComponent);
    ASSERT_TRUE(LsComponentType::LineCode == lsComponent->GetComponentType());

    LsStrokePatternComponent * strokePattern = (LsStrokePatternComponent *)lsComponent;
    ASSERT_TRUE(nullptr != strokePattern);
    uint32_t nStrokes = (uint32_t)strokePattern->GetNumberStrokes();
    ASSERT_TRUE(strokePattern->IsSingleSegment());
    ASSERT_TRUE(LsStrokePatternComponent::PhaseMode::PHASEMODE_Fixed == strokePattern->GetPhaseMode());
    ASSERT_TRUE(0 == strokePattern->GetDistancePhase());
    ASSERT_TRUE(strokePattern->HasIterationLimit());
    ASSERT_TRUE(5 == strokePattern->GetIterationLimit());
 
    ASSERT_TRUE(LsStrokePatternComponent::PhaseMode::PHASEMODE_Fixed == strokePattern->GetPhaseMode());
    for (uint32_t index = 0; index < nStrokes; ++index)
    {
        LsStroke * stroke = strokePattern->GetStrokeP(index);
        ASSERT_TRUE(nullptr != stroke);
        ASSERT_TRUE(10==stroke->GetLength());
        ASSERT_TRUE(3 == stroke->GetWidthMode());
        ASSERT_TRUE(5 == stroke->GetStartWidth());
        ASSERT_TRUE(5 == stroke->GetEndWidth());
        ASSERT_TRUE(stroke->IsDash());
        ASSERT_TRUE(stroke->IsDashFirst());
        ASSERT_TRUE(stroke->IsDashLast());
        ASSERT_TRUE(stroke->IsRigid());
        ASSERT_TRUE(stroke->IsStretchable());
    }
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ridha.Malik                          08/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnLineStyleTest, InsertCompoundComponentAsJson)
    {
    SetupSeedProject();
    DgnDbPtr      project = m_db;
    //Get line styles
    DgnLineStyles& styleTable = project->Styles().LineStyles();
    LsCacheP cache = styleTable.GetLsCacheP();
    LsCacheStyleIterator iterLineStyles = cache->begin();
    DgnModelPtr model = project->Models().GetModel(project->Models().QueryFirstModelId());
    static Utf8CP STYLE_NAME = "TestLineStyle";
    DgnStyleId newStyleId;
    BeFileName outFileName = (BeFileName)cache->GetFileName();
    LsComponentId componentId(LsComponentType::Compound, 4);
    Json::Value     jsonValue(Json::objectValue);

    Json::Value components(Json::arrayValue);

    for (int i = 0; i < 2; ++i)
        {
        LsComponentId componentId(LsComponentType::Internal, i);
        Json::Value  entry(Json::objectValue);
        entry["offset"] = i+1;
        entry["id"] = i;
        uint32_t type=(uint32_t)componentId.GetType();
        entry["type"] = type;
        components.append(entry);
        }

    jsonValue["comps"] = components;

    LsComponent::AddComponentAsJsonProperty(componentId, *project, LsComponentType::Compound, jsonValue);
    EXPECT_TRUE(DbResult::BE_SQLITE_OK == project->SaveChanges());

    EXPECT_EQ(SUCCESS, styleTable.Insert(newStyleId, STYLE_NAME, componentId, 53, 0.0)) << "Insert line style return value should be SUCCESS";
    ASSERT_TRUE(newStyleId.IsValid());

    EXPECT_TRUE(DbResult::BE_SQLITE_OK == project->SaveChanges());
    CloseDb();

    BeSQLite::Db::OpenMode mode = BeSQLite::Db::OpenMode::ReadWrite;

    OpenDb(m_db, outFileName, mode);

    cache = m_db->Styles().LineStyles().GetLsCacheP();
    LsDefinitionP  lsDef = cache->GetLineStyleP(newStyleId);
    ASSERT_TRUE(nullptr != lsDef);
    LsComponentCP lsComponent = lsDef->GetComponentCP(model.get());
    ASSERT_TRUE(nullptr != lsComponent);
    ASSERT_TRUE(LsComponentType::Compound == lsComponent->GetComponentType());
    LsCompoundComponent * compoundComponent = (LsCompoundComponent *)lsComponent;
    ASSERT_TRUE(nullptr != compoundComponent);
    uint32_t numComponents = (uint32_t)compoundComponent->GetNumComponents();
    ASSERT_TRUE(2 == compoundComponent->GetNumComponents());
    for (size_t i = 0; i < numComponents; ++i)
        {
        LsComponent const* child = compoundComponent->GetComponentCP(i);
        ASSERT_TRUE(nullptr != child);
        ASSERT_TRUE(i+1 == compoundComponent->GetOffsetToComponent(i));
        ASSERT_TRUE(LsComponentType::Internal == child->GetComponentType());
        LsInternalComponent * internalComponent = (LsInternalComponent*)child;
        ASSERT_TRUE(nullptr != internalComponent);
        ASSERT_TRUE(0 == internalComponent->GetHardwareStyle());
        }
    }
