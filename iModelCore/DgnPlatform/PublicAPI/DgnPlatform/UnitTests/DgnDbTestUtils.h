/*--------------------------------------------------------------------------------------+
|
|  $Source: PublicAPI/DgnPlatform/UnitTests/DgnDbTestUtils.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once 

//=======================================================================================
// WARNING: Must be careful of dependencies within this file as it is also included by 
// WARNING:   DgnDisplayTests, DgnClientFxTests, ConstructionPlanningTests, etc.
//=======================================================================================
#include <Bentley/BeTest.h>
#include <DgnPlatform/DgnCoreAPI.h>
#include <DgnPlatform/GenericDomain.h>

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//=======================================================================================
// @bsiclass                                                 Sam.Wilson     01/2016
//=======================================================================================
struct DgnDbTestUtils : NonCopyableClass
{
public:
    //! Insert a PhysicalModel 
    //! @note Also creates a PhysicalPartition element for the PhysicalModel to model
    //! @note No need for caller to assert a valid return (asserts within implementation)
    static PhysicalModelPtr InsertPhysicalModel(DgnDbR, Utf8CP partitionName);

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
                                      StandardView rot = StandardView::Iso, Render::RenderMode renderMode = Render::RenderMode::SmoothShade);

    //! Insert a Drawing view
    static DrawingViewDefinitionPtr InsertDrawingView(DrawingModelR model, Utf8CP viewName=nullptr);

    static void FitView(DgnDbR db, DgnViewId viewId);

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
    static DgnCategoryId InsertSpatialCategory(DgnDbR, Utf8CP categoryName, DgnSubCategory::Appearance const& appearance = DgnSubCategory::Appearance(), DgnCategory::Rank rank = DgnCategory::Rank::Application);
    //! Insert a new SpatialCategory
    static DgnCategoryId InsertSpatialCategory(DgnDbR, Utf8CP categoryName, ColorDefCR color, DgnCategory::Rank rank = DgnCategory::Rank::Application);
    //! Get the first SpatialCategory in the DgnDb
    //! @deprecated
    //! @note Instead of using this method you should explicitly insert a SpatialCategory as part of the test setup
    //! @see InsertSpatialCategory
    static DgnCategoryId GetFirstSpatialCategoryId(DgnDbR);

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
    static DgnDbPtr CreateDgnDb(WCharCP relPath, bool isRoot);

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
    static DgnDbPtr OpenSeedDb(WCharCP relSeedPath);
        
    //! Open <em>a copy of</em> the specified seed DgnDb for reading and writing. The result will be a private copy for the use of the caller.
    //! The copy will always be located in a subdirectory with the same name as the calling test.
    //! @note The copy of the file is automatically assigned a unique name, to avoid name collisions with other tests.
    //! @param relSeedPath Identifies a pre-existing seed DgnDb. If you want to open a seed DgnDb that was created by your test class's SetUpTestCase logic, then you must specify the
    //! relative path to it. 
    //! @param newName optional. all or part of the name of the copy. If null, then the name of the copy will be based on the name of the input seed DgnDb. If not null, then
    //! the name of the copy will be based on \a newName and will be modified as necessary to make it unique.
    //! @return a pointer to the open DgnDb, or nullptr if the seed DgnDb does not exist.
    //! @see OpenDgnDb
    static DgnDbPtr OpenSeedDbCopy(WCharCP relSeedPath, WCharCP newName = nullptr);

    //! This is a convenenience function that calls OpenSeedDbCopy and returns the full filename of the opened copy.
    static DgnDbStatus MakeSeedDbCopy(BeFileNameR actualName, WCharCP relSeedPath, WCharCP newName);

    //! Open the specified seed DgnDb.
    //! @param relPath Identifies a seed DgnDb that already exists in the specified subdirectory. Be sure to use forward slash (/) as a directory separator.
    //! @param mode the file open mode
    //! @return a pointer to the open DgnDb, or nullptr if the file does not exist
    static DgnDbPtr OpenDgnDb(WCharCP relPath, DgnDb::OpenMode mode);

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
