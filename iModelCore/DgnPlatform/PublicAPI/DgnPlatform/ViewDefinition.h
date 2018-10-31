/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/ViewDefinition.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include "DgnDb.h"
#include "DgnCategory.h"
#include "DgnElement.h"
#include "ElementHandler.h"
#include "ECSqlStatementIterator.h"
#include "Lighting.h"
#include "Render.h"

#define BIS_CLASS_SpatialViewDefinition "SpatialViewDefinition"
#define BIS_CLASS_OrthographicViewDefinition "OrthographicViewDefinition"
#define BIS_CLASS_DrawingViewDefinition "DrawingViewDefinition"
#define BIS_CLASS_SheetViewDefinition "SheetViewDefinition"

BEGIN_BENTLEY_DGN_NAMESPACE

namespace ViewElementHandler
{
    struct View; struct View3d; struct View2d; struct OrthographicView; struct DrawingView; struct SheetView; struct TemplateView2d; struct TemplateView3d;
    struct ViewModels; struct ViewCategories; struct ViewDisplayStyle; struct ViewDisplayStyle2d; struct ViewDisplayStyle3d;
}

//=======================================================================================
//! The Display Style for a view. DisplayStyles can be shared by many Views. They define the "styling" parameters for rendering the contents of a view.
//! DisplayStyles determine how graphics are rendered, not which elements are rendered. Styles determine the rendering mode,
//! background color, many on/off choices for types of graphics, SubCategory overrides, etc.
//! When a ViewDefinition is loaded into memory, it makes a copy of its DisplayStyle, so any in-memory changes do not affect the original.
//! Changes are not saved unless someone calls Update on the modified copy.
//! A DisplayStyle is composed of various named "Styles". Styles are defined in Json and is stored/loaded with this element.
// @bsiclass                                                    Keith.Bentley   10/16
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE DisplayStyle : DefinitionElement
{
    DGNELEMENT_DECLARE_MEMBERS(BIS_CLASS_DisplayStyle, DefinitionElement);
    friend struct ViewElementHandler::ViewDisplayStyle;
    friend struct ViewDefinition;

public:
    //! The background map type.  Generally supplied through a web mercator provider.
    enum class MapType : int {None=0, Street=1, Aerial=2, Hybrid=3};


    struct BackgroundMapDisplayHandler : RefCountedBase
        {
        virtual void _Initialize(Json::Value const& settings) = 0;
        virtual RefCountedPtr<TileTree::Root> _GetTileTree(SceneContextR sceneContext) = 0;
        };

protected:
    mutable BeMutex m_mutex;
    mutable bmap<DgnSubCategoryId,DgnSubCategory::Appearance> m_subCategories;
    mutable bmap<DgnSubCategoryId,DgnSubCategory::Override> m_subCategoryOverrides;
    Render::ViewFlags m_viewFlags;
    RefCountedPtr<struct BackgroundMapDisplayHandler> m_backGroundMapDisplayHandler;

    DgnSubCategory::Appearance LoadSubCategory(DgnSubCategoryId) const;
    Utf8String ToJson() const;
    DGNPLATFORM_EXPORT void _OnLoadedJsonProperties() override;
    DGNPLATFORM_EXPORT void _OnSaveJsonProperties() override;
    DGNPLATFORM_EXPORT void _CopyFrom(DgnElementCR rhs) override;
    explicit DisplayStyle(CreateParams const& params) : T_Super(params) {}
    virtual DisplayStyle2dCP _ToDisplayStyle2d() const {return nullptr;}
    virtual DisplayStyle3dCP _ToDisplayStyle3d() const {return nullptr;}
    DisplayStyle(DefinitionModelR model, Utf8StringCR name="") : T_Super(CreateParams(model.GetDgnDb(), model.GetModelId(), QueryClassId(model.GetDgnDb()), CreateCode(model, name))) {}

    JsonValueCR GetStyles() const {return m_jsonProperties[json_styles()];}
    JsonValueR GetStylesR() {return m_jsonProperties[json_styles()];}

    void ChangeSubCategoryDisplay(DgnSubCategoryId, bool onOff);
public:
    BE_JSON_NAME(styles);
    BE_JSON_NAME(viewflags);
    BE_JSON_NAME(backgroundColor);
    BE_JSON_NAME(monochromeColor);
    BE_JSON_NAME(subCategory);
    BE_JSON_NAME(subCategoryOvr);
    BE_JSON_NAME(backgroundMap);

    DisplayStyle2dCP ToDisplayStyle2d() const {return _ToDisplayStyle2d();}
    DisplayStyle2dP ToDisplayStyle2dP() {return const_cast<DisplayStyle2dP>(_ToDisplayStyle2d());}
    DisplayStyle3dCP ToDisplayStyle3d() const {return _ToDisplayStyle3d();}
    DisplayStyle3dP ToDisplayStyle3dP() {return const_cast<DisplayStyle3dP>(_ToDisplayStyle3d());}
    bool Is3d() const {return nullptr != ToDisplayStyle3d();}
    DGNPLATFORM_EXPORT bool EqualState(DisplayStyleR other); // Note: this is purposely non-const and takes a non-const argument. DO NOT CHANGE THAT! You may only call it on writeable copies

    void CopyStylesFrom(DisplayStyle& rhs) {rhs._OnSaveJsonProperties(); GetStylesR() = rhs.GetStyles(); _OnLoadedJsonProperties();}

    //! Get the Json::Value associated with a Style within this DisplayStyle. If the Style is not present, the returned Json::Value will be "null".
    //! @param[in] name The name of the Style
    JsonValueCR GetStyle(Utf8CP name) const {return GetStyles()[name];}

    //! Set a Style in this DisplayStyle.
    //! @param[in] name The name of the Style
    //! @param[in] value The value for the the Style
    //! @note  This only changes the Style in memory. It will be saved when/if the DisplayStyle is saved.
    void SetStyle(Utf8CP name, JsonValueCR value) {GetStylesR()[name] = value;}

    //! Remove a Style from this DisplayStyle.
    //! @param[in] name The name of the Style to remove
    //! @note  This only changes the Style in memory. It will be saved when/if the DisplayStyle is saved.
    void RemoveStyle(Utf8CP name) {GetStylesR().removeMember(name);}

    //! Get the background color for this DisplayStyle
    DGNPLATFORM_EXPORT ColorDef GetBackgroundColor() const;

    //! Set the background color for this DisplayStyle
    //! @param[in] val the new background color for this DisplayStyle
    DGNPLATFORM_EXPORT void SetBackgroundColor(ColorDef val);

    DGNPLATFORM_EXPORT ColorDef GetMonochromeColor() const;
    DGNPLATFORM_EXPORT void SetMonochromeColor(ColorDef val);

    //! Get the Rendering flags for this DisplayStyle
    Render::ViewFlags GetViewFlags() const {return m_viewFlags;}

    //! Set the Rendering flags for this DisplayStyle
    void SetViewFlags(Render::ViewFlags flags) {m_viewFlags=flags;}

    //! Determine whether this DisplayStyle has any SubCategory overrides
    bool HasSubCategoryOverride() const {return !m_subCategoryOverrides.empty();}

    //! Override the appearance of a SubCategory for this DisplayStyle
    DGNPLATFORM_EXPORT void OverrideSubCategory(DgnSubCategoryId, DgnSubCategory::Override const&);

    //! Drop the override of the appearance of a SubCategory from this DisplayStyle
    DGNPLATFORM_EXPORT void DropSubCategoryOverride(DgnSubCategoryId);

    //! Look up the appearance override for the specified SubCategory
    DGNPLATFORM_EXPORT DgnSubCategory::Override GetSubCategoryOverride(DgnSubCategoryId id) const;

    //! Get the appearance of a SubCategory, taking into consideration any overrides from this DisplayStyle. If the SubCategory
    //! is not overridden, this will return the default appearance of the SubCategory.
    DGNPLATFORM_EXPORT DgnSubCategory::Appearance GetSubCategoryAppearance(DgnSubCategoryId id) const;

    //! Get the background map.
    BentleyStatus GetBackgroundMapSettings(MapType& mapType, Utf8StringR provider, double& groundBias, bool returnDefaults = true) const;
    DGNPLATFORM_EXPORT void SetBackgroundMapSettings(MapType mapType, Utf8StringCR& provider, double groundBias);
    bool GetDisplayBackgroundMap() const;
    RefCountedPtr<TileTree::Root>  GetBackgroundMapTileTree(SceneContextR sceneContext);

    //! Create a DgnCode for a DisplayStyle given a name that is meant to be unique within the scope of the specified DefinitionModel
    static DgnCode CreateCode(DefinitionModelR scope, Utf8StringCR name) {return name.empty() ? DgnCode() : CodeSpec::CreateCode(BIS_CODESPEC_DisplayStyle, scope, name);}

    static DgnClassId QueryClassId(DgnDbR db) {return DgnClassId(db.Schemas().GetClassId(BIS_ECSCHEMA_NAME, BIS_CLASS_DisplayStyle));}//!< @private
};

//=======================================================================================
//! The DisplayStyle for a 2d view.
// @bsiclass                                                    Keith.Bentley   03/17
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE DisplayStyle2d : DisplayStyle
{
    DGNELEMENT_DECLARE_MEMBERS(BIS_CLASS_DisplayStyle2d, DisplayStyle);
    friend struct ViewElementHandler::ViewDisplayStyle2d;

protected:
    explicit DisplayStyle2d(CreateParams const& params) : T_Super(params) {}
    DisplayStyle2dCP _ToDisplayStyle2d() const override final {return this;}

public:
    //! Construct a new DisplayStyle2d.
    //! @param[in] model The DefinitionModel to contain the DisplayStyle2d
    //! @param[in] name The name of the DisplayStyle2d. Must be unique across all DisplayStyles
    DisplayStyle2d(DefinitionModelR model, Utf8StringCR name="") : T_Super(CreateParams(model.GetDgnDb(), model.GetModelId(), QueryClassId(model.GetDgnDb()), CreateCode(model, name))) {}
    
    static DgnClassId QueryClassId(DgnDbR db) {return DgnClassId(db.Schemas().GetClassId(BIS_ECSCHEMA_NAME, BIS_CLASS_DisplayStyle2d));}//!< @private
};

//=======================================================================================
//! The DisplayStyle for a 3d view.
// @bsiclass                                                    Keith.Bentley   10/16
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE DisplayStyle3d : DisplayStyle
{
    DGNELEMENT_DECLARE_MEMBERS(BIS_CLASS_DisplayStyle3d, DisplayStyle);
    friend struct ViewElementHandler::ViewDisplayStyle3d;

public:
    //! The "environment" Style for this DisplayStyle3d. The environment provides a visual cue of the orientation of the view relative to the earth.
    struct EnvironmentDisplay
    {
        //! A circle drawn at a Z elevation, whose diameter is the the XY diagonal of the project extents
        struct GroundPlane
        {
            bool m_enabled = false;
            double m_elevation = 0.0;   //!< the Z height to draw the ground plane
            ColorDef m_aboveColor;      //!< the color to draw the ground plane if the view shows the ground from above
            ColorDef m_belowColor;      //!< the color to draw the ground plane if the view shows the ground from below
        };
            
        struct SkyBox
        {
            struct Image
            {
                enum class Type
                {
                    None,
                    Spherical,
                    Cylindrical,
                };

                DgnTextureId    m_textureId;
                Type            m_type = Type::None;
            };

            ColorDef m_zenithColor; //!< if no image, the color of the zenith part of the sky gradient (shown when looking straight up.)
            ColorDef m_nadirColor;  //!< if no image, the color of the nadir part of the ground gradient (shown when looking straight down.)
            ColorDef m_groundColor; //!< if no image, the color of the ground part of the ground gradient
            ColorDef m_skyColor;    //!< if no image, the color of the sky part of the sky gradient
            double m_groundExponent=4.0; //!< if no image, the cutoff between ground and nadir
            double m_skyExponent=4.0;    //!< if no image, the cutoff between sky and zenith
            Image m_image;
            bool m_enabled = false;
            bool m_twoColor = false;
        };
       
        GroundPlane m_groundPlane;
        SkyBox m_skybox;

        GroundPlane& GetGroundPlane() {return m_groundPlane;}
        SkyBox& GetSkyBox() {return m_skybox;}

        DGNPLATFORM_EXPORT void Initialize();
    };


protected:
    EnvironmentDisplay m_environment;
    mutable Render::MaterialPtr m_skyboxMaterial;
    mutable Render::TexturePtr m_diffuseLightTexture;
    mutable Render::TexturePtr m_reflectionTexture;

    DGNPLATFORM_EXPORT void _OnLoadedJsonProperties() override;
    DGNPLATFORM_EXPORT void _OnSaveJsonProperties() override;
    DGNPLATFORM_EXPORT void _CopyFrom(DgnElementCR rhs) override;
    explicit DisplayStyle3d(CreateParams const& params) : T_Super(params) {}
    DisplayStyle3dCP _ToDisplayStyle3d() const override final {return this;}

public:
    BE_JSON_NAME(ambient)
    BE_JSON_NAME(flash)
    BE_JSON_NAME(portrait)
    BE_JSON_NAME(sun)
    BE_JSON_NAME(sunDir)
    BE_JSON_NAME(sceneLights);
    BE_JSON_NAME(brightness);
    BE_JSON_NAME(hline)
    BE_JSON_NAME(fstop);
    BE_JSON_NAME(environment);

    //! Construct a new DisplayStyle3d.
    //! @param[in] model The DefinitionModel to contain the DisplayStyle3d
    //! @param[in] name The name of the DisplayStyle3d. Must be unique across all DisplayStyles
    DisplayStyle3d(DefinitionModelR model, Utf8StringCR name="") : T_Super(CreateParams(model.GetDgnDb(), model.GetModelId(), QueryClassId(model.GetDgnDb()), CreateCode(model, name))) {m_environment.Initialize();}

    /** @name Environment Display*/
    /** @{ */
    //! Determine whether the SkyBox is displayed in this DisplayStyle3d.
    bool IsSkyBoxEnabled() const {return m_environment.m_skybox.m_enabled;}

    //! Turn the SkyBox on or off.
    void SetSkyBoxEnabled(bool val) {m_environment.m_skybox.m_enabled = val;}

    //! Determine whether the Ground Plane is displayed in this DisplayStyle3d.
    bool IsGroundPlaneEnabled() const {return m_environment.m_groundPlane.m_enabled;}

    //! Turn the GroundPlane on or off.
    void SetGroundPlaneEnabled(bool val) {m_environment.m_groundPlane.m_enabled = val;}

    Render::HiddenLineParams GetHiddenLineParams() {return Render::HiddenLineParams::FromJson(GetStyle(json_hline()));}
    void SetHiddenLineParams(Render::HiddenLineParams const& params) {SetStyle(json_hline(), params.ToJson());}

    DGNPLATFORM_EXPORT Render::SceneLightsPtr CreateSceneLights(Render::TargetR);
    DGNPLATFORM_EXPORT void SetSceneLight(Lighting::Parameters const&);
    DGNPLATFORM_EXPORT void SetSolarLight(Lighting::Parameters const&, DVec3dCR direction);

    void SetSceneBrightness(double fstop) {fstop = std::max(-3.0, std::min(fstop, 3.0)); GetStylesR()[json_sceneLights()].SetOrRemoveDouble(json_fstop(), fstop, 0.0);}
    double GetSceneBrightness() const {return GetStyles()[json_sceneLights()][json_fstop()].asDouble();}

    //! Get the current values for the Environment Display for this DisplayStyle3d
    EnvironmentDisplay const& GetEnvironmentDisplay() const {return m_environment;}
    EnvironmentDisplay& GetEnvironmentDisplayR() {return m_environment;}

    //! Change the current values for the Environment Display for this DisplayStyle3d
    void SetEnvironmentDisplay(EnvironmentDisplay const& val) {m_environment = val;}
    /** @} */

    void LoadSkyBoxMaterial(Render::SystemCR system);
    Render::MaterialP   GetSkyBoxMaterial() { return m_skyboxMaterial.get(); }

    static DgnClassId QueryClassId(DgnDbR db) {return DgnClassId(db.Schemas().GetClassId(BIS_ECSCHEMA_NAME, BIS_CLASS_DisplayStyle3d));}//!< @private

#ifdef WIP_IBL
    DGNPLATFORM_EXPORT Render::EnvironmentPtr CreateEnvironment(Render::TargetR);
#endif
};

//=======================================================================================
//! A list of GeometricModels for a SpatialViewDefinition.
//! When a SpatialViewDefinition is loaded into a ViewController, it makes a copy of its ModelSelector, so any in-memory changes do not affect the original.
//! Changes are not saved unless someone calls Update on the modified copy.
// @bsiclass                                                      Sam.Wilson    08/16
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ModelSelector : DefinitionElement
{
    DGNELEMENT_DECLARE_MEMBERS(BIS_CLASS_ModelSelector, DefinitionElement);
    friend struct ViewElementHandler::ViewModels;
    friend struct SpatialViewDefinition;

protected:
    DgnModelIdSet m_models;

    bool EqualState(ModelSelectorCR other) const {return m_models==other.m_models;}
    DGNPLATFORM_EXPORT DgnDbStatus _LoadFromDb() override;
    DGNPLATFORM_EXPORT void _CopyFrom(DgnElementCR rhs) override;
    DGNPLATFORM_EXPORT DgnDbStatus _InsertInDb() override;
    DGNPLATFORM_EXPORT DgnDbStatus _OnUpdate(DgnElementCR) override;
    DGNPLATFORM_EXPORT void _OnDeleted() const override;
    DGNPLATFORM_EXPORT void _ToJson(JsonValueR out, JsonValueCR opts) const override;
    DGNPLATFORM_EXPORT void _FromJson(JsonValueR props) override;

    explicit ModelSelector(CreateParams const& params) : T_Super(params) {}
    DgnDbStatus WriteModels();

public:
    BE_JSON_NAME(models)

    //! Construct a new ModelSelector.
    //! @param[in] model The DefinitionModel that will contain this new ModelSelector (not the model or models that the ModelSelector will select)
    //! @param[in] name The name of the ModelSelector.
    ModelSelector(DefinitionModelR model, Utf8StringCR name) : T_Super(CreateParams(model.GetDgnDb(), model.GetModelId(), QueryClassId(model.GetDgnDb()), CreateCode(model, name))) {}

    Utf8String GetName() const {return GetCode().GetValue().GetUtf8();} //!< Get the name of this ModelSelector

    //! Query if the specified DgnModelId is selected by this ModelSelector
    bool ContainsModel(DgnModelId modelId) const {return m_models.Contains(modelId);}

    // Get the set of models selected by this ModelSelector
    DgnModelIdSet const& GetModels() const {return m_models;}
    DgnModelIdSet& GetModelsR() {return m_models;} //! @private

    //! Add a model to this ModelSelector
    void AddModel(DgnModelId id) {m_models.insert(id);}

    //! Drop a model from this ModelSelector. Model will no longer be displayed by views that use this ModelSelector.
    //! @return true if the model was dropped, false if it was not previously in this ModelSelector
    bool DropModel(DgnModelId id) {return 0 != m_models.erase(id);}

    //! Create a DgnCode for a ModelSelector given a name that is meant to be unique within the scope of the specified DefinitionModel
    static DgnCode CreateCode(DefinitionModelR scope, Utf8StringCR name) {return name.empty() ? DgnCode() : CodeSpec::CreateCode(BIS_CODESPEC_ModelSelector, scope, name);}

    static DgnClassId QueryClassId(DgnDbR db) {return DgnClassId(db.Schemas().GetClassId(BIS_ECSCHEMA_NAME, BIS_CLASS_ModelSelector));}//!< @private
};

//=======================================================================================
//! A list of Categories to be displayed in a view.
//! When a ViewDefinition is loaded into memory, it makes a copy of its CategorySelector, so any in-memory changes do not affect the original.
//! Changes are not saved unless someone calls Update on the modified copy.
// @bsiclass                                                      Sam.Wilson    08/16
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE CategorySelector : DefinitionElement
{
    DGNELEMENT_DECLARE_MEMBERS(BIS_CLASS_CategorySelector, DefinitionElement);
    friend struct ViewElementHandler::ViewCategories;
    friend struct ViewDefinition;

protected:
    mutable DgnCategoryIdSet m_categories;

    bool EqualState(CategorySelectorCR other) const;
    DGNPLATFORM_EXPORT DgnDbStatus _LoadFromDb() override;
    DGNPLATFORM_EXPORT void _CopyFrom(DgnElementCR rhs) override;
    DGNPLATFORM_EXPORT DgnDbStatus _InsertInDb() override;
    DGNPLATFORM_EXPORT DgnDbStatus _OnUpdate(DgnElementCR) override;
    DGNPLATFORM_EXPORT void _ToJson(JsonValueR out, JsonValueCR opts) const override;
    DGNPLATFORM_EXPORT void _FromJson(JsonValueR props) override;

    DgnDbStatus WriteCategories();
    explicit CategorySelector(CreateParams const& params) : T_Super(params) {}

public:
    BE_JSON_NAME(categories)

    //! Construct a new CategorySelector
    CategorySelector(DefinitionModelR model, Utf8StringCR name) : T_Super(CreateParams(model.GetDgnDb(), model.GetModelId(), QueryClassId(model.GetDgnDb()), CreateCode(model, name))) {}

    //! Get the name of this CategorySelector
    Utf8String GetName() const {return GetCode().GetValue().GetUtf8();} 

    //! Get the set of currently displayed DgnCategories
    DgnCategoryIdSet const& GetCategories() const {return m_categories;}
    DgnCategoryIdSet& GetCategoriesR() {return m_categories;}//!< @private
    void SetCategories(DgnCategoryIdSet const& categories) {m_categories = categories;}//!< @private

    //! Determine whether this CategorySelector includes the specified category
    bool IsCategoryViewed(DgnCategoryId categoryId) const {return m_categories.Contains(categoryId);}

    //! Add a category to this CategorySelector
    void AddCategory(DgnCategoryId id) {m_categories.insert(id);}

    //! Drop a category from this CategorySelector
    bool DropCategory(DgnCategoryId id) {return 0 != m_categories.erase(id);}

    //! Add or Drop a category to this CategorySelector
    void ChangeCategoryDisplay(DgnCategoryId categoryId, bool add) {if (add) AddCategory(categoryId); else DropCategory(categoryId);}

    static DgnClassId QueryClassId(DgnDbR db) {return DgnClassId(db.Schemas().GetClassId(BIS_ECSCHEMA_NAME, BIS_CLASS_CategorySelector));} //!< @private

    //! Create a DgnCode for a CategorySelector given a name that is meant to be unique within the scope of the specified DefinitionModel
    static DgnCode CreateCode(DefinitionModelR scope, Utf8StringCR name) {return name.empty() ? DgnCode() : CodeSpec::CreateCode(BIS_CODESPEC_CategorySelector, scope, name);}
};

//=======================================================================================
//! The definition element for a view. ViewDefinitions specify the area/volume that is viewed, and points to a DisplayStyle and a CategorySelector.
//! Subclasses of ViewDefinition determine which model(s) are viewed.
//! A ViewController holds an editable copy of a ViewDefinition, and a ViewDefinition holds an editable copy of its DisplayStyle and CategorySelector.
//! @ingroup GROUP_DgnView
// @bsiclass                                                    Keith.Bentley   10/16
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ViewDefinition : DefinitionElement
{
    DGNELEMENT_DECLARE_MEMBERS(BIS_CLASS_ViewDefinition, DefinitionElement);
    friend struct ViewElementHandler::View;
    friend struct ViewController;                
    friend struct DgnViewport;

public:
    //! Parameters used to construct a ViewDefinition
    struct CreateParams : T_Super::CreateParams
    {
        DEFINE_T_SUPER(ViewDefinition::T_Super::CreateParams);
        CategorySelectorPtr m_categorySelector;

    public:
        CreateParams(DgnDbR db, DgnModelId modelId, DgnClassId classId, DgnCodeCR code, CategorySelectorR categorySelector)
            : T_Super(db, modelId, classId, code, nullptr, DgnElementId()), m_categorySelector(&categorySelector) {}

        explicit CreateParams(DgnElement::CreateParams const& params) : T_Super(params) {}
    };

protected:
    DgnElementId m_categorySelectorId;
    DgnElementId m_displayStyleId;
    mutable CategorySelectorPtr m_categorySelector;
    mutable DisplayStylePtr m_displayStyle;

    void ClearState() const {m_categorySelector = nullptr; m_displayStyle = nullptr;}
    BE_PROP_NAME(Description);
    BE_JSON_NAME(viewDetails);

    static bool IsValidCode(DgnCodeCR code);

    explicit ViewDefinition(CreateParams const& params) : T_Super(params) {if (params.m_categorySelector.IsValid()) SetCategorySelector(*params.m_categorySelector);} 

    DGNPLATFORM_EXPORT virtual BentleyStatus _ValidateState();
    DGNPLATFORM_EXPORT virtual bool _EqualState(ViewDefinitionR);
    DGNPLATFORM_EXPORT DgnDbStatus _ReadSelectParams(BeSQLite::EC::ECSqlStatement&, ECSqlClassParamsCR) override;
    DGNPLATFORM_EXPORT void _ToJson(JsonValueR out, JsonValueCR opts) const override;
    DGNPLATFORM_EXPORT void _FromJson(JsonValueR props) override;
    DGNPLATFORM_EXPORT void _BindWriteParams(BeSQLite::EC::ECSqlStatement&, ForInsert) override;
    DGNPLATFORM_EXPORT DgnDbStatus _OnInsert() override;
    void _OnInserted(DgnElementP copiedFrom) const override {ClearState(); T_Super::_OnInserted(copiedFrom);}
    void _OnUpdateFinished() const override {ClearState(); T_Super::_OnUpdateFinished();}
    void _OnDeleted() const override {DeleteThumbnail(); T_Super::_OnDeleted();}
    DGNPLATFORM_EXPORT void _CopyFrom(DgnElementCR el) override;
    DgnCode _GenerateDefaultCode() const override {return DgnCode();}
    bool _SupportsCodeSpec(CodeSpecCR codeSpec) const override {return !codeSpec.IsNullCodeSpec();}
    DgnDbStatus _SetParentId(DgnElementId, DgnClassId) override {return DgnDbStatus::InvalidParent;}
    DgnDbStatus _OnChildInsert(DgnElementCR) const override {return DgnDbStatus::InvalidParent;}
    DgnDbStatus _OnChildUpdate(DgnElementCR, DgnElementCR) const override {return DgnDbStatus::InvalidParent;}
    virtual ViewControllerPtr _SupplyController() const = 0;
    virtual bool _IsValidBaseModel(DgnModelCR model) const {return true;}
    virtual OrthographicViewDefinitionCP _ToOrthographicView() const {return nullptr;}
    virtual ViewDefinition3dCP _ToView3d() const {return nullptr;}
    virtual ViewDefinition2dCP _ToView2d() const {return nullptr;}
    virtual SpatialViewDefinitionCP _ToSpatialView() const {return nullptr;}
    virtual DrawingViewDefinitionCP _ToDrawingView() const {return nullptr;}
    virtual SheetViewDefinitionCP _ToSheetView() const {return nullptr;}
    virtual TemplateViewDefinition2dCP _ToTemplateView2d() const {return nullptr;}
    virtual TemplateViewDefinition3dCP _ToTemplateView3d() const {return nullptr;}
    virtual bool _ViewsModel(DgnModelId mid) = 0;
    virtual DPoint3d _GetOrigin() const = 0;
    virtual DVec3d _GetExtents() const = 0;
    virtual RotMatrix _GetRotation() const = 0;
    virtual void _SetOrigin(DPoint3dCR viewOrg) = 0;
    virtual void _SetExtents(DVec3dCR viewDelta) = 0;
    virtual void _SetRotation(RotMatrixCR viewRot) = 0;
    virtual DPoint3d _GetTargetPoint() const {return GetCenter();}
    DGNPLATFORM_EXPORT virtual ViewportStatus _SetupFromFrustum(Frustum const& inFrustum);
    virtual void _GetExtentLimits(double& minExtent, double& maxExtent) const {minExtent=DgnUnits::OneMillimeter(); maxExtent= 2.0*DgnUnits::DiameterOfEarth(); }
    void SetupDisplayStyle(DisplayStyleR style) {BeAssert(!IsPersistent()); m_displayStyle = &style; m_displayStyleId=style.GetElementId();}
    Utf8String ToDetailJson();
    JsonValueCR GetDetails() const {return m_jsonProperties[json_viewDetails()];}
    JsonValueR GetDetailsR() {return m_jsonProperties[json_viewDetails()];}
    DGNPLATFORM_EXPORT virtual void _AdjustAspectRatio(double windowAspect);

public:
    BE_JSON_NAME(categorySelectorId)
    BE_JSON_NAME(displayStyleId)
    BE_JSON_NAME(width)
    BE_JSON_NAME(height)
    BE_JSON_NAME(format)
    BE_JSON_NAME(clip)
    BE_JSON_NAME(gridOrient)
    BE_JSON_NAME(gridSpaceX)
    BE_JSON_NAME(gridSpaceY)
    BE_JSON_NAME(gridPerRef)
    BE_JSON_NAME(acs)
    BE_JSON_NAME(aspectSkew)

    DGNPLATFORM_EXPORT ViewportStatus ValidateViewDelta(DPoint3dR delta, bool displayMessage);

    //! Determine whether two ViewDefinitions are "equal", including their unsaved state
    bool EqualState(ViewDefinitionR other) {return _EqualState(other);}
    
    Utf8String GetDescription() const {return GetPropertyValueString(prop_Description());} //!< Get description
    DgnDbStatus SetDescription(Utf8StringCR value) {return SetPropertyValue(prop_Description(), value.c_str());} //!< Set description

    DgnViewId GetViewId() const {return DgnViewId(GetElementId().GetValueUnchecked());} //!< This ViewDefinition's Id
    Utf8String GetName() const {return GetCode().GetValue().GetUtf8();} //!< Get the name of this ViewDefinition

    /** @name ViewDefinition Details */
    /** @{ */

    //! Get the current value of a view detail 
    JsonValueCR GetDetail(Utf8CP name) const {return GetDetails()[name];}

    //! Change the value of a view detail 
    void SetDetail(Utf8CP name, JsonValueCR value) {GetDetailsR()[name] = value;}

    //! Remove a view detail 
    void RemoveDetail(Utf8CP name) {GetDetailsR().removeMember(name);}
    /** @} */

    //! Inserts into the database and returns the new persistent copy.
    ViewDefinitionCPtr Insert(DgnDbStatus* status=nullptr) {return GetDgnDb().Elements().Insert<ViewDefinition>(*this, status);}

    //! Updates in the database and returns the updated persistent copy.
    ViewDefinitionCPtr Update(DgnDbStatus* status=nullptr) {return GetDgnDb().Elements().Update<ViewDefinition>(*this, status);}

    //! Create a ViewController for this ViewDefinition
    DGNPLATFORM_EXPORT ViewControllerPtr LoadViewController(bool allowOverrides=true) const;

    //! Create a ViewController for the ViewDefinition with the specified Id
    DGNPLATFORM_EXPORT static ViewControllerPtr LoadViewController(DgnViewId viewId, DgnDbR db);

    //! Create a DgnCode for a ViewDefinition given a name that is meant to be unique within the scope of the specified DefinitionModel
    static DgnCode CreateCode(DefinitionModelCR scope, Utf8StringCR name) {return name.empty() ? DgnCode() : CodeSpec::CreateCode(BIS_CODESPEC_ViewDefinition, scope, name); }

    //! Look up the Id of the view with the specified DgnCode
    DGNPLATFORM_EXPORT static DgnViewId QueryViewId(DgnDbR db, DgnCodeCR code);
    //! Look up the Id of the view with the specified name in the specified DefinitionModel
    static DgnViewId QueryViewId(DefinitionModelCR model, Utf8StringCR name) {return QueryViewId(model.GetDgnDb(), CreateCode(model, name));}

    //! Look up a view by Id
    static ViewDefinitionCPtr Get(DgnDbR db, DgnViewId viewId) {return db.Elements().Get<ViewDefinition>(viewId);}

    //! An entry in an iterator over the views in a DgnDb
    struct Entry : ECSqlStatementEntry
    {
        friend struct ECSqlStatementIterator<Entry>;
        friend struct DgnView;
    private:
        Entry(BeSQLite::EC::ECSqlStatement* stmt=nullptr) : ECSqlStatementEntry(stmt) {}

    public:
        DgnDbP GetDgnDb() const {auto stmt = GetStatement(); return (nullptr != stmt) ? const_cast<DgnDbP>(static_cast<DgnDbCP>(stmt->GetECDb())) : nullptr;}
        DgnViewId GetId() const {return m_statement->GetValueId<DgnViewId>(0);} //!< The view Id
        Utf8CP GetName() const {return m_statement->GetValueText(1);} //!< The name of the view
        bool IsPrivate() const {return m_statement->GetValueBoolean(2);} //!< Whether the view is private or not
        Utf8CP GetDescription() const {return m_statement->GetValueText(3);} //!< The view's description
        DgnClassId GetClassId() const {return m_statement->GetValueId<DgnClassId>(4);} //!< The view's ECClassId

        DGNPLATFORM_EXPORT bool IsView3d() const;
        DGNPLATFORM_EXPORT bool IsSpatialView() const;
        DGNPLATFORM_EXPORT bool IsOrthographicView() const;
        DGNPLATFORM_EXPORT bool IsDrawingView() const;
        DGNPLATFORM_EXPORT bool IsSheetView() const;

        template <typename VIEW_TYPE>
        RefCountedCPtr<VIEW_TYPE> GetViewDefinition(bool (ViewDefinition::Entry::*TestFunc)() const, VIEW_TYPE const* (ViewDefinition::*ToFunc)() const) const
            {
            DgnDbP db = this->GetDgnDb();
            if (!(this->*TestFunc)() || (nullptr == db))
                return nullptr;
            auto vc = ViewDefinition::Get(*db, this->GetId());
            if (!vc.IsValid())
                {
                BeAssert(false);
                return nullptr;
                }
            return ((*vc).*ToFunc)();
            }

        DrawingViewDefinitionCPtr GetDrawingViewDefinition() const {return GetViewDefinition(&Entry::IsDrawingView, &ViewDefinition::ToDrawingView);}
        SheetViewDefinitionCPtr GetSheetViewDefinition() const {return GetViewDefinition(&Entry::IsSheetView,   &ViewDefinition::ToSheetView);}
        SpatialViewDefinitionCPtr GetSpatialViewDefinition() const {return GetViewDefinition(&Entry::IsSpatialView, &ViewDefinition::ToSpatialView);}
        ViewDefinition3dCPtr GetViewDefinition3d() const {return GetViewDefinition(&Entry::IsView3d, &ViewDefinition::ToView3d);}
        OrthographicViewDefinitionCPtr GetOrthographicViewDefinition() const {return GetViewDefinition(&Entry::IsOrthographicView, &ViewDefinition::ToOrthographicView);}
    };

    //! An iterator over the ViewDefinitions stored in a DgnDb
    struct Iterator : ECSqlStatementIterator<Entry>
    {
        //! Construct an iterator over the view definitions within the specified DgnDb
        DGNPLATFORM_EXPORT Iterator(DgnDbR db, Utf8CP whereClause, Utf8CP orderByClause);
    };

    //! Create an iterator over the view definitions within the specified DgnDb
    //! @param[in] db the DgnDb
    //! @param[in] whereClause The optional where clause starting with WHERE
    //! @param[in] orderByClause The optional order by clause starting with ORDER BY
    static Iterator MakeIterator(DgnDbR db, Utf8CP whereClause=nullptr, Utf8CP orderByClause=nullptr) {return Iterator(db, whereClause, orderByClause);}

    //! Return the number of view definitions in the specific DgnDb
    //! @param[in] db the DgnDb
    //! @param[in] whereClause The optional where clause starting with WHERE
    DGNPLATFORM_EXPORT static size_t QueryCount(DgnDbR db, Utf8CP whereClause=nullptr);

    //! Attempt to determine the default DgnViewId for this DgnDb.
    //! @return An invalid DgnViewId will be returned if a default view could not be found.
    DGNPLATFORM_EXPORT static DgnViewId QueryDefaultViewId(DgnDbR);

    bool IsView3d() const {return nullptr != _ToView3d();}
    bool IsOrthographicView() const {return nullptr != _ToOrthographicView();}
    bool IsSpatialView() const {return nullptr != _ToSpatialView();}
    bool IsDrawingView() const {return nullptr != _ToDrawingView();}
    bool IsSheetView() const {return nullptr != _ToSheetView();}
    ViewDefinition2dCP ToView2d() const {return _ToView2d();}
    bool IsTemplateView2d() const {return nullptr != _ToTemplateView2d();}
    bool IsTemplateView3d() const {return nullptr != _ToTemplateView3d();}
    ViewDefinition3dCP ToView3d() const {return _ToView3d();}
    OrthographicViewDefinitionCP ToOrthographicView() const {return _ToOrthographicView();}
    SpatialViewDefinitionCP ToSpatialView() const {return _ToSpatialView();}
    DrawingViewDefinitionCP ToDrawingView() const {return _ToDrawingView();}
    SheetViewDefinitionCP ToSheetView() const {return _ToSheetView();}
    TemplateViewDefinition2dCP ToTemplateView2d() const {return _ToTemplateView2d();}
    TemplateViewDefinition3dCP ToTemplateView3d() const {return _ToTemplateView3d();}
    ViewDefinition3dP ToView3dP() {return const_cast<ViewDefinition3dP>(ToView3d());}
    ViewDefinition2dP ToView2dP() {return const_cast<ViewDefinition2dP>(ToView2d());}
    SpatialViewDefinitionP ToSpatialViewP() {return const_cast<SpatialViewDefinitionP>(ToSpatialView());}
    DrawingViewDefinitionP ToDrawingViewP() {return const_cast<DrawingViewDefinitionP>(ToDrawingView());}
    SheetViewDefinitionP ToSheetViewP() {return const_cast<SheetViewDefinitionP>(ToSheetView());}
    TemplateViewDefinition2dP ToTemplateView2dP() {return const_cast<TemplateViewDefinition2dP>(ToTemplateView2d());}
    TemplateViewDefinition3dP ToTemplateView3dP() {return const_cast<TemplateViewDefinition3dP>(ToTemplateView3d());}

    //! Get the CategorySelector for this ViewDefinition. 
    //! @note this is a non-const method and may only be called on a writeable copy of a ViewDefinition.
    DGNPLATFORM_EXPORT CategorySelectorR GetCategorySelector();
    DgnElementId GetCategorySelectorId() const {return m_categorySelectorId;}

    //! Get the DisplayStyle for this ViewDefinition
    //! @note this is a non-const method and may only be called on a writeable copy of a ViewDefinition.
    DGNPLATFORM_EXPORT DisplayStyleR GetDisplayStyle();
    DgnElementId GetDisplayStyleId() const {return m_displayStyleId;}

    void SetDisplayStyle(DisplayStyleR style) { BeAssert(!IsPersistent()); m_displayStyle = &style; m_displayStyleId = style.GetElementId(); }

    //! Set the CategorySelector for this view.
    void SetCategorySelector(CategorySelectorR categories) {BeAssert(!IsPersistent()); m_categorySelector = &categories; m_categorySelectorId=categories.GetElementId();}

    //! Get the AuxiliaryCoordinateSystem for this ViewDefinition
    DGNPLATFORM_EXPORT DgnElementId GetAuxiliaryCoordinateSystemId() const;

    //! Set the AuxiliaryCoordinateSystem for this view.
    DGNPLATFORM_EXPORT void SetAuxiliaryCoordinateSystem(DgnElementId acsId);

    //! Query if the specified model is displayed in this view
    bool ViewsModel(DgnModelId modelId) {return _ViewsModel(modelId);}

    //! Query if the specified Category is displayed in this view
    bool ViewsCategory(DgnCategoryId id) {return GetCategorySelector().IsCategoryViewed(id);}

    //! Get the origin of this view
    DPoint3d GetOrigin() const {return _GetOrigin();}

    //! Set the origin of this view
    void SetOrigin(DPoint3dCR origin) {_SetOrigin(origin);}

    //! Get the extents of this view
    DVec3d GetExtents() const {return _GetExtents();}

    //! Get the aspect ratio (width/height) of this view
    double GetAspectRatio() const {auto extents=GetExtents(); return extents.x/extents.y;}

    //! Get the aspect ratio skew (x/y, usually 1.0) that can be used to exaggerate one axis of the view.
    double GetAspectRatioSkew() const {return GetDetail(json_aspectSkew()).asDouble(1.0);}

    //! Change the aspect ratio skew (x/y) of this view.
    void SetAspectRatioSkew(double val) {if (val == 1.0) {RemoveDetail(json_aspectSkew());} else {SetDetail(json_aspectSkew(), Json::Value(val));}}

    //! Set the extents of this view
    void SetExtents(DVec3dCR delta) {_SetExtents(delta);}

    //! Get the 3x3 orthonormal rotation matrix for this view.
    RotMatrix GetRotation() const {return _GetRotation();}

    //! Change the rotation of the view.
    //! @note rot must be orthonormal. For 2d views, only the rotation angle about the z axis is used.
    void SetRotation(RotMatrixCR rot) {_SetRotation(rot);}

    //! Get the target point of the view. If there is no camera, Center() is returned.
    DPoint3d GetTargetPoint() const {return _GetTargetPoint();}

    //! Get the point at the geometric center of the view.
    DGNPLATFORM_EXPORT DPoint3d GetCenter() const;

    //! Get the unit vector that points in the view X (left-to-right) direction.
    DVec3d GetXVector() const {DVec3d v; GetRotation().GetRow(v,0); return v;}

    //! Get the unit vector that points in the view Y (bottom-to-top) direction.
    DVec3d GetYVector() const {DVec3d v; GetRotation().GetRow(v,1); return v;}

    //! Get the unit vector that points in the view Z (front-to-back) direction.
    DVec3d GetZVector() const {DVec3d v; GetRotation().GetRow(v,2); return v;}

    //! Change the view orientation to one of the standard views.
    //! @param[in] standardView the rotation to which the view should be set.
    //! @return SUCCESS if the view was changed.
    DGNPLATFORM_EXPORT BentleyStatus SetStandardViewRotation(StandardView standardView);

    //! Set the clipping volume for elements in this view
    DGNPLATFORM_EXPORT void SetViewClip(ClipVectorPtr clip);

    //! Get the clipping volume for elements in this view
    DGNPLATFORM_EXPORT ClipVectorPtr GetViewClip() const;

    //! Set the grid settings for this view
    DGNPLATFORM_EXPORT void SetGridSettings(GridOrientationType, DPoint2dCR, uint32_t);

    //! Get the grid settings for this view
    DGNPLATFORM_EXPORT void GetGridSettings(GridOrientationType&, DPoint2dR, uint32_t&) const;

    /*=================================================================================**//**
    * Margins for "white space" to be left around view volumes for #LookAtVolume.
    * Values mean "percent of view" and must be between 0 and .25.
    * @bsiclass                                     Grigas.Petraitis                02/13
    +===============+===============+===============+===============+===============+======*/
    struct MarginPercent
    {
    private:
        double m_left;
        double m_top;
        double m_right;
        double m_bottom;

        double LimitMargin(double val) {return (val<0.0) ? 0.0 : (val>.25) ? .25 : val;}

    public:
        MarginPercent(double left, double top, double right, double bottom) {Init(left, top, right, bottom);}
        void Init(double left, double top, double right, double bottom)
            {
            m_left   = LimitMargin(left);
            m_top    = LimitMargin(top);
            m_right  = LimitMargin(right);
            m_bottom = LimitMargin(bottom);
            }

        double Left() const   {return m_left;}
        double Top() const    {return m_top;}
        double Right() const  {return m_right;}
        double Bottom() const {return m_bottom;}
    };

    //! Change the volume that this view displays, keeping its current rotation.
    //! @param[in] worldVolume The new volume, in world-coordinates, for the view. The resulting view will show all of worldVolume, by fitting a
    //! view-axis-aligned bounding box around it. For views that are not aligned with the world coordinate system, this will sometimes
    //! result in a much larger volume than worldVolume.
    //! @param[in] aspectRatio The X/Y aspect ratio of the view into which the result will be displayed. If the aspect ratio of the volume does not
    //! match aspectRatio, the shorter axis is lengthened and the volume is centered. If aspectRatio is nullptr, no adjustment is made.
    //! @param[in] margin The amount of "white space" to leave around the view volume (which essentially increases the volume
    //! of space shown in the view.) If nullptr, no additional white space is added.
    //! @param[in] expandClippingPlanes If false, the front and back clipping planes are not moved. This is rarely desired.
    //! @note For 3d views, the camera is centered on the new volume and moved along the view z axis using the default lens angle
    //! such that the entire volume is visible.
    //! @note, for 2d views, only the X and Y values of volume are used.
    DGNPLATFORM_EXPORT void LookAtVolume(DRange3dCR worldVolume, double const* aspectRatio=nullptr, MarginPercent const* margin=nullptr, bool expandClippingPlanes=true);

    DGNPLATFORM_EXPORT void LookAtViewAlignedVolume(DRange3dCR volume, double const* aspectRatio=nullptr, MarginPercent const* margin=nullptr, bool expandClippingPlanes=true);

    /** @name Thumbnails */
    /** @{ */
    //! Create and save a thumbnail for this ViewDefinition. Thumbnails are saved as DgnViewProperty values.
    //! @param[in] size Optional size (x,y) for the thumbnail. Thumbnails are usually square. Default size is 768x768 pixels.
    //! @param[in] modeOverride Optional override for the RenderMode for the thumbnail. If nullptr, use RenderMode from the DisplayStyle.
    //! @param[in] timeout time, in seconds, to wait for thumbnails to generate.
    //! @return BE_SQLITE_OK if the thumbnail was successfully created and saved.
    DGNVIEW_EXPORT BeSQLite::DbResult RenderAndSaveThumbnail(Point2d size, Render::RenderMode const* modeOverride, BeDuration timeout) const;

    //! Create a thumbnail for this ViewDefinition.
    //! @param[out] image The thumbnail image.
    //! @param[out] modeUsed The RenderMode that was used to generate the thumbnail.
    //! @param[in] size Optional size (x,y) for the thumbnail. Thumbnails are usually square. Default size is 768x768 pixels.
    //! @param[in] modeOverride Optional override for the RenderMode for the thumbnail. If nullptr, use RenderMode from the DisplayStyle.
    //! @param[in] timeout time, in seconds, to wait for thumbnails to generate.
    //! @return BE_SQLITE_OK if the thumbnail was successfully created and saved.
    DGNVIEW_EXPORT BeSQLite::DbResult RenderThumbnail(Render::Image& image, Render::RenderMode& modeUsed, Point2d size, Render::RenderMode const* modeOverride, BeDuration timeout) const;

    //! Create a thumbnail for this ViewDefinition using given view controller.
    //! @param[out] image The thumbnail image.
    //! @param[out] modeUsed The RenderMode that was used to generate the thumbnail.
    //! @param[in] viewController the view controller to be used in the thumbnail generation
    //! @param[in] size Optional size (x,y) for the thumbnail. Thumbnails are usually square. Default size is 768x768 pixels.
    //! @param[in] modeOverride Optional override for the RenderMode for the thumbnail. If nullptr, use RenderMode from the DisplayStyle.
    //! @param[in] timeout time, in seconds, to wait for thumbnails to generate.
    //! @return BE_SQLITE_OK if the thumbnail was successfully created and saved.
    DGNVIEW_EXPORT BeSQLite::DbResult RenderThumbnail(Render::Image& image, Render::RenderMode& modeUsed, ViewControllerPtr viewController, Point2d size, Render::RenderMode const* modeOverride, BeDuration timeout) const;

    //! Save a thumbnail for this ViewDefinition. Thumbnails are saved as DgnViewProperty values.
    //! @param[in] size the size (x,y) of the thumbnail.
    //! @param[in] thumbnail The ImageSource data of the thumbnail
    //! @return BE_SQLITE_OK if the thumbnail was successfully saved.
    //! @note this will overwrite any existing thumbnail for this view
    DGNPLATFORM_EXPORT BeSQLite::DbResult SaveThumbnail(Point2d size, Render::ImageSourceCR thumbnail) const;

    //! Read the thumbnail for this ViewDefinition.
    //! @return the Render::ImageSource holding the compressed stream for the thumbnail. Will be invalid if no thumbnail available.
    DGNPLATFORM_EXPORT Render::ImageSource ReadThumbnail() const;

    //! Get the size (x,y) of the thumbnail for this ViewDefinition.
    //! @return the size of the thumbnail. Will be {0,0} if no thumbnail is available.
    DGNPLATFORM_EXPORT Point2d GetThumbnailSize() const;

    //! Return true if thumbnail is present.
    DGNPLATFORM_EXPORT bool HasThumbnail() const;

    //! Delete the thumbnail for this view.
    DGNPLATFORM_EXPORT void DeleteThumbnail() const;
    /** @} */

    void AdjustAspectRatio(double windowAspect) { _AdjustAspectRatio(windowAspect); } //!< @private
};

/** @addtogroup GROUP_DgnView DgnView Module
<h4>%ViewDefintion3d Camera</h4>

This is what the parameters to the camera methods, and the values stored by ViewDefinition3d mean.
@verbatim
               v-- {origin}
          -----+-------------------------------------- -   [back plane]
          ^\   .                                    /  ^
          | \  .                                   /   |        P
        d |  \ .                                  /    |        o
        e |   \.         {targetPoint}           /     |        s
        l |    |---------------+----------------|      |        i    [focus plane]
        t |     \  ^delta.x    ^               /     b |        t
        a |      \             |              /      a |        i
        . |       \            |             /       c |        v
        z |        \           | f          /        k |        e
          |         \          | o         /         D |        Z
          |          \         | c        /          i |        |
          |           \        | u       /           s |        v
          v            \       | s      /            t |
          -     -       -----  | D -----               |   [front plane]
                ^              | i                     |
                |              | s                     |
    frontDist ->|              | t                     |
                |              |                       |
                v           \  v  / <- lens angle      v
                -              + {eyePoint}            -     positiveX ->

@endverbatim

   Notes:
         - Up vector (positiveY) points out of the screen towards you in this diagram. Likewise delta.y.
         - The view origin is in world coordinates. It is the point at the lower left of the rectangle at the
           focus plane, projected onto the back plane.
         - [delta.x,delta.y] are on the focus plane and delta.z is from the back plane to the front plane.
         - The three view vectors come from:
@verbatim
                {vector from eyePoint->targetPoint} : -Z (positive view Z points towards negative world Z)
                {the up vector}                     : +Y
                {Z cross Y}                         : +X
@endverbatim
           these three vectors form the rows of the view's RotMatrix
         - Objects in space in front of the front plane or behind the back plane are not displayed.
         - The focus plane is not necessarily centered between the front plane and back plane (though it often is). It should generally be
           between the front plane and the back plane.
         - targetPoint is not stored in the view parameters. Instead it may be derived from
           {origin},{eyePoint},[RotMatrix] and focusDist.
         - The view volume is completely specified by: @verbatim {origin}<delta>[RotMatrix] @endverbatim
         - Perspective is determined by {eyePoint}, which is independent of the view volume. Sometimes the eyepoint is not centered
           on the rectangle on the focus plane (that is, a vector from the eyepoint along the viewZ does not hit the view center.)
           This creates a 1-point perspective, which can be disconcerting. It is usually best to keep the camera centered.
         - Cameras hold a "lens angle" value which is defines the field-of-view for the camera in radians.
           The lens angle value is not used to compute the perspective transform for a view. Instead, the lens angle value
           can be used to reposition {eyePoint} when the view volume or target changes.
         - View volumes where one dimension is very small or large relative to the other dimensions (e.g. "long skinny telescope" views,
           or "wide and shallow slices", etc.) are problematic and disallowed based on ratio limits.
*/

//=======================================================================================
//! Defines a view of 3d models.
// @bsiclass                                                      Shaun.Sewall    02/16
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ViewDefinition3d : ViewDefinition
{
    DGNELEMENT_DECLARE_MEMBERS(BIS_CLASS_ViewDefinition3d, ViewDefinition);
    friend struct ViewElementHandler::View3d;

public:
    //=======================================================================================
    //! The current position, lens angle, and focus distance of a camera.
    //=======================================================================================
    struct Camera
    {
    private:
        Angle m_lensAngle;
        double m_focusDistance = 0.0;
        DPoint3d m_eyePoint = {0.0,0.0,0.0};

    public:
        BE_JSON_NAME(lens)
        BE_JSON_NAME(focusDist)
        BE_JSON_NAME(eye)

        static bool IsValidLensAngle(Angle val) {return val.Radians()>(Angle::Pi()/8.0) && val<Angle::AnglePi();}
        void InvalidateFocus() {m_focusDistance=0.0;}
        bool IsFocusValid() const {return m_focusDistance > 0.0 && m_focusDistance<1.0e14;}
        double GetFocusDistance() const {return m_focusDistance;}
        void SetFocusDistance(double dist) {m_focusDistance = dist;}
        bool IsLensValid() const {return IsValidLensAngle(m_lensAngle);}
        void ValidateLens() {if (!IsLensValid()) m_lensAngle=Angle::FromRadians(Angle::PiOver2());}
        Angle GetLensAngle() const {return m_lensAngle;}
        void SetLensAngle(Angle angle) {m_lensAngle = angle;}
        DPoint3dCR GetEyePoint() const {return m_eyePoint;}
        void SetEyePoint(DPoint3dCR pt) {m_eyePoint = pt;}
        bool IsValid() const {return IsLensValid() && IsFocusValid();}
        bool IsEqual(Camera const& other) const {return m_lensAngle==other.m_lensAngle && m_focusDistance==other.m_focusDistance && m_eyePoint.IsEqual(other.m_eyePoint);}
        Json::Value ToJson() const;
        static Camera FromJson(JsonValueCR );
    };

    //! Parameters used to construct a ViewDefinition3d
    struct CreateParams : T_Super::CreateParams
    {
        DEFINE_T_SUPER(ViewDefinition3d::T_Super::CreateParams);

        bool m_cameraOn = false;    //!< if true, m_camera is valid.
        DPoint3d m_origin = {0.0,0.0,0.0};    //!< The lower left back corner of the view frustum.
        DVec3d m_extents = DVec3d::From(0.0,0.0,0.0); //!< The extent of the view frustum.
        RotMatrix m_rotation = RotMatrix::FromIdentity(); //!< Rotation of the view frustum.
        Camera m_cameraDef;  //!< The camera used for this view.
        DisplayStyle3dPtr m_displayStyle;

        CreateParams(DgnDbR db, DgnModelId modelId, DgnClassId classId, DgnCodeCR code, CategorySelectorR categorySelector, DisplayStyle3dR displayStyle, Camera const* camera = nullptr)
            : T_Super(db, modelId, classId, code, categorySelector), m_displayStyle(&displayStyle) {if (camera) {m_cameraDef = *camera; m_cameraOn=true;}}

        explicit CreateParams(DgnElement::CreateParams const& params) : T_Super(params) {}
    };

protected:
    bool m_cameraOn = false;    //!< if true, m_camera is valid.
    DPoint3d m_origin = {0.0,0.0,0.0};    //!< The lower left back corner of the view frustum.
    DVec3d m_extents = DVec3d::From(0.0,0.0,0.0); //!< The extent of the view frustum.
    RotMatrix m_rotation = RotMatrix::FromIdentity(); //!< Rotation of the view frustum.
    Camera m_cameraDef;  //!< The camera used for this view.

    DGNPLATFORM_EXPORT DgnDbStatus _ReadSelectParams(BeSQLite::EC::ECSqlStatement&, ECSqlClassParamsCR) override;
    DGNPLATFORM_EXPORT void _ToJson(JsonValueR out, JsonValueCR opts) const override;
    DGNPLATFORM_EXPORT void _FromJson(JsonValueR props) override;
    DGNPLATFORM_EXPORT void _BindWriteParams(BeSQLite::EC::ECSqlStatement&, ForInsert) override;
    DGNPLATFORM_EXPORT bool _EqualState(ViewDefinitionR) override;
    DGNPLATFORM_EXPORT void _CopyFrom(DgnElementCR el) override;
    DGNPLATFORM_EXPORT void SaveCamera();
    DGNPLATFORM_EXPORT ViewportStatus _SetupFromFrustum(Frustum const& inFrustum) override;
    DGNPLATFORM_EXPORT DPoint3d _GetTargetPoint() const override;
    static double CalculateMaxDepth(DVec3dCR delta, DVec3dCR zVec);

    DPoint3d _GetOrigin() const override {return m_origin;}
    DVec3d _GetExtents() const override {return m_extents;}
    RotMatrix _GetRotation() const override {return m_rotation;}
    void _SetOrigin(DPoint3dCR origin) override {m_origin = origin;}
    void _SetExtents(DVec3dCR extents) override {m_extents = extents;}
    void _SetRotation(RotMatrixCR rot) override {m_rotation = rot;}
    ViewDefinition3dCP _ToView3d() const override final {return this;}
    virtual void _EnableCamera() {m_cameraOn = true;}
    virtual bool _SupportsCamera() const {return true;}

public:
    BE_JSON_NAME(cameraOn)
    BE_JSON_NAME(origin)
    BE_JSON_NAME(extents)
    BE_JSON_NAME(angles)
    BE_JSON_NAME(camera)

    double MinimumFrontDistance(double nearScaleLimit = 5.0E-5 /* between resolution of 24 and 32bit */) const;
    static DgnClassId QueryClassId(DgnDbR db) {return DgnClassId(db.Schemas().GetClassId(BIS_ECSCHEMA_NAME, BIS_CLASS_ViewDefinition3d));} //!< private
    void VerifyFocusPlane();//!< private
    bool IsEyePointAbove(double elevation) const {return !IsCameraOn() ? (GetZVector().z > 0) : (GetEyePoint().z > elevation);}//!< private
    DGNPLATFORM_EXPORT DPoint3d ComputeEyePoint(Frustum const& frust) const;//!< private

    explicit ViewDefinition3d(CreateParams const& params) : T_Super(params), m_cameraOn(params.m_cameraOn), m_origin(params.m_origin), m_extents(params.m_extents), 
                                                        m_rotation(params.m_rotation), m_cameraDef(params.m_cameraDef) {if (params.m_displayStyle.IsValid()) SetDisplayStyle3d(*params.m_displayStyle);}

    DisplayStyle3dR GetDisplayStyle3d() {return (DisplayStyle3dR) GetDisplayStyle();}
    void SetDisplayStyle3d(DisplayStyle3dR style) {T_Super::SetupDisplayStyle(style);}

    //! Get the camera for this view
    Camera const& GetCamera() const {return m_cameraDef;}

    //! Get a writable reference to the camera for this view
    Camera& GetCameraR() {return m_cameraDef;}

    //! Determine whether the camera is on for this view
    bool IsCameraOn() const {return m_cameraOn;}

    //! Turn the camera off for this view. After this call, the camera parameters in this view definition are ignored and views that use it will
    //! display with an orthographic (infinite focal length) projection of the view volume from the view direction. 
    //! @note To turn the camera back on, call LookAt
    void TurnCameraOff() {m_cameraOn = false;}

    //! Turn the camera on for this view.  This should be used onl after the camera parameters have been set for this view. 
    //! for internal use only -- generally LookAt should be used.
    void TurnCameraOn() {m_cameraOn = true;}

    //! Determine whether the camera is valid for this view
    bool IsCameraValid() const {return m_cameraDef.IsValid();}

    //! Return true if ViewDefinition supports Camera.
    bool SupportsCamera() const {return _SupportsCamera();}

    //! Calculate the lens angle formed by the current delta and focus distance
    DGNPLATFORM_EXPORT Angle CalcLensAngle() const;

    //! Position the camera for this view and point it at a new target point.
    //! @param[in] eyePoint The new location of the camera.
    //! @param[in] targetPoint The new location to which the camera should point. This becomes the center of the view on the focus plane.
    //! @param[in] upVector A vector that orients the camera's "up" (view y). This vector must not be parallel to the vector from eye to target.
    //! @param[in] viewDelta The new size (width and height) of the view rectangle. The view rectangle is on the focus plane centered on the targetPoint.
    //! If viewDelta is nullptr, the existing size is unchanged.
    //! @param[in] frontDistance The distance from the eyePoint to the front plane. If nullptr, the existing front distance is used.
    //! @param[in] backDistance The distance from the eyePoint to the back plane. If nullptr, the existing back distance is used.
    //! @return a status indicating whether the camera was successfully positioned. See values at #ViewportStatus for possible errors.
    //! @note If the aspect ratio of viewDelta does not match the aspect ratio of a DgnViewport into which this view is displayed, it will be
    //! adjusted when the DgnViewport is synchronized from this view.
    //! @note This method modifies this ViewController. If this ViewController is attached to DgnViewport, you must call DgnViewport::SynchWithViewController
    //! to see the new changes in the DgnViewport.
    DGNPLATFORM_EXPORT ViewportStatus LookAt(DPoint3dCR eyePoint, DPoint3dCR targetPoint, DVec3dCR upVector,
                                             DVec2dCP viewDelta=nullptr, double const* frontDistance=nullptr, double const* backDistance=nullptr);

    //! Position the camera for this view and point it at a new target point, using a specified lens angle.
    //! @param[in] eyePoint The new location of the camera.
    //! @param[in] targetPoint The new location to which the camera should point. This becomes the center of the view on the focus plane.
    //! @param[in] upVector A vector that orients the camera's "up" (view y). This vector must not be parallel to the vector from eye to target.
    //! @param[in] fov The angle, in radians, that defines the field-of-view for the camera. Must be between .0001 and pi.
    //! @param[in] frontDistance The distance from the eyePoint to the front plane. If nullptr, the existing front distance is used.
    //! @param[in] backDistance The distance from the eyePoint to the back plane. If nullptr, the existing back distance is used.
    //! @return Status indicating whether the camera was successfully positioned. See values at #ViewportStatus for possible errors.
    //! @note The aspect ratio of the view remains unchanged.
    //! @note This method modifies this ViewController. If this ViewController is attached to DgnViewport, you must call DgnViewport::SynchWithViewController
    //! to see the new changes in the DgnViewport.
    DGNPLATFORM_EXPORT ViewportStatus LookAtUsingLensAngle(DPoint3dCR eyePoint, DPoint3dCR targetPoint, DVec3dCR upVector,
                                             Angle fov, double const* frontDistance=nullptr, double const* backDistance=nullptr);

    //! Move the camera relative to its current location by a distance in camera coordinates.
    //! @param[in] distance to move camera. Length is in world units, direction relative to current camera orientation.
    //! @return Status indicating whether the camera was successfully positioned. See values at #ViewportStatus for possible errors.
    //! @note This method modifies this ViewController. If this ViewController is attached to DgnViewport, you must call DgnViewport::SynchWithViewController
    //! to see the new changes in the DgnViewport.
    DGNPLATFORM_EXPORT ViewportStatus MoveCameraLocal(DVec3dCR distance);

    //! Move the camera relative to its current location by a distance in world coordinates.
    //! @param[in] distance in world units.
    //! @return Status indicating whether the camera was successfully positioned. See values at #ViewportStatus for possible errors.
    //! @note This method modifies this ViewController. If this ViewController is attached to DgnViewport, you must call DgnViewport::SynchWithViewController
    //! to see the new changes in the DgnViewport.
    DGNPLATFORM_EXPORT ViewportStatus MoveCameraWorld(DVec3dCR distance);

    //! Rotate the camera from its current location about an axis relative to its current orientation.
    //! @param[in] angle The angle to rotate the camera, in radians.
    //! @param[in] axis The axis about which to rotate the camera. The axis is a direction relative to the current camera orientation.
    //! @param[in] aboutPt The point, in world coordinates, about which the camera is rotated. If aboutPt is nullptr, the camera rotates in place
    //! (i.e. about the current eyePoint).
    //! @note Even though the axis is relative to the current camera orientation, the aboutPt is in world coordinates, \b not relative to the camera.
    //! @return Status indicating whether the camera was successfully positioned. See values at #ViewportStatus for possible errors.
    //! @note This method modifies this ViewController. If this ViewController is attached to DgnViewport, you must call DgnViewport::SynchWithViewController
    //! to see the new changes in the DgnViewport.
    DGNPLATFORM_EXPORT ViewportStatus RotateCameraLocal(double angle, DVec3dCR axis, DPoint3dCP aboutPt=nullptr);

    //! Rotate the camera from its current location about an axis in world coordinates.
    //! @param[in] angle The angle to rotate the camera, in radians.
    //! @param[in] axis The world-based axis (direction) about which to rotate the camera.
    //! @param[in] aboutPt The point, in world coordinates, about which the camera is rotated. If aboutPt is nullptr, the camera rotates in place
    //! (i.e. about the current eyePoint).
    //! @return Status indicating whether the camera was successfully positioned. See values at #ViewportStatus for possible errors.
    //! @note This method modifies this ViewController. If this ViewController is attached to DgnViewport, you must call DgnViewport::SynchWithViewController
    //! to see the new changes in the DgnViewport.
    DGNPLATFORM_EXPORT ViewportStatus RotateCameraWorld(double angle, DVec3dCR axis, DPoint3dCP aboutPt=nullptr);

    //! Get the distance from the eyePoint to the front plane for this view.
    double GetFrontDistance() const {return GetBackDistance() - GetExtents().z;}

    //! Get the distance from the eyePoint to the back plane for this view.
    DGNPLATFORM_EXPORT double GetBackDistance() const;

    //! Place the eyepoint of the camera so it is centered in the view. This removes any 1-point perspective skewing that may be
    //! present in the current view.
    //! @param[in] backDistance optional, If not nullptr, the new the distance from the eyepoint to the back plane. Otherwise the distance from the
    //! current eyepoint is used.
    DGNPLATFORM_EXPORT void CenterEyePoint(double const* backDistance=nullptr);

    //! Center the focus distance of the camera halfway between the front plane and the back plane, keeping the eyepoint,
    //! lens angle, rotation, back distance, and front distance unchanged.
    //! @note The focus distance, origin, and delta values are modified, but the view encloses the same volume and appears visually unchanged.
    DGNPLATFORM_EXPORT void CenterFocusDistance();

    //! Get the current location of the eyePoint for camera in this view.
    DPoint3dCR GetEyePoint() const {return GetCamera().GetEyePoint();}

    //! Get the lens angle for this view.
    Angle GetLensAngle() const {return GetCamera().GetLensAngle();}

    //! Set the lens angle for this view.
    //! @param[in] angle The new lens angle in radians. Must be greater than 0 and less than pi.
    //! @note This does not change the view's current field-of-view. Instead, it changes the lens that will be used if the view
    //! is subsequently modified and the lens angle is used to position the eyepoint.
    //! @note To change the field-of-view (i.e. "zoom") of a view, pass a new viewDelta to #LookAt
    void SetLensAngle(Angle angle) {GetCameraR().SetLensAngle(angle);}

    //! Change the location of the eyePoint for the camera in this view.
    //! @param[in] pt The new eyepoint.
    //! @note This method is generally for internal use only. Moving the eyePoint arbitrarily can result in skewed or illegal perspectives.
    //! The most common method for user-level camera positioning is #LookAt.
    void SetEyePoint(DPoint3dCR pt) {GetCameraR().SetEyePoint(pt);}

    //! Set the focus distance for this view.
    //! @note Changing the focus distance changes the plane on which the delta.x and delta.y values lie. So, changing focus distance
    //! without making corresponding changes to delta.x and delta.y essentially changes the lens angle, causing a "zoom" effect.
    void SetFocusDistance(double dist) {GetCameraR().SetFocusDistance(dist);}

    //! Get the distance from the eyePoint to the focus plane for this view.
    double GetFocusDistance() const {return GetCamera().GetFocusDistance();}
};

//=======================================================================================
//! Defines a view of one or more SpatialModels.
//! The list of viewed models is stored by the ModelSelector.
// @bsiclass                                                      Paul.Connelly   10/15
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE SpatialViewDefinition : ViewDefinition3d
{
    DGNELEMENT_DECLARE_MEMBERS(BIS_CLASS_SpatialViewDefinition, ViewDefinition3d);

public:
    //! Parameters used to construct a SpatialViewDefinition
    struct CreateParams : T_Super::CreateParams
    {
        DEFINE_T_SUPER(SpatialViewDefinition::T_Super::CreateParams);
        ModelSelectorPtr m_modelSelector;

    public:
        CreateParams(DgnDbR db, DgnModelId modelId, DgnClassId classId, DgnCodeCR code, CategorySelectorR categorySelector, DisplayStyle3dR displayStyle, ModelSelectorR modelSelector, Camera const* camera=nullptr)
            : T_Super(db, modelId, classId, code, categorySelector, displayStyle, camera), m_modelSelector(&modelSelector) {}

        explicit CreateParams(DgnElement::CreateParams const& params) : T_Super(params) {}
    };

protected:
    DgnElementId m_modelSelectorId;
    mutable ModelSelectorPtr m_modelSelector;

    DGNPLATFORM_EXPORT BentleyStatus _ValidateState() override; 
    DGNPLATFORM_EXPORT DgnDbStatus _ReadSelectParams(BeSQLite::EC::ECSqlStatement&, ECSqlClassParamsCR) override;
    DGNPLATFORM_EXPORT void _ToJson(JsonValueR out, JsonValueCR opts) const override;
    DGNPLATFORM_EXPORT void _FromJson(JsonValueR props) override;
    DGNPLATFORM_EXPORT void _BindWriteParams(BeSQLite::EC::ECSqlStatement&, ForInsert) override;
    DGNPLATFORM_EXPORT bool _EqualState(ViewDefinitionR) override;
    DGNPLATFORM_EXPORT DgnDbStatus _OnInsert() override;
    void _OnInserted(DgnElementP copiedFrom) const override {m_modelSelector=nullptr; T_Super::_OnInserted(copiedFrom);}
    void _OnUpdateFinished() const override {m_modelSelector=nullptr; T_Super::_OnUpdateFinished();}
    DGNPLATFORM_EXPORT void _CopyFrom(DgnElementCR el) override;
    bool _ViewsModel(DgnModelId modelId) override {return GetModelSelector().ContainsModel(modelId);}
    SpatialViewDefinitionCP _ToSpatialView() const override {return this;}
    DGNPLATFORM_EXPORT ViewControllerPtr _SupplyController() const override;

public:
    BE_JSON_NAME(modelSelectorId)

    static DgnClassId QueryClassId(DgnDbR db) {return DgnClassId(db.Schemas().GetClassId(BIS_ECSCHEMA_NAME, BIS_CLASS_SpatialViewDefinition));} //!< private

    //! Create a SpatialViewDefinition from CreateParams
    explicit SpatialViewDefinition(CreateParams const& params) : T_Super(params) {if (params.m_modelSelector.IsValid()) SetModelSelector(*params.m_modelSelector);}

    //! Construct a SpatialViewDefinition in the specified DefinitionModel
    SpatialViewDefinition(DefinitionModelR model, Utf8StringCR name, CategorySelectorR categories, DisplayStyle3dR displayStyle, ModelSelectorR modelSelector, Camera const* camera = nullptr) :
        T_Super(T_Super::CreateParams(model.GetDgnDb(), model.GetModelId(), QueryClassId(model.GetDgnDb()), CreateCode(model, name), categories, displayStyle, camera)) {SetModelSelector(modelSelector);}

    //! Get a writable reference to the ModelSelector for this SpatialViewDefinition
    DGNPLATFORM_EXPORT ModelSelectorR GetModelSelector();
    DgnElementId GetModelSelectorId() const {return m_modelSelectorId;}

    //! Set the ModelSelector for this SpatialViewDefinition
    //! @param[in] models The new ModelSelector.
    void SetModelSelector(ModelSelectorR models) {BeAssert(!IsPersistent()); m_modelSelector = &models; m_modelSelectorId=models.GetElementId();}
};

//=======================================================================================
//! Defines a spatial view that displays geometry on the image plane using a parallel orthographic projection.
// @bsiclass                                                      Sam.Wilson    08/16
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE OrthographicViewDefinition : SpatialViewDefinition
{
    DGNELEMENT_DECLARE_MEMBERS(BIS_CLASS_OrthographicViewDefinition, SpatialViewDefinition);
    friend struct ViewElementHandler::OrthographicView;

protected:
    //! Construct a new OrthographicViewDefinition prior to loading it
    explicit OrthographicViewDefinition(CreateParams const& params) : T_Super(params) {}

    OrthographicViewDefinitionCP _ToOrthographicView() const override {return this;}
    DGNPLATFORM_EXPORT ViewControllerPtr _SupplyController() const override;
    void _EnableCamera() override final {/* nope */}
    bool _SupportsCamera() const override final {return false;}

public:
    //! Construct a new OrthographicViewDefinition in the specified DefinitionModel prior to inserting it
    OrthographicViewDefinition(DefinitionModelR model, Utf8StringCR name, CategorySelectorR categories, DisplayStyle3dR displayStyle, ModelSelectorR modelSelector) :
        T_Super(CreateParams(model.GetDgnDb(), model.GetModelId(), QueryClassId(model.GetDgnDb()), CreateCode(model, name), categories, displayStyle, modelSelector)) {}

    //! Look up the ECClass Id used for OrthographicViewDefinitions within the specified DgnDb
    static DgnClassId QueryClassId(DgnDbR db) {return DgnClassId(db.Schemas().GetClassId(BIS_ECSCHEMA_NAME, BIS_CLASS_OrthographicViewDefinition));}
};

//=======================================================================================
//! Defines a view of a 2d model.
// @bsiclass                                                      Paul.Connelly   10/15
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ViewDefinition2d : ViewDefinition
{
    DGNELEMENT_DECLARE_MEMBERS(BIS_CLASS_ViewDefinition2d, ViewDefinition);
    friend struct ViewElementHandler::View2d;

protected:
    DgnModelId m_baseModelId;   //!< The model displayed in this view
    DPoint2d m_origin={0.0,0.0}; //!< The lower left corner of the view frustum.
    DVec2d m_delta=DVec2d::From(0.0,0.0);   //!< The extent of the view frustum.
    double m_rotAngle=0.0;      //!< Rotation of the view frustum.

    DGNPLATFORM_EXPORT void _RemapIds(DgnImportContext& importer) override;

    DGNPLATFORM_EXPORT DgnDbStatus _ReadSelectParams(BeSQLite::EC::ECSqlStatement&, ECSqlClassParamsCR) override;
    DGNPLATFORM_EXPORT void _ToJson(JsonValueR out, JsonValueCR opts) const override;
    DGNPLATFORM_EXPORT void _FromJson(JsonValueR props) override;
    DGNPLATFORM_EXPORT void _BindWriteParams(BeSQLite::EC::ECSqlStatement&, ForInsert) override;
    DGNPLATFORM_EXPORT bool _EqualState(ViewDefinitionR) override;
    DGNPLATFORM_EXPORT void _CopyFrom(DgnElementCR el) override;
    ViewDefinition2dCP _ToView2d() const override final {return this;}
    DPoint3d _GetOrigin() const override {return DPoint3d::From(m_origin.x, m_origin.y);}
    void _SetExtents(DVec3dCR delta) override {m_delta.x = delta.x; m_delta.y = delta.y;}
    void _SetOrigin(DPoint3dCR origin) override {m_origin.x = origin.x; m_origin.y = origin.y;}
    void _SetRotation(RotMatrixCR rot) override {DVec3d xColumn; rot.GetColumn(xColumn, 0); m_rotAngle = atan2(xColumn.y, xColumn.x);}
    DVec3d _GetExtents() const override {return DVec3d::From(m_delta.x, m_delta.y);}
    RotMatrix _GetRotation() const override {return RotMatrix::FromAxisAndRotationAngle(2, m_rotAngle);}
    bool _ViewsModel(DgnModelId mid) override {return mid == m_baseModelId;}
    explicit ViewDefinition2d(CreateParams const& params) : T_Super(params) {}

public:
    BE_JSON_NAME(baseModelId)
    BE_JSON_NAME(origin)
    BE_JSON_NAME(delta)
    BE_JSON_NAME(angle)

    ViewDefinition2d(DefinitionModelR model, Utf8StringCR name, DgnClassId classId, DgnModelId baseModelId, CategorySelectorR categorySelector, DisplayStyle2dR displayStyle) :
            T_Super(CreateParams(model.GetDgnDb(), model.GetModelId(), classId, CreateCode(model, name), categorySelector)), m_baseModelId(baseModelId) {SetDisplayStyle2d(displayStyle);}

    //! Set the DisplayStyle for this view.
    void SetDisplayStyle2d(DisplayStyle2dR style) {T_Super::SetupDisplayStyle(style);}

    DgnModelId GetBaseModelId() const {return m_baseModelId;}   //!< Get the model displayed in this view
    double GetRotAngle() const {return m_rotAngle;}
    void SetRotAngle(double val) {m_rotAngle = val;}
    DPoint2d GetOrigin2d() const {return m_origin;}
    void SetOrigin2d(DPoint2dCR o) {m_origin = o;}
    DVec2d GetDelta2d() const {return m_delta;}
    void SetDelta2d(DVec2dCR v) {m_delta = v;}
};

//=======================================================================================
//! Defines a view of a DrawingModel.
// @bsiclass                                                      Paul.Connelly   10/15
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE DrawingViewDefinition : ViewDefinition2d
{
    DGNELEMENT_DECLARE_MEMBERS(BIS_CLASS_DrawingViewDefinition, ViewDefinition2d);
    friend struct ViewElementHandler::DrawingView;

protected:
    DGNPLATFORM_EXPORT ViewControllerPtr _SupplyController() const override;

    DrawingViewDefinitionCP _ToDrawingView() const override {return this;}

    //! Construct a DrawingViewDefinition from the supplied params prior to loading
    explicit DrawingViewDefinition(CreateParams const& params) : T_Super(params) {}

public:
    //! Construct a DrawingViewDefinition subclass in the specified DefinitionModel prior to inserting it
    DrawingViewDefinition(DefinitionModelR model, Utf8StringCR name, DgnClassId classId, DgnModelId baseModelId, CategorySelectorR categories, DisplayStyle2dR displayStyle) :
        T_Super(model, name, classId, baseModelId, categories, displayStyle) {}

    //! Construct a DrawingViewDefinition in the specified DefinitionModel prior to inserting it
    DrawingViewDefinition(DefinitionModelR model, Utf8StringCR name, DgnModelId baseModelId, CategorySelectorR categories, DisplayStyle2dR displayStyle) :
        T_Super(model, name, QueryClassId(model.GetDgnDb()), baseModelId, categories, displayStyle) {}

    DGNPLATFORM_EXPORT DrawingViewControllerPtr LoadViewController(bool allowOverrides=true) const;

    //! Look up the ECClass Id used for DrawingViewDefinitions in the specified DgnDb
    static DgnClassId QueryClassId(DgnDbR db) {return DgnClassId(db.Schemas().GetClassId(BIS_ECSCHEMA_NAME, BIS_CLASS_DrawingViewDefinition));}
};

//=======================================================================================
//! Defines a view of a SheetModel
// @bsiclass                                                      Paul.Connelly   10/15
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE SheetViewDefinition : ViewDefinition2d
{
    DGNELEMENT_DECLARE_MEMBERS(BIS_CLASS_SheetViewDefinition, ViewDefinition2d);
    friend struct ViewElementHandler::SheetView;

protected:
    DGNPLATFORM_EXPORT ViewControllerPtr _SupplyController() const override;

    void _GetExtentLimits(double& minExtent, double& maxExtent) const override {minExtent=DgnUnits::OneMillimeter(); maxExtent=100*DgnUnits::OneMeter();}
    SheetViewDefinitionCP _ToSheetView() const override {return this;}

    //! Construct a SheetViewDefinition from the supplied params prior to loading it
    explicit SheetViewDefinition(CreateParams const& params) : T_Super(params) {}

public:
    //! Construct a new SheetViewDefinition subclass prior to inserting it
    SheetViewDefinition(DefinitionModelR model, Utf8StringCR name, DgnClassId classId, DgnModelId baseModelId, CategorySelectorR categories, DisplayStyle2dR displayStyle) :
        T_Super(model, name, classId, baseModelId, categories, displayStyle) {}

    //! Construct a new SheetViewDefinition prior to inserting it
    SheetViewDefinition(DefinitionModelR model, Utf8StringCR name, DgnModelId baseModelId, CategorySelectorR categories, DisplayStyle2dR displayStyle) :
        T_Super(model, name, QueryClassId(model.GetDgnDb()), baseModelId, categories, displayStyle) {}

    DGNPLATFORM_EXPORT Sheet::ViewControllerPtr LoadViewController(bool allowOverrides=true) const;

    //! Look up the ECClass Id used for SheetViewDefinitions in the specified DgnDb
    static DgnClassId QueryClassId(DgnDbR db) {return DgnClassId(db.Schemas().GetClassId(BIS_ECSCHEMA_NAME, BIS_CLASS_SheetViewDefinition));}
};

//=======================================================================================
//! A ViewDefinition used to display a 2D template model.
// @bsiclass                                                      Shaun.Sewall    03/17
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE TemplateViewDefinition2d : ViewDefinition2d
{
    DGNELEMENT_DECLARE_MEMBERS(BIS_CLASS_TemplateViewDefinition2d, ViewDefinition2d);
    friend struct ViewElementHandler::TemplateView2d;

private:
    // Not persisted, only a necessary run-time variable of the current viewed model
    DgnModelId  m_viewedModel;

protected:
    TemplateViewDefinition2dCP _ToTemplateView2d() const override final {return this;}
    bool _ViewsModel(DgnModelId modelId) override final {return modelId == m_viewedModel;}
    DGNPLATFORM_EXPORT ViewControllerPtr _SupplyController() const override;
    explicit TemplateViewDefinition2d(CreateParams const& params) : T_Super(params) {}

public:
    void SetViewedModel(DgnModelId viewedModel) { m_viewedModel = viewedModel; }
    DgnModelId GetViewedModel() const { return m_viewedModel; }

    DGNPLATFORM_EXPORT static TemplateViewDefinition2dPtr Create(DefinitionModelR definitionModel, Utf8StringCR name, CategorySelectorP categories=nullptr, DisplayStyle2dP displayStyle=nullptr);
};

//=======================================================================================
//! A ViewDefinition used to display a 3D template model.
// @bsiclass                                                      Shaun.Sewall    02/17
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE TemplateViewDefinition3d : ViewDefinition3d
{
    DGNELEMENT_DECLARE_MEMBERS(BIS_CLASS_TemplateViewDefinition3d, ViewDefinition3d);
    friend struct ViewElementHandler::TemplateView3d;

private:
    // Not persisted, only a necessary run-time variable of the current viewed model
    DgnModelId  m_viewedModel;

protected:
    TemplateViewDefinition3dCP _ToTemplateView3d() const override final {return this;}
    bool _ViewsModel(DgnModelId modelId) override final {return modelId == m_viewedModel;}
    DGNPLATFORM_EXPORT ViewControllerPtr _SupplyController() const override;
    explicit TemplateViewDefinition3d(CreateParams const& params) : T_Super(params) {}

public:
    void SetViewedModel(DgnModelId viewedModel) { m_viewedModel = viewedModel; }
    DgnModelId GetViewedModel() const { return m_viewedModel; }

    DGNPLATFORM_EXPORT static TemplateViewDefinition3dPtr Create(DefinitionModelR definitionModel, Utf8StringCR name, CategorySelectorP categories=nullptr, DisplayStyle3dP displayStyle=nullptr);
};

namespace ViewElementHandler
{
    using dgn_ElementHandler::Definition;
    struct View : Definition
    {
        ELEMENTHANDLER_DECLARE_MEMBERS_ABSTRACT(BIS_CLASS_ViewDefinition, ViewDefinition, View, Definition, DGNPLATFORM_EXPORT);
        DGNPLATFORM_EXPORT void _RegisterPropertyAccessors(ECSqlClassInfo&, ECN::ClassLayoutCR) override;
    };

    struct View3d : View
    {
        ELEMENTHANDLER_DECLARE_MEMBERS_ABSTRACT(BIS_CLASS_ViewDefinition3d, ViewDefinition3d, View3d, View, DGNPLATFORM_EXPORT);
        DGNPLATFORM_EXPORT void _RegisterPropertyAccessors(ECSqlClassInfo&, ECN::ClassLayoutCR) override;
    };

    struct SpatialView : View3d
    {
        ELEMENTHANDLER_DECLARE_MEMBERS(BIS_CLASS_SpatialViewDefinition, SpatialViewDefinition, SpatialView, View3d, DGNPLATFORM_EXPORT);
        DGNPLATFORM_EXPORT void _RegisterPropertyAccessors(ECSqlClassInfo&, ECN::ClassLayoutCR) override;
    };

    struct View2d : View
    {
        ELEMENTHANDLER_DECLARE_MEMBERS_ABSTRACT(BIS_CLASS_ViewDefinition2d, ViewDefinition2d, View2d, View, DGNPLATFORM_EXPORT);
        DGNPLATFORM_EXPORT void _RegisterPropertyAccessors(ECSqlClassInfo&, ECN::ClassLayoutCR) override;
    };

    struct OrthographicView : SpatialView
    {
        ELEMENTHANDLER_DECLARE_MEMBERS(BIS_CLASS_OrthographicViewDefinition, OrthographicViewDefinition, OrthographicView, SpatialView, DGNPLATFORM_EXPORT);
    };

    struct DrawingView : View2d
    {
        ELEMENTHANDLER_DECLARE_MEMBERS(BIS_CLASS_DrawingViewDefinition, DrawingViewDefinition, DrawingView, View2d, DGNPLATFORM_EXPORT);
    };

    struct SheetView : View2d
    {
        ELEMENTHANDLER_DECLARE_MEMBERS(BIS_CLASS_SheetViewDefinition, SheetViewDefinition, SheetView, View2d, DGNPLATFORM_EXPORT);
    };

    struct TemplateView2d : View2d
    {
        ELEMENTHANDLER_DECLARE_MEMBERS(BIS_CLASS_TemplateViewDefinition2d, TemplateViewDefinition2d, TemplateView2d, View2d, DGNPLATFORM_EXPORT);
    };

    struct TemplateView3d : View3d
    {
        ELEMENTHANDLER_DECLARE_MEMBERS(BIS_CLASS_TemplateViewDefinition3d, TemplateViewDefinition3d, TemplateView3d, View3d, DGNPLATFORM_EXPORT);
    };

    struct ViewModels : Definition
    {
        ELEMENTHANDLER_DECLARE_MEMBERS(BIS_CLASS_ModelSelector, ModelSelector, ViewModels, Definition, DGNPLATFORM_EXPORT);
    };

    struct ViewCategories : Definition
    {
        ELEMENTHANDLER_DECLARE_MEMBERS(BIS_CLASS_CategorySelector, CategorySelector, ViewCategories, Definition, DGNPLATFORM_EXPORT);
    };

    struct ViewDisplayStyle : Definition
    {
        ELEMENTHANDLER_DECLARE_MEMBERS(BIS_CLASS_DisplayStyle, DisplayStyle, ViewDisplayStyle, Definition, DGNPLATFORM_EXPORT);
    };

    struct ViewDisplayStyle2d : ViewDisplayStyle
    {
        ELEMENTHANDLER_DECLARE_MEMBERS(BIS_CLASS_DisplayStyle2d, DisplayStyle2d, ViewDisplayStyle2d, ViewDisplayStyle, DGNPLATFORM_EXPORT);
    };

    struct ViewDisplayStyle3d : ViewDisplayStyle
    {
        ELEMENTHANDLER_DECLARE_MEMBERS(BIS_CLASS_DisplayStyle3d, DisplayStyle3d, ViewDisplayStyle3d, ViewDisplayStyle, DGNPLATFORM_EXPORT);
    };
} // namespace ViewElementHandler

typedef ViewElementHandler::SpatialView SpatialViewHandler;
typedef ViewElementHandler::DrawingView DrawingViewHandler;

//=======================================================================================
//! Handler extension applied to a ViewDefinition handler to override the type of
//! ViewController supplied for ViewDefinitions of that handler's type.
// @bsiclass                                                      Paul.Connelly   10/15
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ViewControllerOverride : DgnDomain::Handler::Extension
{
    HANDLER_EXTENSION_DECLARE_MEMBERS(ViewControllerOverride, DGNPLATFORM_EXPORT);
public:
    //! @param[in] view The ViewDefinition
    //! @return an instance of a ViewController for the supplied ViewDefinition, or nullptr if the ViewDefinition is not of interest.
    virtual ViewControllerPtr _SupplyController(ViewDefinitionCR view) const = 0;
};

END_BENTLEY_DGN_NAMESPACE
