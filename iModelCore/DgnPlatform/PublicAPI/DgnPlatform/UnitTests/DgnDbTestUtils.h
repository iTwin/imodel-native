/*--------------------------------------------------------------------------------------+
|
|  $Source: PublicAPI/DgnPlatform/UnitTests/DgnDbTestUtils.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//=======================================================================================
// WARNING: Must be careful of dependencies within this file as it is also included by 
// WARNING:   DgnDisplayTests, DgnClientFxTests, etc.
//=======================================================================================
#pragma once 

#include <Bentley/BeTest.h>
#include <DgnPlatform/DgnPlatform.h>
#include <DgnPlatform/DgnCategory.h>
#include <DgnPlatform/DgnView.h>

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//=======================================================================================
// @bsiclass                                                 Sam.Wilson     01/2016
//=======================================================================================
struct DgnDbTestUtils : NonCopyableClass
{
public:
    //! Insert a PhysicalModel 
    //! @note Also creates a Subject for the PhysicalModel to describe
    //! @note No need for caller to assert a valid return (asserts within implementation)
    static PhysicalModelPtr InsertPhysicalModel(DgnDbR, DgnCodeCR modelCode);

    //! Insert a DocumentListModel 
    //! @note Also creates a Subject for the DocumentListModel to describe
    //! @note No need for caller to assert a valid return (asserts within implementation)
    static DocumentListModelPtr InsertDocumentListModel(DgnDbR, DgnCodeCR modelCode);

    //! Insert a Drawing element
    //! @note No need for caller to assert a valid return (asserts within implementation)
    static DrawingPtr InsertDrawing(DocumentListModelCR model, DgnCodeCR elementCode=DgnCode(), Utf8CP userLabel=nullptr);

    //! Insert a DrawingModel 
    //! @note No need for caller to assert a valid return (asserts within implementation)
    static DrawingModelPtr InsertDrawingModel(DrawingCR, DgnCodeCR modelCode);

    //! Insert a SheetModel 
    static SheetModelPtr InsertSheetModel(DgnDbR db, DgnCode modelCode);

    //! Create a Camera view of the specified SpatialModel 
    static DgnViewId InsertCameraView(SpatialModelR model, Utf8CP viewName = nullptr, DRange3dCP viewVolume = nullptr, 
                                      StandardView rot = StandardView::Iso, Render::RenderMode renderMode = Render::RenderMode::SmoothShade)
        {
        auto viewDef = CameraViewDefinition::MakeViewOfModel(model, viewName, viewVolume, rot, renderMode);
        viewDef->Insert();
        return viewDef->GetViewId();
        }
    static void FitView(DgnDbR db, DgnViewId viewId);

    //! Create a new PERSISTENT modelselector
    static ModelSelectorCPtr InsertNewModelSelector(DgnDbR db, Utf8CP name, DgnModelId model);

    //! Create a new Category
    static DgnCategoryId InsertCategory(DgnDbR, Utf8CP categoryName, DgnSubCategory::Appearance const& appearance = DgnSubCategory::Appearance(), DgnCategory::Scope scope = DgnCategory::Scope::Any, DgnCategory::Rank rank = DgnCategory::Rank::Application);
    //! Create a new Category
    static DgnCategoryId InsertCategory(DgnDbR, Utf8CP categoryName, ColorDefCR color, DgnCategory::Scope scope = DgnCategory::Scope::Any, DgnCategory::Rank rank = DgnCategory::Rank::Application);

    //! Create a new PERSISTENT CategorySelector
    static CategorySelectorCPtr InsertNewCategorySelector(DgnDbR db, Utf8CP name, DgnCategoryIdSet const* categories = nullptr);

    //! Create a new CodeAuthority
    static DgnAuthorityId InsertNamespaceAuthority(DgnDbR, Utf8CP authorityName);
    
    //! Update the project extents
    static void UpdateProjectExtents(DgnDbR);

    //! Look up a model by its name
    template<typename T>
    static
    RefCountedPtr<T> GetModelByName(DgnDbR db, Utf8StringCR cmname)
        {
        return db.Models().Get<T>(db.Models().QueryModelId(DgnModel::CreateModelCode(cmname)));
        }

    //! Query for the first GeometricModel in the specified DgnDb
    //! @note Only to be used when the DgnCode of the model is not known
    static DgnModelId QueryFirstGeometricModelId(DgnDbR);
};

END_BENTLEY_DGNPLATFORM_NAMESPACE
