/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include "DgnDb.h"
#include "DgnElement.h"
#include "ElementHandler.h"

DGNPLATFORM_TYPEDEFS(DgnCategory);
DGNPLATFORM_TYPEDEFS(DgnSubCategory);
DGNPLATFORM_TYPEDEFS(DrawingCategory);
DGNPLATFORM_TYPEDEFS(SpatialCategory);
DGNPLATFORM_REF_COUNTED_PTR(DgnCategory);
DGNPLATFORM_REF_COUNTED_PTR(DgnSubCategory);
DGNPLATFORM_REF_COUNTED_PTR(DrawingCategory);
DGNPLATFORM_REF_COUNTED_PTR(SpatialCategory);

BEGIN_BENTLEY_DGN_NAMESPACE

namespace dgn_ElementHandler {struct Category; struct DrawingCategory; struct SpatialCategory; struct SubCategory;}

typedef bvector<DgnCategoryId> DgnCategoryIdList;

/**
* @addtogroup GROUP_Appearance Appearance Module
* Types related to determining how graphics appear when drawn
*/

//! A sub-category of a category.
//! @ingroup GROUP_DgnCategory
//=======================================================================================
// @bsiclass                                                    Keith.Bentley   02/17
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE DgnSubCategory : DefinitionElement
{
    DGNELEMENT_DECLARE_MEMBERS(BIS_CLASS_SubCategory, DefinitionElement);
public:
    BE_PROP_NAME(Properties)
    BE_PROP_NAME(Description)

    BE_JSON_NAME(appearance)
    BE_JSON_NAME(description)

    //! The parameters that can determine how graphics on a SubCategory appear when drawn.
    //! @ingroup GROUP_DgnCategory
    //! @ingroup GROUP_Appearance
    struct Appearance
    {
    private:
        bool m_invisible;    //!< Graphics on this SubCategory should not be visible
        bool m_dontPlot;     //!< Graphics on this SubCategory should not be plotted
        bool m_dontSnap;     //!< Graphics on this SubCategory should not be snappable
        bool m_dontLocate;   //!< Graphics on this SubCategory should not be locatable
        ColorDef m_color;
        uint32_t m_weight;
        DgnStyleId m_style;
        int32_t m_displayPriority; // only valid for SubCategories in 2D models
        RenderMaterialId m_material;
        double m_transparency;

    public:
        BE_JSON_NAME(invisible)
        BE_JSON_NAME(dontPlot)
        BE_JSON_NAME(dontSnap)
        BE_JSON_NAME(dontLocate)
        BE_JSON_NAME(color)
        BE_JSON_NAME(weight)
        BE_JSON_NAME(style)
        BE_JSON_NAME(priority)
        BE_JSON_NAME(material)
        BE_JSON_NAME(transp)

        void Init() {memset(this, 0, sizeof(*this)); m_material.Invalidate(); m_color = ColorDef::White();} // white on white reversal makes this a better default color than black.
        Appearance() {Init();}
        explicit Appearance(JsonValueCR val) { FromJson(val); }

        void SetInvisible(bool val) {m_invisible=val;}
        bool GetDontPlot() const {return m_dontPlot;}
        void SetDontPlot(bool val) {m_dontPlot=val;}
        bool GetDontSnap() const {return m_dontSnap;}
        void SetDontSnap(bool val) {m_dontSnap=val;}
        bool GetDontLocate() const {return m_dontLocate;}
        void SetDontLocate(bool val) {m_dontLocate=val;}
        void SetColor(ColorDef val) {m_color=val;}
        void SetWeight(uint32_t val) {m_weight=val;}
        void SetStyle(DgnStyleId val) {m_style=val;}
        void SetDisplayPriority(int32_t val) {m_displayPriority=val;}
        void SetRenderMaterial(RenderMaterialId val) {m_material=val;}
        void SetTransparency(double val) {m_transparency=val;}
        bool IsInvisible() const {return m_invisible;}
        bool IsVisible() const {return !m_invisible;}
        ColorDef GetColor() const {return m_color;}
        uint32_t GetWeight() const {return m_weight;}
        DgnStyleId GetStyle() const {return m_style;}
        int32_t GetDisplayPriority() const {return m_displayPriority;}
        RenderMaterialId GetRenderMaterial() const {return m_material;}
        double GetTransparency() const {return m_transparency;}
        DGNPLATFORM_EXPORT bool operator== (Appearance const& other) const;
        bool IsEqual(Appearance const& other) const {return *this==other;}
        DGNPLATFORM_EXPORT void FromJson(JsonValueCR); //!< initialize this appearance from a previously saved json 
        DGNPLATFORM_EXPORT Json::Value ToJson() const;   //!< convert this appearance to a json value
        void RelocateToDestinationDb(DgnImportContext&);
    };

    //! View-specific overrides of the appearance of a SubCategory
    //! @ingroup GROUP_DgnCategory
    struct Override
    {
    private:
        union
        {
            uint32_t m_int32;
            struct
            {
                uint32_t m_invisible:1;
                uint32_t m_color:1;
                uint32_t m_weight:1;
                uint32_t m_style:1;
                uint32_t m_material:1;
                uint32_t m_priority:1;
                uint32_t m_transparency:1;
            };
        } m_flags;

        Appearance m_value;

    public:
        Override() {Init();}
        explicit Override(JsonValueCR val) {Init(); FromJson(val);}
        void Init() {m_flags.m_int32 = 0; m_value.Init();}

        void SetInvisible(bool val) {m_flags.m_invisible=true; m_value.SetInvisible(val);}
        void SetColor(ColorDef val) {m_flags.m_color=true; m_value.SetColor(val);}
        void SetWeight(uint32_t val) {m_flags.m_weight=true; m_value.SetWeight(val);}
        void SetStyle(DgnStyleId val) {m_flags.m_style=true; m_value.SetStyle(val);}
        void SetDisplayPriority(int32_t val) {m_flags.m_priority=true; m_value.SetDisplayPriority(val);}
        void SetRenderMaterial(RenderMaterialId val) {m_flags.m_material=true; m_value.SetRenderMaterial(val);}
        void SetTransparency(double val) {m_flags.m_transparency=true; m_value.SetTransparency(val);}

        void ToJson(JsonValueR outValue) const;
        DGNPLATFORM_EXPORT void FromJson(JsonValueCR inValue);
        void ApplyTo(Appearance&) const;

        bool GetInvisible(bool& val) const { if (m_flags.m_invisible) val = m_value.IsInvisible(); return m_flags.m_invisible; }
        bool GetColor(ColorDef& val) const { if (m_flags.m_color) val = m_value.GetColor(); return m_flags.m_color; }
        bool GetWeight(uint32_t& val) const { if (m_flags.m_weight) val = m_value.GetWeight(); return m_flags.m_weight; }
        bool GetStyle(DgnStyleId& val) const { if (m_flags.m_style) val = m_value.GetStyle(); return m_flags.m_style; }
        bool GetDisplayPriority(int32_t& val) const { if (m_flags.m_priority) val = m_value.GetDisplayPriority(); return m_flags.m_priority; }
        bool GetMaterial(RenderMaterialId& val) const { if (m_flags.m_material) val = m_value.GetRenderMaterial(); return m_flags.m_material; }
        bool GetTransparency(double& val) const { if (m_flags.m_transparency) val = m_value.GetTransparency(); return m_flags.m_transparency; }

        bool IsAnyOverridden() const { return 0 != m_flags.m_int32; }
    }; // Override

    //! Holds the data which describes a sub-category
    struct Data
    {
        Utf8String m_descr;
        Appearance m_appearance;

        Data(Appearance const& appearance=Appearance(), Utf8StringCR descr="") {Init(appearance, descr);}
        void Init(Appearance const& appearance, Utf8StringCR descr="") {m_appearance=appearance; m_descr=descr;}
        uint32_t GetMemSize() const {return static_cast<uint32_t>(sizeof(*this) + m_descr.length());}
    };

    //! Parameters used to create a sub-category
    struct CreateParams : T_Super::CreateParams
    {
        DEFINE_T_SUPER(DgnSubCategory::T_Super::CreateParams);

        Data m_data;

        //! Construction from base class. Chiefly for internal use.
        explicit CreateParams(DgnElement::CreateParams const& params, Appearance const& appearance=Appearance(), Utf8StringCR descr="")
            : T_Super(params), m_data(appearance, descr) {}

        //! Construct parameters for a sub-category
        //! @param[in] db The DgnDb in which the sub-category will reside
        //! @param[in] categoryId The Id of the category to which the sub-category belongs
        //! @param[in] name The name of the sub-category. Must be unique within the containing category.
        //! @param[in] appearance Describes how the sub-category affects the symbology of elements.
        //! @param[in] descr Optional description
        DGNPLATFORM_EXPORT CreateParams(DgnDbR db, DgnCategoryId categoryId, Utf8StringCR name, Appearance const& appearance, Utf8StringCR descr="");
    };

private:
    friend struct DgnCategory;
    friend struct dgn_ElementHandler::SubCategory;

    Data m_data;

protected:
    DGNPLATFORM_EXPORT DgnDbStatus _ReadSelectParams(BeSQLite::EC::ECSqlStatement& statement, ECSqlClassParams const& selectParams) override;
    DGNPLATFORM_EXPORT void _ToJson(JsonValueR out, JsonValueCR opts) const override;
    DGNPLATFORM_EXPORT void _FromJson(JsonValueR props) override;
    DGNPLATFORM_EXPORT void _BindWriteParams(BeSQLite::EC::ECSqlStatement&, ForInsert) override;
    DGNPLATFORM_EXPORT void _CopyFrom(DgnElementCR source) override;
    DGNPLATFORM_EXPORT DgnDbStatus _SetParentId(DgnElementId parentId, DgnClassId parentRelClassId) override;
    DGNPLATFORM_EXPORT DgnCode _GenerateDefaultCode() const override;
    bool _SupportsCodeSpec(CodeSpecCR codeSpec) const override {return !codeSpec.IsNullCodeSpec();}
    DGNPLATFORM_EXPORT DgnDbStatus _OnInsert() override;
    DGNPLATFORM_EXPORT DgnDbStatus _OnUpdate(DgnElementCR) override;
    DGNPLATFORM_EXPORT void _RemapIds(DgnImportContext&) override;
    
    uint32_t _GetMemSize() const override {return T_Super::_GetMemSize() + m_data.GetMemSize();}
    static CreateParams CreateParamsFromECInstance(DgnDbR db, ECN::IECInstanceCR properties, DgnDbStatus*);

public:
    static DgnSubCategoryId ImportSubCategory(DgnSubCategoryId source, DgnCategoryId destCategoryId, DgnImportContext& importer);

    //! Constructs a new DgnSubCategory with the specified parameters.
    explicit DgnSubCategory(CreateParams const& params) : T_Super(params), m_data(params.m_data) {}

    DgnSubCategoryId GetSubCategoryId() const {return DgnSubCategoryId(GetElementId().GetValue());} //!< The ID of this sub-category
    Utf8String GetSubCategoryName() const {return GetCode().GetValue().GetUtf8();} //!< The name of this sub-category.
    DgnCategoryId GetCategoryId() const {return DgnCategoryId(GetParentId().GetValue());} //!< The ID of the category to which this sub-category belongs
    DGNPLATFORM_EXPORT bool IsDefaultSubCategory() const; //!< Returns true if this is the default sub-category for its category

    DgnSubCategoryCPtr Insert(DgnDbStatus* status = nullptr) {return GetDgnDb().Elements().Insert<DgnSubCategory>(*this, status);} //!< Inserts this sub-category into the DgnDb and returns the persistent sub-category.
    DgnSubCategoryCPtr Update(DgnDbStatus* status = nullptr) {return GetDgnDb().Elements().Update<DgnSubCategory>(*this, status);} //!< Updates this sub-category in the DgnDb and returns the updated persistent sub-category

    Utf8CP GetDescription() const {return m_data.m_descr.empty() ? nullptr : m_data.m_descr.c_str();} //!< The sub-category description, or nullptr if not defined
    Appearance const& GetAppearance() const {return m_data.m_appearance;} //!< This sub-category's appearance
    Appearance& GetAppearanceR() {return m_data.m_appearance;} //!< A writable reference to this sub-category's appearance
    void SetDescription(Utf8StringCR descr) {m_data.m_descr = descr;} //!< Set the description

    //! Create a DgnCode for the name of a sub-category of the specified category
    DGNPLATFORM_EXPORT static DgnCode CreateCode(DgnDbR db, DgnCategoryId categoryId, Utf8StringCR subCategoryName);

    //! Create a DgnCode for the name of a sub-category of the specified category
    DGNPLATFORM_EXPORT static DgnCode CreateCode(DgnCategoryCR category, Utf8StringCR subCategoryName);

    //! Looks up a sub-category ID by code.
    DGNPLATFORM_EXPORT static DgnSubCategoryId QuerySubCategoryId(DgnDbR db, DgnCodeCR code);

    //! Looks up a sub-category by ID.
    static DgnSubCategoryCPtr Get(DgnDbR db, DgnSubCategoryId subCategoryId) {return db.Elements().Get<DgnSubCategory>(subCategoryId);}

    //! Looks up the ID of the category containing the specified sub-category
    DGNPLATFORM_EXPORT static DgnCategoryId QueryCategoryId(DgnDbR db, DgnSubCategoryId subCategoryId);

    //! Make an iterator over sub-categories of the specified category in the specified DgnDb
    //! @param[in] db The DgnDb
    //! @param[in] categoryId Iterate sub-categories of this category
    //! @param[in] whereClause The optional where clause starting with WHERE
    //! @param[in] orderByClause The optional order by clause starting with ORDER BY
    DGNPLATFORM_EXPORT static ElementIterator MakeIterator(DgnDbR db, DgnCategoryId categoryId, Utf8CP whereClause=nullptr, Utf8CP orderByClause=nullptr);

    //! Returns the number of sub-categories of a specific category, or the total number of sub-categories of all categories in the DgnDb.
    //! @param[in]  db The DgnDb in which to query
    //! @param[in]  categoryId If valid, the count includes only sub-categories of the specified category.
    //! @return The number of sub-categories.
    DGNPLATFORM_EXPORT static size_t QueryCount(DgnDbR db, DgnCategoryId categoryId=DgnCategoryId());

    static ECN::ECClassId QueryECClassId(DgnDbR db) {return db.Schemas().GetClassId(BIS_ECSCHEMA_NAME, BIS_CLASS_SubCategory);} //!< Returns the class ID used for sub-categories.
    static DgnClassId QueryDgnClassId(DgnDbR db) {return DgnClassId(QueryECClassId(db));} //!< Returns the class ID used for sub-categories
};

/**
* @addtogroup GROUP_DgnCategory DgnCategory and DgnSubCategory Module
* Types related to working with categories and subcategories
* @see @ref PAGE_CategoryOverview
*/

//=======================================================================================
//! Categorizes a geometric element. Each category has one default sub-category and any number
//! of additional sub-categories.
//! @ingroup GROUP_DgnCategory
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE DgnCategory : DefinitionElement
{
    DGNELEMENT_DECLARE_MEMBERS(BIS_CLASS_Category, DefinitionElement);
    friend struct dgn_ElementHandler::Category;

public:
    BE_JSON_NAME(rank)
    BE_JSON_NAME(description)

    //! The Rank of a category indicates how it was created and where it can be used.
    //! @ingroup GROUP_DgnCategory
    enum class Rank
    {
        System      = 0, //!< This category is predefined by the system
        Domain      = 1, //!< This category is defined by a domain.
        Application = 2, //!< This category is defined by an application.
        User        = 3, //!< This category is defined by a user.
    };

protected:
    Utf8String m_descr;
    Rank m_rank;

protected:
    DGNPLATFORM_EXPORT DgnDbStatus _ReadSelectParams(BeSQLite::EC::ECSqlStatement& statement, ECSqlClassParams const& selectParams) override;
    DGNPLATFORM_EXPORT void _ToJson(JsonValueR out, JsonValueCR opts) const override;
    DGNPLATFORM_EXPORT void _FromJson(JsonValueR props) override;
    DGNPLATFORM_EXPORT void _BindWriteParams(BeSQLite::EC::ECSqlStatement&, ForInsert) override;
    DGNPLATFORM_EXPORT void _CopyFrom(DgnElementCR source) override;
    DGNPLATFORM_EXPORT void _RemapIds(DgnImportContext&) override;
    DGNPLATFORM_EXPORT DgnCode _GenerateDefaultCode() const override;
    DGNPLATFORM_EXPORT DgnDbStatus _OnChildDelete(DgnElementCR child) const override;
    DGNPLATFORM_EXPORT DgnDbStatus _OnDelete() const override;
    DGNPLATFORM_EXPORT DgnDbStatus _OnInsert() override;
    DGNPLATFORM_EXPORT DgnDbStatus _OnUpdate(DgnElementCR) override;
    DGNPLATFORM_EXPORT void _OnInserted(DgnElementP copiedFrom) const override;
    DGNPLATFORM_EXPORT void _OnImported(DgnElementCR original, DgnImportContext& importer) const override;
    
    DgnDbStatus _SetParentId(DgnElementId, DgnClassId) override {return DgnDbStatus::InvalidParent;}
    uint32_t _GetMemSize() const override {return T_Super::_GetMemSize() + static_cast<uint32_t>(sizeof(m_rank) + m_descr.length());}

    //! Construct a new DgnCategory with the specified parameters
    explicit DgnCategory(CreateParams const& params) : T_Super(params), m_rank(Rank::User), m_descr("") {}

public:
    BE_PROP_NAME(Description)
    BE_PROP_NAME(Rank)

    static DgnCategoryId ImportCategory(DgnCategoryId source, DgnImportContext& importer);

    DgnCategoryId GetCategoryId() const {return DgnCategoryId(GetElementId().GetValue());} //!< Returns the ID of this category.
    DgnSubCategoryId GetDefaultSubCategoryId() const {return GetDefaultSubCategoryId(GetCategoryId());} //!< Returns the ID of this category's default sub-category
    Utf8String GetCategoryName() const {return GetCode().GetValue().GetUtf8();} //!< The name of this category

    Utf8CP GetDescription() const {return m_descr.empty() ? nullptr : m_descr.c_str();} //!< The category description, or null if not defined.
    Rank GetRank() const {return m_rank;} //!< The category's rank.

    bool IsSystemCategory() const {return GetRank()==Rank::System;}
    bool IsUserCategory() const {return GetRank()==Rank::User;}

    void SetDescription(Utf8StringCR descr) {m_descr = descr;} //!< Set the category description.
    void SetRank(Rank rank) {m_rank = rank;} //!< Set the category's rank.

    //! Looks up the DgnCategoryId of a category by code.
    DGNPLATFORM_EXPORT static DgnCategoryId QueryCategoryId(DgnDbR db, DgnCodeCR code);

    //! Gets a DgnCategory by ID. 
    //! @note It is better to use DrawingCategory::Get or SpatialCategory::Get if the type of category is known
    static DgnCategoryCPtr Get(DgnDbR db, DgnCategoryId categoryId) {return db.Elements().Get<DgnCategory>(categoryId);}

    //! Returns the ID of the default sub-category of the specified category
    DGNPLATFORM_EXPORT static DgnSubCategoryId GetDefaultSubCategoryId(DgnCategoryId categoryId);

    //! Sets the appearance of the default SubCategory
    DGNPLATFORM_EXPORT void SetDefaultAppearance(DgnSubCategory::Appearance const&) const;

    //! Returns an iterator over all sub-categories of this category
    //! @param[in] whereClause The optional where clause starting with WHERE
    //! @param[in] orderByClause The optional order by clause starting with ORDER BY
    ElementIterator MakeSubCategoryIterator(Utf8CP whereClause=nullptr, Utf8CP orderByClause=nullptr) const {return DgnSubCategory::MakeIterator(GetDgnDb(), GetCategoryId(), whereClause, orderByClause);}

    size_t QuerySubCategoryCount() const {return DgnSubCategory::QueryCount(GetDgnDb(), GetCategoryId());} //! Returns the number of sub-categories belonging to this category

    //! Get a string containing the list of characters that may NOT appear in category codes.
    static Utf8CP GetIllegalCharacters() {return "<>\\/.\"?*|,='&\n\t";}
    
    //! Determine whether the supplied name is a valid category or sub-category name
    DGNPLATFORM_EXPORT static bool IsValidName(Utf8StringCR name);
};

//=======================================================================================
//! Categorizes 2d graphical elements.
//! @ingroup GROUP_DgnCategory
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE DrawingCategory : DgnCategory
{
    DGNELEMENT_DECLARE_MEMBERS(BIS_CLASS_DrawingCategory, DgnCategory);
    friend struct dgn_ElementHandler::DrawingCategory;

protected:
    bool _SupportsCodeSpec(CodeSpecCR codeSpec) const override {return !codeSpec.IsNullCodeSpec();}
    explicit DrawingCategory(CreateParams const& params) : T_Super(params) {}

public:
    //! Create a DgnCode for a DrawingCategory given a name that is meant to be unique within the scope of the specified DefinitionModel
    static DgnCode CreateCode(DefinitionModelCR scope, Utf8StringCR name) {return CodeSpec::CreateCode(BIS_CODESPEC_DrawingCategory, scope, name);}

    //! Looks up the DgnCategoryId of a DrawingCategory by model and name
    static DgnCategoryId QueryCategoryId(DefinitionModelCR model, Utf8StringCR name) {return T_Super::QueryCategoryId(model.GetDgnDb(), CreateCode(model, name));}

    //! Construct a new DrawingCategory
    DrawingCategory(DefinitionModelR model, Utf8StringCR name, Rank rank=Rank::User, Utf8StringCR descr="") : T_Super(CreateParams(model.GetDgnDb(), model.GetModelId(), QueryClassId(model.GetDgnDb()), CreateCode(model, name)))
        {
        m_rank = rank;
        m_descr = descr;
        }

    //! Inserts this DrawingCategory into the DgnDb and initializes its default sub-category with the specified appearance.
    //! @param[in] appearance The appearance associated with the default sub-category
    //! @param[in] status Optional return status
    //! @return The persistent DrawingCategory, or nullptr if insert failed.
    DGNPLATFORM_EXPORT DrawingCategoryCPtr Insert(DgnSubCategory::Appearance const& appearance, DgnDbStatus* status=nullptr);
    //! Updates this DrawingCategory in the DgnDb and returns the updated persistent DrawingCategory
    DrawingCategoryCPtr Update(DgnDbStatus* status = nullptr) {return GetDgnDb().Elements().Update<DrawingCategory>(*this, status);} 

    //! Make an iterator over all DrawingCategory elements in the specified DgnDb
    //! @param[in] db Iterate DrawingCategory elements in this DgnDb
    //! @param[in] whereClause The optional where clause starting with WHERE
    //! @param[in] orderByClause The optional order by clause starting with ORDER BY
    DGNPLATFORM_EXPORT static ElementIterator MakeIterator(DgnDbR db, Utf8CP whereClause=nullptr, Utf8CP orderByClause=nullptr);

    //! Gets a DrawingCategory by ID.
    static DrawingCategoryCPtr Get(DgnDbR db, DgnCategoryId categoryId) {return db.Elements().Get<DrawingCategory>(categoryId);}

    //! Returns the class ID of DrawingCategory in the specified DgnDb
    static DgnClassId QueryClassId(DgnDbR db) {return DgnClassId(db.Schemas().GetClassId(BIS_ECSCHEMA_NAME, BIS_CLASS_DrawingCategory));}
};

//=======================================================================================
//! Categorizes a SpatialElement.
//! @ingroup GROUP_DgnCategory
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE SpatialCategory : DgnCategory
{
    DGNELEMENT_DECLARE_MEMBERS(BIS_CLASS_SpatialCategory, DgnCategory);
    friend struct dgn_ElementHandler::SpatialCategory;

protected:
    bool _SupportsCodeSpec(CodeSpecCR codeSpec) const override {return !codeSpec.IsNullCodeSpec();}
    explicit SpatialCategory(CreateParams const& params) : T_Super(params) {}

public:
    //! Create a DgnCode for a SpatialCategory given a name that is meant to be unique within the scope of the specified DefinitionModel
    static DgnCode CreateCode(DefinitionModelCR scope, Utf8StringCR name) {return CodeSpec::CreateCode(BIS_CODESPEC_SpatialCategory, scope, name);}

    //! Looks up the DgnCategoryId of a SpatialCategory by model and name
    static DgnCategoryId QueryCategoryId(DefinitionModelCR model, Utf8StringCR name) {return T_Super::QueryCategoryId(model.GetDgnDb(), CreateCode(model, name));}

    //! Construct a new SpatialCategory
    SpatialCategory(DefinitionModelR model, Utf8StringCR name, Rank rank=Rank::User, Utf8StringCR descr="") : T_Super(CreateParams(model.GetDgnDb(), model.GetModelId(), QueryClassId(model.GetDgnDb()), CreateCode(model, name)))
        {
        m_rank = rank;
        m_descr = descr;
        }

    //! Inserts this SpatialCategory into the DgnDb and initializes its default sub-category with the specified appearance.
    //! @param[in] appearance The appearance associated with the default sub-category
    //! @param[in] status Optional return status
    //! @return The persistent SpatialCategory, or nullptr if insert failed.
    DGNPLATFORM_EXPORT SpatialCategoryCPtr Insert(DgnSubCategory::Appearance const& appearance, DgnDbStatus* status=nullptr);
    //! Updates this SpatialCategory in the DgnDb and returns the updated persistent SpatialCategory
    SpatialCategoryCPtr Update(DgnDbStatus* status = nullptr) {return GetDgnDb().Elements().Update<SpatialCategory>(*this, status);} 

    //! Make an iterator over all SpatialCategory elements in the specified DgnDb
    //! @param[in] db Iterate SpatialCategory elements in this DgnDb
    //! @param[in] whereClause The optional where clause starting with WHERE
    //! @param[in] orderByClause The optional order by clause starting with ORDER BY
    DGNPLATFORM_EXPORT static ElementIterator MakeIterator(DgnDbR db, Utf8CP whereClause=nullptr, Utf8CP orderByClause=nullptr);

    //! Gets a SpatialCategory by ID.
    static SpatialCategoryCPtr Get(DgnDbR db, DgnCategoryId categoryId) {return db.Elements().Get<SpatialCategory>(categoryId);}

    //! Returns the class ID of SpatialCategory in the specified DgnDb
    static DgnClassId QueryClassId(DgnDbR db) {return DgnClassId(db.Schemas().GetClassId(BIS_ECSCHEMA_NAME, BIS_CLASS_SpatialCategory));}
};

//=======================================================================================
//! Namespace containing handlers for category-related elements
//! @private
//=======================================================================================
namespace dgn_ElementHandler
{
    //! The handler for Category elements.
    struct Category : Definition
    {
        ELEMENTHANDLER_DECLARE_MEMBERS(BIS_CLASS_Category, DgnCategory, Category, Definition, DGNPLATFORM_EXPORT);
        DGNPLATFORM_EXPORT void _RegisterPropertyAccessors(ECSqlClassInfo&, ECN::ClassLayoutCR) override;
    };

    //! The handler for DrawingCategory elements.
    struct DrawingCategory : Category
    {
        ELEMENTHANDLER_DECLARE_MEMBERS(BIS_CLASS_DrawingCategory, Dgn::DrawingCategory, DrawingCategory, Category, DGNPLATFORM_EXPORT);
    };

    //! The handler for SpatialCategory elements.
    struct SpatialCategory : Category
    {
        ELEMENTHANDLER_DECLARE_MEMBERS(BIS_CLASS_SpatialCategory, Dgn::SpatialCategory, SpatialCategory, Category, DGNPLATFORM_EXPORT);
    };

    //! The handler for SubCategory elements.
    struct SubCategory : Definition
    {
        ELEMENTHANDLER_DECLARE_MEMBERS(BIS_CLASS_SubCategory, DgnSubCategory, SubCategory, Definition, DGNPLATFORM_EXPORT);
    protected:
        DgnElementPtr _CreateNewElement(DgnDbR db, ECN::IECInstanceCR, bool ignoreErrors, DgnDbStatus* stat) override;
        void _RegisterPropertyAccessors(ECSqlClassInfo&, ECN::ClassLayoutCR) override;
    };
}

END_BENTLEY_DGN_NAMESPACE
