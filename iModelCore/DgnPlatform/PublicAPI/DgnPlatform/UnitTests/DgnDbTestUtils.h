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
#include <DgnPlatform/DgnPlatform.h>
#include <DgnPlatform/DgnCategory.h>
#include <DgnPlatform/DgnView.h>
#include <DgnPlatform/LinkElement.h>

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

    //! Create a Camera view of the specified SpatialModel 
    static DgnViewId InsertCameraView(SpatialModelR model, Utf8CP viewName = nullptr, DRange3dCP viewVolume = nullptr, 
                                      StandardView rot = StandardView::Iso, Render::RenderMode renderMode = Render::RenderMode::SmoothShade);

    //! Create a Drawing view
    static DrawingViewDefinitionPtr InsertDrawingView(DrawingModelR model, Utf8CP viewDescr=nullptr);

    static void FitView(DgnDbR db, DgnViewId viewId);

    //! Create a new PERSISTENT modelselector
    static ModelSelectorCPtr InsertNewModelSelector(DgnDbR db, Utf8CP name, DgnModelId model);

    //! Create a new DrawingCategory
    static DgnCategoryId InsertDrawingCategory(DgnDbR, Utf8CP categoryName, DgnSubCategory::Appearance const& appearance = DgnSubCategory::Appearance(), DgnCategory::Rank rank = DgnCategory::Rank::Application);
    //! Create a new DrawingCategory
    static DgnCategoryId InsertDrawingCategory(DgnDbR, Utf8CP categoryName, ColorDefCR color, DgnCategory::Rank rank = DgnCategory::Rank::Application);
    //! Get the first DrawingCategory in the DgnDb
    //! @note Instead of using this method you should explicitly insert a DrawingCategory as part of the test setup
    //! @see InsertDrawingCategory
    static DgnCategoryId GetFirstDrawingCategoryId(DgnDbR);

    //! Create a new SpatialCategory
    static DgnCategoryId InsertSpatialCategory(DgnDbR, Utf8CP categoryName, DgnSubCategory::Appearance const& appearance = DgnSubCategory::Appearance(), DgnCategory::Rank rank = DgnCategory::Rank::Application);
    //! Create a new SpatialCategory
    static DgnCategoryId InsertSpatialCategory(DgnDbR, Utf8CP categoryName, ColorDefCR color, DgnCategory::Rank rank = DgnCategory::Rank::Application);
    //! Get the first SpatialCategory in the DgnDb
    //! @note Instead of using this method you should explicitly insert a SpatialCategory as part of the test setup
    //! @see InsertSpatialCategory
    static DgnCategoryId GetFirstSpatialCategoryId(DgnDbR);

    //! Create a new CodeSpec
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
};

END_BENTLEY_DGNPLATFORM_NAMESPACE
