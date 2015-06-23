/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/DgnDbTables.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#define DGN_ECSCHEMA_NAME "dgn"
#define DGN_TABLE_PREFIX  DGN_ECSCHEMA_NAME "_"
#define DGN_SCHEMA(name)  DGN_ECSCHEMA_NAME "." name
#define DGN_TABLE(name)   DGN_TABLE_PREFIX name

//-----------------------------------------------------------------------------------------
// ECClass names (combine with DGN_SCHEMA macro for use in ECSql)
//-----------------------------------------------------------------------------------------
#define DGN_CLASSNAME_Category              "Category"
#define DGN_CLASSNAME_Color                 "Color"
#define DGN_CLASSNAME_ComponentModel        "ComponentModel"
#define DGN_CLASSNAME_DrawingElement        "DrawingElement"
#define DGN_CLASSNAME_DrawingModel          "DrawingModel"
#define DGN_CLASSNAME_Element               "Element"
#define DGN_CLASSNAME_ElementAspect         "ElementAspect"
#define DGN_CLASSNAME_ElementGeom           "ElementGeom"
#define DGN_CLASSNAME_ElementGroup          "ElementGroup"
#define DGN_CLASSNAME_ElementItem           "ElementItem"
#define DGN_CLASSNAME_GeomPart              "GeomPart"
#define DGN_CLASSNAME_Link                  "Link"
#define DGN_CLASSNAME_Model                 "Model"
#define DGN_CLASSNAME_PhysicalElement       "PhysicalElement"
#define DGN_CLASSNAME_PhysicalModel         "PhysicalModel"
#define DGN_CLASSNAME_PhysicalView          "PhysicalView"
#define DGN_CLASSNAME_Model2d               "Model2d"
#define DGN_CLASSNAME_GraphicsModel2d       "GraphicsModel2d"
#define DGN_CLASSNAME_PlanarPhysicalModel   "PlanarPhysicalModel"
#define DGN_CLASSNAME_SectionDrawingModel   "SectionDrawingModel"
#define DGN_CLASSNAME_SheetModel            "SheetModel"
#define DGN_CLASSNAME_Style                 "Style"
#define DGN_CLASSNAME_SubCategory           "SubCategory"
#define DGN_CLASSNAME_View                  "View"

//-----------------------------------------------------------------------------------------
// DgnDb table names
//-----------------------------------------------------------------------------------------
#define DGN_TABLE_Domain        DGN_TABLE("Domain")
#define DGN_TABLE_Font          DGN_TABLE("Font")
#define DGN_TABLE_Handler       DGN_TABLE("Handler")
#define DGN_TABLE_Txns          DGN_TABLE("Txns")
#define DGN_VTABLE_RTree3d      DGN_TABLE("RTree3d")

//-----------------------------------------------------------------------------------------
// ECRelationshipClass names (combine with DGN_SCHEMA macro for use in ECSql)
//-----------------------------------------------------------------------------------------
#define DGN_RELNAME_CategoryOwnsSubCategories   "CategoryOwnsSubCategories"
#define DGN_RELNAME_ElementDrivesElement        "ElementDrivesElement"
#define DGN_RELNAME_ElementHasLinks             "ElementHasLinks"
#define DGN_RELNAME_ElementGeomUsesParts        "ElementGeomUsesParts"
#define DGN_RELNAME_ElementGroupHasMembers      "ElementGroupHasMembers"
#define DGN_RELNAME_ElementOwnsAspects          "ElementOwnsAspects"
#define DGN_RELNAME_ElementOwnsItem             "ElementOwnsItem"
#define DGN_RELNAME_ModelDrivesModel            "ModelDrivesModel"

#include <DgnPlatform/DgnProperties.h>
#include "UnitDefinition.h"
#include "DgnLink.h"
#include "DgnFont.h"
#include "DgnCoreEvent.h"
#include "DgnElement.h"
#include <Bentley/HeapZone.h>

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//! Map of ID to DgnFont object. See method FontNumberMap.
typedef bmap<uint32_t, DgnFontCP> T_FontNumberMap;

namespace dgn_ElementHandler {struct Physical;};
namespace dgn_TxnTable {struct Element; struct Model;};

//=======================================================================================
//! A base class for api's that access a table in a DgnDb
//=======================================================================================
struct DgnDbTable : NonCopyableClass
{
protected:
    friend struct DgnDb;
    DgnDbR m_dgndb;
    explicit DgnDbTable(DgnDbR db) : m_dgndb(db) {}

public:
    DgnDbR GetDgnDb() const {return m_dgndb;}

    //! Determine whether the supplied name contains any of the invalid characters.
    //! @param[in] name the name to check
    //! @param[in] invalidChars the list of character that may NOT appear in name.
    //! @note names may also not start or end with a space.
    DGNPLATFORM_EXPORT static bool IsValidName(Utf8StringCR name, Utf8CP invalidChars);

    //! Replace any instances of the invalid characters in the supplied name with a replacement character.
    //! @param[in] name the name to validate
    //! @param[in] invalidChars the list of invalid characters. All instances of these characters are replaced.
    //! @param[in] replacement the character to substitute for any invalid characters in name.
    DGNPLATFORM_EXPORT static void ReplaceInvalidCharacters(Utf8StringR name, Utf8CP invalidChars, Utf8Char replacement);
};

/** @addtogroup DgnCategoryGroup Categories and SubCategories
@ref PAGE_CategoryOverview 
*/

//=======================================================================================
//! Each Category has an entry in the Category table.
//! @ingroup DgnCategoryGroup
//=======================================================================================
struct DgnCategories : DgnDbTable
    {
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

    //! A sub-category of a category.
    //! @ingroup DgnCategoryGroup
    struct SubCategory
    {
        //! The parameters that can determine how graphics on a SubCategory appear when drawn.
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
            void Init() {memset(this, 0, sizeof(*this)); m_material.Invalidate();}
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

    private:
        friend struct DgnCategories;
        DgnCategoryId      m_categoryId;
        DgnSubCategoryId   m_subCategoryId;
        Utf8String m_code;
        Utf8String m_label; // defaults to code, but can be renamed
        Utf8String m_description;
        Appearance m_appearance;

    public:
        SubCategory() {} //!< Construct an invalid SubCategory.
        SubCategory(DgnCategoryId categoryId, DgnSubCategoryId subCategoryId, Utf8CP code, Appearance const& appearance, Utf8CP descr=nullptr, Utf8CP label=nullptr)
                : m_categoryId(categoryId), m_subCategoryId(subCategoryId), m_code(code), m_appearance(appearance) {m_description.AssignOrClear(descr); m_label=label?label:code;}

        Utf8CP GetCode() const {return m_code.c_str();} //!< The SubCategory code. Unique per category. Not translated. Default SubCategories will return an empty string.
        Utf8CP GetLabel() const {return m_label.c_str();} //!< The SubCategory display label which may be translated.
        Utf8CP GetDescription() const {return m_description.length()>0 ? m_description.c_str() : nullptr;} //!< The SubCategory description. May be empty.
        DgnCategoryId GetCategoryId() const {return m_categoryId;} //!< This SubCategory's DgnCategoryId
        DgnSubCategoryId GetSubCategoryId() const {return m_subCategoryId;} //!< The DgnSubCategoryId
        Appearance const& GetAppearance() const {return m_appearance;} //!< The Appearance for this SubCategory.
        Appearance& GetAppearanceR() {return m_appearance;} //!< Get a writable reference to the Appearance for this SubCategory.
        void SetLabel(Utf8CP label) {m_label.AssignOrClear(label);} //!< Set the SubCategory display label.
        void SetDescription(Utf8CP val) {m_description.AssignOrClear(val);} //!< Set the SubCategory description. @param val the new description. May be nullptr.
        void SetCode(Utf8CP val) {m_code = val;} //!< Set the SubCategory code. @param val the new code for this SubCategory. Must not be nullptr. Must be unique per category. Default SubCategories may not be recoded.
        bool IsValid() const {return m_categoryId.IsValid() && m_subCategoryId.IsValid();} //!< Test if the SubCategory is valid. A failed query will return an invalid SubCategory.
        bool IsDefaultSubCategory() const {return m_categoryId == m_subCategoryId;} //!< Determine if this is the default SubCategory for its category.
    }; // SubCategory

    //! A Category in the category table
    //! @ingroup DgnCategoryGroup
    struct Category
        {
    private:
        friend struct DgnCategories;
        DgnCategoryId  m_categoryId;
        Rank        m_rank;
        Scope       m_scope;
        Utf8String  m_code;
        Utf8String  m_label; // defaults to code, but can be renamed
        Utf8String  m_description;

        void Init(DgnCategoryId id, Utf8CP code, Scope scope, Utf8CP descr, Utf8CP label, Rank rank)
            {m_categoryId=id; m_code=code; m_label=label?label:code; m_scope=scope; m_description.AssignOrClear(descr); m_rank=rank;}

    public:
        Category() {}// so that we can put Categories in a bmap

        //! Ctor for a Category in the category table.
        //! @param[in] code The category's code. Must be unique.
        //! @param[in] scope The Scope of this category.
        //! @param[in] descr The category's description. May be nullptr.
        //! @param[in] label The display label for this category.  May be nullptr.
        //! @param[in] rank The category's rank
        //! @param[in] id The Category's unique ID. This is normally assigned by the Insert function.
        Category(Utf8CP code, Scope scope, Utf8CP descr=nullptr, Utf8CP label=nullptr, Rank rank=Rank::User, DgnCategoryId id=DgnCategoryId()) {Init(id, code, scope, descr, label, rank);}

        Utf8CP GetCode() const {return m_code.c_str();} //!< The category code. Never empty. Unique. Not translated.
        Utf8CP GetLabel() const {return m_label.c_str();} //!< The category display label which may be translated.
        Utf8CP GetDescription() const {return m_description.length()>0 ? m_description.c_str() : nullptr;} //!< The category description. May be empty.
        DgnCategoryId GetCategoryId() const {return m_categoryId;} //!< The category id. Unique. This is an internal identifier and is not displayed in the GUI.
        Scope GetScope() const {return m_scope;} //!< the category's scope.
        Rank GetRank() const {return m_rank;} //!< the category's rank.
        bool IsSystemCategory() const {return GetRank()==Rank::System;}
        bool IsUserCategory() const {return GetRank()==Rank::User;}
        bool IsValid() const {return m_categoryId.IsValid();} //!< Test if the Category is valid. A failed query will return an invalid Category. @see DgnCategories::QueryCategoryById.
        void SetLabel(Utf8CP label) {m_label.AssignOrClear(label);} //!< Set the category display label.
        void SetDescription(Utf8CP val) {m_description.AssignOrClear(val);} //!< Set the category description. @param val the new description. May be nullptr.
        void SetCode(Utf8CP val) {m_code = val;} //!< Set the category code. @param val the new code. Must not be nullptr. Must be unique.
        void SetScope(Scope val) {m_scope= val;} //!< Set the category's scope. @param[in] val the new category scope.
        void SetRank(Rank val) {m_rank=val;} //!< Change the Rank of this category.
        };

    //! An iterator over the categories in the DgnDb.
    //! @ingroup DgnCategoryGroup
    struct Iterator : BeSQLite::DbTableIterator
    {
    public:
        explicit Iterator(DgnDbCR db) : DbTableIterator((BeSQLite::DbCR) db) {}

        //! An entry in the table.
        struct Entry : DbTableIterator::Entry, std::iterator<std::input_iterator_tag, Entry const>
        {
        private:
            friend struct Iterator;
            Entry(BeSQLite::StatementP sql, bool isValid) : DbTableIterator::Entry(sql,isValid) {}
        public:
            DGNPLATFORM_EXPORT DgnCategoryId GetCategoryId() const; //!< The category id. Unique. This is an internal identifier and is not displayed in the GUI.
            DGNPLATFORM_EXPORT Rank GetRank() const; //!< The category's rank
            DGNPLATFORM_EXPORT Utf8CP GetCode() const; //!< The category's code. Never nullptr. Unique. Not translated.
            DGNPLATFORM_EXPORT Utf8CP GetLabel() const; //!< The category's display label which may be translated.
            DGNPLATFORM_EXPORT Utf8CP GetDescription() const; //!< The category's description, if any. May be nullptr.
            DGNPLATFORM_EXPORT Scope GetScope() const; //!< The category's type.
            Entry const& operator*() const {return *this;}
            Category ToCategory() const {return Category(GetCode(), GetScope(), GetDescription(), GetLabel(), GetRank(), GetCategoryId());}
        };

        typedef Entry const_iterator;
        typedef Entry iterator;
        DGNPLATFORM_EXPORT const_iterator begin() const;
        const_iterator end() const {return Entry(m_stmt.get(), false);}
        DGNPLATFORM_EXPORT size_t QueryCount() const;
    }; // Iterator

    //! An iterator over the SubCategories of a category
    //! @ingroup DgnCategoryGroup
    struct SubCategoryIterator : BeSQLite::DbTableIterator
    {
    private:
        DgnCategoryId m_categoryId;
    public:

        //! construct a SubCategoryIterator
        //! @param[in] db The database for the SubCategory table
        //! @param[in] category Limit the iterator to SubCategories of this category. If invalid, iterate all SubCategories of all categories.
        SubCategoryIterator(DgnDbCR db, DgnCategoryId category) : DbTableIterator((BeSQLite::DbCR)db), m_categoryId(category) {}

        //! An entry in the table.
        struct Entry : DbTableIterator::Entry, std::iterator<std::input_iterator_tag, Entry const>
        {
        private:
            friend struct SubCategoryIterator;
            Entry(BeSQLite::StatementP sql, bool isValid) : DbTableIterator::Entry(sql,isValid) {}
        public:
            DGNPLATFORM_EXPORT DgnCategoryId GetCategoryId() const; //! The Category id.
            DGNPLATFORM_EXPORT DgnSubCategoryId GetSubCategoryId() const; //!< The SubCategory id.
            DGNPLATFORM_EXPORT SubCategory::Appearance GetAppearance() const; //!< The Appearance for this SubCategory
            DGNPLATFORM_EXPORT Utf8CP GetCode() const; //!< The SubCategory's code. Never nullptr. Unique. Not translated.
            DGNPLATFORM_EXPORT Utf8String GetLabel() const; //!< The SubCategory's display label which may be translated.
            DGNPLATFORM_EXPORT Utf8CP GetDescription() const; //!< The SubCategory's description, if any. May be nullptr.
            Entry const& operator*() const {return *this;}
        };

        typedef Entry const_iterator;
        typedef Entry iterator;
        DGNPLATFORM_EXPORT const_iterator begin() const;
        const_iterator end() const {return Entry(nullptr, false);}
    }; // SubCategoryIterator

private:
    friend struct DgnDb;
    explicit DgnCategories(DgnDbR db) : DgnDbTable(db) {}

public:
    ///@name Querying and manipulating categories
    //@{
    //! Add a new category to the DgnDb.
    //! @param[in] row The definition of the category to create.
    //! @param[in] appearance the appearance for the default SubCategory for the new category
    //! @return BE_SQLITE_OK if the category was added; non-zero otherwise. BE_SQLITE_CONSTRAINT indicates that the specified DgnCategoryId and/or code is already used.
    DGNPLATFORM_EXPORT BeSQLite::DbResult Insert(Category& row, SubCategory::Appearance const& appearance);

    //! Remove a category from the DgnDb.
    //! @param[in] id the id of the category to remove.
    //! @return whether the delete statement succeeded. Note that this method will return BE_SQLITE_OK even if the categoryId did not exist prior to this call.
    //! @note Deleting a category can result in an inconsistent database. There is no checking that the category to be removed is not in use somehow, and
    //! in general the answer to that question is nearly impossible to determine. It is very rarely possible to use this method unless you
    //! know for sure that the category is no longer necessary (for example, on a blank database). Otherwise, avoid using this method.
    //! @note it is illegal to delete the default category. Any attempt to do so will fail.
    DGNPLATFORM_EXPORT BeSQLite::DbResult Delete(DgnCategoryId id);

    //! Change properties of a category.
    //! @param[in] row The new category data to apply.
    //! @return BE_SQLITE_OK if the update was applied; non-zero otherwise. BE_SQLITE_CONSTRAINT indicates that the specified code is used
    //! by another category in the table.
    DGNPLATFORM_EXPORT BeSQLite::DbResult Update(Category const& row);

    //! Get the Id of a Category from its code.
    //! @param[in] categoryCode The code of the category of interest.
    //! @return the Id of the category. Will be invalid if no category by the specified code exists in the DgnDb.
    DGNPLATFORM_EXPORT DgnCategoryId QueryCategoryId(Utf8CP categoryCode) const;

    //! Query for the DgnCategoryId that owns the specified DgnSubCategoryId.
    //! @param[in] subCategoryId The Id of the SubCategory of interest.
    //! @return the Id of the category.  Will be invalid in the case of an unsuccessful query.
    DGNPLATFORM_EXPORT DgnCategoryId QueryCategoryId(DgnSubCategoryId subCategoryId) const;

    //! Get the DgnCategoryId from the specified element.
    //! @param[in] elementId the element to query
    //! @return the Id of the category. Will be invalid if there is no element whose Id equals elementId.
    DGNPLATFORM_EXPORT DgnCategoryId QueryCategoryId(DgnElementId elementId) const;

    //! Get the information about a category from its Id.
    //! @param[in] id The Id of the category of interest.
    //! @return The data for the category. Call IsValid() on the result to determine whether this method was successful.
    DGNPLATFORM_EXPORT Category Query(DgnCategoryId id) const;

    //! Look up a category by code.
    //! @param[in] categoryCode the category code to look up
    //! @return The data for the category. Call IsValid() on the result to determine whether this method was successful.
    Category QueryCategoryByCode(Utf8CP categoryCode) const {return Query(QueryCategoryId(categoryCode));}

    //! Get an iterator over the categories in this DgnDb.
    Iterator MakeIterator() const {return Iterator(m_dgndb);}

    //! Query the highest DgnCategoryId used in this table
    DGNPLATFORM_EXPORT DgnCategoryId QueryHighestId() const;
    //@}

    ///@name Querying and manipulating SubCategories of a category
    //@{
    //! Add a new SubCategory to a category.
    //! @param[in] subCategory The definition of the SubCategory.
    //! @return BE_SQLITE_OK if the SubCategory was added; non-zero otherwise. BE_SQLITE_CONSTRAINT indicates that the specified DgnCategoryId does not exist or the SubCategory's code
    //! or DgnSubCategoryId is already in use.
    DGNPLATFORM_EXPORT BeSQLite::DbResult InsertSubCategory(SubCategory& subCategory);

    //! Remove a SubCategory from a category.
    //! @param[in] subCategoryId the Id of the SubCategory to be deleted.
    //! @return whether the delete statement succeeded. Note that this method will return BE_SQLITE_OK even if the subCategoryKey did not exist prior to this call.
    //! @note Deleting a SubCategory from a category can result in an inconsistent database. There is no checking that the SubCategory to be removed is not in use somehow, and
    //! in general the answer to that question is nearly impossible to determine. It is very rarely possible to use this method unless you
    //! know for sure that the category is no longer necessary (for example, on a blank database). Otherwise, avoid using this method.
    DGNPLATFORM_EXPORT BeSQLite::DbResult DeleteSubCategory(DgnSubCategoryId subCategoryId);

    //! Change properties of a SubCategory.
    //! @param[in] subCategory The new SubCategory data to apply.
    //! @return BE_SQLITE_OK if the update was applied; non-zero otherwise. BE_SQLITE_CONSTRAINT indicates that the specified code is used
    //! by another SubCategory for the category.
    DGNPLATFORM_EXPORT BeSQLite::DbResult UpdateSubCategory(SubCategory const& subCategory);

    //! Get the Id of a SubCategory from its code.
    //! @param[in] categoryId the category for which the code applies.
    //! @param[in] subCategoryCode The code of the SubCategory of interest.
    //! @return the Id of the SubCategory. Will be invalid if no SubCategory on that category by the specified code exists
    DGNPLATFORM_EXPORT DgnSubCategoryId QuerySubCategoryId(DgnCategoryId categoryId, Utf8CP subCategoryCode) const;

    //! Get the Id of the SubCategory associated with the specified DgnGeomPart.
    //! @param[in] geomPartId the DgnGeomPart to query.
    //! @return the Id of the SubCategory.  Will be invalid if the query fails.
    DGNPLATFORM_EXPORT DgnSubCategoryId QuerySubCategoryId(DgnGeomPartId geomPartId) const;

    //! Get the information about a SubCategory from its Id.
    //! @param[in] subCategoryId The Id of the SubCategory of interest.
    //! @return The data for the SubCategory. Call IsValid() on the result to determine whether this method was successful.
    DGNPLATFORM_EXPORT SubCategory QuerySubCategory(DgnSubCategoryId subCategoryId) const;

    //! Look up a SubCategory by code.
    //! @param[in] categoryId the category for which the code applies.
    //! @param[in] subCategoryCode the SubCategory code to look up
    //! @return The data for the SubCategory. Call IsValid() on the result to determine whether this method was successful.
    SubCategory QuerySubCategoryByCode(DgnCategoryId categoryId, Utf8CP subCategoryCode) const {return QuerySubCategory(QuerySubCategoryId(categoryId, subCategoryCode));}

    //! Get an iterator over the SubCategories of a category or all SubCategories of all categories.
    //! @param[in] categoryId Limit the iterator to SubCategories of this category. If invalid, iterate all SubCategories of all categories.
    SubCategoryIterator MakeSubCategoryIterator(DgnCategoryId categoryId=DgnCategoryId()) const {return SubCategoryIterator(m_dgndb, categoryId);}
    //@}

    //! Get a string containing the list of characters that may NOT appear in category codes.
    static Utf8CP GetIllegalCharacters() {return "<>\\/.\"?*|,='&\n\t";}

    //! Determine whether the supplied code is a valid category code.
    //! @param[in] code The candidate category code to check
    //! @return true if the category code is valid, false otherwise.
    //! @note Category codes may also not start or end with a space.
    static bool IsValidCode(Utf8StringCR code) {return DgnDbTable::IsValidName(code, GetIllegalCharacters());}

    //! Get the Id of the default SubCategory for a given categoryId.
    //! @param[in] categoryId the category from which to get the default SubCategory.
    //! @return the Id of the SubCategory.
    static DgnSubCategoryId DefaultSubCategoryId(DgnCategoryId categoryId) {return DgnSubCategoryId(categoryId.GetValue());}
};

//=======================================================================================
//! The types of views that can exist in a DgnDb.
//! @ingroup DgnViewGroup
//=======================================================================================
enum class DgnViewType
{
    None     = 0,           //!< do not return any views
    Physical = 1<<0,        //!< a view of physical (3d) models
    Drawing  = 1<<1,        //!< a view of a single drawing (2d) model
    Sheet    = 1<<2,        //!< a view of a sheet.
    Component = 1<<4,       //!< a view of a single component model
    TwoD   = (Drawing | Sheet),
    All    = (Physical | Drawing | Sheet | Component),
};

//=======================================================================================
//! The source for the creation a DgnView.
//! @ingroup DgnViewGroup
//=======================================================================================
enum class DgnViewSource
{
    User      = 1<<0,      //!< created by a user
    Generated = 1<<1,      //!< automatically generated by a program, may be relevant to user
    Private   = 1<<2,      //!< used internally and should not be presented to user
};

//=======================================================================================
//! Each View has an entry in the View table.
//! @ingroup DgnViewGroup
//=======================================================================================
struct DgnViews : DgnDbTable
{
private:
    friend struct DgnDb;
    explicit DgnViews(DgnDbR db) : DgnDbTable(db){}

public:
    //! An object that holds the values for a View in the DgnViews.
    struct View
    {
    private:
        friend struct DgnViews;
        DgnViewId          m_viewId;
        DgnClassId         m_classId;
        DgnViewType        m_viewType;
        DgnViewSource      m_viewSource;
        DgnModelId         m_baseModelId;
        Utf8String         m_name;
        Utf8String         m_description;

    public:
        View() {m_viewType=DgnViewType::Physical;}
        View(DgnViewType viewType, DgnClassId classId, DgnModelId baseId, Utf8CP name, Utf8CP descr=0, DgnViewSource source=DgnViewSource::User,DgnViewId id=DgnViewId()) : m_viewId(id), m_baseModelId(baseId),
                    m_viewType(viewType), m_name(name), m_classId(classId)
            {
            m_description.AssignOrClear(descr);
            m_viewSource = source;
            }

        DgnViewId GetId() const {return m_viewId;} //!< The DgnViewId of this view.
        DgnModelId GetBaseModelId() const {return m_baseModelId;}//!< Get the DgnModelId of the base model for this view. Every view has one DgnModel designated its "base" model.
        DgnViewType GetDgnViewType() const {return m_viewType;} //!< Get the DgnViewType for this view.
        DgnClassId GetClassId() const {return m_classId;} //!< Get the DgnClassId of this View
        DgnViewSource GetDgnViewSource() const {return m_viewSource;} //!< Get the DgnViewSource for this view.
        Utf8CP GetName() const {return m_name.c_str();} //!< Get the name for this view.
        Utf8CP GetDescription() const {return m_description.length()>0 ? m_description.c_str() : nullptr;} //!< Get the description (if present) for this view.

        void SetDgnViewType(DgnClassId classId, DgnViewType val) {m_classId=classId, m_viewType = val;}    //!< Set the DgnViewType for this view.
        void SetDgnViewSource(DgnViewSource val) {m_viewSource = val;} //!< Set the DgnViewSource for this view.
        void SetBaseModelId(DgnModelId val) {m_baseModelId = val;} //!< Set the DgnModelId of the base model for this view.
        void SetDescription(Utf8CP val) {m_description.AssignOrClear(val);} //!< Set the description for this view.
        void SetName(Utf8CP val) {m_name = val;}//!< Set the name for this view. View names must be unique for a DgnDb.
        bool IsValid() const {return m_viewId.IsValid();}   //!< Determine whether this View is valid or not.
    };

    //! An iterator over the table.
    struct Iterator : BeSQLite::DbTableIterator
    {
    private:
        int  m_typeMask;

    public:
        Iterator(DgnDbCR db, int viewTypeMask) : DbTableIterator((BeSQLite::DbCR)db) {m_typeMask = viewTypeMask;}

        //! An entry in the table
        struct Entry : DbTableIterator::Entry, std::iterator<std::input_iterator_tag, Entry const>
        {
        private:
            friend struct Iterator;
            Entry(BeSQLite::StatementP sql, bool isValid) : DbTableIterator::Entry(sql,isValid) {}
        public:
            DGNPLATFORM_EXPORT DgnViewId GetDgnViewId() const;
            DGNPLATFORM_EXPORT DgnModelId GetBaseModelId() const;
            DGNPLATFORM_EXPORT DgnViewType GetDgnViewType() const;
            DGNPLATFORM_EXPORT DgnViewSource GetDgnViewSource() const;
            DGNPLATFORM_EXPORT Utf8CP GetName() const;
            DGNPLATFORM_EXPORT Utf8CP GetDescription() const;
            DGNPLATFORM_EXPORT DgnClassId GetClassId() const;
            Entry const& operator*() const {return *this;}
        }; // Entry

    typedef Entry const_iterator;
    typedef Entry iterator;
    DGNPLATFORM_EXPORT size_t QueryCount() const;
    DGNPLATFORM_EXPORT Entry begin() const;
    Entry end() const {return Entry(nullptr, false);}
    }; // Iterator

public:
    //! Add a new view to the database.
    DGNPLATFORM_EXPORT BeSQLite::DbResult Insert(View&);

    //! Delete an existing view from the database.
    DGNPLATFORM_EXPORT BeSQLite::DbResult Delete(DgnViewId viewId);

    //! Change the contents of an existing view in the database.
    DGNPLATFORM_EXPORT BeSQLite::DbResult Update(View const&);

    //! Get the DgnViewId for a view, by name
    DGNPLATFORM_EXPORT DgnViewId QueryViewId(Utf8CP viewName) const;

    //! Get the data for a view, by DgnViewId
    DGNPLATFORM_EXPORT View QueryView(DgnViewId id) const;

    enum class FillModels{No=0, Yes=1};
    DGNPLATFORM_EXPORT ViewControllerPtr LoadViewController(DgnViewId id, FillModels fillModels=FillModels::No) const;

    DGNVIEW_EXPORT BeSQLite::DbResult RenderAndSaveThumbnail(DgnViewId id, int resolution, DgnRenderMode renderModeOverride);

    typedef DgnViewProperty::Spec const& DgnViewPropertySpecCR;

    //! Query a view property
    BeSQLite::DbResult QueryProperty(void* value, uint32_t size, DgnViewId viewId, DgnViewPropertySpecCR spec, uint64_t id=0) const;

    //! Query a view property string
    BeSQLite::DbResult QueryProperty(Utf8StringR value, DgnViewId viewId, DgnViewPropertySpecCR spec, uint64_t id=0) const;

    //! Query the size of a view property
    BeSQLite::DbResult QueryPropertySize(uint32_t& size, DgnViewId viewId, DgnViewPropertySpecCR spec, uint64_t id=0) const;

    //! Change a view property
    BeSQLite::DbResult SaveProperty(DgnViewId viewId, DgnViewPropertySpecCR spec, void const* value, uint32_t size, uint64_t id=0);

    //! Change a view property string
    BeSQLite::DbResult SavePropertyString(DgnViewId viewId, DgnViewPropertySpecCR spec, Utf8StringCR value, uint64_t id=0);

    //! Delete a view property
    BeSQLite::DbResult DeleteProperty(DgnViewId viewId, DgnViewPropertySpecCR spec, uint64_t id=0);

    //! Get an iterator over the Views in this DgnDb.
    Iterator MakeIterator(int viewTypeMask= (int)DgnViewType::All) const {return Iterator(m_dgndb, viewTypeMask);}
};

enum class ModelIterate
    {
    All = 1<<0,   //!< return all iterable models
    Gui = 1<<1,   //!< return only models marked as visible in Model list GUI
    };

//=======================================================================================
//! Each DgnModel has an entry in the DgnModels table
//=======================================================================================
struct DgnModels : DgnDbTable
{
private:
    friend struct DgnDb;
    friend struct DgnModel;
    friend struct dgn_TxnTable::Model;
    typedef bmap<DgnModelId,DgnModelPtr> T_DgnModelMap;

    T_DgnModelMap   m_models;
    QvCache*        m_qvCache;
    bmap<DgnModelId,bpair<uint64_t,DgnModelType>> m_modelDependencyIndexAndType;

    void ClearLoaded();
    DgnModelP LoadDgnModel(DgnModelId modelId);
    bool FreeQvCache();
    void Empty() {ClearLoaded(); FreeQvCache();}
    void AddLoadedModel(DgnModelR);
    void DropLoadedModel(DgnModelR);

    DgnModels(DgnDbR db) : DgnDbTable(db) {m_qvCache= nullptr;}
    ~DgnModels() {} // don't call empty on destructor, Elements() has already been deleted.

public:
    //! An object that holds a row from the DgnModel table.
    struct Model
    {
        enum class CoordinateSpace
        {
            Local   = 0,    // the model has a local coordinate system
            World   = 1,    // the model is in the world coordinate system (only applicable to physical models).
            Aux     = 2,    // the model is in the an auxiliary coordinate system (only applicable to physical models).
        };

        friend struct DgnModels;

    private:
        DgnModelId   m_id;
        DgnClassId   m_classId;
        Utf8String   m_name;
        Utf8String   m_description;
        DgnModelType m_modelType;
        CoordinateSpace  m_space;
        bool         m_inGuiList;

    public:
        Model()
            {
            m_modelType = DgnModelType::Physical;
            m_space = CoordinateSpace::Local;
            m_inGuiList = true;
            };

        Model(Utf8CP name, DgnModelType modelType, CoordinateSpace space, DgnClassId classid, DgnModelId id=DgnModelId()) : m_id(id), m_classId(classid), m_name(name)
            {
            m_modelType = modelType;
            m_space = space;
            m_inGuiList = true;
            }

        Utf8StringR GetNameR() {return m_name;}
        Utf8StringR GetDescriptionR() {return m_description;}
        void SetName(Utf8CP val) {m_name.assign(val);}
        void SetDescription(Utf8CP val) {m_description.AssignOrClear(val);}
        void SetInGuiList(bool val)   {m_inGuiList = val;}
        void SetId(DgnModelId id) {m_id = id;}
        void SetClassId(DgnClassId classId) {m_classId = classId;}
        void SetModelType(DgnClassId classId, DgnModelType val) {m_classId = classId; m_modelType = val;}
        void SetCoordinateSpace(CoordinateSpace val) {m_space = val;}

        DgnModelId GetId() const {return m_id;}
        Utf8CP GetNameCP() const {return m_name.c_str();}
        Utf8String GetName() const {return m_name;}
        Utf8CP GetDescription() const {return m_description.c_str();}
        DgnModelType GetModelType() const {return m_modelType;}
        DgnClassId GetClassId() const {return m_classId;}
        CoordinateSpace GetCoordinateSpace() const {return m_space;}
        bool InGuiList() const {return m_inGuiList;}
        bool Is3d() const {return m_modelType==DgnModelType::Physical;}

    }; // Model

    struct Iterator : BeSQLite::DbTableIterator
    {
    private:
        ModelIterate   m_itType;

    public:
        Iterator(DgnDbCR db, Utf8CP where, ModelIterate itType) : DbTableIterator((BeSQLite::DbCR) db), m_itType(itType) {if (where) m_params.SetWhere(where);}
        struct Entry : DbTableIterator::Entry, std::iterator<std::input_iterator_tag, Entry const>
        {
        private:
            friend struct Iterator;
            Entry(BeSQLite::StatementP sql, bool isValid) : DbTableIterator::Entry(sql,isValid) {}

        public:
            DGNPLATFORM_EXPORT DgnModelId GetModelId() const;
            DGNPLATFORM_EXPORT Utf8CP GetName() const;
            DGNPLATFORM_EXPORT Utf8CP GetDescription() const;
            DGNPLATFORM_EXPORT DgnModelType GetModelType() const;
            DGNPLATFORM_EXPORT DgnClassId GetClassId() const;
            DGNPLATFORM_EXPORT Model::CoordinateSpace GetCoordinateSpace() const;
            DGNPLATFORM_EXPORT uint32_t GetVisibility() const;

            bool Is3d() const {return GetModelType()==DgnModelType::Physical;}
            bool InModelGui() const {return 0 != ((int)ModelIterate::Gui & GetVisibility());}
            Entry const& operator*() const {return *this;}
        };

        typedef Entry const_iterator;
        typedef Entry iterator;
        void SetIterationType(ModelIterate itType) {m_stmt=0; m_itType = itType;}
        DGNPLATFORM_EXPORT size_t QueryCount() const;
        DGNPLATFORM_EXPORT const_iterator begin() const;
        const_iterator end() const {return Entry(nullptr, false);}
    };

public:
    DGNPLATFORM_EXPORT QvCache* GetQvCache(bool createIfNecessary=true);
    void SetQvCache(QvCache* qvCache) {m_qvCache = qvCache;}

    //! Determine the Id of the first model in this DgnDb.
    DGNPLATFORM_EXPORT DgnModelId QueryFirstModelId() const;

    //! Load a DgnModel from this DgnDb. Loading a model does not cause its elements to be filled. Rather, it creates an
    //! instance of the appropriate model type. If the model is already loaded, a pointer to the existing DgnModel is returned.
    //! @param[in] modelId The Id of the model to load.
    DGNPLATFORM_EXPORT DgnModelP GetModel(DgnModelId modelId);

    template<class T> T* Get(DgnModelId id) {return dynamic_cast<T*>(GetModel(id));}

    //! Query if the specified model has already been loaded.
    //! @return a pointer to the model if found, or nullptr if the model has not been loaded.
    //! @see GetLoadedModels
    //! @see LoadModelById
    DGNPLATFORM_EXPORT DgnModelP FindModel(DgnModelId modelId);

    //! Get the currently loaded DgnModels for this DgnDb
    T_DgnModelMap const& GetLoadedModels() const {return m_models;}

    DGNPLATFORM_EXPORT BentleyStatus QueryModelById(Model* out, DgnModelId id) const;
    DGNPLATFORM_EXPORT BentleyStatus GetModelName(Utf8StringR, DgnModelId id) const;

    //! Find the ModelId of the model with the specified name name.
    //! @return The model's ModelId. Check dgnModelId.IsValid() to see if the DgnModelId was found.
    DGNPLATFORM_EXPORT DgnModelId QueryModelId(Utf8CP name) const;

    //! Query for the dependency index and type of the specified model
    //! @param[out] dependencyIndex  The model's DependencyIndex property value
    //! @param[out] modelType   The model's type
    //! @param[in] modelId      The model's ID
    //! @return non-zero if the model does not exist
    DGNPLATFORM_EXPORT BentleyStatus QueryModelDependencyIndexAndType(uint64_t& dependencyIndex, DgnModelType& modelType, DgnModelId modelId);

    //! Make an iterator over the models in this DgnDb.
    Iterator MakeIterator(Utf8CP where=nullptr, ModelIterate itType=ModelIterate::All) const {return Iterator(m_dgndb, where, itType);}
    //@}

    //! Generate a model name that is not currently in use in this file
    //! @param[in]  baseName base model name to start with (optional)
    //! @return unique name that was generated
    DGNPLATFORM_EXPORT Utf8String GetUniqueModelName(Utf8CP baseName);
    //@}

    //! Get a string containing the list of characters that may NOT appear in model names.
    static Utf8CP GetIllegalCharacters() {return "\\/:*?<>|\"\t\n,=&";}

    //! Determine whether the supplied name is a valid model name.
    //! @param[in] name The candidate model name to check
    //! @return true if the model name is valid, false otherwise.
    //! @note Model names may also not start or end with a space.
    static bool IsValidName(Utf8StringCR name) {return DgnDbTable::IsValidName(name, GetIllegalCharacters());}
};

//=======================================================================================
//! The DgnElements for a DgnDb.
//! This class holds a cache of reference-counted DgnElements. All in-memory DgnElements for a DgnDb are held in its DgnElements member.
//! When the reference count of an element goes to zero, it is not immediately freed. Instead, it is held by this class
//! and may be "reclaimed" later if/when it is needed again. The memory held by DgnElements is not actually freed until
//! their reference count goes to 0 and the cache is subsequently purged.
//! @ingroup DgnElementGroup
//=======================================================================================
struct DgnElements : DgnDbTable
{
    friend struct DgnDb;
    friend struct DgnElement;
    friend struct DgnModel;
    friend struct DgnModels;
    friend struct ElementHandler;
    friend struct TxnManager;
    friend struct ProgressiveViewFilter;
    friend struct dgn_TxnTable::Element;

    //! The totals for persistent DgnElements in this DgnDb. These values reflect the current state of the loaded elements.
    struct Totals
    {
        uint32_t m_extant;         //! total number of DgnElements extant (persistent and non-persistent)
        uint32_t m_entries;        //! total number of persistent elements 
        uint32_t m_unreferenced;   //! total number of unreferenced persistent elements 
        int64_t  m_allocedBytes;   //! total number of bytes of data held by persistent elements 
    };

    //! Statistics for element activity in this DgnDb. these values can be reset at any point to gauge "element flux"
    //! (note: the same element may become garbage and then be reclaimed, each such occurrence is reflected here.)
    struct Statistics
    {
        uint32_t m_newElements;    //! number of newly created or loaded elements
        uint32_t m_unReferenced;   //! number of elements that became garbage since last reset
        uint32_t m_reReferenced;   //! number of garbage elements that were referenced
        uint32_t m_purged;         //! number of garbage elements that were purged
    };

    //! Interface to track element loading.
    struct Listener
    {
        virtual ~Listener() {}
        virtual void _OnElementLoaded(DgnElementR) = 0;
    };

private:
    DgnElementId                m_highestElementId;
    EventHandlerList<Listener>* m_listeners;
    struct ElemIdTree*          m_tree;
    HeapZone                    m_heapZone;
    BeSQLite::StatementCache    m_stmts;
    BeSQLite::SnappyFromBlob    m_snappyFrom;
    BeSQLite::SnappyToBlob      m_snappyTo;
    mutable BeSQLite::BeDbMutex m_mutex;

    void OnReclaimed(DgnElementCR);
    void OnUnreferenced(DgnElementCR);
    void Destroy();
    void AddToPool(DgnElementCR) const;
    void DropFromPool(DgnElementCR) const;
    void SendOnLoadedEvent(DgnElementR elRef) const;
    void FinishUpdate(DgnElementCR replacement, DgnElementCR original);
    DgnElementCPtr LoadElement(DgnElement::CreateParams const& params, bool makePersistent) const;
    DgnElementCPtr LoadElement(DgnElementId elementId, bool makePersistent) const;
    bool IsElementIdUsed(DgnElementId id) const;
    DgnElementId GetHighestElementId();
    DgnElementId MakeNewElementId();
    DgnElementCPtr PerformInsert(DgnElementR element, DgnDbStatus&);
    DgnDbStatus PerformDelete(DgnElementCR);
    explicit DgnElements(DgnDbR db);
    ~DgnElements();

    DGNPLATFORM_EXPORT DgnElementCPtr InsertElement(DgnElementR element, DgnDbStatus* stat);
    DGNPLATFORM_EXPORT DgnElementCPtr UpdateElement(DgnElementR element, DgnDbStatus* stat);

public:
    BeSQLite::SnappyFromBlob& GetSnappyFrom() {return m_snappyFrom;}
    BeSQLite::SnappyToBlob& GetSnappyTo() {return m_snappyTo;}
    DGNPLATFORM_EXPORT BeSQLite::CachedStatementPtr GetStatement(Utf8CP sql) const;
    DGNPLATFORM_EXPORT void ChangeMemoryUsed(int32_t delta) const;

    //! Look up an element in the pool of loaded elements for this DgnDb.
    //! @return A pointer to the element, or nullptr if the is not in the pool.
    //! @private
    DGNPLATFORM_EXPORT DgnElementCP FindElement(DgnElementId id) const;

    //! Query the DgnModelId of the specified DgnElementId.
    //! @private
    DGNPLATFORM_EXPORT DgnModelId QueryModelId(DgnElementId elementId) const;

    //! Query for the DgnElementId of the element that has the specified code
    //! @note Element codes are usually, but not necessarily, unique. If not unique, this method returns the first one found.
    DGNPLATFORM_EXPORT DgnElementId QueryElementIdByCode(Utf8CP code) const;

    //! Free unreferenced elements in the pool until the total amount of memory used by the pool is no more than a target number of bytes.
    //! @param[in] memTarget The target number of bytes used by elements in the pool. If the pool is currently using more than this target,
    //! unreferenced elements are freed until the the pool uses no more than targetMem bytes. Least recently used elements are freed first.
    //! If memTarget <= 0, all unreferenced elements are freed.
    //! @note: There is no guarantee that the pool will not actually consume more than memTarget bytes after this call, since elements with
    //! reference counts greater than 0 cannot be purged.
    DGNPLATFORM_EXPORT void Purge(int64_t memTarget);

    //! Get the total counts for the current state of the pool.
    DGNPLATFORM_EXPORT Totals GetTotals() const;

    //! Shortcut to get the Totals.m_allocatedBytes member
    int64_t GetTotalAllocated() const {return GetTotals().m_allocedBytes;}

    //! Get the statistics for the current state of the element pool.
    DGNPLATFORM_EXPORT Statistics GetStatistics() const;

    //! Reset the statistics for the element pool.
    DGNPLATFORM_EXPORT void ResetStatistics();

    //! Get a DgnElement from this DgnDb by its DgnElementId.
    //! @remarks The element is loaded from the database if necessary.
    //! @return Invalid if the element does not exist.
    DGNPLATFORM_EXPORT DgnElementCPtr GetElement(DgnElementId id) const;

    //! Get a DgnElement by its DgnElementId, and dynamic_cast the result to a specific subclass of DgnElement.
    //! This is merely a templated shortcut to dynamic_cast the return of #GetElement to a subclass of DgnElement.
    template<class T> RefCountedCPtr<T> Get(DgnElementId id) const {return dynamic_cast<T const*>(GetElement(id).get());}

    //! Get an editable copy of an element by DgnElementId.
    //! @return Invalid if the element does not exist, or if it cannot be edited.
    template<class T> RefCountedPtr<T> GetForEdit(DgnElementId id) const {RefCountedCPtr<T> orig=Get<T>(id); return orig.IsValid() ?(T*)orig->CopyForEdit().get() : nullptr;}

    //! Insert a copy of the supplied DgnElement into this DgnDb.
    //! @param[in] element The DgnElement to insert.
    //! @param[in] stat An optional status value. Will be DgnDbStatus::Success if the insert was successful, error status otherwise.
    //! @return RefCountedCPtr to the newly persisted /b copy of /c element. Will be invalid if the insert failed.
    template<class T> RefCountedCPtr<T> Insert(T& element, DgnDbStatus* stat=nullptr) {return (T const*) InsertElement(element, stat).get();}

    //! Update the original persistent DgnElement from which the supplied DgnElement was copied.
    //! @param[in] element The modified copy of element to update.
    //! @param[in] stat An optional status value. Will be DgnDbStatus::Success if the update was successful, error status otherwise.
    //! @return RefCountedCPtr to the modified persistent element. Will be invalid if the update failed.
    template<class T> RefCountedCPtr<T> Update(T& element, DgnDbStatus* stat=nullptr) {return (T const*) UpdateElement(element, stat).get();}

    //! Delete a DgnElement from this DgnDb.
    //! @param[in] element The element to delete.
    //! @return DgnDbStatus::Success if the element was deleted, error status otherwise.
    DGNPLATFORM_EXPORT DgnDbStatus Delete(DgnElementCR element);

    //! Delete a DgnElement from this DgnDb by DgnElementId.
    //! @return DgnDbStatus::Success if the element was deleted, error status otherwise.
    //! @note This method is merely a shortcut to #GetElement and then #Delete
    DgnDbStatus Delete(DgnElementId id) {auto el=GetElement(id); return el.IsValid() ? Delete(*el) : DgnDbStatus::ElementNotFound;}

    //! Get the Heapzone for this DgnDb.
    HeapZone& GetHeapZone() {return m_heapZone;}

    //! Query the DgnElementKey for a DgnElement from this DgnDb by its DgnElementId.
    //! @return Invalid key if the element does not exist.
    //! @remarks This queries the database for the DgnClassId for the given DgnElementId. It does not check if the element is loaded, nor does it load the element into memory.
    //! If you have a DgnElement, call GetElementKey on it rather than using this method.
    DGNPLATFORM_EXPORT DgnElementKey QueryElementKey(DgnElementId id) const;

    //! Add element-loaded-from-db event listener.
    DGNPLATFORM_EXPORT void AddListener(Listener* listener);

    //! Drop element-loaded-from-db event listener.
    DGNPLATFORM_EXPORT void DropListener(Listener* listener);
};

//=======================================================================================
//! Each GeomPart has a row in the DgnGeomParts table
//=======================================================================================
struct DgnGeomParts : DgnDbTable
{
    friend struct DgnDb;

private:
    explicit DgnGeomParts(DgnDbR db) : DgnDbTable(db) {}
    DgnGeomPartId m_highestGeomPartId; // 0 means not yet valid. Highest DgnGeomPartId (for current repositoryId)

public:
    DgnGeomPartId GetHighestGeomPartId();
    DgnGeomPartId MakeNewGeomPartId();

public:
    //! Load a geometry part by ID.
    //! @param[in] geomPartId the ID of the geometry part to load
    DGNPLATFORM_EXPORT DgnGeomPartPtr LoadGeomPart(DgnGeomPartId geomPartId);

    //! Query for a DgnGeomPartId by code.
    DGNPLATFORM_EXPORT DgnGeomPartId QueryGeomPartId(Utf8CP code);

    //! Insert a geometry part into the DgnDb.
    //! @param[in] geomPart geometry part to insert
    //! @return The DgnGeomPartId for the newly inserted part. Will be invalid if part could not be added.
    //! @note This method will update the DgnGeomPartId in geomPart.
    DGNPLATFORM_EXPORT BentleyStatus InsertGeomPart(DgnGeomPartR geomPart);

    //! Update an existing geometry part in the DgnDb.
    //! @param[in] geomPart geometry part. Its ID identifies the existing geom part. Its geometry is written to the DgnDb.
    //! @return non-zero error status if the geom part does not exist or if its ID is invalid
    DGNPLATFORM_EXPORT BentleyStatus UpdateGeomPart(DgnGeomPartR geomPart);

    //! Insert the ElementGeomUsesParts relationship between an element and the geom parts it uses.
    //! @note Most apps will not need to call this directly.
    //! @private
    DGNPLATFORM_EXPORT BentleyStatus InsertElementGeomUsesParts(DgnElementId elementId, DgnGeomPartId geomPartId);

    //! Delete the geometry part associated with the specified ID
    DGNPLATFORM_EXPORT BentleyStatus DeleteGeomPart(DgnGeomPartId);
};

//=======================================================================================
//! The DgnColors holds the Named Colors for a DgnDb. Named Colors are RGB values (no transparency) that may
//! be named and from a "color book". The entries in the table are identified by DgnTrueColorId's.
//! Once a True Color is defined, it may not be changed or deleted. Note that there may be multiple enties in the table with the same RGB value.
//! However, if a book name is supplied, there may not be two entries with the same name.
//=======================================================================================
struct DgnColors : DgnDbTable
{
private:
    friend struct DgnDb;
    mutable DgnTrueColorId m_nextColorId;

    explicit DgnColors(DgnDbR db) : DgnDbTable(db){}

public:
    //! Add a new entry to this DgnColors.
    //! @param[in] color The RGB values for the new entry.
    //! @param[in] name The name of the color (or nullptr).
    //! @param[in] bookname The name of the colorbook (or nullptr).
    //! @note For a given bookname, there may not be more than one color with the same name.
    //! @return colorId The DgnTrueColorId for the newly created entry. Will be invalid if name+bookname is not unique.
    DGNPLATFORM_EXPORT DgnTrueColorId Insert(ColorDef color, Utf8CP name=0, Utf8CP bookname=0);

    //! Find the first DgnTrueColorId that has a given color value.
    //! @return A DgnTrueColorId for the supplied color value. If no entry in the table has the given value, the DgnTrueColorId will be invalid.
    //! @note If the table holds more than one entry with the same value, the "first" DgnTrueColorId is returned.
    DGNPLATFORM_EXPORT DgnTrueColorId FindMatchingColor(ColorDef color) const;

    //! Get a color by DgnTrueColorId.
    //! @param[out] color The RGB value for the color
    //! @param[out] name The name for the colorId. May be nullptr.
    //! @param[out] bookname The bookName for the colorId. May be nullptr.
    //! @param[in] colorId the true color id to query
    //! @return SUCCESS if colorId was found in the table and the values are valid. ERROR otherwise.
    DGNPLATFORM_EXPORT BentleyStatus QueryColor(ColorDef& color, Utf8StringP name, Utf8StringP bookname, DgnTrueColorId colorId) const;

    //! Get color by name and bookname.
    //! @param[out] color The RGB value for the color
    //! @param[in] name The name for the colorId.
    //! @param[in] bookname The bookName for the colorId.
    //! @return SUCCESS if color was found in the table and the RGB value is valid. ERROR otherwise.
    DGNPLATFORM_EXPORT BentleyStatus QueryColorByName(ColorDef& color, Utf8StringCR name, Utf8StringCR bookname) const;

    struct Iterator : BeSQLite::DbTableIterator
    {
    public:
        explicit Iterator(DgnDbCR db) : DbTableIterator((BeSQLite::DbCR)db) {}

        struct Entry : DbTableIterator::Entry, std::iterator<std::input_iterator_tag, Entry const>
        {
        private:
            friend struct Iterator;
            Entry(BeSQLite::StatementP sql, bool isValid) : DbTableIterator::Entry(sql,isValid) {}
        public:
            DGNPLATFORM_EXPORT DgnTrueColorId GetId() const;
            DGNPLATFORM_EXPORT ColorDef GetColorValue() const;
            DGNPLATFORM_EXPORT Utf8CP GetName() const;
            DGNPLATFORM_EXPORT Utf8CP GetBookName() const;
            Entry const& operator*() const {return *this;}
        };

    typedef Entry const_iterator;
    typedef Entry iterator;
    DGNPLATFORM_EXPORT size_t QueryCount() const;
    DGNPLATFORM_EXPORT Entry begin() const;
    Entry end() const {return Entry(nullptr, false);}
    };

    //! Make an iterator over the named colors in this DgnDb.
    Iterator MakeIterator() const {return Iterator(m_dgndb);}
};

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     03/2015
//=======================================================================================
struct DgnFonts : NonCopyableClass
{
    typedef bmap<DgnFontId,DgnFontPtr> T_FontMap;

    //=======================================================================================
    // @bsiclass                                                    Jeff.Marker     03/2015
    //=======================================================================================
    struct DbFontMapDirect : NonCopyableClass
    {
    private:
        friend DgnFonts;
        DgnFonts& m_dbFonts;

        DbFontMapDirect(DgnFonts& dbFonts) : m_dbFonts(dbFonts) {}

    public:
        struct Iterator : public BeSQLite::DbTableIterator
        {
        private:
            DgnFonts& m_dbFonts;

        public:
            Iterator(DgnFonts& dbFonts) : BeSQLite::DbTableIterator(dbFonts.m_db), m_dbFonts(dbFonts) {}

            struct Entry : public DbTableIterator::Entry, public std::iterator<std::input_iterator_tag,Entry const>
            {
            private:
                friend struct Iterator;
                Entry(BeSQLite::StatementP sql, bool isValid) : DbTableIterator::Entry(sql, isValid) {}

            public:
                DGNPLATFORM_EXPORT DgnFontId GetId() const;
                DGNPLATFORM_EXPORT DgnFontType GetType() const;
                DGNPLATFORM_EXPORT Utf8CP GetName() const;
                Entry const& operator*() const { return *this; }
            };

            typedef Entry const_iterator;
            DGNPLATFORM_EXPORT const_iterator begin() const;
            const_iterator end() const { return Entry(NULL, false); }
            DGNPLATFORM_EXPORT size_t QueryCount() const;
        };

        bool DoesFontTableExist() const { return m_dbFonts.m_db.TableExists(m_dbFonts.m_tableName.c_str()); }
        DGNPLATFORM_EXPORT BentleyStatus CreateFontTable();
        DGNPLATFORM_EXPORT DgnFontPtr QueryById(DgnFontId) const;
        DGNPLATFORM_EXPORT DgnFontPtr QueryByTypeAndName(DgnFontType, Utf8CP) const;
        DGNPLATFORM_EXPORT DgnFontId QueryIdByTypeAndName(DgnFontType, Utf8CP) const;
        DGNPLATFORM_EXPORT bool ExistsById(DgnFontId) const;
        DGNPLATFORM_EXPORT bool ExistsByTypeAndName(DgnFontType, Utf8CP) const;
        DGNPLATFORM_EXPORT BentleyStatus Insert(DgnFontCR, DgnFontId&);
        DGNPLATFORM_EXPORT BentleyStatus Update(DgnFontCR, DgnFontId);
        DGNPLATFORM_EXPORT BentleyStatus Delete(DgnFontId);
        Iterator MakeIterator() const { return Iterator(m_dbFonts); }
        };

    //=======================================================================================
    // @bsiclass                                                    Jeff.Marker     03/2015
    //=======================================================================================
    struct DbFaceDataDirect : NonCopyableClass
    {
        struct FaceKey
        {
            DGNPLATFORM_EXPORT static const Utf8CP FACE_NAME_Regular;
            DGNPLATFORM_EXPORT static const Utf8CP FACE_NAME_Bold;
            DGNPLATFORM_EXPORT static const Utf8CP FACE_NAME_Italic;
            DGNPLATFORM_EXPORT static const Utf8CP FACE_NAME_BoldItalic;

            DgnFontType m_type;
            Utf8String m_familyName;
            Utf8String m_faceName;

            FaceKey() : m_type((DgnFontType)0) {}
            FaceKey(DgnFontType type, Utf8CP familyName, Utf8CP faceName) : m_type(type), m_familyName(familyName), m_faceName(faceName) {}
            bool Equals(FaceKey const& rhs) const { return ((rhs.m_type == m_type) && m_familyName.EqualsI(rhs.m_familyName) && m_faceName.EqualsI(rhs.m_faceName)); }
        };

        typedef uint64_t DataId;
        typedef FaceKey const& FaceKeyCR;
        typedef uint32_t FaceSubId;
        typedef bmap<FaceSubId, FaceKey> T_FaceMap;
        typedef T_FaceMap const& T_FaceMapCR;

        struct Iterator : public BeSQLite::DbTableIterator
        {
        private:
            DgnFonts& m_dbFonts;

        public:
            Iterator(DgnFonts& dbFonts) : BeSQLite::DbTableIterator(dbFonts.m_db), m_dbFonts(dbFonts) {}

            struct Entry : public DbTableIterator::Entry, public std::iterator<std::input_iterator_tag,Entry const>
            {
            private:
                friend struct Iterator;
                Entry(BeSQLite::StatementP sql, bool isValid) : DbTableIterator::Entry(sql, isValid) {}

            public:
                DGNPLATFORM_EXPORT uint64_t GetId() const;
                DGNPLATFORM_EXPORT T_FaceMap GenerateFaceMap() const;
                Entry const& operator*() const { return *this; }
            };

            typedef Entry const_iterator;
            DGNPLATFORM_EXPORT const_iterator begin() const;
            const_iterator end() const { return Entry(NULL, false); }
            DGNPLATFORM_EXPORT size_t QueryCount() const;
        };

    private:
        friend DgnFonts;
        DgnFonts& m_dbFonts;

        DbFaceDataDirect(DgnFonts& dbFonts) : m_dbFonts(dbFonts) {}

    public:
        DGNPLATFORM_EXPORT BentleyStatus QueryById(bvector<Byte>&, DataId);
        DGNPLATFORM_EXPORT BentleyStatus QueryByFace(bvector<Byte>&, FaceSubId&, FaceKeyCR);
        DGNPLATFORM_EXPORT bool Exists(FaceKeyCR);
        DGNPLATFORM_EXPORT BentleyStatus Insert(ByteCP, size_t dataSize, T_FaceMapCR);
        DGNPLATFORM_EXPORT BentleyStatus Delete(FaceKeyCR);
        Iterator MakeIterator() const { return Iterator(m_dbFonts); }
    };

private:
    DbFontMapDirect m_dbFontMap;
    DbFaceDataDirect m_dbFaceData;
    BeSQLite::DbR m_db;
    Utf8String m_tableName;
    bool m_isFontMapLoaded;
    T_FontMap m_fontMap;

    void Update();

public:
    DgnFonts(BeSQLite::DbR db, Utf8CP tableName) : m_dbFontMap(*this), m_dbFaceData(*this), m_db(db), m_tableName(tableName), m_isFontMapLoaded(false) {}

    DbFontMapDirect& DbFontMap() { return m_dbFontMap; }
    DbFaceDataDirect& DbFaceData() { return m_dbFaceData; }
    void Invalidate() { m_isFontMapLoaded = false; m_fontMap.clear(); }
    DGNPLATFORM_EXPORT DgnFontCP FindFontById(DgnFontId) const;
    DGNPLATFORM_EXPORT DgnFontCP FindFontByTypeAndName(DgnFontType, Utf8CP) const;
    DGNPLATFORM_EXPORT DgnFontId FindId(DgnFontCR) const;
    DGNPLATFORM_EXPORT DgnFontId AcquireId(DgnFontCR);
};

/** @cond BENTLEY_SDK_Internal */
//=======================================================================================
// @bsiclass
//=======================================================================================
struct DgnMaterials : DgnDbTable
{
private:
    friend struct DgnDb;
    explicit DgnMaterials(DgnDbR db) : DgnDbTable(db) { }

public:
    struct Row
    {
    private:
        friend struct DgnMaterials;

        DgnMaterialId       m_materialId;
        Utf8String          m_name;
        Utf8String          m_palette;

    public:
        Row() { }
        Row(Utf8CP name, Utf8CP palette, DgnMaterialId id = DgnMaterialId()) : m_materialId(id), m_name(name), m_palette(palette) { }

        DgnMaterialId GetId() const { return m_materialId; }
        Utf8CP GetName() const { return m_name.c_str(); }
        Utf8CP GetPalette() const { return m_palette.c_str(); }
        void SetName(Utf8CP val) { m_name = val; }
        void SetPalette(Utf8CP val) { m_palette = val; }
        bool IsValid() const { return m_materialId.IsValid(); }
    };

    struct Iterator : BeSQLite::DbTableIterator
    {
    public:
        explicit Iterator(DgnDbCR db) : DbTableIterator((BeSQLite::DbCR)db) { }

        struct Entry : DbTableIterator::Entry, std::iterator<std::input_iterator_tag, Entry const>
        {
        private:
            friend struct Iterator;
            Entry(BeSQLite::StatementP sql, bool isValid) : DbTableIterator::Entry(sql,isValid) {}
        public:
            DGNPLATFORM_EXPORT DgnMaterialId GetId() const;
            DGNPLATFORM_EXPORT Utf8CP GetName() const;
            DGNPLATFORM_EXPORT Utf8CP GetPalette() const;
            Entry const& operator*() const {return *this;}
        };

    typedef Entry const_iterator;
    typedef Entry iterator;
    DGNPLATFORM_EXPORT size_t QueryCount() const;
    DGNPLATFORM_EXPORT Entry begin() const;
    Entry end() const {return Entry(nullptr, false);}
    };

    Iterator MakeIterator() const {return Iterator(m_dgndb);}

    DGNPLATFORM_EXPORT BeSQLite::DbResult InsertMaterial(Row& row);
    DGNPLATFORM_EXPORT BeSQLite::DbResult DeleteMaterial(DgnMaterialId materialId);
    DGNPLATFORM_EXPORT BeSQLite::DbResult UpdateMaterial(Row const& row);
    DGNPLATFORM_EXPORT Row QueryMaterialById(DgnMaterialId id) const;
};
/** @endcond */

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   09/13
//=======================================================================================
struct DgnUnits : NonCopyableClass
{
private:
    friend struct DgnDb;
    DgnDbR          m_dgndb;
    double          m_azimuth;
    AxisAlignedBox3d m_extent;
    DPoint3d        m_globalOrigin;      //!< in meters
    mutable bool    m_hasCheckedForGCS;
    mutable DgnGCS* m_gcs;
    mutable IGeoCoordinateServicesP m_geoServices;

    DgnUnits(DgnDbR db);
    void LoadProjectExtents();

public:
    DGNPLATFORM_EXPORT void Save();
    DGNPLATFORM_EXPORT DgnDbStatus Load();

    DgnDbR GetDgnDb() const {return m_dgndb;}

    void SetGlobalOrigin(DPoint3dCR origin) {m_globalOrigin=origin;}
    DPoint3dCR GetGlobalOrigin() const {return m_globalOrigin;}

    DGNPLATFORM_EXPORT BeSQLite::DbResult SaveProjectExtents(AxisAlignedBox3dCR newExtents);

    //! (Re-)compute the project extents by looking at the range tree.
    DGNPLATFORM_EXPORT AxisAlignedBox3d ComputeProjectExtents();

    //! Get the union of the range (axis-aligned bounding box) of all physical elements in this DgnDb
    DGNPLATFORM_EXPORT AxisAlignedBox3d GetProjectExtents();

    //! Convert a GeoPoint to an XYZ point
    //! @param[out] outUors     The output XYZ point
    //! @param[in] inLatLong    The input GeoPoint
    //! @return non-zero error status if the point cannot be converted or if this DgnDb is not geo-located
    DGNPLATFORM_EXPORT BentleyStatus UorsFromLatLong(DPoint3dR outUors, GeoPointCR inLatLong) const;

    //! Convert a an XYZ point to a GeoPoint
    //! @param[out] outLatLong  The output GeoPoint
    //! @param[in] inUors    The input XYZ point
    //! @return non-zero error status if the point cannot be converted or if this DgnDb is not geo-located
    DGNPLATFORM_EXPORT BentleyStatus LatLongFromUors(GeoPointR outLatLong, DPoint3dCR inUors) const;

    //! Query the GCS of this DgnDb, if any.
    //! @return this DgnDb's GCS or nullptr if this DgnDb is not geo-located
    DGNPLATFORM_EXPORT DgnGCS* GetDgnGCS() const;

    //! Gets the azimuth angle (true north offset) of the global coordinate system of this DgnDb <em>if it has one</em>.
    double GetAzimuth() const {return m_azimuth;}

    //! Set the azimuth of the global coordinate system of this DgnDb.
    void SetAzimuth(double azimuth) {m_azimuth = azimuth;}
};

//=======================================================================================
// @bsiclass
//=======================================================================================
struct DgnStyles : DgnDbTable
{
private:
    friend struct DgnDb;

    struct DgnLineStyles* m_lineStyles;
    struct DgnAnnotationTextStyles* m_annotationTextStyles;
    struct DgnAnnotationFrameStyles* m_annotationFrameStyles;
    struct DgnAnnotationLeaderStyles* m_annotationLeaderStyles;
    struct DgnTextAnnotationSeeds* m_textAnnotationSeeds;

    explicit DgnStyles(DgnDbR);
    ~DgnStyles();

public:
    //! Provides accessors for line styles.
    DGNPLATFORM_EXPORT struct DgnLineStyles& LineStyles();

    //! Provides accessors for annotation text styles.
    DGNPLATFORM_EXPORT struct DgnAnnotationTextStyles& AnnotationTextStyles();

    //! Provides accessors for annotation frame styles.
    DGNPLATFORM_EXPORT struct DgnAnnotationFrameStyles& AnnotationFrameStyles();

    //! Provides accessors for annotation leader styles.
    DGNPLATFORM_EXPORT struct DgnAnnotationLeaderStyles& AnnotationLeaderStyles();

    //! Provides accessors for text annotation seeds.
    DGNPLATFORM_EXPORT struct DgnTextAnnotationSeeds& TextAnnotationSeeds();
};

//=======================================================================================
// Links are shared resources, referenced by elements. As such, it doesn't make sense to expose an explicit API to create and delete them. Either the detach API will clean it up if it's the last use, or this will be part of a larger GC scheme for shared resources.
// @bsiclass
//=======================================================================================
struct DgnLinks : public DgnDbTable
{
private:
    DEFINE_T_SUPER(DgnDbTable);
    friend struct DgnDb;

    DgnLinks(DgnDbR db) : T_Super(db) {}

public:
    //=======================================================================================
    // @bsiclass
    //=======================================================================================
    struct Iterator : public BeSQLite::DbTableIterator
    {
    private:
        DEFINE_T_SUPER(BeSQLite::DbTableIterator);

    public:
        Iterator(DgnDbCR db) : T_Super((BeSQLite::DbCR)db) {}

        //=======================================================================================
        // @bsiclass
        //=======================================================================================
        struct Entry : public DbTableIterator::Entry, public std::iterator < std::input_iterator_tag, Entry const >
        {
        private:
            DEFINE_T_SUPER(DbTableIterator::Entry);
            friend struct Iterator;

            Entry(BeSQLite::StatementP sql, bool isValid) : T_Super(sql, isValid) {}

        public:
            DGNPLATFORM_EXPORT DgnLinkId GetId() const;
            DGNPLATFORM_EXPORT DgnLinkType GetType() const;
            DGNPLATFORM_EXPORT Utf8CP GetDisplayLabel() const;
            Entry const& operator*() const { return *this; }
        }; // Entry

        typedef Entry const_iterator;
        typedef Entry iterator;
        DGNPLATFORM_EXPORT const_iterator begin() const;
        const_iterator end() const { return Entry(nullptr, false); }
        DGNPLATFORM_EXPORT size_t QueryCount() const;
    }; // Iterator

    //=======================================================================================
    // @bsiclass
    //=======================================================================================
    struct OnElementIterator : public BeSQLite::DbTableIterator
    {
    private:
        DEFINE_T_SUPER(BeSQLite::DbTableIterator);

        DgnElementKey m_elementKey;

    public:
        OnElementIterator(DgnDbCR db, DgnElementKey elementKey) : T_Super((BeSQLite::DbCR)db), m_elementKey(elementKey) {}

        //=======================================================================================
        // @bsiclass
        //=======================================================================================
        struct Entry : DbTableIterator::Entry, std::iterator < std::input_iterator_tag, Entry const >
        {
        private:
            DEFINE_T_SUPER(DbTableIterator::Entry);
            friend struct OnElementIterator;

            Entry(BeSQLite::StatementP sql, bool isValid) : T_Super(sql, isValid) {}

        public:
            DGNPLATFORM_EXPORT DgnLinkId GetId() const;
            DGNPLATFORM_EXPORT DgnLinkType GetType() const;
            DGNPLATFORM_EXPORT Utf8CP GetDisplayLabel() const;
            Entry const& operator*() const { return *this; }

        }; // Entry

        typedef Entry const_iterator;
        typedef Entry iterator;
        DGNPLATFORM_EXPORT const_iterator begin() const;
        const_iterator end() const { return Entry(nullptr, false); }
        DGNPLATFORM_EXPORT size_t QueryCount() const;
    }; // OnElementIterator

    //=======================================================================================
    // @bsiclass
    //=======================================================================================
    struct ReferencesLinkIterator : public BeSQLite::DbTableIterator
    {
    private:
        DEFINE_T_SUPER(BeSQLite::DbTableIterator);

        DgnLinkId m_linkId;

    public:
        ReferencesLinkIterator(DgnDbCR db, DgnLinkId linkId) : T_Super((BeSQLite::DbCR)db), m_linkId(linkId) {}

        //=======================================================================================
        // @bsiclass
        //=======================================================================================
        struct Entry : DbTableIterator::Entry, std::iterator < std::input_iterator_tag, Entry const >
        {
        private:
            DEFINE_T_SUPER(DbTableIterator::Entry);
            friend struct OnElementIterator;

            Entry(BeSQLite::StatementP sql, bool isValid) : T_Super(sql, isValid) {}

        public:
            DGNPLATFORM_EXPORT DgnElementKey GetKey() const;
            Entry const& operator*() const { return *this; }

        }; // Entry

        typedef Entry const_iterator;
        typedef Entry iterator;
        DGNPLATFORM_EXPORT const_iterator begin() const;
        const_iterator end() const { return Entry(nullptr, false); }
        DGNPLATFORM_EXPORT size_t QueryCount() const;
    }; // ReferencesLinkIterator

    DGNPLATFORM_EXPORT DgnLinkPtr QueryById(DgnLinkId) const;
    Iterator MakeIterator() const { return Iterator(m_dgndb); }
    OnElementIterator MakeOnElementIterator(DgnElementKey elementKey) const { return OnElementIterator(m_dgndb, elementKey); }
    ReferencesLinkIterator MakeReferencesLinkIterator(DgnLinkId linkId) const { return ReferencesLinkIterator(m_dgndb, linkId); }
    DGNPLATFORM_EXPORT BentleyStatus Update(DgnLinkCR);

    DGNPLATFORM_EXPORT BentleyStatus InsertOnElement(DgnElementKey, DgnLinkR);
    DGNPLATFORM_EXPORT BentleyStatus InsertOnElement(DgnElementKey, DgnLinkId);
    DGNPLATFORM_EXPORT BentleyStatus DeleteFromElement(DgnElementKey, DgnLinkId);
    DGNPLATFORM_EXPORT void PurgeUnused();
};

END_BENTLEY_DGNPLATFORM_NAMESPACE
