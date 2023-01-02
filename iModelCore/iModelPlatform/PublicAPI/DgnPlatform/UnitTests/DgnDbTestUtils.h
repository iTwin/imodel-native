/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

//=======================================================================================
// WARNING: Must be careful of dependencies within this file as it is also included by
// WARNING:   DgnDisplayTests, DgnClientFxTests, ConstructionPlanningTests, etc.
//=======================================================================================
#include <Bentley/BeTest.h>
#include <DgnPlatform/DgnCoreAPI.h>
#include <DgnPlatform/GenericDomain.h>
#include <DgnPlatform/PlatformLib.h>

#define MUST_HAVE_HOST(BAD_RETURN) if (nullptr == PlatformLib::QueryHost())\
        {\
        EXPECT_FALSE(true) << "Your SetUpTestCase function must set up a host. Just put an instance of ScopedDgnHost on the stack at the top of your function.";\
        return BAD_RETURN;\
        }

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//=======================================================================================
// @bsiclass
//=======================================================================================
struct DgnDbTestUtils : NonCopyableClass
{
public:
    //! Insert a PhysicalModel
    //! @note Also creates a PhysicalPartition element for the PhysicalModel to model
    //! @note No need for caller to assert a valid return (asserts within implementation)
    static PhysicalModelPtr InsertPhysicalModel(DgnDbR db, Utf8CP partitionName)
        {
        MUST_HAVE_HOST(nullptr);
        SubjectCPtr rootSubject = db.Elements().GetRootSubject();
        PhysicalPartitionCPtr partition = PhysicalPartition::CreateAndInsert(*rootSubject, partitionName);
        EXPECT_TRUE(partition.IsValid());
        PhysicalModelPtr model = PhysicalModel::CreateAndInsert(*partition);
        EXPECT_TRUE(model.IsValid());
        EXPECT_EQ(partition->GetSubModelId(), model->GetModelId());
        return model;
        }

    //! Insert a SpatialLocationModel
    //! @note Also creates a SpatialLocationPartition element for the SpatialLocationModel to model
    //! @note No need for caller to assert a valid return (asserts within implementation)
    static SpatialLocationModelPtr InsertSpatialLocationModel(DgnDbR, Utf8CP partitionName);

    //! Insert a DocumentListModel
    //! @note Also creates a DocumentPartition element for the DocumentListModel to model
    //! @note No need for caller to assert a valid return (asserts within implementation)
    static DocumentListModelPtr InsertDocumentListModel(DgnDbR, Utf8CP partitionName);

    //! Insert an InformationRecordModel
    //! @note Also creates an InformationRecordPartition element for the InformationRecordModel to model
    //! @note No need for caller to assert a valid return (asserts within implementation)
    static InformationRecordModelPtr InsertInformationRecordModel(DgnDbR, Utf8CP partitionName);

    //! Insert a Drawing element
    //! @note No need for caller to assert a valid return (asserts within implementation)
    static DrawingPtr InsertDrawing(DocumentListModelCR model, Utf8CP name);

    //! Insert a SectionDrawing element
    //! @note No need for caller to assert a valid return (asserts within implementation)
    static SectionDrawingPtr InsertSectionDrawing(DocumentListModelCR model, Utf8CP name);

    //! Insert a Sheet element
    //! @note No need for caller to assert a valid return (asserts within implementation)
    static Sheet::ElementPtr InsertSheet(DocumentListModelCR model, double scale, double height, double width, Utf8CP name);

    //! Insert a Sheet element
    //! @note No need for caller to assert a valid return (asserts within implementation)
    static Sheet::ElementPtr InsertSheet(DocumentListModelCR model, double scale, DgnElementId templateId, Utf8CP name);

    //! Insert a DrawingModel
    //! @note No need for caller to assert a valid return (asserts within implementation)
    static DrawingModelPtr InsertDrawingModel(DrawingCR);

    //! Insert a SheetModel
    //! @note No need for caller to assert a valid return (asserts within implementation)
    static Sheet::ModelPtr InsertSheetModel(Sheet::ElementCR);

    //! Insert a LinkModel
    //! @note Also creates an InformationPartitionElement for the DocumentListModel to model
    //! @note No need for caller to assert a valid return (asserts within implementation)
    static LinkModelPtr InsertLinkModel(DgnDbR, Utf8CP partitionName);

    //! Insert a DefinitionModel
    //! @note Also creates a DefinitionPartition for the DefinitionModel to model
    //! @note No need for caller to assert a valid return (asserts within implementation)
    static DefinitionModelPtr InsertDefinitionModel(DgnDbR, Utf8CP partitionName);

    //! Insert a GroupInformationModel
    //! @note Also creates a GroupInformationPartition for the GroupInformationModel to model
    //! @note No need for caller to assert a valid return (asserts within implementation)
    static GenericGroupModelPtr InsertGroupInformationModel(DgnDbR, Utf8CP partitionName);

    //! Create a Camera view of the specified SpatialModel
    static DgnViewId InsertCameraView(SpatialModelR model, Utf8CP viewName = nullptr, DRange3dCP viewVolume = nullptr,
        StandardView rot = StandardView::Iso, Render::RenderMode renderMode = Render::RenderMode::SmoothShade)
        {
        DgnDbR db = model.GetDgnDb();
        DefinitionModelR dictionary = db.GetDictionaryModel();
        ModelSelectorPtr modelSelector = new ModelSelector(dictionary, "");
        modelSelector->AddModel(model.GetModelId());

        SpatialViewDefinition viewDef(dictionary, viewName ? viewName : model.GetName(), *new CategorySelector(dictionary,""), *new DisplayStyle3d(dictionary,""), *modelSelector);

        for (ElementIteratorEntryCR categoryEntry : SpatialCategory::MakeIterator(db))
            viewDef.GetCategorySelector().AddCategory(categoryEntry.GetId<DgnCategoryId>());

        EXPECT_TRUE(viewDef.Insert().IsValid());
        return viewDef.GetViewId();
        }

    //! Insert a Drawing view
    static DrawingViewDefinitionPtr InsertDrawingView(DrawingModelR model, Utf8CP viewName=nullptr);

    //! Insert a new persistent modelselector
    static ModelSelectorCPtr InsertModelSelector(DgnDbR db, Utf8CP name, DgnModelId model);

    //! Insert a new DrawingCategory
    static DgnCategoryId InsertDrawingCategory(DgnDbR, Utf8CP categoryName, DgnSubCategory::Appearance const& appearance = DgnSubCategory::Appearance(), DgnCategory::Rank rank = DgnCategory::Rank::Application);
    //! Insert a new DrawingCategory
    static DgnCategoryId InsertDrawingCategory(DgnDbR, Utf8CP categoryName, ColorDefCR color, DgnCategory::Rank rank = DgnCategory::Rank::Application);
    //! Get the first DrawingCategory in the DgnDb
    //! @deprecated
    //! @note Instead of using this method you should explicitly insert a DrawingCategory as part of the test setup
    //! @see InsertDrawingCategory
    static DgnCategoryId GetFirstDrawingCategoryId(DgnDbR);

    //! Insert a new SpatialCategory
    static DgnCategoryId InsertSpatialCategory(DgnDbR db, Utf8CP categoryName, DgnSubCategory::Appearance const& appearance = DgnSubCategory::Appearance(), DgnCategory::Rank rank = DgnCategory::Rank::Application)
        {
        MUST_HAVE_HOST(DgnCategoryId());

        SpatialCategory category(db.GetDictionaryModel(), categoryName, rank);
        SpatialCategoryCPtr persistentCategory = category.Insert(appearance);
        EXPECT_TRUE(persistentCategory.IsValid());

        return persistentCategory.IsValid()? persistentCategory->GetCategoryId(): DgnCategoryId();
        }

    //! Insert a new SpatialCategory
    static DgnCategoryId InsertSpatialCategory(DgnDbR db, Utf8CP categoryName, ColorDefCR color, DgnCategory::Rank rank = DgnCategory::Rank::Application)
        {
        DgnSubCategory::Appearance appearance;
        appearance.SetColor(color);
        return InsertSpatialCategory(db, categoryName, appearance, rank);
        }

    //! Get the first SpatialCategory in the DgnDb
    //! @deprecated
    //! @note Instead of using this method you should explicitly insert a SpatialCategory as part of the test setup
    //! @see InsertSpatialCategory
    static DgnCategoryId GetFirstSpatialCategoryId(DgnDbR db)
        {
        MUST_HAVE_HOST(DgnCategoryId());
        DgnCategoryId categoryId = (*SpatialCategory::MakeIterator(db).begin()).GetId<DgnCategoryId>();
        EXPECT_TRUE(categoryId.IsValid());
        return categoryId;
        }

    //! Insert a new SubCategory
    static DgnSubCategoryId InsertSubCategory(DgnDbR, DgnCategoryId, Utf8CP, ColorDefCR);

    //! Insert a new CodeSpec
    static CodeSpecId InsertCodeSpec(DgnDbR, Utf8CP codeSpecName);

    //! Update the project extents
    static void UpdateProjectExtents(DgnDbR);

    //! Query for the first GeometricModel in the specified DgnDb
    //! @note Only to be used as a last resort
    static DgnModelId QueryFirstGeometricModelId(DgnDbR);

    //! Use ECSql to SELECT COUNT(*) from the specified ECClass name
    static int SelectCountFromECClass(DgnDbR, Utf8CP className);
    //! Use BeSQLite to SELECT COUNT(*) from the specified table
    static int SelectCountFromTable(DgnDbR, Utf8CP tableName);

    //! Return true if any element has the specified CodeValue.
    //! @note CodeScope and CodeSpecId are not considered
    static bool CodeValueExists(DgnDbR, Utf8CP codeValue);

    //! @name BIM Management Utilities
    //! @{

    //! @private - internal function called by seed data managers
    static DgnDbPtr CreateDgnDb(WCharCP relPath, bool isRoot)
        {
        MUST_HAVE_HOST(nullptr);
        BeFileName fileName = GetOutputPath(relPath);
        if (fileName.GetExtension().empty())
            {
            EXPECT_FALSE(true) << WPrintfString(L"%ls - missing a file extension", relPath).c_str();
            return nullptr;
            }

        if (fileName.DoesPathExist())
            {
            EXPECT_FALSE(true) << WPrintfString(L"%ls - already exists. Use a unique name for your test group's files", fileName.c_str()).c_str();
            return nullptr;
            }

        if (!isRoot && BeFileName::GetDirectoryName(relPath).empty())
            {
            EXPECT_FALSE(true) << "DgnDbTestUtils::CreateDgnDb - the destination must be in a sub-directory with the same name as the test group.";
            return nullptr;
            }

        CreateDgnDbParams createDgnDbParams("DgnDbTestUtils");
        BeSQLite::DbResult createStatus;
        DgnDbPtr db = DgnDb::CreateIModel(&createStatus, fileName, createDgnDbParams);
        if (!db.IsValid())
            EXPECT_FALSE(true) << WPrintfString(L"%ls - create failed", fileName.c_str()).c_str();

        return db;
        }

    //! Create a DgnDb from scratch in the test output directory.
    //! The specified name must be a relative path, including an optional subdirectory path, and a filename.
    //! If the file already exists, that is an ERROR, indicating that two test classs are trying to create seed DgnDbs with the same names.
    //! Each test class should use its own class-specific, unique name for its subdirectory and/or seed DgnDbs.
    //! @param relPath  The subdirectory/filename for the new file. Be sure to use forward slash (/) as a directory separator.
    //! @return a pointer to the newly created file, or nullptr if the location is invalid
    //! @note Normally, this should be called only once for an entire test class in the class's SetUpTestCase function.
    static DgnDbPtr CreateSeedDb(WCharCP relPath) {return CreateDgnDb(relPath, false);}

    //! Open the specified seed DgnDb read-only
    //! @param relSeedPath Identifies a pre-existing seed DgnDb. If you want to open a seed DgnDb that was created by your test class's SetUpTestCase logic, then you must specify the
    //! relative path to it.
    //! @return a pointer to the open DgnDb, or nullptr if the seed DgnDb does not exist
    static DgnDbPtr OpenSeedDb(WCharCP relSeedPath) { return OpenIModelDb(relSeedPath, DgnDb::OpenMode::Readonly); }

    //! Open <em>a copy of</em> the specified seed DgnDb for reading and writing. The result will be a private copy for the use of the caller.
    //! The copy will always be located in a subdirectory with the same name as the calling test.
    //! @note The copy of the file is automatically assigned a unique name, to avoid name collisions with other tests.
    //! @param relSeedPath Identifies a pre-existing seed DgnDb. If you want to open a seed DgnDb that was created by your test class's SetUpTestCase logic, then you must specify the
    //! relative path to it.
    //! @param newName optional. all or part of the name of the copy. If null, then the name of the copy will be based on the name of the input seed DgnDb. If not null, then
    //! the name of the copy will be based on \a newName and will be modified as necessary to make it unique.
    //! @return a pointer to the open DgnDb, or nullptr if the seed DgnDb does not exist.
    //! @see OpenIModelDb
    static DgnDbPtr OpenSeedDbCopy(WCharCP relSeedPathIn, WCharCP newName = nullptr)
        {
        WString relSeedPath(relSeedPathIn);
        SupplyMissingDbExtension(relSeedPath);

        BeFileName infileName = GetOutputPath(relSeedPath.c_str());
        if (!infileName.DoesPathExist())
            {
            EXPECT_FALSE(true) << WPrintfString(L"%ls - file not found", infileName.c_str()).c_str();
            return nullptr;
            }

        //  Create a unique name for the output file, based on the input seed filename.

        //  1. Establish the basename and extension
        BeFileName ccRelPathBase;
        if (nullptr == newName)
            ccRelPathBase.SetName(relSeedPath);
        else
            {
            ccRelPathBase.SetName(BeFileName::GetDirectoryName(relSeedPath.c_str()));
            ccRelPathBase.AppendToPath(newName);
            SupplyMissingDbExtension(ccRelPathBase);
            }

        //  2. Make sure that it is located in a subdirectory that is specific to the current test case. (That's how we keep test groups out of each other's way.)
        Utf8String tcname = BeTest::GetNameOfCurrentTestCase();
        if (!tcname.empty())
            {
            WString wtcname(tcname.c_str(), BentleyCharEncoding::Utf8);
            if (!wtcname.Equals(ccRelPathBase.substr(0, tcname.size())))
                {
                WString tmp(ccRelPathBase);
                ccRelPathBase.SetName(wtcname);
                ccRelPathBase.AppendToPath(tmp.c_str());
                }
            }
        else
            {
            // The caller is not a test. Caller must be a SetUpTestCase function. We don't know the caller's TC name. At least check that he's specifying a subdirectory.
            if (ccRelPathBase.GetDirectoryName().empty())
                {
                EXPECT_FALSE(true) << "DgnDbTestUtils::OpenDgnDbCopy - the destination must be in a sub-directory.";
                return nullptr;
                }
            }

        //  Make sure the output subdirectory exists
        BeFileName subDir = GetOutputPath(ccRelPathBase.GetDirectoryName());
        if (!BeFileName::IsDirectory(subDir.c_str()))
            {
            BeFileName::CreateNewDirectory(subDir.c_str());
            }

        //  3. Make sure it's unique
        BeFileName ccfileName;
        BeFileName ccRelPathUnique;
        int ncopies = 0;
        do  {
            if (0 == ncopies)
                ccRelPathUnique = ccRelPathBase;
            else
                ccRelPathUnique.SetName(WPrintfString(L"%ls-%d.%ls", ccRelPathBase.substr(0, ccRelPathBase.find(L".")).c_str(), ncopies, ccRelPathBase.GetExtension().c_str()));
            ccfileName = GetOutputPath(ccRelPathUnique.c_str());
            ++ncopies;
            }
        while (ccfileName.DoesPathExist());

        BeFileNameStatus fileStatus = BeFileName::BeCopyFile(infileName.c_str(), ccfileName.c_str(), /*failIfFileExists*/true);
        EXPECT_EQ(BeFileNameStatus::Success, fileStatus) << WPrintfString(L"%ls => %ls - copy failed", infileName.c_str(), ccfileName.c_str()).c_str();

        return OpenIModelDb(ccRelPathUnique.c_str(), DgnDb::OpenMode::ReadWrite);
        }

    //! This is a convenenience function that calls OpenSeedDbCopy and returns the full filename of the opened copy.
    static DgnDbStatus MakeSeedDbCopy(BeFileNameR actualName, WCharCP relSeedPath, WCharCP newName);

    //! Open the specified seed DgnDb.
    //! @param relPath Identifies a seed DgnDb that already exists in the specified subdirectory. Be sure to use forward slash (/) as a directory separator.
    //! @param mode the file open mode
    //! @return a pointer to the open DgnDb, or nullptr if the file does not exist
    static DgnDbPtr OpenIModelDb(WCharCP relPath, DgnDb::OpenMode mode)
        {
        BeFileName fileName = GetOutputPath(relPath);
        BeSQLite::DbResult openStatus;
        DgnDb::OpenParams openParams(mode);
        DgnDbPtr db = DgnDb::OpenIModelDb(&openStatus, fileName, openParams);
        if (!db.IsValid())
            EXPECT_FALSE(true) << WPrintfString(L"%ls - open failed with %x", fileName.c_str(), (int)openStatus).c_str();
        return db;
        }

    static void SupplyMissingDbExtension(WStringR name)
        {
        if (name.find(L".") == WString::npos)
            name.append(L".bim");
        }

    static BeFileName GetOutputPath(WStringCR relPath)
        {
        BeFileName outputPathName;
        BeTest::GetHost().GetOutputRoot(outputPathName);
        outputPathName.AppendToPath(relPath.c_str());
        return outputPathName;
        }

    //! Create a subdirectory in the test output directory.
    //! If the directory already exists, that is an ERROR, indicating that two test classs are trying to use the same output directory.
    //! Each test class should use its own class-specific, unique name for its subdirectory and/or seed DgnDbs.
    //! @param relPath  The name of the subdirectory to create. Be sure to use forward slash (/) as a directory separator.
    //! @note Normally, this should be called only in the SetUpTestCase function, once for an entire test class
    static BeFileNameStatus CreateSubDirectory(WCharCP relPath);

    //! Empty a subdirectory in the test output directory.
    //! @param relPath  The name of the subdirectory to empty. Be sure to use forward slash (/) as a directory separator.
    static void EmptySubDirectory(WCharCP relPath);
    //! @}
};

END_BENTLEY_DGNPLATFORM_NAMESPACE

#undef MUST_HAVE_HOST

