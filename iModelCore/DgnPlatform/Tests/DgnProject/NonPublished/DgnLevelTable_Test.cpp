
#ifdef WIP_MERGE_AHMED

#include "DgnHandlersTests.h"
#include <Bentley/BeTimeUtilities.h>
#include <DgnPlatform/DgnCore/ColorUtil.h>

#if defined (_MSC_VER)
#pragma warning (disable:4702)
#endif

USING_NAMESPACE_BENTLEY_SQLITE 

//=======================================================================================
// @bsiclass                                                    Majd.Uddin   04/12
//=======================================================================================
struct TestLevelProperties
    {
    public:
        WString         tlName;
        uint32_t        tlColor;
        uint32_t        tlWeight;
        uint32_t        tlStyle;

        void SetTestLevelProperties (WString Name, uint32_t color, uint32_t weight, uint32_t style)
            {
            tlName = Name;
            tlColor = color;
            tlWeight = weight;
            tlStyle = style;
            };
        void IsEqual (TestLevelProperties testLevel)
            {
            EXPECT_STREQ (tlName.c_str(), testLevel.tlName.c_str()) << "Names don't match";
            EXPECT_EQ (tlColor, testLevel.tlColor) << "Colors don't match";
            EXPECT_EQ (tlWeight, testLevel.tlWeight) << "Weights don't match";
            EXPECT_EQ (tlStyle, testLevel.tlStyle) << "Styles don't match";
            };
    };

TEST(DgnLevelTableTests,MergeLevelMasks)
    {
    ScopedDgnHost autoDgnHost;

    //  All levels are display in ref.dgn
    if (true)
        {
        DgnDbTestDgnManager tdm (L"refLevelMasks.idgndb", __FILE__, OPENMODE_READONLY);
        DgnDbP project = tdm.GetDgnProjectP();
        ASSERT_TRUE( project != NULL );

        DgnViewId viewId;
        FOR_EACH(DgnViews::Iterator::Entry const& view, project->Views().MakeIterator())
            {
            viewId = view.GetDgnViewId();
            break;
            }

        ViewControllerPtr vi = new DrawingViewController(*project, viewId);
        ASSERT_TRUE( vi.get() != NULL );
        vi->Load();
        
        BitMaskCR levelMask = vi->GetLevelDisplayMask();

        DgnLevelTableRow const& refLevel = project->Levels().QueryLevelByName ("RefLevel2");
        ASSERT_TRUE( refLevel.GetLevelId().GetValue() < 64 );

        FOR_EACH(DgnLevels::Iterator::Entry const& level, project->Levels().MakeIterator())
            {
            ASSERT_TRUE( levelMask.Test(level.GetLevelId().GetValue()-1) ) << L"ref file should have all levels on. But level " << WString(level.GetName(),BentleyCharEncoding::Utf8).c_str() << L" is turned off";
            }
        }

    //  We turned off RefLevel2 in master.dgn. RefLevel2 had LevelID=1 in the original .dgn file. It will be assigned ROWID=65 in the combined project level table.
    if (true)
        {
        DgnDbTestDgnManager tdm (L"masterLevelMasks.idgndb", __FILE__, OPENMODE_READONLY);
        DgnDbP project = tdm.GetDgnProjectP();
        ASSERT_TRUE( project != NULL );

        DgnViewId viewId;
        FOR_EACH(DgnViews::Iterator::Entry const& view, project->Views().MakeIterator())
            {
            viewId = view.GetDgnViewId();
            break;
            }

        ViewControllerPtr vi = new DrawingViewController (*project, viewId);
        vi->Load();

        BitMaskCR levelMask = vi->GetLevelDisplayMask();

        FOR_EACH(DgnLevels::Iterator::Entry const& level, project->Levels().MakeIterator())
            {
            uint32_t iBit = (uint32_t)(level.GetLevelId().GetValue()-1);

            if (0 == strcmp (level.GetName(), "RefLevel2"))
                ASSERT_FALSE( levelMask.Test(iBit) ) << L"master file turns off display of 'RefLevel2'";
            else
                ASSERT_TRUE( levelMask.Test(iBit) );
            }
        }
    }

TEST(DgnLevelTableTests,TurnOffLevelInView)
    {
    ScopedDgnHost autoDgnHost;

    DgnDbTestDgnManager tdm (L"2dMetricGeneral.idgndb", __FILE__, OPENMODE_READONLY);
    DgnDbP project = tdm.GetDgnProjectP();
    ASSERT_TRUE( project != NULL );

    DgnViewId viewId;
    FOR_EACH(DgnViews::Iterator::Entry const& view, project->Views().MakeIterator())
        {
        viewId = view.GetDgnViewId();
        break;
        }

    DgnLevelTableRow level = project->Levels().QueryLevelByName ("Default");
    ASSERT_TRUE( level.IsValid() );

    ViewControllerPtr vi = new DrawingViewController (*project, viewId);
    vi->Load();

    BitMaskHolder levelsOnInView (vi->GetLevelDisplayMask());

    ASSERT_TRUE( levelsOnInView->Test(level.GetLevelId().GetValue()-1) );

    levelsOnInView->SetBit (level.GetLevelId().GetValue()-1, false);

    ASSERT_FALSE( levelsOnInView->Test(level.GetLevelId().GetValue()-1) );

    vi->SetLevelDisplayMask (*levelsOnInView);

    ASSERT_FALSE( vi->GetLevelDisplayMask().Test(level.GetLevelId().GetValue()-1) );
    }

TEST(DgnLevelTableTests,IModelMergeLevels)
    {
    ScopedDgnHost autoDgnHost;

    DgnDbTestDgnManager tdm (L"master.i.idgndb", __FILE__, OPENMODE_READONLY, TestDgnManager::DGNINITIALIZEMODE_None);
    DgnDbP project = tdm.GetDgnProjectP();
    ASSERT_TRUE( project != NULL );

    // Check that levels were correctly imported:
    //      Default
    //      master (blue)
    //      ref (red)
    size_t count = 0;
    FOR_EACH(DgnLevels::Iterator::Entry const& level, project->Levels().MakeIterator())
        {
        ASSERT_TRUE(level.GetLevelId().IsValid());
        ++count;
        }
    ASSERT_EQ(count,3);
    DgnLevelTableRow ldefault = project->Levels().QueryLevelByName ("Default");
    DgnLevelTableRow lmaster  = project->Levels().QueryLevelByName ("master");
    DgnLevelTableRow lref     = project->Levels().QueryLevelByName ("ref");

    ASSERT_TRUE(ldefault.IsValid());
    ASSERT_TRUE(lmaster.IsValid());
    ASSERT_TRUE(lref.IsValid());

    ASSERT_TRUE(0 == strcmp (lmaster.GetName(), "master"));
    ASSERT_TRUE(0 == strcmp (lref.GetName(), "ref"));

    /* *** NEEDS WORK??!!
    ASSERT_EQ(lmaster.GetColor(), BLUE_MENU_COLOR_INDEX);
    ASSERT_EQ(lref.GetColor(), RED_MENU_COLOR_INDEX);
    */

    // Check that we have 2 models. 
    //      model 1: 1 text element on level 'master'
    //      model 2: 1 text element on level 'ref'
    size_t modelCount=0;
    LevelId expectedLevelIds[2];
    expectedLevelIds[0] = lmaster.GetLevelId();
    expectedLevelIds[1] = lref.GetLevelId();
    DgnFileR file = project->GetDgnFile();
    FOR_EACH (DgnModels::Iterator::Entry const& entry, file.MakeModelIterator (MODEL_ITERATE_All))
        {
        DgnModelP model = project->Models().GetModel (entry.GetModelId());
        ASSERT_TRUE( model != NULL );
        ASSERT_TRUE( model->IsFilled (DgnModelSections::Model) != DgnModelSections::Model ) << "models should not be filled by default";
        modelCount++;
        ASSERT_LE( modelCount, 2U );

        ASSERT_TRUE( model->FillSections (DgnModelSections::Model) == SUCCESS );
        size_t ecount=0;
        FOR_EACH (PersistentElementRefP ref, *model->GetGraphicElementsP())
            {
            ElementHandle eh (ref, model);
            ++ecount;

            ASSERT_EQ( eh.GetElementRef()->GetElementType(), TEXT_ELM );
            DgnLevelTableRow const& level = project->Levels().QueryLevelById (eh.GetElementRef()->GetLevel());
            ASSERT_TRUE( level.IsValid() );
            ASSERT_EQ( level.GetLevelId().GetValue(), expectedLevelIds[model->GetModelId().GetValue()].GetValue() );
            }
        ASSERT_EQ(ecount, 1);
        }
    ASSERT_EQ( modelCount, 2 );
    }

TEST(DgnLevelTableTests,IModelCopyAndRenameLevels)
    {
    Utf8CP LNDefault = "Default"; // The level from master2 should come in under its own name
    Utf8CP LNRefDefault = "Default (Default [ref2])"; // The default level from ref2 should have been renamed. Note that the "Default" model from ref2 is also renamed.

    ScopedDgnHost autoDgnHost;

    DgnDbTestDgnManager tdm (L"master2.i.idgndb", __FILE__, OPENMODE_READONLY, TestDgnManager::DGNINITIALIZEMODE_None);
    DgnDbP project = tdm.GetDgnProjectP();
    ASSERT_TRUE( project != NULL );

    // Check that levels were correctly imported:
    //      Default (blue)
    //      ref_dgn__Default (red)
    size_t count = 0;
    FOR_EACH(DgnLevels::Iterator::Entry const& level, project->Levels().MakeIterator())
        {
        ASSERT_TRUE(level.GetLevelId().IsValid());
        ++count;
        }
    ASSERT_EQ(count,2);
    DgnLevelTableRow ldefault    = project->Levels().QueryLevelByName (LNDefault);
    DgnLevelTableRow lrefdefault = project->Levels().QueryLevelByName (LNRefDefault);

    ASSERT_TRUE(ldefault.IsValid());
    ASSERT_TRUE(lrefdefault.IsValid());

    /* *** NEEDS WORK??!!
    ASSERT_EQ(lmaster.GetColor(),       BLUE_MENU_COLOR_INDEX);
    ASSERT_EQ(lrefdefault.GetColor(),   RED_MENU_COLOR_INDEX);
    */

    // Check that we have 2 models. 
    //      model 0: 1 text element on level LNDefault
    //      model 1: 1 text element on level LNRefDefault
    size_t modelCount=0;
    LevelId expectedLevelIds[2];
    expectedLevelIds[0] = ldefault.GetLevelId();
    expectedLevelIds[1] = lrefdefault.GetLevelId();
    DgnFileR file = project->GetDgnFile();
    FOR_EACH (DgnModels::Iterator::Entry const& entry, file.MakeModelIterator (MODEL_ITERATE_All))
        {
        DgnModelP model = project->Models().GetModel (entry.GetModelId());
        ASSERT_TRUE( model != NULL );
        ASSERT_TRUE( model->IsFilled (DgnModelSections::Model) != DgnModelSections::Model ) << "models should not be filled by default";
        modelCount++;
        ASSERT_LE( modelCount, 2U );

        ASSERT_TRUE( model->FillSections (DgnModelSections::Model) == SUCCESS );
        size_t ecount=0;
        FOR_EACH (PersistentElementRefP ref, *model->GetGraphicElementsP())
            {
            ElementHandle eh (ref, model);
            ++ecount;

            ASSERT_EQ (eh.GetElementRef()->GetElementType(), TEXT_ELM );
            DgnLevelTableRow const& level = project->Levels().QueryLevelById (eh.GetElementRef()->GetLevel());
            ASSERT_TRUE( level.IsValid() );
            ASSERT_EQ( level.GetLevelId().GetValue(), expectedLevelIds[model->GetModelId().GetValue()].GetValue() );
            }
        ASSERT_EQ(ecount, 1);
        }
    ASSERT_EQ( modelCount, 2 );
    }

TEST(DgnLevelTableTests,UpdateLevel)
    {
    ScopedDgnHost autoDgnHost;

    DgnDbTestDgnManager tdm (L"2dMetricGeneral.idgndb", __FILE__, OPENMODE_READWRITE);
    DgnDbP project = tdm.GetDgnProjectP();
    ASSERT_TRUE( project != NULL );

    DgnLevelTableRow level = project->Levels().QueryLevelByName ("Default");
    ASSERT_TRUE( level.IsValid() );

    ASSERT_EQ( level.GetAppearance().GetColor(), 0 );

    level.GetAppearanceR().SetColor(1);
    ASSERT_EQ( project->Levels().UpdateLevel(level), BE_SQLITE_OK );

    DgnLevelTableRow levelAfterChange = project->Levels().QueryLevelByName ("Default");

    ASSERT_EQ( level.GetAppearance().GetColor(), 1 );
    }


/*---------------------------------------------------------------------------------**//**
* Work with Levels Table
* @bsimethod                                    Majd.Uddin                   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DgnLevelTableTests, WorkWithLevels)
    {
    ScopedDgnHost autoDgnHost;

    DgnDbTestDgnManager tdm (L"ElementsSymbologyByLevel.idgndb", __FILE__, OPENMODE_READONLY);
    DgnDbP project = tdm.GetDgnProjectP();
    ASSERT_TRUE( project != NULL);

    auto const& levelTable = project->Levels();
    DgnLevels::Iterator levelIter = levelTable.MakeIterator();
    ASSERT_EQ (4, levelIter.QueryCount() ) << "The Levels count should be 5 but it is: "<< levelIter.QueryCount();
    
    TestLevelProperties levels[5], testLevel;
    levels[0].SetTestLevelProperties (L"Level1", 1, 1 ,1);
    levels[1].SetTestLevelProperties (L"Level2", 3, 3, 3);
    levels[2].SetTestLevelProperties (L"Default", 0, 0, 0);
    levels[3].SetTestLevelProperties (L"master", 1, 0, 0);

    int i = 0;
    for (auto const & entry : levelIter)
        {
        WString entryNameW (entry.GetName(), true); // string conversion
        testLevel.SetTestLevelProperties (entryNameW.c_str(), entry.GetAppearance().GetColor(), entry.GetAppearance().GetWeight(), entry.GetAppearance().GetStyle());
        testLevel.IsEqual (levels[i]);
        i++;
        }
    ASSERT_EQ (4, i);
    }

struct SetVt : BeSQLite::VirtualSet
{
    bset<uint32_t> m_set;
    virtual bool _IsInSet (int nVals, DbValue const* vals) const override {BeAssert(nVals==1); return m_set.end() != m_set.find(vals[0].GetValueInt());}
};

/*---------------------------------------------------------------------------------**//**
* Test query level id by name
* @bsimethod                                    Algirdas.Mikoliunas          01/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DgnLevelTableTests, QueryLevelIdFromTable)
    {
    ScopedDgnHost autoDgnHost;

    DgnDbTestDgnManager tdm (L"SiteLayout.i.idgndb", __FILE__, OPENMODE_READWRITE);
    DgnDbP project = tdm.GetDgnProjectP();
    ASSERT_TRUE(project != NULL);

    auto const& levelTable = project->Levels();
    LevelId levelId = levelTable.QueryLevelId("ECT_PAGE_LINENUMBER_REGION");
    EXPECT_EQ(65, levelId.GetValue()) << "Retrieved level id is wrong";

    SetVt idSet;
    idSet.m_set.insert(20);
    idSet.m_set.insert(74);
    idSet.m_set.insert(84);
    idSet.m_set.insert(64);

    SetVt styleSet;
    styleSet.m_set.insert(10);
    styleSet.m_set.insert(1);

    Statement stmt;
    DbResult rc = stmt.Prepare (*project, "SELECT Id FROM " DGNPROJECT_TABLE_Level " WHERE InVirtualSet(?,Id) or InVirtualSet(?,style)");
    BeAssert (rc==BE_SQLITE_OK);
    stmt.BindVirtualSet(1, idSet);
    stmt.BindVirtualSet(2, styleSet);
    int count=0;
    while (BE_SQLITE_ROW ==stmt.Step())
        ++count;

    EXPECT_EQ (count, 2);
    }

/*---------------------------------------------------------------------------------**//**
* Test query non existing level id by name
* @bsimethod                                    Algirdas.Mikoliunas          01/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DgnLevelTableTests, QueryNonExistingLevelIdFromTable)
    {
    ScopedDgnHost autoDgnHost;

    DgnDbTestDgnManager tdm (L"SiteLayout.i.idgndb", __FILE__, OPENMODE_READWRITE);
    DgnDbP project = tdm.GetDgnProjectP();
    ASSERT_TRUE(project != NULL);

    auto const& levelTable = project->Levels();
    LevelId levelId = levelTable.QueryLevelId("TestLevel");
    EXPECT_FALSE(levelId.IsValid()) << "Retrieved level id must be invalid";
    }
    
/*---------------------------------------------------------------------------------**//**
* Test query level id
* @bsimethod                                    Algirdas.Mikoliunas          01/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DgnLevelTableTests, LevelsAndFacets)
    {
    ScopedDgnHost autoDgnHost;
    Utf8CP nameValue = "TestLevel";
    Utf8CP descriptionValue = "TestDescription";

    DgnDbTestDgnManager tdm (L"SiteLayout.i.idgndb", __FILE__, OPENMODE_READWRITE);
    DgnDbP project = tdm.GetDgnProjectP();
    ASSERT_TRUE(project != NULL);

    auto& levelTable = project->Levels();
    DgnLevels::Row::Flags flags;
    DgnLevels::Appearance graphics (1, 2, 3, DgnMaterialId(4), 5.0f, 6);
    DgnLevels::Row newLevel (LevelId(1000), nameValue, DgnLevels::Scope::Physical, flags, graphics, descriptionValue);

    EXPECT_EQ(BE_SQLITE_OK, levelTable.InsertLevel(newLevel));

    DgnLevels::Row level = levelTable.QueryLevelById(LevelId(1000));
    
    EXPECT_STREQ(nameValue, level.GetName());
    EXPECT_STREQ(descriptionValue, level.GetDescription()); // Retrieved level description must be TestDescription
    EXPECT_TRUE(newLevel.EqualLevelData(level)); //Inserted and retrieved level's data must be identicial

    // test invalid level names
    {
    BeTest::SetFailOnAssert(false);
    DgnLevels::Row badLevel (LevelId(10001), "invalid<>level", DgnLevels::Scope::Physical);
    EXPECT_NE(BE_SQLITE_OK, levelTable.InsertLevel(badLevel));

    badLevel.SetName (" space at begin");
    EXPECT_NE(BE_SQLITE_OK, levelTable.InsertLevel(badLevel));

    badLevel.SetName ("space at end ");
    EXPECT_NE(BE_SQLITE_OK, levelTable.InsertLevel(badLevel));

    badLevel.SetName ("    ");
    EXPECT_NE(BE_SQLITE_OK, levelTable.InsertLevel(badLevel));

    badLevel.SetName ("");
    EXPECT_NE(BE_SQLITE_OK, levelTable.InsertLevel(badLevel));
    BeTest::SetFailOnAssert(true);
    }

    // test level facets
    static Utf8CP facet1Name = "Facet1";
    static Utf8CP facet2Name = "Facet2";
    DgnLevels::Appearance facetAppear (10, 20, 30, DgnMaterialId(), 5.0f, 6);
    DgnLevels::Facet facet (level.GetLevelId(), LevelFacetId(200), facet1Name, facetAppear);
    EXPECT_EQ(BE_SQLITE_OK, levelTable.InsertFacet(facet));
    EXPECT_NE(BE_SQLITE_OK, levelTable.InsertFacet(facet)); // should not be able to insert with duplicate name and id

    facet.SetName (facet2Name);
    EXPECT_NE(BE_SQLITE_OK, levelTable.InsertFacet(facet)); // not if duplicate id but different name

    DgnLevels::Facet badLevelFacet (LevelId(10001), LevelFacetId(201), facet2Name, facetAppear, "description 2");
    EXPECT_NE(BE_SQLITE_OK, levelTable.InsertFacet(badLevelFacet)); // level must be exist

    DgnLevels::Facet facet2 (level.GetLevelId(), LevelFacetId(201), facet2Name, facetAppear, "description 2");
    EXPECT_EQ(BE_SQLITE_OK, levelTable.InsertFacet(facet2)); // insert a second facet on same level

    DgnLevels::Facet facetOut = levelTable.QueryFacetById(level.GetLevelId(), facet2.GetId());
    ASSERT_TRUE(facetOut.IsValid());

    // make sure we can look up a facet by name
    LevelFacetId testId = levelTable.QueryFacetId(level.GetLevelId(), facet2Name);
    ASSERT_TRUE (testId == facet2.GetId());

    // make sure we return an invalid id for not found facets.
    testId = levelTable.QueryFacetId(level.GetLevelId(), "not found");
    ASSERT_TRUE (!testId.IsValid());

    // change the color of a facet
    facetOut.GetAppearanceR().SetColor(33);
    EXPECT_EQ(33, facetOut.GetAppearance().GetColor());
    EXPECT_EQ(BE_SQLITE_OK, levelTable.UpdateFacet(facetOut));
    
    // make sure we get the same value back. This time query by name.
    Utf8String facet2Upper(facet2Name);
    facet2Upper.ToLower(); // make sure facet names are not case sensitive
    DgnLevels::Facet facetOut2 = levelTable.QueryFacetByName(level.GetLevelId(), facet2Upper.c_str()); 
    ASSERT_TRUE(facetOut2.IsValid());
    ASSERT_TRUE(facetOut2.GetAppearance().IsEqual(facetOut.GetAppearance()));

    LevelFacetId highest = levelTable.QueryHighestFacetId(level.GetLevelId());
    EXPECT_EQ (201, highest.GetValue());

    // iterate over facets of this level
    int count=0;
    for (auto const& facet : levelTable.MakeFacetIterator(level.GetLevelId()))
        {
        DgnLevels::Appearance appear;
        appear = facet.GetAppearance();
        EXPECT_EQ (5.0, appear.GetTransparency()); // we added 2 facets, but they each have the same transparency
        ++count;
        }
    EXPECT_EQ (2, count);

    // iterate over a level with no facets
    count=0;
    for (auto const& facet : levelTable.MakeFacetIterator(LevelId(1002)))
        {
        DgnLevels::Appearance appear;
        appear = facet.GetAppearance();
        EXPECT_EQ (5.0, appear.GetTransparency());
        ++count;
        }
    EXPECT_EQ (0, count); // we shouldn't find any

    // delete a facet
    EXPECT_EQ(BE_SQLITE_OK, levelTable.DeleteFacet(level.GetLevelId(), facetOut.GetId()));
    facetOut = levelTable.QueryFacetById(level.GetLevelId(), facetOut.GetId());
    ASSERT_TRUE(!facetOut.IsValid());     // it has been deleted, we shouldn't find it
    }

/*---------------------------------------------------------------------------------**//**
* Test insert level with existing name
* @bsimethod                                    Algirdas.Mikoliunas          01/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DgnLevelTableTests, InsertLevelExistingNameToTable)
    {
    ScopedDgnHost autoDgnHost;
    Utf8CP nameValue = "ECT_PAGE_LINENUMBER_REGION";
    Utf8CP descriptionValue = "TestDescription";

    DgnDbTestDgnManager tdm (L"SiteLayout.i.idgndb", __FILE__, OPENMODE_READWRITE);
    DgnDbP project = tdm.GetDgnProjectP();
    ASSERT_TRUE(project != NULL);

    auto& levelTable = project->Levels();
    DgnLevels::Row::Flags flags;
    DgnLevels::Appearance graphics (1, 2, 3, DgnMaterialId(4), 5.0f, 6);
    DgnLevels::Row newLevel (LevelId(1000), nameValue, DgnLevels::Scope::Physical, flags, graphics, descriptionValue);

    EXPECT_NE(BE_SQLITE_OK, levelTable.InsertLevel(newLevel)) << "Method InsertLevel should not return BE_SQLITE_OK, when inserting level with existing name";
    }

/*---------------------------------------------------------------------------------**//**
* Test delete level
* @bsimethod                                    Algirdas.Mikoliunas          01/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DgnLevelTableTests, DeleteLevelFromTable)
    {
    ScopedDgnHost autoDgnHost;

    DgnDbTestDgnManager tdm (L"SiteLayout.i.idgndb", __FILE__, OPENMODE_READWRITE);
    DgnDbP project = tdm.GetDgnProjectP();
    ASSERT_TRUE(project != NULL);

    auto& levelTable = project->Levels();
    DgnLevels::Row level1 = levelTable.QueryLevelByName("ECT_PAGE_LINENUMBER_REGION");
    EXPECT_TRUE(level1.IsValid());

    EXPECT_EQ(BE_SQLITE_OK, levelTable.DeleteLevel(level1.GetLevelId())) << "Method DeleteLevel should return BE_SQLITE_OK";

    DgnLevels::Row level = levelTable.QueryLevelById(level1.GetLevelId());
    EXPECT_FALSE(level.IsValid()) << "Deleted level should be not valid";

    EXPECT_NE(BE_SQLITE_OK, levelTable.DeleteLevel(LEVEL_DEFAULT_LEVEL_ID)) << "Should not be able to delete the default level";
    }

/*---------------------------------------------------------------------------------**//**
* Test update level
* @bsimethod                                    Algirdas.Mikoliunas          01/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DgnLevelTableTests, UpdateLevelInTable)
    {
    ScopedDgnHost autoDgnHost;

    DgnDbTestDgnManager tdm (L"SiteLayout.i.idgndb", __FILE__, OPENMODE_READWRITE);
    DgnDbP project = tdm.GetDgnProjectP();
    ASSERT_TRUE(project != NULL);

    auto& levelTable = project->Levels();
    DgnLevels::Row level = levelTable.QueryLevelByName("ECT_PAGE_LINENUMBER_REGION");
    EXPECT_TRUE(level.IsValid()) << "Retrieved level should be valid";

    level.SetName("TestLevel");
    level.GetAppearanceR().SetColor(15);
    level.GetAppearanceR().SetWeight(20);
    level.GetAppearanceR().SetStyle(1);
    level.GetAppearanceR().SetMaterial(DgnMaterialId(1));
    level.GetAppearanceR().SetDisplayPriority(1);
    level.GetAppearanceR().SetTransparency(0.5f);

    EXPECT_EQ(BE_SQLITE_OK, levelTable.UpdateLevel(level)) << "Method UpdateLevel should return BE_SQLITE_OK";

    DgnLevels::Row updatedLevel = levelTable.QueryLevelByName("TestLevel");
    EXPECT_TRUE(updatedLevel.IsValid()) << "Updated level should be valid";
    EXPECT_TRUE(updatedLevel.EqualLevelData(level)) << "Update data and updated entry should match";
    }

/*---------------------------------------------------------------------------------**//**
* Test update level color
* @bsimethod                                    Algirdas.Mikoliunas          01/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DgnLevelTableTests, UpdateLevelColor)
    {
    ScopedDgnHost autoDgnHost;

    DgnDbTestDgnManager tdm (L"SiteLayout.i.idgndb", __FILE__, OPENMODE_READWRITE);
    DgnDbP project = tdm.GetDgnProjectP();
    ASSERT_TRUE(project != NULL);

    auto& levelTable = project->Levels();
    DgnLevels::Row level = levelTable.QueryLevelByName("ECT_PAGE_LINENUMBER_REGION");
    EXPECT_TRUE(level.IsValid()) << "Retrieved level should be valid";

    level.GetAppearanceR().SetColor(15);
    EXPECT_EQ(BE_SQLITE_OK, levelTable.UpdateLevel(level)) << "Method UpdateLevel should return BE_SQLITE_OK";

    DgnLevels::Row updatedLevel = levelTable.QueryLevelByName("ECT_PAGE_LINENUMBER_REGION");
    EXPECT_TRUE(updatedLevel.IsValid()) << "Updated level should be valid";
    EXPECT_EQ(15, updatedLevel.GetAppearance().GetColor()) << "Updated level color must be equal to 15";
    }

/*---------------------------------------------------------------------------------**//**
* Test update level with existing name
* @bsimethod                                    Algirdas.Mikoliunas          01/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DgnLevelTableTests, UpdateLevelWithExistingName)
    {
    ScopedDgnHost autoDgnHost;

    DgnDbTestDgnManager tdm (L"SiteLayout.i.idgndb", __FILE__, OPENMODE_READWRITE);
    DgnDbP project = tdm.GetDgnProjectP();
    ASSERT_TRUE(project != NULL);

    auto& levelTable = project->Levels();
    DgnLevels::Row level = levelTable.QueryLevelByName("ECT_PAGE_LINENUMBER_REGION");
    EXPECT_TRUE(level.IsValid()) << "Retrieved level should be valid";
    
    level.SetName("Panel");
    BeSQLite::DbResult updateResult;
    
    try
    {
        updateResult = levelTable.UpdateLevel(level);
    }
    catch(wchar_t const*) 
        {
        updateResult = BE_SQLITE_ERROR;
        }
    
    EXPECT_EQ(BE_SQLITE_OK, updateResult) << "Method UpdateLevel should return BE_SQLITE_OK, when updating element with already existing name. This update just changes the name.";

    EXPECT_TRUE(levelTable.QueryLevelByName("Panel").IsValid()) << "Updated level should be valid";
    EXPECT_FALSE(levelTable.QueryLevelByName("ECT_PAGE_LINENUMBER_REGION").IsValid()) << "Level with old name should not be valid";
    }

/*---------------------------------------------------------------------------------**//**
* Test iterator with filter
* @bsimethod                                    Keith.Bentley                   11/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DgnLevelTableTests, LevelIterFilter)
    {
    ScopedDgnHost autoDgnHost;

    DgnDbTestDgnManager tdm (L"SiteLayout.i.idgndb", __FILE__, OPENMODE_READWRITE);
    DgnDbP project = tdm.GetDgnProjectP();
    ASSERT_TRUE(project != NULL);

    DgnLevels::Iterator levelIter = project->Levels().MakeIterator("WHERE Name LIKE @filter");

    BeSQLite::NamedParams params;
    params.AddStringParameter ("@filter", "%dgrm%");
    levelIter.SetParams(&params);

    int i=0;
    for (auto const& level : levelIter)
        {
        Utf8String name (level.GetName());
        name.ToLower();
        EXPECT_FALSE (Utf8String::npos == name.find_first_of("dgrm")) << "found wrong level name";
        ++i;
        }

    EXPECT_EQ (i, 1);
    }

/*---------------------------------------------------------------------------------**//**
* Test get highest id
* @bsimethod                                    Algirdas.Mikoliunas          01/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DgnLevelTableTests, GetHighestLevelId)
    {
    ScopedDgnHost autoDgnHost;

    DgnDbTestDgnManager tdm (L"SiteLayout.i.idgndb", __FILE__, OPENMODE_READWRITE);
    DgnDbP project = tdm.GetDgnProjectP();
    ASSERT_TRUE(project != NULL);

    auto const& levelTable = project->Levels();
    EXPECT_EQ(69, levelTable.QueryHighestId().GetValue());
    }

/*---------------------------------------------------------------------------------**//**
* Test get iterator row properties
* @bsimethod                                    Algirdas.Mikoliunas          02/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DgnLevelTableTests, GetIteratorRowProperties)
    {
    ScopedDgnHost autoDgnHost;

    DgnDbTestDgnManager tdm (L"SiteLayout.i.idgndb", __FILE__, OPENMODE_READWRITE);
    DgnDbP project = tdm.GetDgnProjectP();
    ASSERT_TRUE(project != NULL);

    DgnLevels::Iterator iter = DgnLevels::Iterator (*project, " WHERE Name = 'ECT_PAGE_LINENUMBER_REGION'");
    DgnLevels::Iterator::Entry entry = iter.begin();
    
    EXPECT_TRUE(entry.ToRow().IsValid());
    EXPECT_EQ(0, entry.GetAppearance().GetDisplayPriority());
    EXPECT_EQ(-1, entry.GetAppearance().GetMaterial().GetValueUnchecked());
    }

#endif
    