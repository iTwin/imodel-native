/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ConverterTestsBaseFixture.h"

//----------------------------------------------------------------------------------------
// @bsiclass                                    Umar.Hayat                      08/15
//----------------------------------------------------------------------------------------
struct LevelTests : public ConverterTestBaseFixture
{
    void TestLevelConversion(size_t nModelsExpected, size_t nCategoriesExpected, size_t nSubCategoriesExpected, bool expectUncategorized);
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(LevelTests, NoLevelChangesInTile)
    {
    m_opts.m_useTiledConverter = true;
    LineUpFiles(L"NoLevelChangesInTile.bim", L"Test3d.dgn", true); // creates TestAddRef.bim from Test3d.dgn and defines m_dgnDbFileName, and m_v8FileName
    ASSERT_EQ( 0 , m_count ) << L"The initial V8 file is supposed to be empty!";

    BentleyApi::BeFileName tileFile;
    MakeCopyOfFile(tileFile, L"-Tile1");

    m_opts.m_tiles.push_back(tileFile);
//    m_opts.m_expectModelToBeMappedIn = false; // *** WIP_SYNCINFO -- We defined the syncinfo "Model" table such that there can be only 1 V8 model mapped to a given DgnDb model, so we can't map all tiles to the one target model

    if (true)
        {
        V8FileEditor v8editor;
        v8editor.Open(tileFile);
        DgnV8Api::FileLevelCache& lc = v8editor.m_file->GetLevelCacheR();
        ASSERT_TRUE( lc.CreateLevel(L"New level", LEVEL_NULL_CODE, DGNV8_LEVEL_NULL_ID).IsValid() );
        ASSERT_EQ( DgnV8Api::LevelCacheErrorCode::None , lc.Write() );
        }

    DoUpdate(m_dgnDbFileName, m_v8FileName, false, false); // update should fail because of the level change in the tile
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/15
+---------------+---------------+---------------+---------------+---------------+------*/
void LevelTests::TestLevelConversion(size_t numGeometricModelsExpected, size_t nCategoriesExpected, size_t nSubCategoriesExpected, bool expectUncategorized)
    {
    DoConvert(m_dgnDbFileName, m_v8FileName);

    DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
    ASSERT_TRUE(db.IsValid());
    ASSERT_EQ(numGeometricModelsExpected, CountGeometricModels(*db));

    DgnCategoryIdSet categories = SpatialCategory::MakeIterator(*db).BuildIdSet<DgnCategoryId>();
    DgnCategoryId uncategorizedId = SpatialCategory::QueryCategoryId(*GetJobDefinitionModel(*db), "Default");
    ASSERT_EQ(expectUncategorized , categories.Contains(uncategorizedId));
    ASSERT_EQ(nCategoriesExpected, categories.size());

    size_t subCategoryCount = 0;
    for (ElementIteratorEntry categoryEntry : SpatialCategory::MakeIterator(*db))
        {
        ASSERT_TRUE(categories.Contains(categoryEntry.GetId<DgnCategoryId>()));
        for (ElementIteratorEntry subCategoryEntry : DgnSubCategory::MakeIterator(*db, categoryEntry.GetId<DgnCategoryId>()))
            {
            ++subCategoryCount;
            }
        }

    ASSERT_EQ( nSubCategoriesExpected, subCategoryCount );
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(LevelTests, LevelMerge1)
    {
    LineUpFiles(L"LevelMerge1.bim", L"Test3d.dgn", false);
    TestLevelConversion (2, 1, 1, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(LevelTests, LevelMergeFromAttachments_CopyIfDifferent1)
    {
    LineUpFiles(L"LevelMergeFromAttachments_CopyIfDifferent.bim", L"Test3d.dgn", false);

    AddV8Level("Level1"); // Create a level (in addition to the default level)

    BentleyApi::BeFileName refV8File;
    CreateAndAddV8Attachment(refV8File,1);  // Create a copy of Test3d.dgn and attach it. Note that it will have the same level table.

    m_params.SetCopyLevel(Converter::Params::CopyLevel::IfDifferent);

    // Expect:
    //  Uncategorized
    //      Default
    //  Level1
    //      Default
    TestLevelConversion(3, 2, 2, true); // We expect the "default" level to be defined differently everywhere, but not Level1.
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(LevelTests, LevelMergeFromAttachments_CopyIfDifferent12)
    {
    LineUpFiles(L"LevelMergeFromAttachments_CopyIfDifferent.bim", L"Test3d.dgn", false);

    AddV8Level("Level1"); // Create a level (in addition to the default level)

    // Put an element on Level1. Note that converter will not make copies of unused levels.
    if (true)
        {
        V8FileEditor v8editor;
        v8editor.Open(m_v8FileName);
        v8editor.SetActiveLevel(v8editor.GetLevelByName(L"Level1"));
        v8editor.AddLine(nullptr);
        }

    BentleyApi::BeFileName refV8File;
    CreateAndAddV8Attachment(refV8File,1);  // Create a copy of Test3d.dgn and attach it. Note that it will have the same level table.

    SetV8LevelColor("Level1", 1, m_v8FileName);
    SetV8LevelColor("Level1", 2, refV8File);

    m_params.SetCopyLevel(Converter::Params::CopyLevel::IfDifferent);

    // Expect:
    //  Uncategorized
    //      Default
    //  Level1
    //      Default
    //      Ref1
    TestLevelConversion(3, 2, 3, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(LevelTests, LevelMergeFromAttachments_CopyIfDifferent2)
    {
    LineUpFiles(L"LevelMergeFromAttachments_CopyIfDifferent.bim", L"Test3d.dgn", false);

    BentleyApi::BeFileName refV8File;
    CreateAndAddV8Attachment(refV8File,1);
    CreateAndAddV8Attachment(refV8File,2);

    m_params.SetCopyLevel(Converter::Params::CopyLevel::IfDifferent);

    // All of the files just have the default level. We don't modify them.  So, they should all be mapped to Uncategorized (with the same appearance).
    // Expect:
    //  Uncategorized
    //      Default
    TestLevelConversion(4, 1, 1, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(LevelTests, LevelMergeFromAttachments_CopyNever)
    {
    LineUpFiles(L"LevelMergeFromAttachments_CopyNever.bim", L"Test3d.dgn", false);

    BentleyApi::BeFileName refV8File;
    CreateAndAddV8Attachment(refV8File,1);
    CreateAndAddV8Attachment(refV8File,2);

    ASSERT_EQ( m_params.GetCopyLevel() , Converter::Params::CopyLevel::UseConfig ); // the config file should specify "Never"

    TestLevelConversion(4, 1, 1, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(LevelTests, LevelMergeFromAttachments_Always)
    {
    LineUpFiles(L"LevelMergeFromAttachments_Always.bim", L"Test3d.dgn", false);

    // Put an element on default level. Note that converter will not make copies of unused levels.
    if (true)
        {
        V8FileEditor v8editor;
        v8editor.Open(m_v8FileName);
        v8editor.AddLine(nullptr);
        }

    BentleyApi::BeFileName refV8File;
    CreateAndAddV8Attachment(refV8File,1);
    CreateAndAddV8Attachment(refV8File,2);

    m_params.SetCopyLevel(Converter::Params::CopyLevel::Always);

    TestLevelConversion(4, 1, 3, true);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/15
+---------------+---------------+---------------+---------------+---------------+------*/
#ifdef WIP_UPDATER // *** think about level update -- allow or forbid?
TEST_F(LevelTests, LevelMerge_FailUpdateOnLevelChange)
    {
    LineUpFiles(L"LevelMerge_FailUpdateOnLevelChange.bim", L"Test3d.dgn", false);

    AddV8Level("Level1"); // Create a level (in addition to the default level)

    DoConvert(m_dgnDbFileName, m_v8FileName);

    SetV8LevelColor("Level1", 1);

    DoUpdate(m_dgnDbFileName, m_v8FileName, true);
    }
#endif
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ridha.Malik                      05/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(LevelTests, LevelMaskfromSheetAttachment_toViewAttachment)
    {
    LineUpFiles(L"LevelMaskfromSheetAttachment_toViewAttachment.bim", L"Test3d.dgn", false); // defines m_dgnDbFileName, and m_v8FileName
    AddV8Level("Level1");
    AddV8Level("Level2");
    AddV8Level("Level3");
    if (true)
        {
        V8FileEditor v8editor;
        v8editor.Open(m_v8FileName);
        DgnV8Api::DgnModelStatus modelStatus;
        DgnV8Api::ElementId eid;
        // Put a line in the 3D model
        Bentley::DgnModelP threeDModel = v8editor.m_defaultModel;
        ASSERT_TRUE(threeDModel->Is3d());
        v8editor.AddLine(&eid, threeDModel);
        DgnV8Api::EditElementHandle eh1(eid, threeDModel);
        Bentley::EditElementHandleR eh1r = eh1;
        DgnV8Api::ElementPropertiesSetterPtr eProps1 = DgnV8Api::ElementPropertiesSetter::Create();
        eProps1->SetLevel(v8editor.m_file->GetLevelCacheR().GetLevelByName(L"Level1").GetLevelId());
        EXPECT_EQ(true, eProps1->DgnV8Api::ElementPropertiesSetter::Apply(eh1r));
        eh1r.AddToModel();

        // Create a drawing1 model ...
        Bentley::DgnModelP drawingModel = v8editor.m_file->CreateNewModel(&modelStatus, L"Drawing1", DgnV8Api::DgnModelType::Normal, /*is3D*/ false);
        EXPECT_TRUE(DgnV8Api::DGNMODEL_STATUS_Success == modelStatus);
        DgnV8Api::ElementId eid2;
        v8editor.AddLine(&eid2, drawingModel);
        DgnV8Api::EditElementHandle eh2(eid2, drawingModel);
        Bentley::EditElementHandleR eh2r = eh2;
        DgnV8Api::ElementPropertiesSetterPtr eProps2 = DgnV8Api::ElementPropertiesSetter::Create();
        eProps2->SetLevel(v8editor.m_file->GetLevelCacheR().GetLevelByName(L"Level2").GetLevelId());
        EXPECT_EQ(true, eProps2->DgnV8Api::ElementPropertiesSetter::Apply(eh2r));
        eh2r.AddToModel();
        Bentley::DgnDocumentMonikerPtr moniker = DgnV8Api::DgnDocumentMoniker::CreateFromFileName(m_v8FileName.c_str());
        DgnV8Api::DgnAttachment* attachment;
        ASSERT_EQ(BentleyApi::SUCCESS, drawingModel->CreateDgnAttachment(attachment, *moniker, threeDModel->GetModelName(), true));
        ASSERT_EQ(BentleyApi::SUCCESS, attachment->WriteToModel());

        // Create a SheetModel1 ...
        Bentley::DgnModelP SheetModel = v8editor.m_file->CreateNewModel(&modelStatus, L"sheet1", DgnV8Api::DgnModelType::Sheet, /*is3D*/ false);
        EXPECT_TRUE(DgnV8Api::DGNMODEL_STATUS_Success == modelStatus);
        // and attach the 2D drawing as a reference to the 2D sheet1 
        Bentley::DgnDocumentMonikerPtr moniker2 = DgnV8Api::DgnDocumentMoniker::CreateFromFileName(m_v8FileName.c_str());
        DgnV8Api::DgnAttachment* attachment2;
        ASSERT_EQ(BentleyApi::SUCCESS, SheetModel->CreateDgnAttachment(attachment2, *moniker, L"Drawing1", true));
        ASSERT_EQ(BentleyApi::SUCCESS, attachment2->WriteToModel());
        // Put a line in the 2D sheet model
        DgnV8Api::ElementId eid3;
        v8editor.AddLine(&eid3, SheetModel, DPoint3d::From(10, 10, 0));
        DgnV8Api::EditElementHandle eh3(eid3, SheetModel);
        Bentley::EditElementHandleR eh3r = eh3;
        DgnV8Api::ElementPropertiesSetterPtr eProps3 = DgnV8Api::ElementPropertiesSetter::Create();
        eProps3->SetLevel(v8editor.m_file->GetLevelCacheR().GetLevelByName(L"Level3").GetLevelId());
        EXPECT_EQ(true, eProps3->DgnV8Api::ElementPropertiesSetter::Apply(eh3r));
        eh3r.AddToModel();
        v8editor.Save();
        }

    m_params.SetCopyLevel(Converter::Params::CopyLevel::IfDifferent);

    TestLevelConversion(4, 4, 4, true);
    }