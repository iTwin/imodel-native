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
#define DGN_CLASSNAME_AnnotationFrameStyle  "AnnotationFrameStyle"
#define DGN_CLASSNAME_AnnotationLeaderStyle "AnnotationLeaderStyle"
#define DGN_CLASSNAME_AnnotationTextStyle   "AnnotationTextStyle"
#define DGN_CLASSNAME_Authority             "Authority"
#define DGN_CLASSNAME_TrueColor             "TrueColor"
#define DGN_CLASSNAME_ComponentModel        "ComponentModel"
#define DGN_CLASSNAME_ComponentSolution     "ComponentSolution"
#define DGN_CLASSNAME_DictionaryElement     "DictionaryElement"
#define DGN_CLASSNAME_DictionaryModel       "DictionaryModel"
#define DGN_CLASSNAME_DrawingElement        "DrawingElement"
#define DGN_CLASSNAME_DrawingModel          "DrawingModel"
#define DGN_CLASSNAME_Element               "Element"
#define DGN_CLASSNAME_ElementAspect         "ElementAspect"
#define DGN_CLASSNAME_ElementDescription    "ElementDescription"
#define DGN_CLASSNAME_ElementExternalKey    "ElementExternalKey"
#define DGN_CLASSNAME_ElementGeom           "ElementGeom"
#define DGN_CLASSNAME_ElementGroup          "ElementGroup"
#define DGN_CLASSNAME_ElementItem           "ElementItem"
#define DGN_CLASSNAME_ElementMultiAspect    "ElementMultiAspect"
#define DGN_CLASSNAME_GeomPart              "GeomPart"
#define DGN_CLASSNAME_Link                  "Link"
#define DGN_CLASSNAME_LocalAuthority        "LocalAuthority"
#define DGN_CLASSNAME_Model                 "Model"
#define DGN_CLASSNAME_Model2d               "Model2d"
#define DGN_CLASSNAME_NamespaceAuthority    "NamespaceAuthority"
#define DGN_CLASSNAME_PhysicalElement       "PhysicalElement"
#define DGN_CLASSNAME_PhysicalModel         "PhysicalModel"
#define DGN_CLASSNAME_PhysicalView          "PhysicalView"
#define DGN_CLASSNAME_PlanarPhysicalModel   "PlanarPhysicalModel"
#define DGN_CLASSNAME_ResourceModel         "ResourceModel"
#define DGN_CLASSNAME_SectionDrawingModel   "SectionDrawingModel"
#define DGN_CLASSNAME_SheetModel            "SheetModel"
#define DGN_CLASSNAME_Style                 "Style"
#define DGN_CLASSNAME_TextAnnotationSeed    "TextAnnotationSeed"
#define DGN_CLASSNAME_Texture               "Texture"
#define DGN_CLASSNAME_View                  "View"
#define DGN_CLASSNAME_CameraView            "CameraView"

//-----------------------------------------------------------------------------------------
// DgnDb table names
//-----------------------------------------------------------------------------------------
#define DGN_TABLE_Domain   DGN_TABLE("Domain")
#define DGN_TABLE_Font     DGN_TABLE("Font")
#define DGN_TABLE_Handler  DGN_TABLE("Handler")
#define DGN_TABLE_Txns     DGN_TABLE("Txns")
#define DGN_VTABLE_RTree3d DGN_TABLE("RTree3d")

//-----------------------------------------------------------------------------------------
// ECRelationshipClass names (combine with DGN_SCHEMA macro for use in ECSql)
//-----------------------------------------------------------------------------------------
#define DGN_RELNAME_ElementDrivesElement        "ElementDrivesElement"
#define DGN_RELNAME_ElementGeomUsesParts        "ElementGeomUsesParts"
#define DGN_RELNAME_ElementGroupHasMembers      "ElementGroupHasMembers"
#define DGN_RELNAME_ElementHasLinks             "ElementHasLinks"
#define DGN_RELNAME_ElementOwnsItem             "ElementOwnsItem"
#define DGN_RELNAME_ElementUsesStyles           "ElementUsesStyles"
#define DGN_RELNAME_ModelDrivesModel            "ModelDrivesModel"

#include <DgnPlatform/DgnProperties.h>
#include "UnitDefinition.h"
#include "DgnLink.h"
#include "DgnFont.h"
#include "DgnCoreEvent.h"
#include <Bentley/HeapZone.h>

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

namespace dgn_ElementHandler {struct Physical;};
namespace dgn_TxnTable {struct Element; struct Model;};

struct DgnImportContext;

//=======================================================================================
//! A Code is an identifier associated with some object in a DgnDb and issued by a
//! DgnAuthority according to some scheme. The meaning of a Code is determined by the
//! issuing authority. The issuing authority determines
//! how (or if) an object's code is transformed when the object is cloned.
//!
//! The Code is stored as a three-part identifier: DgnAuthorityId, namespace, and value.
//! The combination of the three must be unique within all objects of a given type
//! (e.g., Elements, Models) within a DgnDb. 
//!
//! The authority ID must be non-null and identify a valid authority.
//! The namespace may not be null, but may be a blank string.
//! The value may be null if and only if the namespace is blank, signifying that the authority
//! assigns no special meaning to the object's code.
//! The value may not be an empty string.
//!
//! To obtain a Code, talk to the relevant DgnAuthority.
// @bsiclass                                                     Paul.Connelly  09/15
//=======================================================================================
struct AuthorityIssuedCode
{
private:
    DgnAuthorityId  m_authority;
    Utf8String      m_value;
    Utf8String      m_nameSpace;

    friend struct DgnAuthority;
    friend struct DgnElements;
    friend struct DgnModel;
    friend struct DgnModels;
    friend struct SystemAuthority;

    AuthorityIssuedCode(DgnAuthorityId authorityId, Utf8StringCR value, Utf8StringCR nameSpace) : m_authority(authorityId), m_value(value), m_nameSpace(nameSpace) { }
public:
    //! Constructs an empty, invalid code
    AuthorityIssuedCode() { }

    //! Determine whether this Code is valid. A valid code has a valid authority ID and either:
    //!     - An empty namespace and value; or
    //!     - A non-empty value
    bool IsValid() const {return m_authority.IsValid() && (IsEmpty() || !m_value.empty());}
    //! Determine if this code is valid but not otherwise meaningful (and therefore not necessarily unique)
    bool IsEmpty() const {return m_authority.IsValid() && m_nameSpace.empty() && m_value.empty();}
    //! Determine if two Codes are equivalent
    bool operator==(AuthorityIssuedCode const& other) const {return m_authority==other.m_authority && m_value==other.m_value && m_nameSpace==other.m_nameSpace;}

    //! Get the value for this Code
    Utf8StringCR GetValue() const {return m_value;}
    Utf8CP GetValueCP() const {return !m_value.empty() ? m_value.c_str() : nullptr;}
    //! Get the namespace for this Code
    Utf8StringCR GetNameSpace() const {return m_nameSpace;}
    //! Get the DgnAuthorityId of the DgnAuthority that issued this Code.
    DgnAuthorityId GetAuthority() const {return m_authority;}
    void RelocateToDestinationDb(DgnImportContext&);
};

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
//! @see DgnDb::Views
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

    //! Generate a view name that is not currently in use in this file
    //! @param[in]  baseName base view name to start with (optional)
    //! @return unique name that was generated
    DGNPLATFORM_EXPORT Utf8String GetUniqueViewName(Utf8CP baseName);

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
//! @see DgnDb::Models
//! @ingroup DgnModelGroup
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
    bmap<DgnModelId,uint64_t> m_modelDependencyIndices;

    void ClearLoaded();
    DgnModelPtr LoadDgnModel(DgnModelId modelId);
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
        friend struct DgnModels;

    private:
        DgnModelId          m_id;
        DgnClassId          m_classId;
        AuthorityIssuedCode m_code;
        Utf8String          m_description;
        bool                m_inGuiList;

    public:
        Model()
            {
            m_inGuiList = true;
            };

        Model(AuthorityIssuedCode code, DgnClassId classid, DgnModelId id=DgnModelId()) : m_id(id), m_classId(classid), m_code(code)
            {
            m_inGuiList = true;
            }

        Utf8StringR GetDescriptionR() {return m_description;}
        void SetCode(AuthorityIssuedCode code) {m_code = code;}
        void SetDescription(Utf8CP val) {m_description.AssignOrClear(val);}
        void SetInGuiList(bool val)   {m_inGuiList = val;}
        void SetId(DgnModelId id) {m_id = id;}
        void SetClassId(DgnClassId classId) {m_classId = classId;}
        void SetModelType(DgnClassId classId) {m_classId = classId;}

        DgnModelId GetId() const {return m_id;}
        AuthorityIssuedCode const& GetCode() const {return m_code;}
        Utf8CP GetDescription() const {return m_description.c_str();}
        DgnClassId GetClassId() const {return m_classId;}
        bool InGuiList() const {return m_inGuiList;}

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
            DGNPLATFORM_EXPORT AuthorityIssuedCode GetCode() const;
            DGNPLATFORM_EXPORT Utf8CP GetCodeValue() const;
            DGNPLATFORM_EXPORT Utf8CP GetCodeNameSpace() const;
            DGNPLATFORM_EXPORT DgnAuthorityId GetCodeAuthorityId() const;
            DGNPLATFORM_EXPORT Utf8CP GetDescription() const;
            DGNPLATFORM_EXPORT DgnClassId GetClassId() const;
            DGNPLATFORM_EXPORT bool InGuiList() const;

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
    static AuthorityIssuedCode GetModelCode(Iterator::Entry const& entry); //!< @private
    DGNPLATFORM_EXPORT QvCache* GetQvCache(bool createIfNecessary=true);
    void SetQvCache(QvCache* qvCache) {m_qvCache = qvCache;}

    //! Determine the Id of the first non-dictionary model in this DgnDb.
    DGNPLATFORM_EXPORT DgnModelId QueryFirstModelId() const;

    //! Load a DgnModel from this DgnDb. Loading a model does not cause its elements to be filled. Rather, it creates an
    //! instance of the appropriate model type. If the model is already loaded, a pointer to the existing DgnModel is returned.
    //! @param[in] modelId The Id of the model to load.
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
    DGNPLATFORM_EXPORT BentleyStatus GetModelCode(AuthorityIssuedCode& code, DgnModelId id) const;

    //! Find the ModelId of the model with the specified code.
    //! @return The model's ModelId. Check dgnModelId.IsValid() to see if the DgnModelId was found.
    DGNPLATFORM_EXPORT DgnModelId QueryModelId(AuthorityIssuedCode code) const;

    //! Query for the dependency index of the specified model
    //! @param[out] dependencyIndex  The model's DependencyIndex property value
    //! @param[in] modelId      The model's ID
    //! @return non-zero if the model does not exist
    DGNPLATFORM_EXPORT BentleyStatus QueryModelDependencyIndex(uint64_t& dependencyIndex, DgnModelId modelId);

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
//! Each GeomPart has a row in the DgnGeomParts table
//! @see DgnDb::GeomParts
//! @ingroup ElementGeometryGroup
//=======================================================================================
struct DgnGeomParts : DgnDbTable
{
    friend struct DgnDb;

private:
    explicit DgnGeomParts(DgnDbR db) : DgnDbTable(db) {}
    DgnGeomPartId m_highestGeomPartId; // 0 means not yet valid. Highest DgnGeomPartId (for current briefcaseId)

public:
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
        DGNPLATFORM_EXPORT BentleyStatus Insert(Byte const*, size_t dataSize, T_FaceMapCR);
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

public:
    DgnFonts(BeSQLite::DbR db, Utf8CP tableName) : m_dbFontMap(*this), m_dbFaceData(*this), m_db(db), m_tableName(tableName), m_isFontMapLoaded(false) {}

    DbFontMapDirect& DbFontMap() { return m_dbFontMap; }
    DbFaceDataDirect& DbFaceData() { return m_dbFaceData; }
    void Invalidate() { m_isFontMapLoaded = false; m_fontMap.clear(); }
    void Update();
    DGNPLATFORM_EXPORT DgnFontCP FindFontById(DgnFontId) const;
    DGNPLATFORM_EXPORT DgnFontCP FindFontByTypeAndName(DgnFontType, Utf8CP) const;
    DGNPLATFORM_EXPORT DgnFontId FindId(DgnFontCR) const;
    DGNPLATFORM_EXPORT DgnFontId AcquireId(DgnFontCR);
};

//=======================================================================================
//! A DgnElement within a DgnDb can be identified by a "code" which is unique among all
//! elements in the DgnDb. The meaning of the code is determined by the "authority" by which
//! the code was assigned. Therefore the code includes the ID of the authority.
//! DgnAuthorities holds all such authorities associated with a DgnDb. The name of an authority
//! must be unique. An optional URI can be provided to specify how to contact the authority.
//! @see DgnDb::Authorities
//=======================================================================================
struct DgnAuthorities : DgnDbTable
{
private:
    friend struct DgnDb;
    explicit DgnAuthorities(DgnDbR db) : DgnDbTable(db) {}

    typedef bvector<DgnAuthorityPtr> LoadedAuthorities;

    LoadedAuthorities   m_loadedAuthorities;
    BeSQLite::BeDbMutex m_mutex;

    DgnAuthorityPtr LoadAuthority(DgnAuthorityId authorityId, DgnDbStatus* status = nullptr);
public:
    //! Look up the ID of the authority with the specified name.
    DGNPLATFORM_EXPORT DgnAuthorityId QueryAuthorityId(Utf8CP name) const;

    //! Look up an authority by ID. The authority will be loaded from the database if necessary.
    //! @param[in] authorityId The ID of the authority to load
    //! @returns The DgnAuthority with the specified ID, or nullptr if the authority could not be loaded
    DGNPLATFORM_EXPORT DgnAuthorityCPtr GetAuthority(DgnAuthorityId authorityId);

    //! Look up an authority by name. The authority will be loaded from the database if necessary.
    //! @param[in] name The name of the authority to load
    //! @returns The DgnAuthority with the specified name, or nullptr if the authority could not be loaded
    DGNPLATFORM_EXPORT DgnAuthorityCPtr GetAuthority(Utf8CP name);

    //! Look up an authority of a particular type by ID. The authority will be loaded from the database if necessary.
    //! @param[in] authorityId The ID of the authority to load
    //! @returns The DgnAuthority with the specified ID, or nullptr if the authority could not be loaded or is not of the desired type.
    template<typename T> RefCountedCPtr<T> Get(DgnAuthorityId authorityId) { return dynamic_cast<T const*>(GetAuthority(authorityId).get()); }

    //! Look up an authority of a particular type by name. The authority will be loaded from the database if necessary.
    //! @param[in] name The name of the authority to load
    //! @returns The DgnAuthority with the specified name, or nullptr if the authority could not be loaded or is not of the desired type.
    template<typename T> RefCountedCPtr<T> Get(Utf8CP name) { return dynamic_cast<T const*>(GetAuthority(name).get()); }
    //! Add a new Authority to the table.
    //! @param[in]  authority The new entry to add.
    //! @return The result of the insert operation.
    //! @remarks If successful, this method will assign a valid DgnAuthorityId to the supplied authority
    DGNPLATFORM_EXPORT DgnDbStatus Insert(DgnAuthorityR authority);
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
    //! @param updateExisting If true, programs already registered are updated from soruce found in \a jsDir
    //! @see QueryScript
    DGNPLATFORM_EXPORT DgnDbStatus RegisterScript(Utf8CP sName, Utf8CP sText, DgnScriptType stype, bool updateExisting);

    //! Look up an imported script program by the specified name.
    //! @param[out] sText           The text of the script that was found in the library
    //! @param[out] stypeFound      The type of script actually found in the library
    //! @param[in] sName            Identifies the script in the library
    //! @param[in] stypePreferred   The type of script that the caller prefers, if there are multiple kinds stored for the specified name.
    //! @see RegisterScript
    DGNPLATFORM_EXPORT DgnDbStatus QueryScript(Utf8StringR sText, DgnScriptType& stypeFound, Utf8CP sName, DgnScriptType stypePreferred);

    //! Utility function to read the text of the specified file
    //! @param contents[out]    The content of the file
    //! @param fileName[in]     The name of the file to read
    //! @return non-zero error status if the file could not be found
    DGNPLATFORM_EXPORT static DgnDbStatus ReadText(Utf8StringR contents, BeFileNameCR fileName);
};

//=======================================================================================
//! @see DgnDb::Units
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

    //! Gets the azimuth angle (true north offset) of the global coordinate system of this DgnDb <em>if it has one</em>.
    double GetAzimuth() const {return m_azimuth;}

    //! Set the azimuth of the global coordinate system of this DgnDb.
    void SetAzimuth(double azimuth) {m_azimuth = azimuth;}

    static double const OneMeter() {return 1.;}
    static double const OneKilometer() {return 1000. * OneMeter();}
    static double const OneMillimeter() {return OneMeter() / 1000.;}
};

//=======================================================================================
//! @see DgnDb::Styles
// @bsiclass
//=======================================================================================
struct DgnStyles : DgnDbTable
{
private:
    friend struct DgnDb;

    struct DgnLineStyles* m_lineStyles;

    explicit DgnStyles(DgnDbR);
    ~DgnStyles();

public:
    //! Provides accessors for line styles.
    DGNPLATFORM_EXPORT struct DgnLineStyles& LineStyles();
};

//=======================================================================================
// Links are shared resources, referenced by elements. 
//! @see DgnDb::Links
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
        };

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

        };

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
