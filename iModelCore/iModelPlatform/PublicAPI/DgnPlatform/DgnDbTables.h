/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

//-----------------------------------------------------------------------------------------
// Macros associated with the BisCore ECSchema
//-----------------------------------------------------------------------------------------
#define BIS_ECSCHEMA_NAME       "BisCore"
#define BISCORE_ECSCHEMA_PATH   L"ECSchemas/Dgn/BisCore.ecschema.xml"
#define BIS_SCHEMA(name)        BIS_ECSCHEMA_NAME "." name
#define BIS_TABLE(name)         "bis_" name
#define BE_PROP_NAME(__val__)   static constexpr Utf8CP prop_##__val__() {return #__val__;}
#define BE_JSON_PROP_NAMESPACE(__val__) static constexpr Utf8CP json_prop_namespace_##__val__() {return #__val__;}

//-----------------------------------------------------------------------------------------
// ECClass names (combine with BIS_SCHEMA macro for use in ECSql)
//-----------------------------------------------------------------------------------------
#define BIS_CLASS_AnnotationElement2d       "AnnotationElement2d"
#define BIS_CLASS_AnnotationFrameStyle      "AnnotationFrameStyle"
#define BIS_CLASS_AnnotationLeaderStyle     "AnnotationLeaderStyle"
#define BIS_CLASS_AnnotationTextStyle       "AnnotationTextStyle"
#define BIS_CLASS_AuxCoordSystem            "AuxCoordSystem"
#define BIS_CLASS_AuxCoordSystem2d          "AuxCoordSystem2d"
#define BIS_CLASS_AuxCoordSystem3d          "AuxCoordSystem3d"
#define BIS_CLASS_AuxCoordSystemSpatial     "AuxCoordSystemSpatial"
#define BIS_CLASS_Category                  "Category"
#define BIS_CLASS_CategorySelector          "CategorySelector"
#define BIS_CLASS_CodeSpec                  "CodeSpec"
#define BIS_CLASS_ColorBook                 "ColorBook"
#define BIS_CLASS_DefinitionElement         "DefinitionElement"
#define BIS_CLASS_DefinitionModel           "DefinitionModel"
#define BIS_CLASS_DefinitionPartition       "DefinitionPartition"
#define BIS_CLASS_DictionaryModel           "DictionaryModel"
#define BIS_CLASS_DisplayStyle              "DisplayStyle"
#define BIS_CLASS_DisplayStyle2d            "DisplayStyle2d"
#define BIS_CLASS_DisplayStyle3d            "DisplayStyle3d"
#define BIS_CLASS_Document                  "Document"
#define BIS_CLASS_DocumentListModel         "DocumentListModel"
#define BIS_CLASS_DocumentPartition         "DocumentPartition"
#define BIS_CLASS_Drawing                   "Drawing"
#define BIS_CLASS_DrawingCategory           "DrawingCategory"
#define BIS_CLASS_DrawingGraphic            "DrawingGraphic"
#define BIS_CLASS_DrawingModel              "DrawingModel"
#define BIS_CLASS_DriverBundleElement       "DriverBundleElement"
#define BIS_CLASS_Element                   "Element"
#define BIS_CLASS_ElementAspect             "ElementAspect"
#define BIS_CLASS_ElementMultiAspect        "ElementMultiAspect"
#define BIS_CLASS_ElementUniqueAspect       "ElementUniqueAspect"
#define BIS_CLASS_ExternalSource            "ExternalSource"
#define BIS_CLASS_ExternalSourceAspect      "ExternalSourceAspect"
#define BIS_CLASS_ExternalSourceAttachment  "ExternalSourceAttachment"
#define BIS_CLASS_ExternalSourceGroup       "ExternalSourceGroup"
#define BIS_CLASS_GeometricElement          "GeometricElement"
#define BIS_CLASS_GeometricElement2d        "GeometricElement2d"
#define BIS_CLASS_GeometricElement3d        "GeometricElement3d"
#define BIS_CLASS_GeometricModel            "GeometricModel"
#define BIS_CLASS_GeometricModel2d          "GeometricModel2d"
#define BIS_CLASS_GeometricModel3d          "GeometricModel3d"
#define BIS_CLASS_GeometryPart              "GeometryPart"
#define BIS_CLASS_GraphicalElement2d        "GraphicalElement2d"
#define BIS_CLASS_GraphicalElement3d        "GraphicalElement3d"
#define BIS_CLASS_GraphicalModel2d          "GraphicalModel2d"
#define BIS_CLASS_GraphicalModel3d          "GraphicalModel3d"
#define BIS_CLASS_GraphicalPartition3d      "GraphicalPartition3d"
#define BIS_CLASS_GraphicalType2d           "GraphicalType2d"
#define BIS_CLASS_GroupInformationElement   "GroupInformationElement"
#define BIS_CLASS_GroupInformationModel     "GroupInformationModel"
#define BIS_CLASS_GroupInformationPartition "GroupInformationPartition"
#define BIS_CLASS_InformationCarrierElement "InformationCarrierElement"
#define BIS_CLASS_InformationContentElement "InformationContentElement"
#define BIS_CLASS_InformationModel          "InformationModel"
#define BIS_CLASS_InformationPartitionElement "InformationPartitionElement"
#define BIS_CLASS_InformationRecordElement  "InformationRecordElement"
#define BIS_CLASS_InformationRecordModel    "InformationRecordModel"
#define BIS_CLASS_InformationRecordPartition "InformationRecordPartition"
#define BIS_CLASS_InformationReferenceElement "InformationReferenceElement"
#define BIS_CLASS_ISubModeledElement        "ISubModeledElement"
#define BIS_CLASS_LightLocation             "LightLocation"
#define BIS_CLASS_LineStyle                 "LineStyle"
#define BIS_CLASS_Model                     "Model"
#define BIS_CLASS_ModelSelector             "ModelSelector"
#define BIS_CLASS_PhysicalElement           "PhysicalElement"
#define BIS_CLASS_PhysicalMaterial          "PhysicalMaterial"
#define BIS_CLASS_PhysicalModel             "PhysicalModel"
#define BIS_CLASS_PhysicalPartition         "PhysicalPartition"
#define BIS_CLASS_PhysicalType              "PhysicalType"
#define BIS_CLASS_RenderMaterial            "RenderMaterial"
#define BIS_CLASS_RenderTimeline            "RenderTimeline"
#define BIS_CLASS_RepositoryModel           "RepositoryModel"
#define BIS_CLASS_RoleElement               "RoleElement"
#define BIS_CLASS_RoleModel                 "RoleModel"
#define BIS_CLASS_SectionDrawing            "SectionDrawing"
#define BIS_CLASS_SectionDrawingLocation    "SectionDrawingLocation"
#define BIS_CLASS_SectionDrawingModel       "SectionDrawingModel"
#define BIS_CLASS_SectionLocation           "SectionLocation"
#define BIS_CLASS_Sheet                     "Sheet"
#define BIS_CLASS_SheetModel                "SheetModel"
#define BIS_CLASS_SpatialCategory           "SpatialCategory"
#define BIS_CLASS_SpatialElement            "SpatialElement"
#define BIS_CLASS_SpatialIndex              "SpatialIndex"
#define BIS_CLASS_SpatialLocationElement    "SpatialLocationElement"
#define BIS_CLASS_SpatialLocationModel      "SpatialLocationModel"
#define BIS_CLASS_SpatialLocationPartition  "SpatialLocationPartition"
#define BIS_CLASS_SpatialLocationType       "SpatialLocationType"
#define BIS_CLASS_SpatialModel              "SpatialModel"
#define BIS_CLASS_SubCategory               "SubCategory"
#define BIS_CLASS_Subject                   "Subject"
#define BIS_CLASS_SynchronizationConfigLink "SynchronizationConfigLink"
#define BIS_CLASS_TemplateRecipe2d          "TemplateRecipe2d"
#define BIS_CLASS_TemplateRecipe3d          "TemplateRecipe3d"
#define BIS_CLASS_TemplateViewDefinition2d  "TemplateViewDefinition2d"
#define BIS_CLASS_TemplateViewDefinition3d  "TemplateViewDefinition3d"
#define BIS_CLASS_TextAnnotationSeed        "TextAnnotationSeed"
#define BIS_CLASS_Texture                   "Texture"
#define BIS_CLASS_ViewDefinition            "ViewDefinition"
#define BIS_CLASS_ViewDefinition2d          "ViewDefinition2d"
#define BIS_CLASS_ViewDefinition3d          "ViewDefinition3d"
#define BIS_CLASS_VolumeElement             "VolumeElement"
#define BIS_CLASS_WebMercatorModel          "WebMercatorModel"

//-----------------------------------------------------------------------------------------
// ECRelationshipClass names (combine with BIS_SCHEMA macro for use in ECSql)
//-----------------------------------------------------------------------------------------
#define BIS_REL_BaseModelForView2d                  "BaseModelForView2d"
#define BIS_REL_CategoryOwnsSubCategories           "CategoryOwnsSubCategories"
#define BIS_REL_DrawingGraphicRepresentsElement     "DrawingGraphicRepresentsElement"
#define BIS_REL_ElementDrivesElement                "ElementDrivesElement"
#define BIS_REL_ElementGroupsMembers                "ElementGroupsMembers"
#define BIS_REL_ElementIsFromSource                 "ElementIsFromSource"
#define BIS_REL_ElementOwnsChildElements            "ElementOwnsChildElements"
#define BIS_REL_ElementOwnsMultiAspects             "ElementOwnsMultiAspects"
#define BIS_REL_ElementOwnsUniqueAspect             "ElementOwnsUniqueAspect"
#define BIS_REL_ElementRefersToElements             "ElementRefersToElements"
#define BIS_REL_ExternalSourceAttachmentAttachesSource "ExternalSourceAttachmentAttachesSource"
#define BIS_REL_ExternalSourceGroupGroupsSources    "ExternalSourceGroupGroupsSources"
#define BIS_REL_ExternalSourceIsInRepository        "ExternalSourceIsInRepository"
#define BIS_REL_ExternalSourceOwnsAttachments       "ExternalSourceOwnsAttachments"
#define BIS_REL_GraphicalElement2dIsOfType          "GraphicalElement2dIsOfType"
#define BIS_REL_GraphicalType2dHasTemplateRecipe    "GraphicalType2dHasTemplateRecipe"
#define BIS_REL_ModelContainsElements               "ModelContainsElements"
#define BIS_REL_ModelModelsElement                  "ModelModelsElement"
#define BIS_REL_ModelSelectorRefersToModels         "ModelSelectorRefersToModels"
#define BIS_REL_PartitionOriginatesFromRepository   "PartitionOriginatesFromRepository"
#define BIS_REL_PhysicalElementAssemblesElements    "PhysicalElementAssemblesElements"
#define BIS_REL_PhysicalElementIsOfType             "PhysicalElementIsOfType"
#define BIS_REL_PhysicalTypeHasTemplateRecipe       "PhysicalTypeHasTemplateRecipe"
#define BIS_REL_RenderMaterialOwnsRenderMaterials   "RenderMaterialOwnsRenderMaterials"
#define BIS_REL_SubjectOwnsSubjects                 "SubjectOwnsSubjects"
#define BIS_REL_SubjectOwnsPartitionElements        "SubjectOwnsPartitionElements"
#define BIS_REL_SynchronizationConfigSpecifiesRootSources "SynchronizationConfigSpecifiesRootSources"
#define BIS_REL_SynchronizationConfigProcessesSources "SynchronizationConfigProcessesSources"

//-----------------------------------------------------------------------------------------
// ExternalSourceAspect Kinds
//-----------------------------------------------------------------------------------------
#define BIS_EXTERNAL_SOURCE_ASPECT_KIND_ExternalSource  "ExternalSource"
#define BIS_EXTERNAL_SOURCE_ASPECT_KIND_Model           "Model"

//-----------------------------------------------------------------------------------------
// DgnDb table names
//-----------------------------------------------------------------------------------------
#define DGN_TABLE_Domain                    "dgn_Domain"
#define DGN_TABLE_Font                      "dgn_Font"
#define DGN_TABLE_Handler                   "dgn_Handler"
#define DGN_TABLE_Txns                      "dgn_Txns"
#define DGN_TABLE_Rebase                    "dgn_Rebase"
#define DGN_VTABLE_SpatialIndex             "dgn_" BIS_CLASS_SpatialIndex

#include <cstddef>
#include <DgnPlatform/DgnProperties.h>
#include "UnitDefinition.h"
#include "DbFont.h"
#include "ECSqlClassParams.h"
#include "ECSqlStatementIterator.h"
#include <Bentley/HeapZone.h>
#include <GeoCoord/BaseGeoCoord.h>

BEGIN_BENTLEY_DGN_NAMESPACE

namespace dgn_ElementHandler {struct Physical;};
namespace dgn_TxnTable {struct Element; struct Model;};

struct DgnImportContext;
struct ModelIterator;

//=======================================================================================
//! The "value" portion of a DgnCode uses sqlite's COLLATE NOCASE collation for
//! comparison operations. This is "case-insensitive" over the ASCII character set only.
//! In other words, to compare two code values:
//!  1. The characters [A-Z] are folded to their lower-case equivalents [a-z]
//!  2. The bytes of the two resultant strings are compared directly.
//! Two code values are equal if and only if the resultant folded strings contain exactly
//! the same bytes. Therefore all non-ascii characters are treated case-sensitively.
//! @note It is invalid for a DgnCodeValue to have leading or trailing whitespace, so the value will be *trimmed* automatically.
// @bsistruct
//=======================================================================================
struct DgnCodeValue
{
    enum class CompareResult { Less, Equal, Greater };
private:
    Utf8String  m_value; //!< Note: can be "empty" (persisted as null)

    static Utf8Char Fold(Utf8Char ch);
    static CompareResult Compare(DgnCodeValueCR lhs, DgnCodeValueCR rhs) { return Compare(lhs.GetUtf8(), rhs.GetUtf8()); }
    DGNPLATFORM_EXPORT static CompareResult Compare(Utf8StringCR lhs, Utf8StringCR rhs);
    DGNPLATFORM_EXPORT static CompareResult Compare(Utf8Char lhs, Utf8Char rhs);
public:
    //! Create an empty code value
    DgnCodeValue() { }
    //! Create a code value from a Utf8String
    DgnCodeValue(Utf8StringCR str) : m_value(str) { m_value.Trim(); }
    //! Create a code value from a pointer to a UTF-8 string
    DgnCodeValue(Utf8CP str) : m_value(str) { m_value.Trim(); }

    //! Get the value as a Utf8String
    Utf8StringCR GetUtf8() const { return m_value; }
    //! Get the value as a pointer to a UTF-8 string, or nullptr if the string is empty
    Utf8CP GetUtf8CP() const { return empty() ? nullptr : GetUtf8().c_str(); }

    //! The number of bytes in this value's string
    size_t size() const { return GetUtf8().size(); }
    //! Return whether this value is an empty string
    bool empty() const { return GetUtf8().empty(); }
    //! Set this value to an empty string
    void clear() { m_value.clear(); }

    //! Compare this value to another value
    CompareResult CompareTo(DgnCodeValueCR rhs) const { return Compare(*this, rhs); }
    //! Compare this value to a pointer to a UTF-8 string
    DGNPLATFORM_EXPORT bool Equals(Utf8CP str) const;

    //! Compare for equality using COLLATE NOCASE rules
    bool operator==(DgnCodeValueCR rhs) const { return size() == rhs.size() && CompareResult::Equal == CompareTo(rhs); }
    //! Compare for inequality using COLLATE NOCASE rules
    bool operator!=(DgnCodeValueCR rhs) const { return !(*this == rhs); }
    //! Compare for less-than using COLLATE NOCASE rules
    bool operator<(DgnCodeValueCR rhs) const { return CompareResult::Less == CompareTo(rhs); }
    //! Compare for greater-than using COLLATE NOCASE rules
    bool operator>(DgnCodeValueCR rhs) const { return CompareResult::Greater == CompareTo(rhs); }
    //! Compare for less-or-equal using COLLATE NOCASE rules
    bool operator<=(DgnCodeValueCR rhs) const { return !(*this > rhs); }
    //! Compare for greater-or-equal using COLLATE NOCASE rules
    bool operator>=(DgnCodeValueCR rhs) const { return !(*this < rhs); }
};

//=======================================================================================
//! A DgnCode is a structure that holds the "name" of an element in a DgnDb.
//! The DgnCode is stored as a three-part identifier: CodeSpecId, scope, and value.
//! The combination of the three must be unique across all elements within a DgnDb.
//! The meaning of a DgnCode is determined by the CodeSpec.
//!
//! The CodeSpecId must be non-null and identify a valid CodeSpec.
//! The scope must not be null and identifies the uniqueness scope for the value.
//! The value may be null if the element does not have a name. The value may not be an empty string.
//!
//! @see CodeSpec
// @bsiclass
//=======================================================================================
struct DgnCode {
private:
    CodeSpecId m_specId;  //!< @see CodeSpec
    Utf8String m_scope;   //!< Note: stored as a string, but must be a valid/serialized ElementId
    DgnCodeValue m_value; //!< Note: can be "empty" (persisted as null)

public:
    //! Constructs an invalid DgnCode
    DgnCode() {}

    //! Construct a DgnCode scoped to an existing element.
    DgnCode(CodeSpecId specId, DgnElementId scopeElementId, Utf8StringCR value) : m_specId(specId), m_scope(scopeElementId.ToHexStr()), m_value(value) {}

    //! Invalidate this DgnCode
    void Invalidate() {
        m_specId.Invalidate();
        m_scope.clear();
        m_value.clear();
    }

    //! Determine whether this DgnCode is valid.
    bool IsValid() const { return m_specId.IsValid(); }
    //! Determine if this code is valid but not otherwise meaningful (and therefore not necessarily unique)
    bool IsEmpty() const { return m_specId.IsValid() && m_value.empty(); }
    //! Determine if two DgnCodes are equivalent
    bool operator==(DgnCodeCR other) const { return m_specId == other.m_specId && m_value == other.m_value && m_scope == other.m_scope; }
    //! Determine if two DgnCodes are not equivalent
    bool operator!=(DgnCodeCR other) const { return !(*this == other); }
    //! Perform ordered comparison, e.g. for inclusion in associative containers
    DGNPLATFORM_EXPORT bool operator<(DgnCodeCR rhs) const;


    //! Get the value for this DgnCode
    DgnCodeValueCR GetValue() const { return m_value; }
    //! Get the value for this DgnCode as a Utf8String
    Utf8StringCR GetValueUtf8() const { return m_value.GetUtf8(); }
    //! Get the value for this DgnCode as a Utf8CP, or nullptr if the value is empty.
    Utf8CP GetValueUtf8CP() const { return m_value.GetUtf8CP(); }

    //! Get the DgnElementId of the element providing the uniqueness scope for the code value.
    DGNPLATFORM_EXPORT DgnElementId GetScopeElementId(DgnDbR db) const;
    //! Return the scope serialized to a string whose format is dependent on ScopeRequirement
    Utf8StringCR GetScopeString() const { return m_scope; }

    //! Get the CodeSpecId of the CodeSpec that issued this DgnCode.
    CodeSpecId GetCodeSpecId() const { return m_specId; }

    DGNPLATFORM_EXPORT void RelocateToDestinationDb(DgnImportContext&);

    //! Create an empty, non-unique code with no special meaning.
    DGNPLATFORM_EXPORT static DgnCode CreateEmpty();

    BE_JSON_NAME(spec)
    BE_JSON_NAME(scope)
    BE_JSON_NAME(value)
    DGNPLATFORM_EXPORT void ToJson(BeJsValue) const;
    DGNPLATFORM_EXPORT static DgnCode FromJson(BeJsConst value, DgnDbCR);
};

typedef bset<DgnCode> DgnCodeSet;

//=======================================================================================
//! A base class for api's that access a table in a DgnDb
//=======================================================================================
struct DgnDbTable : NonCopyableClass {
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
};

//=======================================================================================
//! Each DgnModel has an entry in the DgnModels table
//! @see DgnDb::Models
//! @ingroup GROUP_DgnModel
//=======================================================================================
struct DgnModels : DgnDbTable {
    typedef bmap<DgnModelId, DgnModelPtr> T_DgnModelMap;
private:
    friend struct DgnDb;
    friend struct DgnModel;
    friend struct dgn_TxnTable::Model;
    typedef bmap<DgnClassId, ECSqlClassInfo> T_ClassInfoMap;

    mutable BeMutex m_mutex;
    T_DgnModelMap m_models;
    T_ClassInfoMap m_classInfos;

    DgnModelPtr LoadDgnModel(DgnModelId modelId);
    void Empty();
    void AddLoadedModel(DgnModelR);
    void DropLoadedModel(DgnModelR);

    ECSqlClassInfo const& FindClassInfo(DgnModelR model);
    DGNPLATFORM_EXPORT BeSQLite::EC::CachedECSqlStatementPtr GetSelectStmt(DgnModelR model); // used by imodeljs node addon
    BeSQLite::EC::CachedECSqlStatementPtr GetInsertStmt(DgnModelR model);
    BeSQLite::EC::CachedECSqlStatementPtr GetUpdateStmt(DgnModelR model);

    DgnModels(DgnDbR db) : DgnDbTable(db) {}
    ~DgnModels() {}

public:
    void ClearCache() { m_models.clear(); }
    void ClearECCaches() {
        BeMutexHolder _v_v(m_mutex);
        m_classInfos.clear();
    }

    //! Create a new, non-persistent model from the supplied ECInstance.
    //! The supplied instance must contain the model's Code.
    //! @param stat     Optional. If not null, an error status is returned here if the model cannot be created.
    //! @param properties The instance that contains all of the model's business properties
    //! @return a new, non-persistent model if successful, or an invalid ptr if not.
    //! @note The returned model, if any, is non-persistent. The caller must call the model's Insert method to add it to the bim.
    DGNPLATFORM_EXPORT DgnModelPtr CreateModel(DgnDbStatus* stat, ECN::IECInstanceCR properties);

    //! Get a DgnModel from this DgnDb by its DgnModelId.
    //! @param[in] modelId The DgnModelId of the model
    //! @remarks The model is loaded from the database if necessary. If the model is already loaded, a pointer to the existing DgnModel is returned.
    //! @return Invalid if the model does not exist.
    DGNPLATFORM_EXPORT DgnModelPtr GetModel(DgnModelId modelId);

    template <class T>
    RefCountedPtr<T> Get(DgnModelId id) { return dynamic_cast<T*>(GetModel(id).get()); }

    //! Query if the specified model has already been loaded.
    //! @return a pointer to the model if found, or nullptr if the model has not been loaded.
    //! @see GetLoadedModels
    //! @see LoadModelById
    DGNPLATFORM_EXPORT DgnModelPtr FindModel(DgnModelId modelId);

    //! Get the currently loaded DgnModels for this DgnDb. Not thread-safe!
    T_DgnModelMap const& GetLoadedModels() const { return m_models; }

    //! Access the currently loaded DgnModels in the thread-safe manner.
    template<typename Func> auto WithLoadedModels(Func func)
        {
        BeMutexHolder lock(m_mutex);
        return func(m_models);
        }

    //! Query for a DgnModelId by the DgnCode of the element that is being modeled
    DGNPLATFORM_EXPORT DgnModelId QuerySubModelId(DgnCodeCR modeledElementCode) const;

    //! Make an iterator over models of the specified ECClass in this DgnDb.
    //! @param[in] className The <i>full</i> ECClass name.  For example: BIS_SCHEMA(BIS_CLASS_PhysicalModel)
    //! @param[in] whereClause The optional where clause starting with WHERE
    //! @param[in] orderByClause The optional order by clause starting with ORDER BY
    DGNPLATFORM_EXPORT ModelIterator MakeIterator(Utf8CP className, Utf8CP whereClause = nullptr, Utf8CP orderByClause = nullptr) const;

    //! Get a string containing the list of characters that may NOT appear in model names.
    constexpr static Utf8CP GetIllegalCharacters() { return "\\/:*?<>|\"\t\n,=&"; }

    //! Determine whether the supplied name is a valid model name.
    //! @param[in] name The candidate model name to check
    //! @return true if the model name is valid, false otherwise.
    //! @note Model names may also not start or end with a space.
    static bool IsValidName(Utf8StringCR name) { return DgnDbTable::IsValidName(name, GetIllegalCharacters()); }
};

/**
 * The map of FontId to DbFontP for a DgnDb
 * Also holds a FontDb for all the embedded fonts loaded from this DgnDb.
 */
struct DgnDbFonts : NonCopyableClass {
    struct FontInfo {
        FontType m_type;
        Utf8String m_fontName;
        DbFontP m_font;
        FontInfo(FontType type, Utf8CP name, DbFontP font) : m_type(type), m_fontName(name), m_font(font) {}
    };
    typedef bmap<FontId, FontInfo> T_FontIdMap;

    FontDb m_fontDb;
    mutable T_FontIdMap m_fontMap;
    mutable bool m_isFontMapLoaded = false;

private:
    DGNPLATFORM_EXPORT void Load() const;
    DbFontR ResolveFont(FontType fontType, Utf8CP name, Utf8StringCR metadata) const;

public:
    DgnDbFonts(DgnDbR db);
    T_FontIdMap const& FontMap() {
        Load();
        return m_fontMap;
    }
    void Invalidate() {
        m_isFontMapLoaded = false;
        m_fontMap.clear();
    }
    DGNPLATFORM_EXPORT FontId FindId(FontType, Utf8CP name) const;
    DGNPLATFORM_EXPORT FontId InsertFont(FontType, Utf8CP name);
    DGNPLATFORM_EXPORT FontId GetId(FontType, Utf8CP name);
    DGNPLATFORM_EXPORT FontInfo const* GetFontInfo(FontId) const;

    Utf8CP GetFontName(FontId id) const {
        auto info = GetFontInfo(id);
        return info ? info->m_fontName.c_str() : nullptr;
    }
    FontType GetFontType(FontId id) const {
        auto info = GetFontInfo(id);
        return info ? info->m_type : FontType::TrueType;
    }
    /** Get the DbFont for a FontId. If fontId is not valid, returns the last-resort font. */
    DbFontR FindFont(FontId id) const {
        auto info = GetFontInfo(id);
        return info ? *info->m_font : FontManager::GetFallbackFont();
    }
};

//=======================================================================================
//! A DgnElement within a DgnDb can be identified by a DgnCode which is unique among all
//! elements in the DgnDb. The meaning of the code is determined by the CodeSpec by which
//! the code was assigned. Therefore the DgnCode includes the Id of the CodeSpec.
//! DgnCodeSpecs holds all such CodeSpecs associated with a DgnDb. The name of a CodeSpec
//! must be unique.
//! @see DgnDb::CodeSpecs
//=======================================================================================
struct DgnCodeSpecs : DgnDbTable
{
private:
    friend struct DgnDb;
    explicit DgnCodeSpecs(DgnDbR db) : DgnDbTable(db) {}

    typedef bvector<CodeSpecPtr> LoadedCodeSpecs;

    LoadedCodeSpecs m_loadedCodeSpecs;
    BeSQLite::BeDbMutex m_mutex;

    CodeSpecPtr LoadCodeSpec(CodeSpecId codeSpecId, DgnDbStatus* status = nullptr);

public:
    //! Look up the Id of the CodeSpec with the specified name.
    DGNPLATFORM_EXPORT CodeSpecId QueryCodeSpecId(Utf8CP name) const;

    //! Look up an CodeSpec by Id. The CodeSpec will be loaded from the database if necessary.
    //! @param[in] codeSpecId The Id of the CodeSpec to load
    //! @returns The CodeSpec with the specified Id, or nullptr if the CodeSpec could not be loaded
    DGNPLATFORM_EXPORT CodeSpecCPtr GetCodeSpec(CodeSpecId codeSpecId);

    //! Look up an CodeSpec by name. The CodeSpec will be loaded from the database if necessary.
    //! @param[in] name The name of the CodeSpec to load
    //! @returns The CodeSpec with the specified name, or nullptr if the CodeSpec could not be loaded
    DGNPLATFORM_EXPORT CodeSpecCPtr GetCodeSpec(Utf8CP name);

    //! Add a new CodeSpec to the table.
    //! @param[in]  codeSpec The new entry to add.
    //! @return The result of the insert operation.
    //! @remarks If successful, this method will assign a valid CodeSpecId to the supplied CodeSpec
    DGNPLATFORM_EXPORT DgnDbStatus Insert(CodeSpecR codeSpec);
};

//=======================================================================================
/// The location/orientation of an iModel on the earth via [ECEF](https://en.wikipedia.org/wiki/ECEF) (Earth Centered Earth Fixed) coordinates
// @bsiclass
//=======================================================================================
struct EcefLocation
{
    BE_JSON_NAME(origin);
    BE_JSON_NAME(orientation);
    BE_JSON_NAME(cartographicOrigin);
    BE_JSON_NAME(xVector);
    BE_JSON_NAME(yVector);

    bool m_isValid;
    DPoint3d m_origin;
    YawPitchRollAngles m_angles;
    bool m_haveCartographicOrigin;
    GeoPoint m_cartographicOrigin;
    bool m_haveVectors;
    DVec3d m_xVector, m_yVector;
    DGNPLATFORM_EXPORT void ToJson(BeJsValue) const; //!< Convert to json
    DGNPLATFORM_EXPORT void FromJson(BeJsConst); //!< load from json
    EcefLocation() {m_isValid = false; m_haveCartographicOrigin = false; }
    EcefLocation(DPoint3dCR origin, YawPitchRollAnglesCR angles, GeoPointCP cartographicOrigin = nullptr, DVec3dCP xVector = nullptr, DVec3dCP yVector = nullptr) : m_origin(origin), m_angles(angles)
    {
    m_isValid = true;
    if (m_haveCartographicOrigin = (nullptr != cartographicOrigin))
        m_cartographicOrigin = *cartographicOrigin;

    if (m_haveVectors = (nullptr != xVector && nullptr != yVector))
        {
        m_xVector = *xVector;
        m_yVector = *yVector;
        }
    }
    // Constructor to compute ECEF location from an cartographic origin
    DGNPLATFORM_EXPORT EcefLocation(GeoPointCR originLL, DPoint3dCR originIn, GeoCoordinates::BaseGCSCP GCS);
};

//! Source of DgnGeoLocation.GetProjectExtents
enum class ProjectExtentsSource : int
{
    Calculated = 0, //!< Project extents set by querying the volume of all spatial models (default)
    User = 1,       //!< Project extents established from user input
};

//=======================================================================================
// @bsiclass
//=======================================================================================
struct DgnGeoLocation : NonCopyableClass
{
    BE_JSON_NAME(globalOrigin);
    BE_JSON_NAME(ecefLocation);
    BE_JSON_NAME(initialProjectCenter);
    BE_JSON_NAME(extentsSource);

private:
    friend struct DgnDb;
    DgnDbR  m_dgndb;
    mutable AxisAlignedBox3d m_extent;
    mutable ProjectExtentsSource m_extentSource = ProjectExtentsSource::Calculated;
    DPoint3d m_globalOrigin;
    DPoint3d m_initialProjectCenter;
    mutable EcefLocation m_ecefLocation;
    mutable bool m_hasCheckedForGCS = false;

    // If m_GCSHasBeenSet is false then m_gcs points to the internal DgnGCS held by DgnAppData.
    // If true then it is not held and has been set but not stored yet.
    mutable bool m_GCSHasBeenSet = false;
    mutable DgnGCS* m_gcs = nullptr;

    mutable IGeoCoordinateServicesP m_geoServices = nullptr;
    mutable GeoCoordinates::BaseGCSPtr m_wgs84GCS;

    DgnGeoLocation(DgnDbR db);
    ~DgnGeoLocation();
    void LoadProjectExtents() const;
    GeoCoordinates::BaseGCS* GetWGS84GCS() const;

public:
    //! @private
    //! Called only internally, on the rare occasions when a new GCS has been stored to the DgnDb.
    void NewGCSStored () {m_hasCheckedForGCS = false;}

    //! @private
    //! Return a reasonable value that can be used if the project extents have never been established for this BIM.
    //! This is an arbitrary volume intended to represent a cube of a reasonable size, if we have no idea what is being modeled by
    //! this BIM. The default is (in meters) : [{-50,-50,-10},{50,50,30}]
    AxisAlignedBox3d GetDefaultProjectExtents() const {return AxisAlignedBox3d(DRange3d::From(-50,-50,-10, 50,50,30));}

    //! @private
    //! Initialize the project extents by querying the volume of all the spatial models. Should only be used after conversion from external formats.
    DGNPLATFORM_EXPORT void InitializeProjectExtents(DRange3dP rangeWithOutliers = nullptr, bvector<BeInt64Id>* elementOutliers = nullptr);

    //! @private
    //! Compute the project extents by querying the volume of all the spatial models. Should only be used after conversion from external formats.
    //! ###TODO: Remove once ScalableMesh folks fix their _QueryModelRange() to produce valid result during conversion from V8
    DGNPLATFORM_EXPORT AxisAlignedBox3d ComputeProjectExtents(DRange3dP rangeWithOutliers = nullptr, bvector<BeInt64Id>* elementOutliers = nullptr) const;

    //! @private
    //! Update the global origin. @note Changing global origin invalidates all existing xyz coordinates stored in the BIM.
    DGNPLATFORM_EXPORT void SetGlobalOrigin(DPoint3dCR origin);

    DGNPLATFORM_EXPORT void Save();
    DGNPLATFORM_EXPORT DgnDbStatus Load();

    DgnDbR GetDgnDb() const {return m_dgndb;}

    //! Get the iModel's global origin. All spatial coordinates in the BIM are stored relative to its global origin.
    DPoint3dCR GetGlobalOrigin() const {return m_globalOrigin;}

    //! Get the source for GetProjectExtents.
    ProjectExtentsSource GetProjectExtentsSource() const {return m_extentSource;}

    //! Set the source for GetProjectExtents.
    void SetProjectExtentsSource(ProjectExtentsSource source) const {m_extentSource = source;}

    //! Update the project extents for this BIM
    DGNPLATFORM_EXPORT void SetProjectExtents(AxisAlignedBox3dCR newExtents);

    //! Reset the in-memory project extents to a null range. Strictly for tests.
    void ResetProjectExtents() {
      m_extent.low.x = 100;
      m_extent.high.x = -100;
      BeAssert(m_extent.IsEmpty());
    }

    //! Get the "spatial area of interest" for this project. This volume is used for many purposes where the software needs to
    //! include space "in the project", and exclude space outside it. For example, viewing volumes are automatically adjusted to set
    //! the depth (front and back planes) to this volume. Also, there are times when tools adjust the volume of searches or view operations
    //! to include only space inside the project extents.
    //! There are tools to visualize and adjust the project extents. Note that if the project extent is too small (does not
    //! include space occupied by elements or other artifacts of interest), they may disappear under some operations since they're not considered
    //! "of interest". Likewise, if this volume is too large, some operations may work poorly due to the large volume of "wasted space".
    DGNPLATFORM_EXPORT AxisAlignedBox3d GetProjectExtents() const;

    //! Get the EcefLocation for this iModel. May not be valid if iModel is not geolocated.
    DGNPLATFORM_EXPORT EcefLocation GetEcefLocation() const;

    //! Return ecef transform at a point.
    DGNPLATFORM_EXPORT StatusInt GetEcefTransformAtPoint(TransformR ecefTrans, DPoint3dCR  originIn) const;

    //! Get the intial center of the project.  This is set only when the project is created and is not
    //! updated when the project extents are altered.
    DPoint3dCR GetInitialProjectCenter() const { return m_initialProjectCenter; }

    //! Set the EcefLocation for this iModel.
    void SetEcefLocation(EcefLocation const& location) const {m_ecefLocation = location;}

    //! Convert a GeoPoint to an XYZ point
    //! @param[out] outXyz The output XYZ point
    //! @param[in] inLatLong The input GeoPoint
    //! @return non-zero error status if the point cannot be converted or if this DgnDb is not geo-located
    DGNPLATFORM_EXPORT BentleyStatus XyzFromLatLong(DPoint3dR outXyz, GeoPointCR inLatLong) const;

    //! Convert a GeoPoint in the WGS84 Datum to an XYZ point
    //! @param[out] outXyz The output XYZ point
    //! @param[in] inLatLong The input GeoPoint in the WGS84 datum.
    //! @return non-zero error status if the point cannot be converted or if this DgnDb is not geo-located
    DGNPLATFORM_EXPORT BentleyStatus XyzFromLatLongWGS84(DPoint3dR outXyz, GeoPointCR inLatLong) const;

    //! Convert a an XYZ point to a GeoPoint
    //! @param[out] outLatLong  The output GeoPoint
    //! @param[in] inXyz The input XYZ point
    //! @return non-zero error status if the point cannot be converted or if this DgnDb is not geo-located
    DGNPLATFORM_EXPORT BentleyStatus LatLongFromXyz(GeoPointR outLatLong, DPoint3dCR inXyz) const;

    //! Query the GCS of this DgnDb, if any.
    //! @return this DgnDb's GCS or nullptr if this DgnDb is not geo-located
    DGNPLATFORM_EXPORT DgnGCS* GetDgnGCS() const;

    //! Set the GCS for this iModel.
    //! @param[in] newGCS The new GCS. To indicate there is no GCS nullptr can be provided.
    DGNPLATFORM_EXPORT void SetGCS(GeoCoordinates::BaseGCSCP newGCS);
};

//=======================================================================================
// @bsiclass
//=======================================================================================
struct DgnUnits
{
    static double constexpr OneMeter() {return 1.0;}
    static double constexpr OneKilometer() {return 1000.0 * OneMeter();}
    static double constexpr OneMillimeter() {return OneMeter() / 1000.0;}
    static double constexpr OneCentimeter() {return OneMeter() / 100.0;}
    static double constexpr DiameterOfEarth() {return 12742.0 * OneKilometer();} // approximate
    static double constexpr CircumferenceOfEarth() {return 40075.0 * OneKilometer();} // approximate
};

//=======================================================================================
//! Every DgnDb has a table for storing searchable text for use with SQLite's
//! FTS5 full text search features. Each search term is qualified by a "text type" and associated
//! with an Id into some other table from which the search term originated. The meaning of the
//! Id field can be interpreted according to the text type. Applications can
//! populate this table with search terms to enable efficient, database-wide full text search.
//! Such queries may be optionally constrained to one or more text types.
//! In general, an application should limit its queries to those text types which it either
//! created or understands the meaning of.
//! @see DgnDb::SearchableText
// @bsiclass
//=======================================================================================
struct DgnSearchableText : DgnDbTable
{
private:
    friend struct DgnDb;

    DgnSearchableText(DgnDbR db) : DgnDbTable(db) {}

    static BeSQLite::DbResult CreateTable(DgnDb& db);
public:
    //! Identifies a record in the searchable text table
    struct Key
    {
    private:
        Utf8String m_type;
        BeInt64Id m_id;
    public:
        //! Constructor.
        //! @param[in]      textType Specifies the type of text. May not be empty.
        //! @param[in]      id       The Id of the associated object. Must be valid.
        Key(Utf8StringCR textType, BeInt64Id id) : m_type(textType), m_id(id) {m_type.Trim();}

        //! Default constructor producing an invalid Key.
        Key() {}

        Utf8StringCR GetTextType() const {return m_type;} //!< The search text type
        BeInt64Id GetId() const {return m_id;} //!< The Id of the object associated with this record
        bool IsValid() const {return !m_type.empty() && m_id.IsValid();} //!< Determine whether this is a valid Key
   };

    //! A record in the searchable text table
    struct Record
    {
    private:
        Key m_key;
        Utf8String  m_text;
    public:
        //! Constructor
        //! @param[in]      textType Identifies both the meaning of the Id and the "type" of the text. May not be empty.
        //! @param[in]      id       The Id of the object associated with this text. Must be valid.
        //! @param[in]      text     The searchable text. May not be empty.
        //! @remarks The combination of text type and Id must be unique within the searchable text table
        Record(Utf8StringCR textType, BeInt64Id id, Utf8StringCR text) : Record(Key(textType, id), text) {}

        //! Constructor
        //! @param[in]      key  Uniquely identifies this record within the table. Must be valid.
        //! @param[in]      text The searchable text. May not be empty.
        Record(Key const& key, Utf8StringCR text) : m_key(key), m_text(text) {m_text.Trim();}

        //! Default constructor. Produces an invalid record.
        Record() {}

        Utf8StringCR GetTextType() const {return m_key.GetTextType();} //!< The search text type
        BeInt64Id GetId() const {return m_key.GetId();} //!< The Id of the object associated with the text
        Utf8StringCR GetText() const {return m_text;} //!< The searchable text
        Key const& GetKey() const {return m_key;} //!< The record key
        bool IsValid() const {return m_key.IsValid() && !m_text.empty();} //!< Determine if this is a valid record
   };

    //! A list of text types by which to filter full-text search queries
    typedef bvector<Utf8String> TextTypes;

    //! Specifies a query on the searchable text table, optionally filtered by one or more text types.
    struct Query
    {
    private:
        TextTypes  m_types;
        Utf8String m_matchExpression;

        friend struct DgnSearchableText;
        Utf8String ToWhereClause() const;
    public:
        //! Constructor
        //! @param[in]      matchExpression An expression conforming to sqlite's MATCH syntax indicating the text for which to search
        //! @param[in]      textType        If supplied, only text of the specified type will be included in the query
        //! @remarks The matchExpression will be single-quoted and concatenated with a query to produce a where clause like WHERE searchable_text MATCH 'matchExpression'.
        //! @remarks The caller is responsible for ensuring that search phrases within the expression are properly double-quoted and that the expression conforms to sqlite's MATCH syntax.
        DGNPLATFORM_EXPORT explicit Query(Utf8StringCR matchExpression, Utf8CP textType=nullptr);

        //! Add a text type by which to filter. The query will only include text belonging to the specified types.
        //! @param[in]      textType The type by which to filter.
        DGNPLATFORM_EXPORT void AddTextType(Utf8CP textType);

        //! Specify columns to be include in select statement
        //! The columns are returned by the select statement in the order Type,Id,Text
        enum class Column : uint8_t
        {
            None = 0,
            Type = 1 << 0,
            Id = 1 << 1,
            Text = 1 << 2,
            IdAndText = Id | Text,
            TextAndType = Text | Type,
            All = Id | Text | Type,
            Count = 0xff,
        };

        //! Converts this Query specification into a SELECT statement
        //! @param[in]      includedColumns Specifies the columns to be selected
        //! @return A properly formatted and quoted SELECT statement
        DGNPLATFORM_EXPORT Utf8String ToSelectStatement(Column includedColumns=Column::Id) const;
    };

    //! An iterator over the results of a full text search query
    struct Iterator : BeSQLite::DbTableIterator
    {
    private:
        friend struct DgnSearchableText;

        Iterator(DgnDb& db, Query const& query) : DbTableIterator((BeSQLite::DbCR)db), m_query(query) {}

        Query m_query;
    public:
        //! An entry in a full text search results iterator
        struct Entry : DbTableIterator::Entry
        {
            using iterator_category=std::input_iterator_tag;
            using value_type=Entry const;
            using difference_type=std::ptrdiff_t;
            using pointer=Entry const*;
            using reference=Entry const&;

        private:
            friend struct Iterator;
            Entry(BeSQLite::StatementP sql, bool isValid) : DbTableIterator::Entry(sql, isValid) {}
        public:
            DGNPLATFORM_EXPORT Utf8CP GetTextType() const; //!< The type of text
            DGNPLATFORM_EXPORT BeInt64Id GetId() const; //!< The Id of the associated object
            DGNPLATFORM_EXPORT Utf8CP GetText() const; //!< The search text

            Key GetKey() const {return Key(GetTextType(), GetId());} //!< The unique Key identifying this entry
            Record GetRecord() const {return Record(GetTextType(), GetId(), GetText());} //!< A record representing this entry
            Entry const& operator*() const {return *this;} //!< Dereference this entry
        };

        typedef Entry const_iterator;
        typedef Entry iterator;
        DGNPLATFORM_EXPORT Entry begin() const; //!< An iterator to the first entry in the results
        Entry end() const {return Entry(nullptr, false);} //!< An iterator beyond the last entry in the results
    };

    //! Query the number of records which match a full text search query
    //! @param[in] query The searchable text query
    //! @return The number of records which match the query.
    DGNPLATFORM_EXPORT size_t QueryCount(Query const& query) const;

    //! Query the records which match a full text search query
    //! @param[in] query The searchable text query
    //! @return An iterator over the matching records.
    DGNPLATFORM_EXPORT Iterator QueryRecords(Query const& query) const;

    //! Query the record with the specified text type and Id.
    //! @param[in] key The unique key identifying the record
    //! @return The corresponding record, or an invalid record if no such record exists.
    DGNPLATFORM_EXPORT Record QueryRecord(Key const& key) const;

    //! Query the types of text present in the searchable text table
    //! @return The list of available text types
    DGNPLATFORM_EXPORT TextTypes QueryTextTypes() const;

    //! Insert a new record into the searchable text table
    //! @param[in] record The record to insert
    //! @return Success if the new record was inserted, or else an error code.
    DGNPLATFORM_EXPORT BeSQLite::DbResult Insert(Record const& record);

    //! Insert a new record into the searchable text table
    //! @param[in] textType Identifies both the meaning of the Id and the "type" of the text. May not be empty.
    //! @param[in] id The Id of the object associated with this text. Must be valid.
    //! @param[in] text The searchable text. May not be empty.
    //! @return Success if the new record was inserted, or else an error code.
    //! @remarks The combination of text type and Id must be unique within the searchable text table
    DGNPLATFORM_EXPORT BeSQLite::DbResult InsertRecord(Utf8CP textType, BeInt64Id id, Utf8CP text);

    //! Update an existing record in the searchable text table
    //! @param[in] record The modified record
    //! @param[in] originalKey If non-null, identifies the existing record.
    //! @return Success if the record was updated, or else an error code.
    //! @remarks If originalKey is not supplied, the key is assumed to remain unchanged. Otherwise, the record will be looked up by original key, allowing the text type and/or Id to be updated.
    DGNPLATFORM_EXPORT BeSQLite::DbResult Update(Record const& record, Key const* originalKey=nullptr);

    //! Removes all data from the searchable text table
    //! @return Success if the table was cleared, or an error code.
    DGNPLATFORM_EXPORT BeSQLite::DbResult DropAll();

    //! Drops all records of the specified text type
    //! @param[in] textType The text type to drop
    //! @return Success if the associated records were dropped, or an error code.
    DGNPLATFORM_EXPORT BeSQLite::DbResult DropTextType(Utf8CP textType);

    //! Drop a single record from the searchable text table
    //! @param[in] key The key identifying the record to drop.
    //! @return Success if the record was dropped, or an error code.
    DGNPLATFORM_EXPORT BeSQLite::DbResult DropRecord(Key const& key);

    static bool IsUntrackedFts5Table(Utf8CP tableName);
};

ENUM_IS_FLAGS(DgnSearchableText::Query::Column);

END_BENTLEY_DGN_NAMESPACE
