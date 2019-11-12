/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "DgnHandlersTests.h"
#include <Bentley/BeTimeUtilities.h>
#include <DgnPlatform/ColorUtil.h>
#include "../TestFixture/DgnDbTestFixtures.h"

#if defined (_MSC_VER)
#pragma warning (disable:4702)
#endif

USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_RENDER

/*---------------------------------------------------------------------------------**//**
* Test fixture for testing DgnStyles
* @bsimethod                                    Algirdas.Mikoliunas            03/2013
+---------------+---------------+---------------+---------------+---------------+------*/
struct DgnLineStyleTest : public DgnDbTestFixture
    {};

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
        void IsEqual(TestStyleProperties testStyle)
            {
            EXPECT_TRUE(testStyle.tsType == tsType) << "Types don't match";
            EXPECT_STREQ(testStyle.tsName.c_str(), tsName.c_str()) << "Names don't match";
            //EXPECT_STREQ (tsDescription.c_str(), testStyle.tsDescription.c_str()) << "Descriptions don't match";
            };
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Muhammad Hassan                   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnLineStyleTest, InsertReadLineStyles)
    {
    Utf8CP dbFilePath;
    {
    SetupSeedProject();

    // Add new component.
    LsComponentId componentId0(LsComponentType::LineCode, 10);
    ASSERT_TRUE(componentId0.IsValid());
    LsComponentId componentId1(LsComponentType::LineCode, 11);
    ASSERT_TRUE(componentId1.IsValid());
    Json::Value     jsonValue(Json::objectValue);
    LsComponent::AddComponentAsJsonProperty(componentId0, *m_db, LsComponentType::LineCode, jsonValue);
    LsComponent::AddComponentAsJsonProperty(componentId1, *m_db, LsComponentType::LineCode, jsonValue);

    // Add new line styles
    DgnLineStyles& styleTable = m_db->LineStyles();
    DgnStyleId styleId0;
    DgnStyleId styleId1;
    ASSERT_EQ(SUCCESS, styleTable.Insert(styleId0, "ATestLineStyle0", componentId0, 53, 0.0)) << "Insert line style return value should be SUCCESS";
    ASSERT_TRUE(styleId0.IsValid());

    ASSERT_EQ(SUCCESS, styleTable.Insert(styleId1, "ATestLineStyle1", componentId1, 53, 0.0)) << "Insert line style return value should be SUCCESS";
    ASSERT_TRUE(styleId1.IsValid());
    ASSERT_TRUE(DbResult::BE_SQLITE_OK == m_db->SaveChanges());
    dbFilePath = m_db->GetDbFileName();
    }

    BeSQLite::Db::OpenMode mode = Db::OpenMode::Readonly;
    DbResult result = BE_SQLITE_OK;
    DgnDbPtr project = DgnDb::OpenDgnDb(&result, BeFileName(dbFilePath), DgnDb::OpenParams(mode));
    ASSERT_EQ(BE_SQLITE_OK, result);

    //Get line styles count
    ASSERT_TRUE(2 == LineStyleElement::QueryCount(*project));

    //Get line styles
    DgnLineStyles& styleTable = project->LineStyles();
    LsCacheP cache = &styleTable.GetCache();
    LsCacheStyleIterator iterLineStyles_begin = cache->begin();
    LsCacheStyleIterator iterLineStyles_end = cache->end();

    //Iterate through each line style and make sure they have correct information
    TestStyleProperties testStyles[4];
    testStyles[0].SetTestStyleProperties(LsComponentType::LineCode, "ATestLineStyle0", "");
    testStyles[1].SetTestStyleProperties(LsComponentType::LineCode, "ATestLineStyle1", "");

    int i = 0;
    while (iterLineStyles_begin != iterLineStyles_end)
        {
        TestStyleProperties testStyle;
        LsCacheStyleEntry const& entry = *iterLineStyles_begin;
        LsDefinitionCP lsDef = entry.GetLineStyleCP();
        EXPECT_TRUE(nullptr != lsDef);
        LsComponentCP lsComponent = lsDef->GetComponentCP();
        EXPECT_TRUE(nullptr != lsComponent);
        if (lsComponent)
            {
            testStyle.SetTestStyleProperties(lsComponent->GetComponentType(), entry.GetStyleName(), lsComponent->GetDescription());
            testStyle.IsEqual(testStyles[i]);
            }
        ++iterLineStyles_begin;
        ++i;
        }
    }

/*---------------------------------------------------------------------------------**//**
* Insert a new LineStyle into a DefinitionModel other than the DictionaryModel.
* @bsimethod                                    Shaun.Sewall                    07/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnLineStyleTest, InsertLineStyleIntoDefinitionModel)
    {
    SetupSeedProject();
    DefinitionModelPtr definitionModel = DgnDbTestUtils::InsertDefinitionModel(*m_db, "OtherDefinitionModel");
    ASSERT_TRUE(definitionModel.IsValid());

    // Create new component.
    LsComponentId componentId(LsComponentType::LineCode, 10);
    ASSERT_TRUE(componentId.IsValid());
    Json::Value componentJson(Json::objectValue);
    LsComponent::AddComponentAsJsonProperty(componentId, *m_db, LsComponentType::LineCode, componentJson);

    // Insert LineStyle
    DgnStyleId styleId;
    Utf8CP styleName = "TestLineStyle";
    Utf8CP styleDescription = "TestLineStyleDescription";
    ASSERT_EQ(BentleyStatus::SUCCESS, m_db->LineStyles().Insert(styleId, definitionModel->GetModelId(), styleName, styleDescription, componentId, 53, 0.0)) << "Insert LineStyle should return SUCCESS";
    ASSERT_TRUE(styleId.IsValid());

    // Query for the newly inserted LineStyle
    ASSERT_TRUE(LineStyleElement::QueryId(*definitionModel, styleName).IsValid()) << "Expect to find LineStyle in OtherDefinitionModel";
    ASSERT_FALSE(LineStyleElement::QueryId(*m_db, styleName).IsValid()) << "Do not expect to find LineStyle in DictionaryModel";
    }

/*---------------------------------------------------------------------------------**//**
* Test new style insert and query without reloading the cache
* @bsimethod                                    Umar.Hayat                          03/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnLineStyleTest, InsertAndQueryWithoutCacheReLoad)
    {
    SetupSeedProject();
    DgnDbPtr project = m_db;

    //Get line styles
    DgnLineStyles& styleTable = project->LineStyles();
    LsCacheP cache = &styleTable.GetCache();
    LsCacheStyleIterator iterLineStyles = cache->begin();
    PhysicalModelPtr model = GetDefaultPhysicalModel();
    static Utf8CP STYLE_NAME = "ATestLineStyle";

    // Add new line style
    DgnStyleId newStyleId;
    LsComponentId componentId(LsComponentType::LineCode, 6);
    EXPECT_EQ(SUCCESS, styleTable.Insert(newStyleId, STYLE_NAME, componentId, 53, 0.0)) << "Insert line style return value should be SUCCESS";
    ASSERT_TRUE(newStyleId.IsValid());
    ASSERT_TRUE(DbResult::BE_SQLITE_OK == project->SaveChanges());

    LsDefinitionP  lsDef = cache->GetLineStyleP(newStyleId);
    ASSERT_TRUE(nullptr != lsDef);
    LsComponentCP lsComponent = lsDef->GetComponentCP();
    ASSERT_TRUE(nullptr != lsComponent);

    TestStyleProperties expectedTestStyle, testStyle;
    expectedTestStyle.SetTestStyleProperties(LsComponentType::LineCode, STYLE_NAME, "");
    testStyle.SetTestStyleProperties(lsComponent->GetComponentType(), lsDef->GetStyleName(), lsComponent->GetDescription());
    expectedTestStyle.IsEqual(testStyle);
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
    DgnLineStyles& styleTable = project->LineStyles();
    LsCacheP cache = &styleTable.GetCache();
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
    DgnLineStyles& styleTable = project->LineStyles();
    LsCacheP cache = &styleTable.GetCache();
    LsCacheStyleIterator iterLineStyles = cache->begin();
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
    Utf8CP dbFilePath;
    {
    SetupSeedProject();

    //Get line styles
    DgnLineStyles& styleTable = m_db->LineStyles();
    LsCacheP cache = &styleTable.GetCache();
    LsCacheStyleIterator iterLineStyles = cache->begin();

    // Add new line styles
    DgnStyleId styleId0;
    LsComponentId componentId0(LsComponentType::LineCode, 6);
    ASSERT_EQ(SUCCESS, styleTable.Insert(styleId0, "NewLineStyle", componentId0, 53, 0.0)) << "Insert line style return value should be SUCCESS";
    ASSERT_TRUE(styleId0.IsValid());
    DgnStyleId styleId1;
    LsComponentId componentId1(LsComponentType::LineCode, 7);
    ASSERT_EQ(SUCCESS, styleTable.Insert(styleId1, "LineStyle", componentId1, 53, 0.0)) << "Insert line style return value should be SUCCESS";
    ASSERT_TRUE(styleId1.IsValid());
    ASSERT_TRUE(DbResult::BE_SQLITE_OK == m_db->SaveChanges());
    dbFilePath = m_db->GetDbFileName();
    }

    BeSQLite::Db::OpenMode mode = Db::OpenMode::Readonly;
    DbResult result = BE_SQLITE_OK;
    DgnDbPtr project = DgnDb::OpenDgnDb(&result, BeFileName(dbFilePath), DgnDb::OpenParams(mode));
    ASSERT_EQ(BE_SQLITE_OK, result);

    //Get line styles
    DgnLineStyles& styleTable = project->LineStyles();
    LsCacheP cache = &styleTable.GetCache();
    LsCacheStyleIterator iterLineStyles = cache->begin();
    LsCacheStyleEntry const& entry = *iterLineStyles;
    static Utf8CP STYLE_NAME = "NewLineStyle";
    EXPECT_STREQ(STYLE_NAME, entry.GetStyleName());

    // This would normally trigger an assertion failure.
    LsComponentId componentId(LsComponentType::LineCode, 7);
    BeTest::SetFailOnAssert(false);
    BentleyStatus updateResult = styleTable.Update(entry.GetLineStyleP()->GetStyleId(), "Continuous", componentId, 53, 0.0);
    BeTest::SetFailOnAssert(true);

    EXPECT_NE(SUCCESS, updateResult) << "UpdateLineStyle with existing name must not be successfull";

    // Check if line style name changed
    LsCacheStyleIterator iterUpdated = cache->begin();
    LsCacheStyleEntry const& updatedEntry = *iterUpdated;
    EXPECT_STREQ(STYLE_NAME, updatedEntry.GetStyleName());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Muhammad Hassan                   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnLineStyleTest, IteratorLineStyleElement)
    {
    Utf8CP dbFilePath;
    {
    SetupSeedProject();

    //Get line styles
    DgnLineStyles& styleTable = m_db->LineStyles();
    LsCacheP cache = &styleTable.GetCache();
    LsCacheStyleIterator iterLineStyles = cache->begin();

    // Add new line styles
    DgnStyleId styleId0;
    LsComponentId componentId0(LsComponentType::LineCode, 6);
    ASSERT_EQ(SUCCESS, styleTable.Insert(styleId0, "NewLineStyle", componentId0, 53, 0.0)) << "Insert line style return value should be SUCCESS";
    ASSERT_TRUE(styleId0.IsValid());

    ASSERT_TRUE(DbResult::BE_SQLITE_OK == m_db->SaveChanges());
    dbFilePath = m_db->GetDbFileName();
    }

    BeSQLite::Db::OpenMode mode = Db::OpenMode::Readonly;
    DbResult result = BE_SQLITE_OK;
    DgnDbPtr project = DgnDb::OpenDgnDb(&result, BeFileName(dbFilePath), DgnDb::OpenParams(mode));
    ASSERT_EQ(BE_SQLITE_OK, result);

    int count = 0;
    for (LineStyleElement::Entry entry : LineStyleElement::MakeIterator(*project))
        {
        count++;
        }
    EXPECT_EQ(1, count) << "Only 1 lineStyle should exist in Db";
    }

/*---------------------------------------------------------------------------------**//**
* Test InsertRasterComponentAsJson
* @bsimethod                                    Ridha.Malik                          07/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnLineStyleTest, InsertRasterComponentAsJson)
    {
    SetupSeedProject();
    DgnDbPtr project = m_db;

    uint32_t width = 100;
    uint32_t height = 200;
    ByteStream testImage(height * width * 3);
    Byte* p = testImage.GetDataP();
    for (uint8_t y = 0; y < height; ++y)
        {
        for (uint8_t x = 0; x < width; ++x)
            {
            *p++ = (y % 256); // R
            *p++ = (x % 256); // G
            *p++ = (0x33);  // B
            }
        }

    Image image(width, height, std::move(testImage), Image::Format::Rgb);

    Json::Value     jsonValue(Json::objectValue);
    jsonValue["x"] = image.GetWidth();
    jsonValue["y"] = image.GetHeight();
    jsonValue["flags"] = 12;
    jsonValue["trueWidth"] = 100;
    LsComponentId componentId(LsComponentType::RasterImage, 7);
    EXPECT_TRUE(BSISUCCESS == LsComponent::AddRasterComponentAsJson(componentId, *project, jsonValue, image.GetByteStream().GetData(), image.GetByteStream().GetSize()));

    // Add new line style
    DgnLineStyles& styleTable = project->LineStyles();
    static Utf8CP STYLE_NAME = "RidhaTestLineStyle";
    static Utf8CP STYLE_DESCRIPTION = "TestDescription";
    DgnStyleId newStyleId;
    EXPECT_EQ(SUCCESS, styleTable.Insert(newStyleId, DgnModel::DictionaryId(), STYLE_NAME, STYLE_DESCRIPTION, componentId, 53, 0.0)) << "Insert line style return value should be SUCCESS";
    ASSERT_TRUE(newStyleId.IsValid());
    EXPECT_TRUE(DbResult::BE_SQLITE_OK == project->SaveChanges());
    ASSERT_TRUE(1 == LineStyleElement::QueryCount(*project));

    LsCacheP cache = &styleTable.GetCache();
    LsDefinitionP  lsDef = cache->GetLineStyleP(newStyleId);
    ASSERT_TRUE(nullptr != lsDef);
    LsComponentCP lsComponent = lsDef->GetComponentCP();
    ASSERT_TRUE(nullptr != lsComponent);
    ASSERT_TRUE(LsComponentType::RasterImage == lsComponent->GetComponentType());

    LsIdNodeP nodeid = cache->SearchIdsForName(STYLE_NAME);
    LsDefinitionP checkComponentype = nodeid->GetValue();
    ASSERT_TRUE(checkComponentype->IsOfType(LsComponentType::RasterImage));

    checkComponentype = LsCache::FindInMap(*project, newStyleId);
    ASSERT_TRUE(checkComponentype->IsOfType(LsComponentType::RasterImage));

    LsRasterImageComponent *rasterimage = (LsRasterImageComponent *) lsComponent;
    ASSERT_TRUE(image.GetWidth() == rasterimage->GetWidth());
    ASSERT_TRUE(image.GetHeight() == rasterimage->GetHeight());
    ASSERT_TRUE(12 == rasterimage->GetFlags());
    Byte const* src1 = image.GetByteStream().GetData();
    Byte const* dst1 = rasterimage->GetImage();
    for (uint8_t y = 0; y < height; ++y)
        {
        for (uint8_t x = 0; x < width; ++x)
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
    Utf8CP seedFilePath;
    DgnStyleId styleId;
    {
    SetupSeedProject();

    double phase = 0.0;
    uint32_t options = 0;
    uint32_t  maxIterate = 5;

    // Set phase, maxIterate, and options
    options |= LCOPT_ITERATION;
    options |= LCOPT_SEGMENT;
    options |= LsStrokePatternComponent::PhaseMode::PHASEMODE_Fixed;
    phase = 10;

    Json::Value     jsonValue(Json::objectValue);
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
        entry["widthMode"] = LCWIDTH_FULL;
        entry["capMode"] = LCCAP_CLOSED;
        strokes[i] = entry;
        }

    jsonValue["strokes"] = strokes;

    // Add component
    LsComponentId componentId(LsComponentType::LineCode, 10);
    ASSERT_TRUE(componentId.IsValid());
    LsComponent::AddComponentAsJsonProperty(componentId, *m_db, LsComponentType::LineCode, jsonValue);
    EXPECT_TRUE(DbResult::BE_SQLITE_OK == m_db->SaveChanges());

    // Insert Line Style
    DgnLineStyles& styleTable = m_db->LineStyles();
    static Utf8CP STYLE_NAME = "TestLineStyle";
    EXPECT_EQ(SUCCESS, styleTable.Insert(styleId, STYLE_NAME, componentId, 53, 0.0)) << "Insert line style return value should be SUCCESS";
    ASSERT_TRUE(styleId.IsValid());

    EXPECT_TRUE(DbResult::BE_SQLITE_OK == m_db->SaveChanges());
    uint32_t id = componentId.GetValue();
    uint32_t nextid = id + 1;
    BeSQLite::PropertySpec spec = LineStyleProperty::LineCode();
    LsComponent::GetNextComponentNumber(id, *m_db, spec);
    ASSERT_TRUE(nextid == id);

    ASSERT_TRUE(DbResult::BE_SQLITE_OK == m_db->SaveChanges());
    seedFilePath = m_db->GetDbFileName();
    }

    BeSQLite::Db::OpenMode mode = BeSQLite::Db::OpenMode::ReadWrite;
    OpenDb(m_db, BeFileName(seedFilePath), mode);

    DgnLineStyles& styleTable = m_db->LineStyles();
    LsCacheP cache = &styleTable.GetCache();
    LsDefinitionP  lsDef = cache->GetLineStyleP(styleId);
    ASSERT_TRUE(nullptr != lsDef);
    LsComponentCP lsComponent = lsDef->GetComponentCP();
    ASSERT_TRUE(nullptr != lsComponent);
    ASSERT_TRUE(LsComponentType::LineCode == lsComponent->GetComponentType());

    LsStrokePatternComponent * strokePattern = (LsStrokePatternComponent *) lsComponent;
    ASSERT_TRUE(nullptr != strokePattern);
    uint32_t nStrokes = (uint32_t) strokePattern->GetNumberStrokes();
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
        ASSERT_TRUE(10 == stroke->GetLength());
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
    Utf8CP seedFilePath;
    DgnStyleId styleId;
    {
    SetupSeedProject();

    Json::Value components(Json::arrayValue);
    for (int i = 0; i < 2; ++i)
        {
        LsComponentId componentId(LsComponentType::Internal, i);
        Json::Value  entry(Json::objectValue);
        entry["offset"] = i + 1;
        entry["id"] = i;
        uint32_t type = (uint32_t) componentId.GetType();
        entry["type"] = type;
        components.append(entry);
        }
    Json::Value     jsonValue(Json::objectValue);
    jsonValue["comps"] = components;

    LsComponentId componentId(LsComponentType::Compound, 4);
    LsComponent::AddComponentAsJsonProperty(componentId, *m_db, LsComponentType::Compound, jsonValue);

    DgnLineStyles& styleTable = m_db->LineStyles();
    static Utf8CP STYLE_NAME = "TestLineStyle";
    ASSERT_EQ(SUCCESS, styleTable.Insert(styleId, STYLE_NAME, componentId, 53, 0.0)) << "Insert line style return value should be SUCCESS";
    ASSERT_TRUE(styleId.IsValid());

    ASSERT_TRUE(DbResult::BE_SQLITE_OK == m_db->SaveChanges());
    seedFilePath = m_db->GetDbFileName();
    }

    BeSQLite::Db::OpenMode mode = BeSQLite::Db::OpenMode::ReadWrite;
    OpenDb(m_db, BeFileName(seedFilePath), mode);

    LsCacheP cache = &m_db->LineStyles().GetCache();
    LsDefinitionP  lsDef = cache->GetLineStyleP(styleId);
    ASSERT_TRUE(nullptr != lsDef);
    LsComponentCP lsComponent = lsDef->GetComponentCP();
    ASSERT_TRUE(nullptr != lsComponent);
    ASSERT_TRUE(LsComponentType::Compound == lsComponent->GetComponentType());
    LsCompoundComponent * compoundComponent = (LsCompoundComponent *) lsComponent;
    ASSERT_TRUE(nullptr != compoundComponent);
    uint32_t numComponents = (uint32_t) compoundComponent->GetNumComponents();
    ASSERT_TRUE(2 == compoundComponent->GetNumComponents());
    for (size_t i = 0; i < numComponents; ++i)
        {
        LsComponent const* child = compoundComponent->GetComponentCP(i);
        ASSERT_TRUE(nullptr != child);
        ASSERT_TRUE(i + 1 == compoundComponent->GetOffsetToComponent(i));
        ASSERT_TRUE(LsComponentType::Internal == child->GetComponentType());
        LsInternalComponent * internalComponent = (LsInternalComponent*) child;
        ASSERT_TRUE(nullptr != internalComponent);
        }
    }
