/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/DgnCategory.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include "DgnDb.h"
#include "DgnElement.h"
#include "ElementHandler.h"

#define DGN_CLASSNAME_Category "Category"
#define DGN_CLASSNAME_SubCategory "SubCategory"

DGNPLATFORM_TYPEDEFS(DgnCategory);
DGNPLATFORM_TYPEDEFS(DgnSubCategory);
DGNPLATFORM_REF_COUNTED_PTR(DgnCategory);
DGNPLATFORM_REF_COUNTED_PTR(DgnSubCategory);

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//! A sub-category of a category.
//! @ingroup DgnCategoryGroup
struct EXPORT_VTABLE_ATTRIBUTE DgnSubCategory : DictionaryElement
{
    DGNELEMENT_DECLARE_MEMBERS(DGN_CLASSNAME_SubCategory, DictionaryElement);
public:
    //! The parameters that can determine how graphics on a SubCategory appear when drawn.
    //! @ingroup DgnCategoryGroup
    struct Appearance
    {
    private:
        bool            m_invisible;    //!< Graphics on this SubCategory should not be visible
        bool            m_dontPlot;     //!< Graphics on this SubCategory should not be plotted
        bool            m_dontSnap;     //!< Graphics on this SubCategory should not be snappable
        bool            m_dontLocate;   //!< Graphics on this SubCategory should not be locatable
        ColorDef        m_color;
        uint32_t        m_weight;
        DgnStyleId      m_style;
        int32_t         m_displayPriority; // only valid for SubCategories in 2D models
        DgnMaterialId   m_material;
        double          m_transparency;

    public:
        void Init() {memset(this, 0, sizeof(*this)); m_material.Invalidate(); m_color = ColorDef::White();} // white on white reversal makes this a better default color than black.
        Appearance() {Init();}
        explicit Appearance(Utf8StringCR val) {FromJson(val);}

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
        void SetMaterial(DgnMaterialId val) {m_material=val;}
        void SetTransparency(double val) {m_transparency=val;}
        bool IsInvisible() const {return m_invisible;}
        bool IsVisible() const {return !m_invisible;}
        ColorDef GetColor() const {return m_color;}
        uint32_t GetWeight() const {return m_weight;}
        DgnStyleId GetStyle() const {return m_style;}
        int32_t GetDisplayPriority() const {return m_displayPriority;}
        DgnMaterialId GetMaterial() const {return m_material;}
        double GetTransparency() const {return m_transparency;}
        DGNPLATFORM_EXPORT bool operator== (Appearance const& other) const;
        bool IsEqual(Appearance const& other) const {return *this==other;}
        void FromJson(Utf8StringCR); //!< initialize this appearance from a previously saved json string
        DGNPLATFORM_EXPORT Utf8String ToJson() const;   //!< convert this appearance to a json string
    };// Appearance

    //! View-specific overrides of the appearance of a SubCategory
    //! @ingroup DgnCategoryGroup
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
        void SetMaterial(DgnMaterialId val) {m_flags.m_material=true; m_value.SetMaterial(val);}
        void SetTransparency(double val) {m_flags.m_transparency=true; m_value.SetTransparency(val);}
        void ToJson(JsonValueR outValue) const;
        void FromJson(JsonValueCR inValue);
        void ApplyTo(Appearance&) const;
    }; // Override

    //! Holds the data which describes a sub-category
    struct Data
    {
        Utf8String m_descr;
        Appearance m_appearance;

        Data(Appearance const& appearance=Appearance(), Utf8StringCR descr="") { Init(appearance, descr); }

        void Init(Appearance const& appearance, Utf8StringCR descr="") { m_appearance=appearance; m_descr=descr; }

        uint32_t GetMemSize() const { return static_cast<uint32_t>(sizeof(*this) + m_descr.length()); }
    };

    //! Parameters used to create a sub-category
    struct CreateParams : T_Super::CreateParams
    {
        DEFINE_T_SUPER(DgnSubCategory::T_Super::CreateParams);

        Data m_data;

        //! Construction from base class. Chiefly for internal use.
        explicit CreateParams(DgnElement::CreateParams const& params, Appearance const& appearance=Appearance(), Utf8StringCR descr="")
            : T_Super(params), m_data(appearance, descr) { }

        //! Construct parameters for a sub-category
        //! @param[in]      db         The DgnDb in which the sub-category will reside
        //! @param[in]      categoryId The ID of the category to which the sub-category belongs
        //! @param[in]      name       The name of the sub-category. Must be unique within the containing category.
        //! @param[in]      appearance Describes how the sub-category affects the symbology of elements.
        //! @param[in]      descr      Optional description
        DGNPLATFORM_EXPORT CreateParams(DgnDbR db, DgnCategoryId categoryId, Utf8StringCR name, Appearance const& appearance, Utf8StringCR descr="");
    };
private:
    friend struct DgnCategory;

    Data m_data;

    DgnDbStatus BindParams(BeSQLite::EC::ECSqlStatement& stmt);
protected:
    DGNPLATFORM_EXPORT virtual DgnDbStatus _ExtractSelectParams(BeSQLite::EC::ECSqlStatement& statement, ECSqlClassParams const& selectParams) override;
    DGNPLATFORM_EXPORT virtual DgnDbStatus _BindInsertParams(BeSQLite::EC::ECSqlStatement& stmt) override;
    DGNPLATFORM_EXPORT virtual DgnDbStatus _BindUpdateParams(BeSQLite::EC::ECSqlStatement& stmt) override;
    DGNPLATFORM_EXPORT virtual void _CopyFrom(DgnElementCR source) override;
    DGNPLATFORM_EXPORT virtual DgnDbStatus _SetParentId(DgnElementId parentId) override;
    DGNPLATFORM_EXPORT virtual DgnDbStatus _SetCode(Code const& code) override;
    DGNPLATFORM_EXPORT virtual Code _GenerateDefaultCode() override;
    DGNPLATFORM_EXPORT virtual DgnDbStatus _OnInsert() override;
    DGNPLATFORM_EXPORT void _RemapIds(DgnImportContext&) override;
    
    virtual uint32_t _GetMemSize() const override { return T_Super::_GetMemSize() + m_data.GetMemSize(); }
//__PUBLISH_SECTION_END__
public:
    static DgnSubCategoryId ImportSubCategory(DgnSubCategoryId source, DgnCategoryId destCategoryId, DgnImportContext& importer, DgnRemapTables& remap);
//__PUBLISH_SECTION_START__
public:
    //! Constructs a new DgnSubCategory with the specified parameters.
    explicit DgnSubCategory(CreateParams const& params) : T_Super(params), m_data(params.m_data) { }

    DgnSubCategoryId GetSubCategoryId() const { return DgnSubCategoryId(GetElementId().GetValue()); } //!< The ID of this sub-category
    Utf8String GetSubCategoryName() const { return GetCode().GetValue(); } //!< The name of this sub-category.
    DgnCategoryId GetCategoryId() const { return DgnCategoryId(GetParentId().GetValue()); } //!< The ID of the category to which this sub-category belongs
    DGNPLATFORM_EXPORT bool IsDefaultSubCategory() const; //!< Returns true if this is the default sub-category for its category

    DgnSubCategoryCPtr Insert(DgnDbStatus* status = nullptr) { return GetDgnDb().Elements().Insert<DgnSubCategory>(*this, status); } //!< Inserts this sub-category into the DgnDb and returns the persistent sub-category.
    DgnSubCategoryCPtr Update(DgnDbStatus* status = nullptr) { return GetDgnDb().Elements().Update<DgnSubCategory>(*this, status); } //!< Updates this sub-category in the DgnDb and returns the updated persistent sub-category

    Utf8CP GetDescription() const { return m_data.m_descr.empty() ? nullptr : m_data.m_descr.c_str(); } //!< The sub-category description, or nullptr if not defined
    Appearance const& GetAppearance() const { return m_data.m_appearance; } //!< This sub-category's appearance
    Appearance& GetAppearanceR() { return m_data.m_appearance; } //!< A writable reference to this sub-category's appearance
    void SetDescription(Utf8StringCR descr) { m_data.m_descr = descr; } //!< Set the description

    //! Create a Code for the name of a sub-category of the specified category
    DGNPLATFORM_EXPORT static Code CreateSubCategoryCode(DgnCategoryId categoryId, Utf8StringCR subCategoryName, DgnDbR db);

    //! Create a Code for the name of a sub-category of the specified category
    DGNPLATFORM_EXPORT static Code CreateSubCategoryCode(DgnCategoryCR category, Utf8StringCR subCategoryName, DgnDbR db);

    //! Looks up a sub-category ID by code.
    DGNPLATFORM_EXPORT static DgnSubCategoryId QuerySubCategoryId(Code const& code, DgnDbR db);

    //! Looks up a sub-category ID by name and category ID.
    static DgnSubCategoryId QuerySubCategoryId(DgnCategoryId categoryId, Utf8StringCR subCategoryName, DgnDbR db) { return QuerySubCategoryId(CreateSubCategoryCode(categoryId, subCategoryName, db), db); }

    //! Looks up a sub-category by ID.
    static DgnSubCategoryCPtr QuerySubCategory(DgnSubCategoryId subCategoryId, DgnDbR db) { return db.Elements().Get<DgnSubCategory>(subCategoryId); }

    //! Looks up the ID of the category containing the specified sub-category
    DGNPLATFORM_EXPORT static DgnCategoryId QueryCategoryId(DgnSubCategoryId subCategoryId, DgnDbR db);

    //! Returns a set of sub-category IDs.
    //! @param[in]      db         The DgnDb in which to query.
    //! @param[in]      categoryId If valid, include only sub-categories of the specified category; otherwise, include all sub-categories within the DgnDb.
    //! @return A set of sub-category IDs, optionally limited to those belonging to a specific category.
    DGNPLATFORM_EXPORT static DgnSubCategoryIdSet QuerySubCategories(DgnDbR db, DgnCategoryId categoryId=DgnCategoryId());

    //! Returns the number of sub-categories of a specific category, or the total number of sub-categories of all categories in the DgnDb.
    //! @param[in]      db         The DgnDb in which to query
    //! @param[in]      categoryId If valid, the count includes only sub-categories of the specified category.
    //! @return The number of sub-categories.
    DGNPLATFORM_EXPORT static size_t QueryCount(DgnDbR db, DgnCategoryId categoryId=DgnCategoryId());

    static ECN::ECClassId QueryECClassId(DgnDbR db) { return db.Schemas().GetECClassId(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_SubCategory); } //!< Returns the class ID used for sub-categories.
    static DgnClassId QueryDgnClassId(DgnDbR db) { return DgnClassId(QueryECClassId(db)); } //!< Returns the class ID used for sub-categories
};

//=======================================================================================
//! Categorizes a geometric element. Each category one default sub-category and any number
//! of additional sub-categories.
//! @ingroup DgnCategoryGroup
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE DgnCategory : DictionaryElement
{
    DGNELEMENT_DECLARE_MEMBERS(DGN_CLASSNAME_Category, DictionaryElement);
public:
    //! The Rank of a category indicates how it was created and where it can be used.
    //! @ingroup DgnCategoryGroup
    enum class Rank
    {
        System      = 0,    //!< This category is predefined by the system
        Domain      = 1,    //!< This category is defined by a domain. Elements in this category may be unknown to system functionality.
        Application = 2,    //!< This category is defined by an application. Elements in this category may be unknown to system and domain functionality.
        User        = 3,    //!< This category is defined by a user. Elements in this category may be unknown to system, domain, and application functionality.
    };

    //! The Scope of a category determines what types of models may use it.
    //! @ingroup DgnCategoryGroup
    enum class Scope
    {
        Any         = 0,    //!< This category may be used in any type of model. Generally, this means it came from some external source (e.g. a dgn or dwg file)
        Physical    = 1,    //!< This category may only be used in physical models
        Annotation  = 2,    //!< This category may only be used in annotation models (e.g. sheets or drawings)
        Analytical  = 3,    //!< This category may only be used in analytical models
    };

    //! Holds the data which describes a category
    struct Data
    {
        Utf8String m_descr;
        Scope m_scope;
        Rank m_rank;

        Data(Scope scope=Scope::Any, Rank rank=Rank::User, Utf8StringCR descr="") { Init(scope, rank, descr); }

        void Init(Scope scope, Rank rank=Rank::User, Utf8StringCR descr="") { m_descr=descr; m_scope=scope; m_rank=rank; }

        uint32_t GetMemSize() const { return static_cast<uint32_t> (sizeof(*this) + m_descr.length()); }
    };

    //! Parameters used to construct a DgnCategory
    struct CreateParams : T_Super::CreateParams
    {
        DEFINE_T_SUPER(DgnCategory::T_Super::CreateParams);

        Data m_data;

        //! Constructor from base class. Chiefly for internal use.
        explicit CreateParams(DgnElement::CreateParams const& params, Scope scope=Scope::Any, Rank rank=Rank::User, Utf8StringCR descr="")
            : T_Super(params), m_data(scope, rank, descr) { }

        //! Constructs parameters for a category. Chiefly for internal use.
        CreateParams(DgnDbR db, DgnModelId modelId, DgnClassId classId, Code const& code, DgnElementId id=DgnElementId(), DgnElementId parent=DgnElementId(),
                Scope scope=Scope::Any, Rank rank=Rank::User, Utf8StringCR descr="")
            : T_Super(db, modelId, classId, code, id, parent), m_data(scope, rank, descr) { }

        //! Constructs parameters for creating a category.
        //! @param[in]      db    The DgnDb in which the category resides
        //! @param[in]      name  The name of the category. Must be unique within the DgnDb.
        //! @param[in]      scope The scope of the category
        //! @param[in]      rank  The rank of the category
        //! @param[in]      descr Optional category description
        DGNPLATFORM_EXPORT CreateParams(DgnDbR db, Utf8StringCR name, Scope scope, Rank rank=Rank::User, Utf8StringCR descr="");
    };
private:
    Data m_data;

    DgnDbStatus BindParams(BeSQLite::EC::ECSqlStatement& stmt);
protected:
    DGNPLATFORM_EXPORT virtual DgnDbStatus _ExtractSelectParams(BeSQLite::EC::ECSqlStatement& statement, ECSqlClassParams const& selectParams) override;
    DGNPLATFORM_EXPORT virtual DgnDbStatus _BindInsertParams(BeSQLite::EC::ECSqlStatement& stmt) override;
    DGNPLATFORM_EXPORT virtual DgnDbStatus _BindUpdateParams(BeSQLite::EC::ECSqlStatement& stmt) override;
    DGNPLATFORM_EXPORT virtual void _CopyFrom(DgnElementCR source) override;
    DGNPLATFORM_EXPORT virtual DgnDbStatus _SetCode(Code const& code) override;
    DGNPLATFORM_EXPORT virtual Code _GenerateDefaultCode() override;
    DGNPLATFORM_EXPORT virtual DgnDbStatus _OnChildImport(DgnElementCR child, DgnModelR destModel, DgnImportContext& importer) const override;
    DGNPLATFORM_EXPORT virtual DgnDbStatus _OnChildDelete(DgnElementCR child) const override;
    DGNPLATFORM_EXPORT virtual DgnDbStatus _OnDelete() const override;
    DGNPLATFORM_EXPORT virtual void _OnInserted(DgnElementP copiedFrom) const override;
    DGNPLATFORM_EXPORT virtual void _OnImported(DgnElementCR original, DgnImportContext& importer) const override;
    
    virtual DgnDbStatus _SetParentId(DgnElementId parentId) override { return DgnDbStatus::InvalidParent; }
    virtual uint32_t _GetMemSize() const override { return T_Super::_GetMemSize() + m_data.GetMemSize(); }
//__PUBLISH_SECTION_END__
public:
    static DgnCategoryId ImportCategory(DgnCategoryId source, DgnImportContext& importer, DgnRemapTables& remap);
//__PUBLISH_SECTION_START__
public:
    DgnCategoryId GetCategoryId() const { return DgnCategoryId(GetElementId().GetValue()); } //!< Returns the ID of this category.
    DgnSubCategoryId GetDefaultSubCategoryId() const { return GetDefaultSubCategoryId(GetCategoryId()); } //!< Returns the ID of this category's default sub-category
    Utf8String GetCategoryName() const { return GetCode().GetValue(); } //!< The name of this category

    //! Construct a new DgnCategory with the specified parameters
    explicit DgnCategory(CreateParams const& params) : T_Super(params), m_data(params.m_data) { }

    //! Inserts this category into the DgnDb and initializes its default sub-category with the specified appearance.
    //! @note Using DgnElement::Insert() instead of this method will also create a default sub-category, using a default appearance.
    //! @param[in]      appearance The appearance associated with the default sub-category
    //! @param[in]      status     Optional return status
    //! @return The persistent DgnCategory, or nullptr if insertion failed.
    DGNPLATFORM_EXPORT DgnCategoryCPtr Insert(DgnSubCategory::Appearance const& appearance, DgnDbStatus* status=nullptr);

    DgnCategoryCPtr Update(DgnDbStatus* status = nullptr) { return GetDgnDb().Elements().Update<DgnCategory>(*this, status); } //!< Updates this category in the DgnDb and returns the updated persistent category

    Utf8CP GetDescription() const { return m_data.m_descr.empty() ? nullptr : m_data.m_descr.c_str(); } //!< The category description, or null if not defined.
    Scope GetScope() const { return m_data.m_scope; } //!< The category's scope.
    Rank GetRank() const { return m_data.m_rank; } //!< The category's rank.

    bool IsSystemCategory() const {return GetRank()==Rank::System;}
    bool IsUserCategory() const {return GetRank()==Rank::User;}

    void SetDescription(Utf8StringCR descr) { m_data.m_descr = descr; } //!< Set the category description.
    void SetScope(Scope scope) { m_data.m_scope = scope; } //!< Set the category's scope.
    void SetRank(Rank rank) { m_data.m_rank = rank; } //!< Set the category's rank.

    DGNPLATFORM_EXPORT static Code CreateCategoryCode(Utf8StringCR categoryName, DgnDbR db); //!< Creates a Code for a category name.
    DGNPLATFORM_EXPORT static DgnCategoryId QueryCategoryId(Code const& code, DgnDbR db); //!< Looks up the ID of a category by code.
    static DgnCategoryId QueryCategoryId(Utf8StringCR categoryName, DgnDbR db) { return QueryCategoryId(CreateCategoryCode(categoryName, db), db); } //!< Looks up the ID of a category by name.
    static DgnCategoryCPtr QueryCategory(DgnCategoryId categoryId, DgnDbR db) { return db.Elements().Get<DgnCategory>(categoryId); } //!< Looks up a category by ID.
    static DgnCategoryCPtr QueryCategory(Utf8StringCR categoryName, DgnDbR db) { return QueryCategory(QueryCategoryId(categoryName, db), db); } //!< Looks up a category by name.

    //! Returns the ID of the default sub-category of the specified category
    DGNPLATFORM_EXPORT static DgnSubCategoryId GetDefaultSubCategoryId(DgnCategoryId categoryId);

    //! Returns the IDs of all categories in the specified DgnDb
    DGNPLATFORM_EXPORT static DgnCategoryIdSet QueryCategories(DgnDbR db);

    //! Returns the IDs of all sub-categories of this category
    DgnSubCategoryIdSet QuerySubCategories(DgnDbR db) { return DgnSubCategory::QuerySubCategories(db, GetCategoryId()); }

    DGNPLATFORM_EXPORT static size_t QueryCount(DgnDbR db); //! Returns the number of categories in the DgnDb
    size_t QuerySubCategoryCount() const { return DgnSubCategory::QueryCount(GetDgnDb(), GetCategoryId()); } //! Returns the number of sub-categories belonging to this category
    DGNPLATFORM_EXPORT static DgnCategoryId QueryFirstCategoryId(DgnDbR db); //!< Returns the ID of the first category found in the DgnDb
    DGNPLATFORM_EXPORT static DgnCategoryId QueryHighestCategoryId(DgnDbR db); //!< Returns the highest category ID found in the DgnDb

    //! Returns the ID of the category to which the element with the specified ID belongs, or invalid if the element does not belong to a category.
    DGNPLATFORM_EXPORT static DgnCategoryId QueryElementCategoryId(DgnElementId elementId, DgnDbR db);

    static ECN::ECClassId QueryECClassId(DgnDbR db) { return db.Schemas().GetECClassId(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_Category); } //!< Returns the class ID used for categories.
    static DgnClassId QueryDgnClassId(DgnDbR db) { return DgnClassId(QueryECClassId(db)); } //!< Returns the class ID used for categories

    //! Get a string containing the list of characters that may NOT appear in category codes.
    static Utf8CP GetIllegalCharacters() {return "<>\\/.\"?*|,='&\n\t";}
};

namespace dgn_ElementHandler
{
    //=======================================================================================
    //! The handler for category elements.
    //! @bsistruct                                                  Paul.Connelly   09/15
    //=======================================================================================
    struct Category : Element
    {
        ELEMENTHANDLER_DECLARE_MEMBERS(DGN_CLASSNAME_Category, DgnCategory, Category, Element, DGNPLATFORM_EXPORT);
    protected:
        DGNPLATFORM_EXPORT virtual void _GetClassParams(ECSqlClassParams& params) override;
    };

    //=======================================================================================
    //! The handler for sub-category elements.
    //! @bsistruct                                                  Paul.Connelly   09/15
    //=======================================================================================
    struct SubCategory : Element
    {
        ELEMENTHANDLER_DECLARE_MEMBERS(DGN_CLASSNAME_SubCategory, DgnSubCategory, SubCategory, Element, DGNPLATFORM_EXPORT);
    protected:
        DGNPLATFORM_EXPORT virtual void _GetClassParams(ECSqlClassParams& params) override;
    };
}

END_BENTLEY_DGNPLATFORM_NAMESPACE

