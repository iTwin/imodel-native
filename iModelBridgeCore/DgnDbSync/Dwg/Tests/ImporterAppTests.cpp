/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "ImporterBaseFixture.h"
#include "ImporterCommandBuilder.h"

/*================================================================================**//**
* @bsiclass                                                     Umar Hayat      05/16
+===============+===============+===============+===============+===============+======*/
struct ImporterAppTests : public ImporterTests, public ImporterCommandBuilder
{
    BentleyApi::StatusInt RunCMD(BentleyApi::WString);
};

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Umar Hayat      05/16
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt ImporterAppTests::RunCMD(BentleyApi::WString cmd)
    {
    wprintf(L"Command to Run = %ls \n" , cmd.c_str());
    char *cmdLocal = new char[cmd.GetMaxLocaleCharBytes()];
    cmd.ConvertToLocaleChars (cmdLocal);
    int status = system(cmdLocal);
    printf("status = %d \n" , status);
    delete cmdLocal;
    return status == 0 ? SUCCESS: ERROR ;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Umar Hayat      05/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ImporterAppTests, createIBim)
    {
    WString fileName = L"basictype.dwg";
    BeFileName inFile;
    MakeWritableCopyOf(inFile, fileName.c_str());
    createCommand();
    addInputFile(inFile.c_str());
    addOutputFile(GetOutputDir() );
#ifndef WIP_GENRATE_THUMBNAILS
    // don't generate thumbnails for now
    addNoThumbnails();
#endif
    ASSERT_EQ( SUCCESS, RunCMD(m_command));

    BeFileName outFile = GetIBimFileName(inFile);
    
    EXPECT_PRESENT(outFile.c_str())
    EXPECT_PRESENT(inFile.c_str() )
    }
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Umar Hayat      05/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ImporterAppTests, createIBIMandIModel)
    {
    WString fileName = L"basictype.dwg";
    BeFileName inFile;
    MakeWritableCopyOf(inFile, fileName.c_str());
    createCommand();
    addInputFile(inFile.c_str());
    addOutputFile(GetOutputDir());
    addCompressFlag();
#ifndef WIP_GENRATE_THUMBNAILS
    // don't generate thumbnails for now
    addNoThumbnails();
#endif
    ASSERT_EQ(SUCCESS, RunCMD(m_command));

    BeFileName outFile = GetIBimFileName(inFile);

    EXPECT_PRESENT(outFile.c_str())
    EXPECT_PRESENT(inFile.c_str())
    }
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Umar Hayat      05/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ImporterAppTests, createIBIMandIModelFromDxf)
    {
    WString fileName = L"tr135537.dxf";
    BeFileName inFile;
    MakeWritableCopyOf(inFile, fileName.c_str());
    createCommand();
    addInputFile(inFile.c_str());
    addOutputFile(GetOutputDir());
    addCompressFlag();
#ifndef WIP_GENRATE_THUMBNAILS
    // don't generate thumbnails for now
    addNoThumbnails();
#endif
    ASSERT_EQ(SUCCESS, RunCMD(m_command));

    BeFileName outFile = GetIBimFileName(inFile);

    EXPECT_PRESENT(outFile.c_str())
    EXPECT_PRESENT(inFile.c_str())
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Umar Hayat      05/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ImporterAppTests, Description)
    {
    WString fileName = L"basictype.dwg";
    BeFileName inFile;
    MakeWritableCopyOf(inFile, fileName.c_str());
    createCommand();
    addInputFile(inFile.c_str());
    addOutputFile(GetOutputDir());
    addDescription(L"TestDescription");
#ifndef WIP_GENRATE_THUMBNAILS
    // don't generate thumbnails for now
    addNoThumbnails();
#endif
    ASSERT_EQ(SUCCESS, RunCMD(m_command));

    BeFileName outFile = GetIBimFileName(inFile);

    EXPECT_PRESENT(outFile.c_str())
    EXPECT_PRESENT(inFile.c_str())

    DgnDbPtr db = OpenExistingDgnDb(outFile);
    ASSERT_TRUE(db.IsValid());
    BentleyApi::Utf8String description;
#ifdef FILE_DESCRIPTION
    db->QueryProperty(description, DgnProjectProperty::Description());
#else   // root subject decscription
    SubjectCPtr rootSubject = db->Elements().GetRootSubject ();
    if (rootSubject.IsValid())
        description = rootSubject->GetDescription ();
#endif

    EXPECT_TRUE(description.CompareTo("TestDescription") == 0) << "Description does not match";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ImporterAppTests, BudweiserBenchmarks)
    {
    WString fileName = L"budweiser2018.dwg";
    BeFileName inFile;
    MakeWritableCopyOf(inFile, fileName.c_str());
    createCommand();
    addInputFile(inFile.c_str());
    addOutputFile(GetOutputDir());
    addCompressFlag();
#ifndef WIP_GENRATE_THUMBNAILS
    // don't generate thumbnails for now
    addNoThumbnails();
#endif
    ASSERT_EQ(SUCCESS, RunCMD(m_command));

    BeFileName outFile = GetIBimFileName(inFile);

    EXPECT_PRESENT(outFile.c_str())
    EXPECT_PRESENT(inFile.c_str())

    DgnDbPtr db = OpenExistingDgnDb(outFile);
    ASSERT_TRUE(db.IsValid());

    // check physical models:
    size_t  count = 0;
    DgnModels&  models = db->Models ();
    for (auto const& entry : models.MakeIterator(BIS_SCHEMA(BIS_CLASS_PhysicalModel)))
        {
        auto model = models.GetModel (entry.GetModelId());
        ASSERT_TRUE(model.IsValid());
        if (model->GetName().EqualsI("Model[budweiser2018]"))
            {
            size_t  elemCount = model->MakeIterator().BuildIdList<DgnElementId>().size ();
            // PRG & firebugs do not have RealDWG registries - subtract 7 AEC objects from the total count.
            EXPECT_GE(elemCount, 2010) << "Model[budweiser2018] should have 2010 or more elements!";
            }
        count++;
        }
    EXPECT_EQ(1, count) << "Should have exactly 1 physical model!";

    // check GeometryParts model
    count = 0;
    for (auto const& entry : models.MakeIterator(BIS_SCHEMA(BIS_CLASS_DefinitionModel)))
        {
        auto model = models.GetModel (entry.GetModelId());
        ASSERT_TRUE(model.IsValid());
        auto name = model->GetName ();
        if (name.StartsWithI("DwgGeometryParts"))
            count++;
        }
    EXPECT_EQ(1, count) << "Should always & only have 1 DwgGeometryParts model!";

    // check sheet models
    count = 0;
    for (auto const& entry : models.MakeIterator(BIS_SCHEMA(BIS_CLASS_SheetModel)))
        {
        auto model = models.GetModel (entry.GetModelId());
        ASSERT_TRUE(model.IsValid());
        size_t  elemCount = model->MakeIterator().BuildIdList<DgnElementId>().size ();
        // at least 1 element in each sheet
        EXPECT_GT(elemCount, 1);
        count++;
        }
    EXPECT_EQ(5, count) << "Should have 5 sheet models!";

    // 1 raster model
    count = models.MakeIterator(RASTER_SCHEMA(RASTER_CLASSNAME_RasterFileModel)).BuildIdSet().size ();
    EXPECT_EQ(1, count) << "Should have a raster model!";

    // count views
    count = ViewDefinition::QueryCount (*db);
    EXPECT_EQ(39, count) << "Should have 39 views!";
    // count spatial categories
    count = SpatialCategory::MakeIterator(*db).BuildIdSet<DgnCategoryId>().size ();
    EXPECT_EQ(117, count) << "Should have 117 spatial categories!";
    // count drawing categories
    count = DrawingCategory::MakeIterator(*db).BuildIdSet<DgnCategoryId>().size ();
    EXPECT_EQ(3, count) << "Should have 3 drawing categories!";
    // 7 fonts, but 1 comes from an AEC object which is not known on PRG or firebug boxes:
    count = db->Fonts().DbFontMap().MakeIterator().QueryCount ();
    EXPECT_LE(6, count) << "Should have 7 fonts!";
    // 13 linestyles
    count = 0;
    auto& lscache = db->LineStyles().GetCache ();
    for (auto iter = lscache.begin(); iter != lscache.end(); ++iter)
        count++;
    EXPECT_EQ(13, count) << "Should have 13 line styles!";
    // 2 materials
    count = 0;
    for (auto const& entry : RenderMaterial::MakeIterator(*db))
        count++;
    EXPECT_EQ(2, count) << "Should have 2 materials!";

    // Check benchmark #14: 2 ElementAspects for 2 attributes:
    count = 0;
    for (auto entry : db->Elements().MakeAspectIterator(BIS_SCHEMA(BIS_CLASS_ElementMultiAspect)))
        count++;
    EXPECT_EQ(2, count) << "Should have 2 element aspects!";

    // check the default view: it should be the modelspace viewport and should be a SpatialView named "Model - Active":
    DgnViewId   viewId = ViewDefinition::QueryDefaultViewId (*db);
    EXPECT_TRUE (viewId.IsValid()) << "The default view is not set!";
    ViewDefinitionCPtr  view = ViewDefinition::Get (*db, viewId);
    EXPECT_TRUE (view.IsValid()) << "The default view is invalid!";
    EXPECT_TRUE (view->IsSpatialView()) << "The default view is not a SpatialView!";
    Utf8String  name = view->GetName ();
    auto expected = BuildModelspaceModelname (inFile);
    EXPECT_TRUE (name.EqualsI(expected.c_str())) << "The default view should be \"Model[filename]\"!";
    }
