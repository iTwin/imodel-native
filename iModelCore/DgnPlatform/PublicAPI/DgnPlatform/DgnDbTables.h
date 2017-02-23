/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnDbTables.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

//-----------------------------------------------------------------------------------------
// Macros associated with the BisCore ECSchema
//-----------------------------------------------------------------------------------------
#define BIS_ECSCHEMA_NAME   "BisCore"
#define BIS_SCHEMA(name)    BIS_ECSCHEMA_NAME "." name
#define BIS_TABLE(name)     "bis_" name

//-----------------------------------------------------------------------------------------
// ECClass names (combine with BIS_SCHEMA macro for use in ECSql)
//-----------------------------------------------------------------------------------------
#define BIS_CLASS_AnnotationElement2d       "AnnotationElement2d"
#define BIS_CLASS_AnnotationFrameStyle      "AnnotationFrameStyle"
#define BIS_CLASS_AnnotationLeaderStyle     "AnnotationLeaderStyle"
#define BIS_CLASS_AnnotationTextStyle       "AnnotationTextStyle"
#define BIS_CLASS_Category                  "Category"
#define BIS_CLASS_CategorySelector          "CategorySelector"
#define BIS_CLASS_CodeSpec                  "CodeSpec"
#define BIS_CLASS_DefinitionElement         "DefinitionElement"
#define BIS_CLASS_DefinitionModel           "DefinitionModel"
#define BIS_CLASS_DefinitionPartition       "DefinitionPartition"
#define BIS_CLASS_DictionaryModel           "DictionaryModel"
#define BIS_CLASS_DisplayStyle              "DisplayStyle"
#define BIS_CLASS_DisplayStyle3d            "DisplayStyle3d"
#define BIS_CLASS_Document                  "Document"
#define BIS_CLASS_DocumentListModel         "DocumentListModel"
#define BIS_CLASS_DocumentPartition         "DocumentPartition"
#define BIS_CLASS_Drawing                   "Drawing"
#define BIS_CLASS_DrawingCategory           "DrawingCategory"
#define BIS_CLASS_DrawingGraphic            "DrawingGraphic"
#define BIS_CLASS_DrawingModel              "DrawingModel"
#define BIS_CLASS_Element                   "Element"
#define BIS_CLASS_ElementAspect             "ElementAspect"
#define BIS_CLASS_ElementExternalKey        "ElementExternalKey"
#define BIS_CLASS_ElementMultiAspect        "ElementMultiAspect"
#define BIS_CLASS_ElementUniqueAspect       "ElementUniqueAspect"
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
#define BIS_CLASS_GraphicalRecipe2d         "GraphicalRecipe2d"
#define BIS_CLASS_GraphicalType2d           "GraphicalType2d"
#define BIS_CLASS_GroupInformationElement   "GroupInformationElement"
#define BIS_CLASS_GroupInformationModel     "GroupInformationModel"
#define BIS_CLASS_GroupInformationPartition "GroupInformationPartition"
#define BIS_CLASS_IModellableElement        "IModellableElement"
#define BIS_CLASS_InformationCarrierElement "InformationCarrierElement"
#define BIS_CLASS_InformationContentElement "InformationContentElement"
#define BIS_CLASS_InformationModel          "InformationModel"
#define BIS_CLASS_InformationPartitionElement "InformationPartitionElement"
#define BIS_CLASS_InformationRecordElement  "InformationRecordElement"
#define BIS_CLASS_LightDefinition           "LightDefinition"
#define BIS_CLASS_LineStyle                 "LineStyle"
#define BIS_CLASS_MaterialElement           "MaterialElement"
#define BIS_CLASS_Model                     "Model"
#define BIS_CLASS_ModelSelector             "ModelSelector"
#define BIS_CLASS_PhysicalElement           "PhysicalElement"
#define BIS_CLASS_PhysicalModel             "PhysicalModel"
#define BIS_CLASS_PhysicalPartition         "PhysicalPartition"
#define BIS_CLASS_PhysicalRecipe            "PhysicalRecipe"
#define BIS_CLASS_PhysicalType              "PhysicalType"
#define BIS_CLASS_RepositoryModel           "RepositoryModel"
#define BIS_CLASS_RoleElement               "RoleElement"
#define BIS_CLASS_RoleModel                 "RoleModel"
#define BIS_CLASS_SectionDrawing            "SectionDrawing"
#define BIS_CLASS_SectionDrawingModel       "SectionDrawingModel"
#define BIS_CLASS_Session                   "Session"
#define BIS_CLASS_SessionModel              "SessionModel"
#define BIS_CLASS_Sheet                     "Sheet"
#define BIS_CLASS_SheetModel                "SheetModel"
#define BIS_CLASS_SpatialCategory           "SpatialCategory"
#define BIS_CLASS_SpatialElement            "SpatialElement"
#define BIS_CLASS_SpatialIndex              "SpatialIndex"
#define BIS_CLASS_SpatialLocationElement    "SpatialLocationElement"
#define BIS_CLASS_SpatialLocationModel      "SpatialLocationModel"
#define BIS_CLASS_SpatialLocationPartition  "SpatialLocationPartition"
#define BIS_CLASS_SpatialModel              "SpatialModel"
#define BIS_CLASS_StreetMapModel            "StreetMapModel"
#define BIS_CLASS_SubCategory               "SubCategory"
#define BIS_CLASS_Subject                   "Subject"
#define BIS_CLASS_TextAnnotationSeed        "TextAnnotationSeed"
#define BIS_CLASS_Texture                   "Texture"
#define BIS_CLASS_TrueColor                 "TrueColor"
#define BIS_CLASS_ViewDefinition            "ViewDefinition"
#define BIS_CLASS_ViewDefinition2d          "ViewDefinition2d"
#define BIS_CLASS_ViewDefinition3d          "ViewDefinition3d"
#define BIS_CLASS_VolumeElement             "VolumeElement"

//-----------------------------------------------------------------------------------------
// ECRelationshipClass names (combine with BIS_SCHEMA macro for use in ECSql)
//-----------------------------------------------------------------------------------------
#define BIS_REL_BaseModelForView2d                  "BaseModelForView2d"
#define BIS_REL_CategoryOwnsSubCategories           "CategoryOwnsSubCategories"
#define BIS_REL_ElementDrivesElement                "ElementDrivesElement"
#define BIS_REL_ElementGroupsMembers                "ElementGroupsMembers"
#define BIS_REL_ElementOwnsChildElements            "ElementOwnsChildElements"
#define BIS_REL_ElementOwnsExternalKeys             "ElementOwnsExternalKeys"
#define BIS_REL_ElementOwnsMultiAspects             "ElementOwnsMultiAspects"
#define BIS_REL_ElementOwnsUniqueAspect             "ElementOwnsUniqueAspect"
#define BIS_REL_ElementRefersToElements             "ElementRefersToElements"
#define BIS_REL_ElementUsesGeometryParts            "ElementUsesGeometryParts"
#define BIS_REL_GraphicDerivedFromElement           "GraphicDerivedFromElement"
#define BIS_REL_GraphicalElement2dIsOfType          "GraphicalElement2dIsOfType"
#define BIS_REL_GraphicalType2dHasRecipe            "GraphicalType2dHasRecipe"
#define BIS_REL_MaterialOwnsChildMaterials          "MaterialOwnsChildMaterials"
#define BIS_REL_ModelContainsElements               "ModelContainsElements"
#define BIS_REL_ModelModelsElement                  "ModelModelsElement"
#define BIS_REL_ModelSelectorRefersToModels         "ModelSelectorRefersToModels"
#define BIS_REL_PhysicalElementAssemblesElements    "PhysicalElementAssemblesElements"
#define BIS_REL_PhysicalElementIsOfType             "PhysicalElementIsOfType"
#define BIS_REL_PhysicalTypeHasRecipe               "PhysicalTypeHasRecipe"
#define BIS_REL_SubjectOwnsChildSubjects            "SubjectOwnsChildSubjects"
#define BIS_REL_SubjectOwnsPartitionElements        "SubjectOwnsPartitionElements"

//-----------------------------------------------------------------------------------------
// DgnDb table names
//-----------------------------------------------------------------------------------------
#define DGN_TABLE_Domain                    "dgn_Domain"
#define DGN_TABLE_Font                      "dgn_Font"
#define DGN_TABLE_Handler                   "dgn_Handler"
#define DGN_TABLE_Txns                      "dgn_Txns"
#define DGN_VTABLE_SpatialIndex             "dgn_" BIS_CLASS_SpatialIndex

#include <DgnPlatform/DgnProperties.h>
#include "UnitDefinition.h"
#include "DgnFont.h"
#include "DgnCoreEvent.h"
#include "ECSqlClassParams.h"
#include "ECSqlStatementIterator.h"
#include <Bentley/HeapZone.h>

BEGIN_BENTLEY_DGN_NAMESPACE

namespace dgn_ElementHandler {struct Physical;};
namespace dgn_TxnTable {struct Element; struct Model;};

struct DgnImportContext;
struct ModelIterator;

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
// @bsiclass                                                     Paul.Connelly  09/15
//=======================================================================================
struct DgnCode
{
private:
    CodeSpecId m_codeSpecId;
    Utf8String m_value;
    Utf8String m_scope;

public:
    //! Constructs an invalid code
    DgnCode() {}

    //! Constructor
    DgnCode(CodeSpecId codeSpecId, Utf8StringCR value, Utf8StringCR scope) : m_codeSpecId(codeSpecId), m_value(value), m_scope(scope) {}

    //! Construct a code with the specified Id as its scope
    DGNPLATFORM_EXPORT DgnCode(CodeSpecId codeSpecId, Utf8StringCR value, BeInt64Id scopeId);

    //! Determine whether this DgnCode is valid.
    bool IsValid() const {return m_codeSpecId.IsValid();}
    //! Determine if this code is valid but not otherwise meaningful (and therefore not necessarily unique)
    bool IsEmpty() const {return m_codeSpecId.IsValid() && m_value.empty();}
    //! Determine if two DgnCodes are equivalent
    bool operator==(DgnCode const& other) const {return m_codeSpecId==other.m_codeSpecId && m_value==other.m_value && m_scope==other.m_scope;}
    //! Determine if two DgnCodes are not equivalent
    bool operator!=(DgnCode const& other) const {return !(*this == other);}
    //! Perform ordered comparison, e.g. for inclusion in associative containers
    DGNPLATFORM_EXPORT bool operator<(DgnCode const& rhs) const;

    //! Get the value for this DgnCode
    Utf8StringCR GetValue() const {return m_value;}
    Utf8CP GetValueCP() const {return !m_value.empty() ? m_value.c_str() : nullptr;}
    //! Get the scope for this DgnCode
    Utf8StringCR GetScope() const {return m_scope;}
    //! Get the CodeSpecId of the CodeSpec that issued this DgnCode.
    CodeSpecId GetCodeSpecId() const {return m_codeSpecId;}
    void RelocateToDestinationDb(DgnImportContext&);

    //! Re-initialize to the specified values.
    void From(CodeSpecId codeSpecId, Utf8StringCR value, Utf8StringCR scope);

    //! Create an empty, non-unique code with no special meaning.
    DGNPLATFORM_EXPORT static DgnCode CreateEmpty();

    struct Entry : ECSqlStatementEntry
    {
        friend struct ECSqlStatementIterator<Entry>;
        friend struct DgnCode;

    private:
        Entry(BeSQLite::EC::ECSqlStatement* stmt=nullptr) : ECSqlStatementEntry(stmt) {}

    public:
        CodeSpecId GetCodeSpecId() const {return m_statement->GetValueId<CodeSpecId>(0);}
        Utf8CP GetValue() const {return m_statement->GetValueText(1);}
        Utf8CP GetScope() const {return m_statement->GetValueText(2);}
        DgnCode GetCode() const {return DgnCode(GetCodeSpecId(), GetValue(), GetScope());}
    };

    struct Iterator : ECSqlStatementIterator<Entry>
    {
    public:
        struct Options
        {
        private:
            bool    m_includeEmpty;
        public:
            Options(bool includeEmpty=false) : m_includeEmpty(includeEmpty) {}
            Utf8CP GetECSql() const;
        };

        DGNPLATFORM_EXPORT explicit Iterator(DgnDbR db, Options options);
    };

    static Iterator MakeIterator(DgnDbR db, Iterator::Options options = Iterator::Options()) {return Iterator(db, options);}

    DGNPLATFORM_EXPORT void ToJson(JsonValueR value) const; //!< Convert to JSON representation
    DGNPLATFORM_EXPORT bool FromJson(JsonValueCR value); //!< Attempt to initialize from JSON representation
};

typedef bset<DgnCode> DgnCodeSet;

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

//=======================================================================================
//! Each DgnModel has an entry in the DgnModels table
//! @see DgnDb::Models
//! @ingroup GROUP_DgnModel
//=======================================================================================
struct DgnModels : DgnDbTable
{
private:
    friend struct DgnDb;
    friend struct DgnModel;
    friend struct dgn_TxnTable::Model;
    typedef bmap<DgnModelId,DgnModelPtr> T_DgnModelMap;
    typedef bmap<DgnClassId, ECSqlClassInfo> T_ClassInfoMap;

    mutable BeSQLite::BeDbMutex m_mutex;
    T_DgnModelMap  m_models;
    T_ClassInfoMap m_classInfos;

    DgnModelPtr LoadDgnModel(DgnModelId modelId);
    void Empty();
    void AddLoadedModel(DgnModelR);
    void DropLoadedModel(DgnModelR);

    ECSqlClassInfo const& FindClassInfo(DgnModelR model);
    BeSQLite::EC::CachedECSqlStatementPtr GetSelectStmt(DgnModelR model);
    BeSQLite::EC::CachedECSqlStatementPtr GetInsertStmt(DgnModelR model);
    BeSQLite::EC::CachedECSqlStatementPtr GetUpdateStmt(DgnModelR model);

    DgnModels(DgnDbR db) : DgnDbTable(db) {}
    ~DgnModels() {}

public:
    //! An object that holds a row from the DgnModel table.
    struct Model
    {
        friend struct DgnModels;

    private:
        DgnModelId m_id;
        DgnClassId m_classId;
        DgnElementId m_modeledElementId;
        bool m_inGuiList = true;
        bool m_isTemplate = false;

    public:
        Model() {}
        Model(DgnClassId classid, DgnElementId modeledElementId, DgnModelId id=DgnModelId()) : m_id(id), m_classId(classid) {}

        void SetInGuiList(bool inGuiList) {m_inGuiList = inGuiList;}
        void SetId(DgnModelId id) {m_id = id;}
        void SetClassId(DgnClassId classId) {m_classId = classId;}
        void SetModeledElementId(DgnElementId modeledElementId) {m_modeledElementId = modeledElementId;}
        void SetModelType(DgnClassId classId) {m_classId = classId;}
        void SetIsTemplate(bool isTemplate) {m_isTemplate = isTemplate;}

        bool GetInGuiList() const {return m_inGuiList;}
        DgnModelId GetId() const {return m_id;}
        DgnClassId GetClassId() const {return m_classId;}
        DgnElementId GetModeledElementId() const {return m_modeledElementId;}
        bool GetIsTemplate() const {return m_isTemplate;}
    }; // Model

public:
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

    template<class T> RefCountedPtr<T> Get(DgnModelId id) {return dynamic_cast<T*>(GetModel(id).get());}

    //! Query if the specified model has already been loaded.
    //! @return a pointer to the model if found, or nullptr if the model has not been loaded.
    //! @see GetLoadedModels
    //! @see LoadModelById
    DGNPLATFORM_EXPORT DgnModelPtr FindModel(DgnModelId modelId);

    //! Get the currently loaded DgnModels for this DgnDb
    T_DgnModelMap const& GetLoadedModels() const {return m_models;}

    DGNPLATFORM_EXPORT BentleyStatus QueryModelById(Model* out, DgnModelId id) const;

    //! Query for a DgnModelId by the DgnCode of the element that it is modeling
    DGNPLATFORM_EXPORT DgnModelId QuerySubModelId(DgnCodeCR modeledElementCode) const;

    //! Make an iterator over models of the specified ECClass in this DgnDb.
    //! @param[in] className The <i>full</i> ECClass name.  For example: BIS_SCHEMA(BIS_CLASS_PhysicalModel)
    //! @param[in] whereClause The optional where clause starting with WHERE
    //! @param[in] orderByClause The optional order by clause starting with ORDER BY
    DGNPLATFORM_EXPORT ModelIterator MakeIterator(Utf8CP className, Utf8CP whereClause=nullptr, Utf8CP orderByClause=nullptr) const;

    //! Get a string containing the list of characters that may NOT appear in model names.
    static Utf8CP GetIllegalCharacters() {return "\\/:*?<>|\"\t\n,=&";}

    //! Determine whether the supplied name is a valid model name.
    //! @param[in] name The candidate model name to check
    //! @return true if the model name is valid, false otherwise.
    //! @note Model names may also not start or end with a space.
    static bool IsValidName(Utf8StringCR name) {return DgnDbTable::IsValidName(name, GetIllegalCharacters());}

    //! Tell all models to drop any cached graphics associated with the specified viewport.
    //! This is typically invoked by applications when a viewport is closed or its attributes modified such that the cached graphics
    //! no longer reflect its state.
    //! @param[in]      viewport The viewport for which to drop graphics
    DGNPLATFORM_EXPORT void DropGraphicsForViewport(DgnViewportCR viewport);
};

//=======================================================================================
//! @see DgnDb::Fonts
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
                Entry const& operator*() const {return *this;}
            };

            typedef Entry const_iterator;
            DGNPLATFORM_EXPORT const_iterator begin() const;
            const_iterator end() const {return Entry(NULL, false);}
            DGNPLATFORM_EXPORT size_t QueryCount() const;
        };

        bool DoesFontTableExist() const {return m_dbFonts.m_db.TableExists(m_dbFonts.m_tableName.c_str());}
        DGNPLATFORM_EXPORT BentleyStatus CreateFontTable();
        DGNPLATFORM_EXPORT DgnFontPtr QueryById(DgnFontId) const;
        DGNPLATFORM_EXPORT DgnFontPtr QueryByTypeAndName(DgnFontType, Utf8CP) const;
        DGNPLATFORM_EXPORT DgnFontId QueryIdByTypeAndName(DgnFontType, Utf8CP) const;
        DGNPLATFORM_EXPORT bool ExistsById(DgnFontId) const;
        DGNPLATFORM_EXPORT bool ExistsByTypeAndName(DgnFontType, Utf8CP) const;
        DGNPLATFORM_EXPORT BentleyStatus Insert(DgnFontCR, DgnFontId&);
        DGNPLATFORM_EXPORT BentleyStatus Update(DgnFontCR, DgnFontId);
        DGNPLATFORM_EXPORT BentleyStatus Delete(DgnFontId);
        Iterator MakeIterator() const {return Iterator(m_dbFonts);}
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
            bool Equals(FaceKey const& rhs) const {return ((rhs.m_type == m_type) && m_familyName.EqualsI(rhs.m_familyName) && m_faceName.EqualsI(rhs.m_faceName));}
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
                Entry const& operator*() const {return *this;}
            };

            typedef Entry const_iterator;
            DGNPLATFORM_EXPORT const_iterator begin() const;
            const_iterator end() const {return Entry(NULL, false);}
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
        DGNPLATFORM_EXPORT BentleyStatus Insert(Byte const*, size_t dataSize, T_FaceMapCR);
        DGNPLATFORM_EXPORT BentleyStatus Delete(FaceKeyCR);
        Iterator MakeIterator() const {return Iterator(m_dbFonts);}
    };

private:
    DbFontMapDirect m_dbFontMap;
    DbFaceDataDirect m_dbFaceData;
    BeSQLite::DbR m_db;
    Utf8String m_tableName;
    bool m_isFontMapLoaded;
    T_FontMap m_fontMap;

public:
    DgnFonts(BeSQLite::DbR db, Utf8CP tableName) : m_dbFontMap(*this), m_dbFaceData(*this), m_db(db), m_tableName(tableName), m_isFontMapLoaded(false) {}

    DbFontMapDirect& DbFontMap() {return m_dbFontMap;}
    DbFaceDataDirect& DbFaceData() {return m_dbFaceData;}
    void Invalidate() {m_isFontMapLoaded = false; m_fontMap.clear();}
    DGNPLATFORM_EXPORT void Update();
    DGNPLATFORM_EXPORT DgnFontCP FindFontById(DgnFontId) const;
    DGNPLATFORM_EXPORT DgnFontCP FindFontByTypeAndName(DgnFontType, Utf8CP) const;
    DGNPLATFORM_EXPORT DgnFontId FindId(DgnFontCR) const;
    DGNPLATFORM_EXPORT DgnFontId AcquireId(DgnFontCR);
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
//! @private
//=======================================================================================
struct DgnScriptLibrary : DgnDbTable
{
public:
    DgnScriptLibrary(DgnDbR db) : DgnDbTable(db) {}

    //! Register the specified script in the DgnDb's script library.
    //! @param sName    The name to assign to the script in the library
    //! @param sText    The content of the script program
    //! @param lastModifiedTime The last modified time to record. This will be used to track versions.
    //! @param updateExisting If true, programs already registered are updated from soruce found in \a jsDir
    //! @see QueryScript
    DGNPLATFORM_EXPORT DgnDbStatus RegisterScript(Utf8CP sName, Utf8CP sText, DgnScriptType stype, DateTime const& lastModifiedTime, bool updateExisting);

    //! Look up an imported script program by the specified name.
    //! @param[out] sText           The text of the script that was found in the library
    //! @param[out] stypeFound      The type of script actually found in the library
    //! @param[out] lastModifiedTime The last modified time recorded.
    //! @param[in] sName            Identifies the script in the library
    //! @param[in] stypePreferred   The type of script that the caller prefers, if there are multiple kinds stored for the specified name.
    //! @see RegisterScript
    DGNPLATFORM_EXPORT DgnDbStatus QueryScript(Utf8StringR sText, DgnScriptType& stypeFound, DateTime& lastModifiedTime, Utf8CP sName, DgnScriptType stypePreferred);

    //! Utility function to read the text of the specified file
    //! @param contents[out]    The content of the file
    //! @param fileName[in]     The name of the file to read
    //! @return non-zero error status if the file could not be found
    DGNPLATFORM_EXPORT static DgnDbStatus ReadText(Utf8StringR contents, BeFileNameCR fileName);
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   09/13
//=======================================================================================
struct DgnGeoLocation : NonCopyableClass
{
private:
    friend struct DgnDb;
    DgnDbR  m_dgndb;
    mutable AxisAlignedBox3d m_extent;
    DPoint3d m_globalOrigin;
    mutable bool m_hasCheckedForGCS = false;
    mutable DgnGCS* m_gcs = nullptr;
    mutable IGeoCoordinateServicesP m_geoServices = nullptr;

    DgnGeoLocation(DgnDbR db);
    void    LoadProjectExtents() const;

public:
    //! @private
    //! Called only internally, on the rare occasions when a new GCS has been stored to the DgnDb.
    void    NewGCSStored () {m_hasCheckedForGCS = false;}

    //! @private
    //! Return a reasonable value that can be used if the project extents have never been established for this BIM.
    //! This is an arbitrary volume intended to represent a cube of a reasonable size, if we have no idea what is being modeled by
    //! this BIM. The default is (in meters) : [{-50,-50,-10},{50,50,30}]
    AxisAlignedBox3d GetDefaultProjectExtents() const {return AxisAlignedBox3d(DRange3d::From(-50,-50,-10, 50,50,30));}

    //! @private
    //! Initialize the project extents by querying the volume of all the spatial models. Should only be used after conversion from external formats.
    DGNPLATFORM_EXPORT void InitializeProjectExtents();

    //! @private
    //! Update the global origin. @note Changing global origin invalidates all existing xyz coordinates stored in the BIM.
    DGNPLATFORM_EXPORT void SetGlobalOrigin(DPoint3dCR origin);

    DGNPLATFORM_EXPORT void Save();
    DGNPLATFORM_EXPORT DgnDbStatus Load();

    DgnDbR GetDgnDb() const {return m_dgndb;}

    //! Get the BIM's global origin. All spatial coordinates in the BIM are stored relative to its global origin.
    DPoint3dCR GetGlobalOrigin() const {return m_globalOrigin;}

    //! Update the project extents for this BIM
    DGNPLATFORM_EXPORT void SetProjectExtents(AxisAlignedBox3dCR newExtents);

    //! Get the "spatial area of interest" for this project. This volume is used for many purposes where the software needs to
    //! include space "in the project", and exclude space outside it. For example, viewing volumes are automatically adjusted to set
    //! the depth (front and back planes) to this volume. Also, there are times when tools adjust the volume of searches or view operations
    //! to include only space inside the project extents.
    //! There are tools to visualize and adjust the project extents. Note that if the project extent is too small (does not
    //! include space occupied by elements or other artifacts of interest), they may disappear under some operations since they're not considered
    //! "of interest". Likewise, if this volume is too large, some operations may work poorly due to the large volume of "wasted space".
    DGNPLATFORM_EXPORT AxisAlignedBox3d GetProjectExtents() const;

    //! Convert a GeoPoint to an XYZ point
    //! @param[out] outXyz The output XYZ point
    //! @param[in] inLatLong The input GeoPoint
    //! @return non-zero error status if the point cannot be converted or if this DgnDb is not geo-located
    DGNPLATFORM_EXPORT BentleyStatus XyzFromLatLong(DPoint3dR outXyz, GeoPointCR inLatLong) const;

    //! Convert a an XYZ point to a GeoPoint
    //! @param[out] outLatLong  The output GeoPoint
    //! @param[in] inXyz The input XYZ point
    //! @return non-zero error status if the point cannot be converted or if this DgnDb is not geo-located
    DGNPLATFORM_EXPORT BentleyStatus LatLongFromXyz(GeoPointR outLatLong, DPoint3dCR inXyz) const;

    //! Query the GCS of this DgnDb, if any.
    //! @return this DgnDb's GCS or nullptr if this DgnDb is not geo-located
    DGNPLATFORM_EXPORT DgnGCS* GetDgnGCS() const;
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   09/13
//=======================================================================================
struct DgnUnits
{
    static double constexpr OneMeter() {return 1.0;}
    static double constexpr OneKilometer() {return 1000.0 * OneMeter();}
    static double constexpr OneMillimeter() {return OneMeter() / 1000.0;}
    static double constexpr OneCentimeter() {return OneMeter() / 100.0;}
    static double constexpr DiameterOfEarth() {return 12742. * OneKilometer();} // approximately, obviously
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
// @bsiclass                                                    Paul.Connelly   10/15
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
        struct Entry : DbTableIterator::Entry, std::iterator<std::input_iterator_tag, Entry const>
        {
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
