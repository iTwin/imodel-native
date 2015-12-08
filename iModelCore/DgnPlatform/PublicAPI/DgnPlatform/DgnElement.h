/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnElement.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

/** @addtogroup DgnElementGroup

Classes for working with %DgnElements in memory.
@ref PAGE_ElementOverview
*/

BENTLEY_NAMESPACE_TYPEDEFS(HeapZone);

#include <Bentley/BeAssert.h>
#include "DgnAuthority.h"
#include "MemoryManager.h"

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

namespace dgn_ElementHandler {struct Element; struct Physical; struct Annotation; struct Drawing; struct Sheet; struct Group;};
namespace dgn_TxnTable {struct Element; struct Model;};

DEFINE_REF_COUNTED_PTR(ElementGeometry)

//=======================================================================================
//! Holds ID remapping tables
//=======================================================================================
struct DgnRemapTables
{
protected:
    // *** NEEDS WORK: We may have to move these remappings into temp tables
    bmap<DgnModelId, DgnModelId> m_modelId;
    bmap<DgnGeomPartId, DgnGeomPartId> m_geomPartId;
    bmap<DgnElementId, DgnElementId> m_elementId;
    bmap<DgnClassId, DgnClassId> m_classId;
    bmap<DgnAuthorityId, DgnAuthorityId> m_authorityId;

    template<typename T> T Find(bmap<T,T> const& table, T sourceId) const {auto i = table.find(sourceId); return (i == table.end())? T(): i->second;}
    template<typename T> T FindElement(T sourceId) const {return T(Find<DgnElementId>(m_elementId, sourceId).GetValueUnchecked());}
public:
    DgnRemapTables& Get(DgnDbR);
    DgnAuthorityId Find(DgnAuthorityId sourceId) const {return Find<DgnAuthorityId>(m_authorityId, sourceId);}
    DgnAuthorityId Add(DgnAuthorityId sourceId, DgnAuthorityId targetId) {return m_authorityId[sourceId] = targetId;}
    DgnModelId Find(DgnModelId sourceId) const {return Find<DgnModelId>(m_modelId, sourceId);}
    DgnModelId Add(DgnModelId sourceId, DgnModelId targetId) {return m_modelId[sourceId] = targetId;}
    DgnElementId Find(DgnElementId sourceId) const {return Find<DgnElementId>(m_elementId, sourceId);}
    DgnElementId Add(DgnElementId sourceId, DgnElementId targetId) {return m_elementId[sourceId] = targetId;}
    DgnGeomPartId Find(DgnGeomPartId sourceId) const {return Find<DgnGeomPartId>(m_geomPartId, sourceId);}
    DgnGeomPartId Add(DgnGeomPartId sourceId, DgnGeomPartId targetId) {return m_geomPartId[sourceId] = targetId;}
    DgnCategoryId Find(DgnCategoryId sourceId) const {return FindElement<DgnCategoryId>(sourceId);}
    DgnCategoryId Add(DgnCategoryId sourceId, DgnCategoryId targetId) {return DgnCategoryId((m_elementId[sourceId] = targetId).GetValueUnchecked());}
    DgnMaterialId Find(DgnMaterialId sourceId) const { return FindElement<DgnMaterialId>(sourceId); }
    DgnMaterialId Add(DgnMaterialId sourceId, DgnMaterialId targetId) { return DgnMaterialId((m_elementId [sourceId] = targetId).GetValueUnchecked()); }
    DgnTextureId Find(DgnTextureId sourceId) const {return FindElement<DgnTextureId>(sourceId);}
    DgnTextureId Add(DgnTextureId sourceId, DgnTextureId targetId) {return DgnTextureId((m_elementId [sourceId] = targetId).GetValueUnchecked()); }
    DgnStyleId Find(DgnStyleId sourceId) const {return FindElement<DgnStyleId>(sourceId);}
    DgnStyleId Add(DgnStyleId sourceId, DgnStyleId targetId) {return DgnStyleId((m_elementId [sourceId] = targetId).GetValueUnchecked()); }

    DgnSubCategoryId Find(DgnSubCategoryId sourceId) const {return FindElement<DgnSubCategoryId>(sourceId);}
    DgnSubCategoryId Add(DgnSubCategoryId sourceId, DgnSubCategoryId targetId) {return DgnSubCategoryId((m_elementId[sourceId] = targetId).GetValueUnchecked());}

    DgnClassId Find(DgnClassId sourceId) const {return Find<DgnClassId>(m_classId, sourceId);}
    DgnClassId Add(DgnClassId sourceId, DgnClassId targetId) {return m_classId[sourceId] = targetId;}
};

//=======================================================================================
//! Context used by elements when they clone themselves
//=======================================================================================
struct DgnCloneContext
{
protected:
    DgnRemapTables  m_remap;

public:
    //! Construct a DgnCloneContext object.
    DGNPLATFORM_EXPORT DgnCloneContext();

    //! Look up a copy of an element
    DgnElementId FindElementId(DgnElementId sourceId) const {return m_remap.Find(sourceId);}
    //! Register a copy of an element
    DgnElementId AddElementId(DgnElementId sourceId, DgnElementId targetId) {return m_remap.Add(sourceId, targetId);}
};

//=======================================================================================
//! Helps models, elements, aspects and other data structures copy themselves between DgnDbs
//=======================================================================================
struct DgnImportContext : DgnCloneContext
{
private:
    bool            m_areCompatibleDbs;
    DPoint2d        m_xyOffset;
    AngleInDegrees  m_yawAdj;
    DgnDbR          m_sourceDb;
    DgnDbR          m_destDb;

    void ComputeGcsAdjustment();

public:
    //! Construct a DgnImportContext object.
    DGNPLATFORM_EXPORT DgnImportContext(DgnDbR source, DgnDbR dest);

    //! @name Source and Destination Dbs
    //! @{
    DgnDbR GetSourceDb() const {return m_sourceDb;}
    DgnDbR GetDestinationDb() const {return m_destDb;}
    bool IsBetweenDbs() const {return &GetDestinationDb() != &GetSourceDb();}
    //! @}

    //! @name ID remapping
    //! @{
    //! Make sure that a DgnAuthority has been imported
    DGNPLATFORM_EXPORT DgnAuthorityId RemapAuthorityId(DgnAuthorityId sourceId);
    //! Register a copy of a DgnAuthority
    DgnAuthorityId AddAuthorityId(DgnAuthorityId sourceId, DgnAuthorityId targetId) {return m_remap.Add(sourceId, targetId);}
    //! Look up a copy of a model
    DgnModelId FindModelId(DgnModelId sourceId) const {return m_remap.Find(sourceId);}
    //! Register a copy of a model
    DgnModelId AddModelId(DgnModelId sourceId, DgnModelId targetId) {return m_remap.Add(sourceId, targetId);}
    //! Make sure that a GeomPart has been imported
    DGNPLATFORM_EXPORT DgnGeomPartId RemapGeomPartId(DgnGeomPartId sourceId);
    //! Look up a copy of a Category
    DgnCategoryId FindCategory(DgnCategoryId sourceId) const { return m_remap.Find(sourceId); }
    //! Register a copy of a Category
    DgnCategoryId AddCategory(DgnCategoryId sourceId, DgnCategoryId targetId) { return m_remap.Add(sourceId, targetId); }
    //! Make sure that a Category has been imported
    DGNPLATFORM_EXPORT DgnCategoryId RemapCategory(DgnCategoryId sourceId);
    //! Look up a copy of an subcategory
    DgnSubCategoryId FindSubCategory(DgnSubCategoryId sourceId) const {return m_remap.Find(sourceId);}
    //! Register a copy of a SubCategory
    DgnSubCategoryId AddSubCategory(DgnSubCategoryId sourceId, DgnSubCategoryId targetId) {return m_remap.Add(sourceId, targetId);}
    //! Make sure that a SubCategory has been imported
    DGNPLATFORM_EXPORT DgnSubCategoryId RemapSubCategory(DgnCategoryId destCategoryId, DgnSubCategoryId sourceId);
    //! Make sure that an ECClass has been imported
    DGNPLATFORM_EXPORT DgnClassId RemapClassId(DgnClassId sourceId);
    //! Look up a copy of a Material
    DgnMaterialId FindMaterialId(DgnMaterialId sourceId) const {return m_remap.Find(sourceId);}
    //! Register a copy of a Material
    DgnMaterialId AddMaterialId(DgnMaterialId sourceId, DgnMaterialId targetId) {return m_remap.Add(sourceId, targetId);}
    //! Make sure that a Material has been imported
    DGNPLATFORM_EXPORT DgnMaterialId RemapMaterialId(DgnMaterialId sourceId);
    //! Look up a copy of a Material
    DgnTextureId FindTextureId(DgnTextureId sourceId) const {return m_remap.Find(sourceId);}
    //! Register a copy of a Texture
    DgnTextureId AddTextureId(DgnTextureId sourceId, DgnTextureId targetId) {return m_remap.Add(sourceId, targetId);}
    //! Make sure that a Texture has been imported
    DGNPLATFORM_EXPORT DgnTextureId RemapTextureId(DgnTextureId sourceId);
    //! Look up a copy of a LineStyle
    DgnStyleId FindLineStyleId(DgnStyleId sourceId) const {return m_remap.Find(sourceId);}
    //! Register a copy of a LineStyle
    DgnStyleId AddLineStyleId(DgnStyleId sourceId, DgnStyleId targetId) {return m_remap.Add(sourceId, targetId); }
    //! Make sure that a LineStyle has been imported
    DgnStyleId RemapLineStyleId(DgnStyleId sourceId);
    //! Look up a copy of a Material
    //! Make sure that any ids referenced by the supplied GeomStream have been imported
    DGNPLATFORM_EXPORT DgnDbStatus RemapGeomStreamIds(GeomStreamR geom);
    //! @}

    //! @name GCS coordinate system shift
    //! @{
    //! Check if the source and destination GCSs are compatible, such that elements can be copied between them.
    DgnDbStatus CheckCompatibleGCS() const {return m_areCompatibleDbs? DgnDbStatus::Success: DgnDbStatus::BadRequest;}
    //! When copying between different DgnDbs, X and Y coordinates may need to be offset
    DPoint2d GetOriginOffset() const {return m_xyOffset;}
    //! When copying between different DgnDbs, the Yaw angle may need to be adjusted.
    AngleInDegrees GetYawAdjustment() const {return m_yawAdj;}
    //! @}
};

//=======================================================================================
//! The basic element importer. Imports elements and their children.
// @bsiclass                                                BentleySystems
//=======================================================================================
struct ElementImporter
{
protected:
    DgnImportContext& m_context;
    bool m_copyChildren;

public:
    DGNPLATFORM_EXPORT ElementImporter(DgnImportContext&);

    DgnImportContext& GetImportContext() {return m_context;}

    //! Specify if children should be deep-copied or not. The default is yes, deep-copy children.
    void SetCopyChildren(bool b) {m_copyChildren=b;}

    //! Make a persistent copy of a specified Physical element, along with all of its children.
    //! @param[out] stat        Optional. If not null, then an error code is stored here in case the copy fails.
    //! @param[in] destModel    The model where the instance is to be inserted
    //! @param[in] sourceElement The element that is to be copied
    //! @note Parent elements must be imported before child elements. The parent of the new element will 
    //!         be the element in the destination db to which the source parent has been remapped, or it will be invalid if the parent has not been remapped.
    //! @return a new element if successful
    DGNPLATFORM_EXPORT DgnElementCPtr ImportElement(DgnDbStatus* stat, DgnModelR destModel, DgnElementCR sourceElement);
};

template <class _QvKey> struct QvElemSet;

//=======================================================================================
// @bsiclass
//=======================================================================================
struct QvKey32
{
private:
    uint32_t m_key;

public:
    inline bool LessThan(QvKey32 const& other) const {return m_key < other.m_key;}
    inline bool Equal(QvKey32 const& other) const    {return m_key == other.m_key;}
    void DeleteQvElem(QvElem* qvElem);
    QvKey32(uint32_t key) {m_key = key;} // allow non-explicit!
};

typedef QvElemSet<QvKey32> T_QvElemSet;

//=======================================================================================
//! A list of parameters used in ECSql SELECT, INSERT, and UPDATE statements for a
//! specific ECClass. Maps names to indices in the results of a SELECT statement or in
//! the bindings of an INSERT or UPDATE statement.
//! @ingroup DgnElementGroup
// @bsiclass                                                     Paul.Connelly   09/15
//=======================================================================================
struct ECSqlClassParams
{
public:
    enum class StatementType
    {
        Select          = 1 << 0, //!< Property should be included in SELECT statements from DgnElement::_LoadFromDb()
        Insert          = 1 << 1, //!< Property should be included in INSERT statements from DgnElement::_InsertInDb()
        Update          = 1 << 2, //!< Property should be included in UPDATE statements from DgnElement::_UpdateInDb()
        ReadOnly        = Select | Insert, //!< Property cannot be modified via UPDATE statement
        All             = Select | Insert | Update, //!< Property should be included in all ECSql statements
        InsertUpdate    = Insert | Update, //!< Property should not be included in SELECT statements
    };

    struct Entry
    {
        Utf8CP          m_name;
        StatementType   m_type;

        Entry() : m_name(nullptr), m_type(StatementType::All) { }
        Entry(Utf8CP name, StatementType type) : m_name(name), m_type(type) { }
    };

    typedef bvector<Entry> Entries;
private:
    Entries m_entries;
public:
    //! Adds a parameter to the list
    //! @param[in]      parameterName The name of the parameter. @em Must be a pointer to a string with static storage duration.
    //! @param[in]      type          The type(s) of statements in which this parameter is used.
    DGNPLATFORM_EXPORT void Add(Utf8CP parameterName, StatementType type=StatementType::All);

    //! Returns an index usable for accessing the columns with the specified name in the results of an ECSql SELECT query.
    //! @param[in]      parameterName The name of the parameter
    //! @return The index of the corresponding column in the query results, or -1 if no such column exists
    DGNPLATFORM_EXPORT int GetSelectIndex(Utf8CP parameterName) const;
//__PUBLISH_SECTION_END__
    Entries const& GetEntries() const { return m_entries; }
    void RemoveAllButSelect();
//__PUBLISH_SECTION_START__
};

ENUM_IS_FLAGS(ECSqlClassParams::StatementType);

#define DGNELEMENT_DECLARE_MEMBERS(__ECClassName__,__superclass__) \
    private: typedef __superclass__ T_Super;\
    public: static Utf8CP MyECClassName() {return __ECClassName__;}\
    protected: virtual Utf8CP _GetECClassName() const override {return MyECClassName();}\
               virtual Utf8CP _GetSuperECClassName() const override {return T_Super::_GetECClassName();}

#define DGNASPECT_DECLARE_MEMBERS(__ECSchemaName__,__ECClassName__,__superclass__) \
    private:    typedef __superclass__ T_Super;\
    public:     static Utf8CP MyECSchemaName() {return __ECSchemaName__;}\
                static Utf8CP MyECClassName() {return __ECClassName__;}\
    protected:  virtual Utf8CP _GetECSchemaName() const override {return MyECSchemaName();}\
                virtual Utf8CP _GetECClassName() const override {return MyECClassName();}

//=======================================================================================
//! An instance of a DgnElement in memory. DgnElements are the building blocks for a DgnDb.
//! @ingroup DgnElementGroup
// @bsiclass                                                     KeithBentley    10/13
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE DgnElement : NonCopyableClass
{
public:
    friend struct DgnElements;
    friend struct DgnModel;
    friend struct ElemIdTree;
    friend struct dgn_ElementHandler::Element;
    friend struct dgn_TxnTable::Element;
    friend struct MultiAspect;
    friend struct GeometrySource;

    typedef DgnAuthority::Code Code;

    //! Parameters for creating a new DgnElement
    struct CreateParams
    {
    public:
        DgnDbR          m_dgndb;
        DgnModelId      m_modelId;
        DgnClassId      m_classId;
        Code            m_code;
        Utf8String      m_label;
        DgnElementId    m_id;
        DgnElementId    m_parentId;

        CreateParams(DgnDbR db, DgnModelId modelId, DgnClassId classId, Code const& code=Code(), Utf8CP label=nullptr, DgnElementId parent=DgnElementId())
            : m_dgndb(db), m_modelId(modelId), m_classId(classId), m_code(code), m_parentId(parent) {SetLabel(label);}

        DGNPLATFORM_EXPORT void RelocateToDestinationDb(DgnImportContext&);
        void SetCode(Code code) {m_code = code;}                    //!< Set the Code for elements created with this CreateParams
        void SetLabel(Utf8CP label) {m_label.AssignOrClear(label);} //!< Set the Label for elements created with this CreateParams
        void SetElementId(DgnElementId id) {m_id = id;}             //!< @private
        void SetParentId(DgnElementId parent) {m_parentId=parent;}  //!< Set the ParentId for elements created with this CreateParams
    };

    //! The Hilite state of a DgnElement. If an element is "hilited", its appearance is changed to call attention to it.
    enum class Hilited : uint8_t
    {
        None         = 0, //!< the element is displayed normally (not hilited)
        Normal       = 1, //!< the element is displayed using the normal hilite appearance
        Background   = 2, //!< the element is displayed with the background color
    };

    //! Identifies actions which may be restricted for elements created by a handler for a missing subclass of DgnElement.
    struct RestrictedAction : DgnDomain::Handler::RestrictedAction
    {
        DEFINE_T_SUPER(DgnDomain::Handler::RestrictedAction);

        static const uint64_t Clone = T_Super::NextAvailable; //!< Create a copy of element. "Clone"
        static const uint64_t SetParent = Clone << 1; //!< Change the parent element. "SetParent"
        static const uint64_t InsertChild = SetParent << 1; //!< Insert an element with this element as its parent. "InsertChild"
        static const uint64_t UpdateChild = InsertChild << 1; //!< Modify a child of this element. "UpdateChild"
        static const uint64_t DeleteChild = UpdateChild << 1; //!< Delete a child of this element. "DeleteChild"
        static const uint64_t SetCode = DeleteChild << 1; //!< Change this element's code. "SetCode"
        static const uint64_t Move = SetCode << 1; //!< Rotate and/or translate. "Move"
        static const uint64_t SetCategory = Move << 1; //!< Change element category. "SetCategory"
        static const uint64_t SetGeometry = SetCategory << 1; //!< Change element geometry. "SetGeometry"

        static const uint64_t Reserved_1 = SetGeometry << 1; //!< Reserved for future use 
        static const uint64_t Reserved_2 = Reserved_1 << 1; //!< Reserved for future use 
        static const uint64_t Reserved_3 = Reserved_2 << 1; //!< Reserved for future use 
        static const uint64_t Reserved_4 = Reserved_3 << 1; //!< Reserved for future use 
        static const uint64_t Reserved_5 = Reserved_4 << 1; //!< Reserved for future use 
        static const uint64_t Reserved_6 = Reserved_5 << 1; //!< Reserved for future use 

        static const uint64_t NextAvailable = Reserved_6 << 1; //!< Subclasses can add new actions beginning with this value

        DGNPLATFORM_EXPORT static uint64_t Parse(Utf8CP name); //!< Parse action name from ClassHasHandler custom attribute
    };

    //! Application data attached to a DgnElement. Create a subclass of this to store non-persistent information on a DgnElement.
    struct EXPORT_VTABLE_ATTRIBUTE AppData : RefCountedBase
    {
        //! A unique identifier for this type of AppData. Use a static instance of this class to identify your AppData.
        struct Key : NonCopyableClass {};

        virtual DgnDbStatus _OnInsert(DgnElementR el) {return DgnDbStatus::Success;}
        virtual DgnDbStatus _OnUpdate(DgnElementR el, DgnElementCR original){return DgnDbStatus::Success;}
        virtual DgnDbStatus _OnDelete(DgnElementCR el) {return DgnDbStatus::Success;}

        enum class DropMe {No=0, Yes=1};

        //! Called after the element was Inserted.
        //! @param[in]  el the new persistent DgnElement that was Inserted
        //! @return true to drop this appData, false to leave it attached to the DgnElement.
        //! @note el will not be the writable element onto which this AppData was attached. It will be the new persistent copy of that element.
        //! If you wish for your AppData to reside on the new element, call el.AddAppData(key,this) inside this method.
        virtual DropMe _OnInserted(DgnElementCR el){return DropMe::No;}

        //! Called after the element was Updated.
        //! @param[in] modified the modified DgnElement
        //! @param[in] original the original DgnElement
        //! @return true to drop this appData, false to leave it attached to the DgnElement.
        //! @note This method is called for @b all AppData on both the original and the modified DgnElements.
        virtual DropMe _OnUpdated(DgnElementCR modified, DgnElementCR original) {return DropMe::No;}

        //! Called after an update to the element was reversed by undo.
        //! @param[in] original the original DgnElement (after undo)
        //! @param[in] modified the modified DgnElement (before undo)
        //! @return true to drop this appData, false to leave it attached to the DgnElement.
        //! @note This method is called for @b all AppData on both the original and the modified DgnElements.
        virtual DropMe _OnReversedUpdate(DgnElementCR original, DgnElementCR modified) {return DropMe::No;}

        //! Called after the element was Deleted.
        //! @param[in]  el the DgnElement that was deleted
        //! @return true to drop this appData, false to leave it attached to the DgnElement.
        virtual DropMe _OnDeleted(DgnElementCR el) {return DropMe::Yes;}
    };

    //! Holds changes to a dgn.ElementAspect in memory and writes out the changes when the host DgnElement is inserted or updated.
    //! All aspects are actually subclasses of either dgn.ElementUniqueAspect or dgn.ElementMultiAspect. dgn.ElementItem is a special case of dgn.ElementUniqueAspect.
    //! A domain that defines a subclass of one of these ECClasses in the schema should normally also define a subclass of one of the
    //! subclasses of DgnElement::Aspect in order to manage transactions.
    //! A domain will normally subclass one of the following more specific subclasses:
    //!     * DgnElement::UniqueAspect when the domain defines a subclass of dgn.ElementUniqueAspect for aspects that must be 1:1 with the host element.
    //!     *** WIP_ELEMENT_ITEM *** pending redesign * DgnElement::Item when the domain defines a subclass of dgn.ElementItem.
    //!     * DgnElement::MultiAspect when the domain defines a subclass of dgn.ElementMultiAspect for cases where multiple instances of the class can be associated with a given element.
    //! The domain must also define and register a subclass of ElementAspectHandler to load instances of its aspects.
    struct EXPORT_VTABLE_ATTRIBUTE Aspect : AppData
    {
    private:
        DGNPLATFORM_EXPORT DropMe _OnInserted(DgnElementCR el) override final;
        DGNPLATFORM_EXPORT DropMe _OnUpdated(DgnElementCR modified, DgnElementCR original) override final;
        friend struct MultiAspectMux;

    protected:
        enum class ChangeType{None, Write, Delete};

        ChangeType m_changeType;

        DgnDbStatus InsertThis(DgnElementCR el);
        Utf8String  GetFullEcSqlClassName() {return Utf8String(_GetECSchemaName()).append(".").append(_GetECClassName());}

        DGNPLATFORM_EXPORT Aspect();

        //! The subclass must implement this method to return the name of the schema that defines the aspect.
        virtual Utf8CP _GetECSchemaName() const = 0;

        //! The subclass must implement this method to return the name of the class that defines the aspect.
        virtual Utf8CP _GetECClassName() const = 0;

        //! The subclass must implement this method to report an existing instance on the host element that this instance will replace.
        virtual BeSQLite::EC::ECInstanceKey _QueryExistingInstanceKey(DgnElementCR) = 0;

        //! The subclass must override this method in order to insert an empty instance into the Db and associate it with the host element.
        //! @param el   The host element
        //! @note The caller will call _UpdateProperties immediately after calling this method.
        virtual DgnDbStatus _InsertInstance(DgnElementCR el) = 0;

        //! The subclass must override this method in order to delete an existing instance in the Db, plus any ECRelationship that associates it with the host element.
        //! @param el   The host element
        virtual DgnDbStatus _DeleteInstance(DgnElementCR el) = 0;

        //! The subclass must implement this method to update the instance properties.
        virtual DgnDbStatus _UpdateProperties(DgnElementCR el) = 0;

        //! The subclass must implement this method to load properties from the Db.
        //! @param el   The host element
        virtual DgnDbStatus _LoadProperties(DgnElementCR el) = 0;

    public:
        //! Prepare to delete this aspect.
        //! @note The aspect will not actually be deleted in the Db until you call DgnElements::Update on the aspect's host element.
        void Delete() {m_changeType = ChangeType::Delete;}

        //! Get the ID of the ECClass for this aspect
        DGNPLATFORM_EXPORT DgnClassId GetECClassId(DgnDbR) const;

        //! Get the ECClass for this aspect
        DGNPLATFORM_EXPORT ECN::ECClassCP GetECClass(DgnDbR) const;

        //! The aspect should make a copy of itself.
        DGNPLATFORM_EXPORT virtual RefCountedPtr<DgnElement::Aspect> _CloneForImport(DgnElementCR sourceEl, DgnImportContext& importer) const;

        //! The subclass should override this method if it holds any IDs that must be remapped when it is copied (perhaps between DgnDbs)
        virtual DgnDbStatus _RemapIds(DgnElementCR el, DgnImportContext& context) {return DgnDbStatus::Success;}
    };

    //! Represents an ElementAspect subclass for the case where the host Element can have multiple instances of the subclass.
    //! Use ECSql to query existing instances and their properties. Use GetAspectP or GetP to buffer changes to a particular instance.
    //! <p>A subclass of MultiAspect must override the following methods:
    //!     * _GetECSchemaName
    //!     * _GetECClassName
    //!     * _UpdateProperties
    //!     * _LoadProperties
    //! @see UniqueAspect
    //! (Note: This is not stored directly as AppData, but is held by an AppData that aggregates instances for this class.)
    //! @note A domain that defines a subclass of MultiAspect must also define a subclass of ElementAspectHandler to load it.
    struct EXPORT_VTABLE_ATTRIBUTE MultiAspect : Aspect
    {
        DEFINE_T_SUPER(Aspect)
    protected:
        DGNPLATFORM_EXPORT BeSQLite::EC::ECInstanceKey _QueryExistingInstanceKey(DgnElementCR) override final;
        DGNPLATFORM_EXPORT DgnDbStatus _DeleteInstance(DgnElementCR el) override final;
        DGNPLATFORM_EXPORT DgnDbStatus _InsertInstance(DgnElementCR el) override final;

        BeSQLite::EC::ECInstanceId m_instanceId;

    public:
        //! Get the ID of this aspect
        BeSQLite::EC::ECInstanceId GetAspectInstanceId() const {return m_instanceId;}

        //! Load the specified instance
        //! @param el   The host element
        //! @param ecclass The class of ElementAspect to load
        //! @param ecinstanceid The ID of the ElementAspect to load
        //! @note Call this method only if you intend to modify the aspect. Use ECSql to query existing instances of the subclass.
        DGNPLATFORM_EXPORT static MultiAspect* GetAspectP(DgnElementR el, ECN::ECClassCR ecclass, BeSQLite::EC::ECInstanceId ecinstanceid);

        template<typename T> static T* GetP(DgnElementR el, ECN::ECClassCR cls, BeSQLite::EC::ECInstanceId id) {return dynamic_cast<T*>(GetAspectP(el,cls,id));}

        //! Prepare to insert an aspect for the specified element
        //! @param el   The host element
        //! @param aspect The new aspect to be adopted by the host.
        //! @note \a el will add a reference to \a aspect and will hold onto it.
        //! @note The aspect will not actually be inserted into the Db until you call DgnElements::Insert or DgnElements::Update on \a el
        DGNPLATFORM_EXPORT static void AddAspect(DgnElementR el, MultiAspect& aspect);
    };

    //! Represents an ElementAspect subclass in the case where the host Element can have 0 or 1 instance of the subclass. The aspect's ID is the same as the element's ID,
    //! and the aspect class must be stored in its own table (TablePerClass).
    //! A subclass of UniqueAspect must override the following methods:
    //!     * _GetECSchemaName
    //!     * _GetECClassName
    //!     * _UpdateProperties
    //!     * _LoadProperties
    //! @see MultiAspect
    //! @note A domain that defines a subclass of UniqueAspect must also define a subclass of ElementAspectHandler to load it.
    struct EXPORT_VTABLE_ATTRIBUTE UniqueAspect : Aspect
    {
        DEFINE_T_SUPER(Aspect)
    protected:
        static Key& GetKey(ECN::ECClassCR cls) {return *(Key*)&cls;}
        Key& GetKey(DgnDbR db) {return GetKey(*GetECClass(db));}
        static UniqueAspect* Find(DgnElementCR, ECN::ECClassCR);
        static RefCountedPtr<DgnElement::UniqueAspect> Load0(DgnElementCR, DgnClassId); // Loads *but does not call AddAppData*
        static UniqueAspect* Load(DgnElementCR, DgnClassId);
        DGNPLATFORM_EXPORT BeSQLite::EC::ECInstanceKey _QueryExistingInstanceKey(DgnElementCR) override;
        static void SetAspect0(DgnElementCR el, UniqueAspect& aspect);
        DGNPLATFORM_EXPORT DgnDbStatus _DeleteInstance(DgnElementCR el) override;
        DGNPLATFORM_EXPORT DgnDbStatus _InsertInstance(DgnElementCR el) override final;

    public:
#ifdef WIP_ELEMENT_ITEM // *** pending redesign
#endif
        //! The reason why GenerateElementGeometry is being called
        enum class GenerateReason
        {
            Insert,         //!< The Element is being inserted into the Db
            Update,         //!< Some aspect of the Element's content has changed.
            TempDraw,       //!< A tool wants to draw the Element temporarily (the Element may not be persistent)
            BulkInsert,     //!< An application is creating a large number of Elements 
            Other           //!< An unspecified reason
        };

        //! Get the ID of this aspect. The aspect's ID is always the same as the host element's ID. This is a convenience function that converts from DgnElementId to ECInstanceId.
        BeSQLite::EC::ECInstanceId GetAspectInstanceId(DgnElementCR el) const {return el.GetElementId();}

        //! Prepare to insert or update an Aspect for the specified element
        //! @param el   The host element
        //! @param aspect The new aspect to be adopted by the host.
        //! @note \a el will add a reference to \a aspect and will hold onto it.
        //! @note The aspect will not actually be inserted into the Db until you call DgnElements::Insert or DgnElements::Update on \a el
        DGNPLATFORM_EXPORT static void SetAspect(DgnElementR el, UniqueAspect& aspect);

        //! Get read-write access to the Aspect for the specified element
        //! @param el   The host element
        //! @param ecclass The class of ElementAspect to load
        //! @return The currently cached Aspect object, or nullptr if the element has no such aspect or if DeleteAspect was called.
        //! @note call this method \em only if you plan to \em modify the aspect
        //! @see Get, GetAspect for read-only access
        //! @note The aspect will not actually be updated in the Db until you call DgnElements::Update on \a el
        DGNPLATFORM_EXPORT static UniqueAspect* GetAspectP(DgnElementR el, ECN::ECClassCR ecclass);

        template<typename T> static T* GetP(DgnElementR el, ECN::ECClassCR cls) {return dynamic_cast<T*>(GetAspectP(el,cls));}

        //! Get read-only access to the Aspect for the specified element
        //! @param el   The host element
        //! @param ecclass The class of ElementAspect to load
        //! @return The currently cached Aspect object, or nullptr if the element has no such aspect or if DeleteAspect was called.
        //! @see GetP, GetAspectP for read-write access
        DGNPLATFORM_EXPORT static UniqueAspect const* GetAspect(DgnElementCR el, ECN::ECClassCR ecclass);

        template<typename T> static T const* Get(DgnElementCR el, ECN::ECClassCR cls) {return dynamic_cast<T const*>(GetAspect(el,cls));}
    };

#ifdef WIP_ELEMENT_ITEM // *** pending redesign
    //! Represents a dgn.ElementItem.
    //! dgn.ElementItem is-a dgn.ElementUniqueAspect. A dgn.Element can have 0 or 1 dgn.ElementItems, and the dgn.ElementItem always has the Id of its host dgn.Element.
    //! Note that the item's actual class can vary, as long as it is a subclass of dgn.ElementItem.
    //! ElementItems instances are always stored in the dgn.ElementItem table.
    //! <p>
    //! A dgn.ElementItem is normally used to capture the definition of the host element's geometry.
    //! The ElementItem is also expected to supply the algorithm for generating the host element's geometry from its definition.
    //! The platform enables the Item to keep the element's geometry up to date by calling the DgnElement::Item::_GenerateElementGeometry method when the element is inserted and whenever it is updated.
    //! <p>
    //! A subclass of Item must override the following methods:
    //!     * _GetECSchemaName
    //!     * _GetECClassName
    //!     * _UpdateProperties
    //!     * _LoadProperties
    //!     * _GenerateElementGeometry
    //! @note It will be common for a single ElementAspectHandler to be registered for a single ECClass and then used for \em multiple ECClasses, all of which are subclasses of the registered ECClass.
    //! Therefore, the Item subclass should not assume that it knows the ECClass of the item at compile time; it must query the DgnDb or the ECInstance (if it holds one) in order to
    //! determine the actual class of the item.
    struct EXPORT_VTABLE_ATTRIBUTE Item : UniqueAspect
    {
        DEFINE_T_SUPER(UniqueAspect)

        //! The reason why _GenerateElementGeometry is being called
        enum class GenerateReason 
            {
            Insert,         //!< The Element is being inserted into the Db
            Update,         //!< Some aspect of the Element's content has changed.
            TempDraw,       //!< A tool wants to draw the Element temporarily (the Element may not be persistent)
            BulkInsert,     //!< An application is creating a large number of Elements 
            Other           //!< An unspecified reason
            };

    private:
        static Key& GetKey();
        static Item* Find(DgnElementCR);
        static Item* Load(DgnElementCR);

        DGNPLATFORM_EXPORT DgnDbStatus _DeleteInstance(DgnElementCR el) override final; // *** WIP_ECSQL Polymorphic delete
        DGNPLATFORM_EXPORT BeSQLite::EC::ECInstanceKey _QueryExistingInstanceKey(DgnElementCR) override final;
        DgnDbStatus _OnInsert(DgnElementR el) override final {return CallGenerateGeometry(el, GenerateReason::Insert);}
        DgnDbStatus _OnUpdate(DgnElementR el, DgnElementCR original) override final {return CallGenerateGeometry(el, GenerateReason::Update);}
        static void SetItem0(DgnElementCR el, Item& item);
        DGNPLATFORM_EXPORT DgnDbStatus CallGenerateGeometry(DgnElementR, GenerateReason);

    protected:
        //! The subclass must implement this method to generate geometry and store it on \a el.
        //! The platform invokes _GenerateElementGeometry just \em before an element is inserted and/or updated.
        //! @param el   The element to be updated.
        //! @param reason An indication of why the caller is requesting the element's geometry to be generated.
        virtual DgnDbStatus _GenerateElementGeometry(GeometricElementR el, GenerateReason reason) = 0;

        //! Utility method to return the ECSchema name of an ECInstance.
        //! @param instance The instance currently assigned to this Item, or null if the Item has no in-memory instance.
        //! @return the ECSchema name from the instance or the empty string if \a instance is nullptr.
        //! @remarks For *Items* the _GetECSchemaName and _GetECClassName virtual are called *only* during an "on-updated" event. In that one case, the
        //! caller wants to know if the current in-memory Item is an instance of a different ECClass than the stored instance.
        //! Therefore the subclass of Item should implement _GetECSchemaName and _GetECClassName to return the class of its its
        //! in-memory instance data, if it has in-memory instance data. If not, it can assert and/or return empty strings. This base
        //! class cannot implement _GetECSchemaName and _GetECClassName to do that, since only the subclass knows its in-memory instance.
        //! If the subclass holds an IECInstancePtr, then it can implement those methods by calling these utility methods.
        DGNPLATFORM_EXPORT static Utf8CP GetECSchemaNameOfInstance(ECN::IECInstanceCP instance);

        //! Utility method to return the ECClass name of an ECInstance.
        //! @param instance The instance currently assigned to this Item, or null if the Item has no in-memory instance.
        //! @return the ECClass name from the instance or the empty string if \a instance is nullptr.
        //! @see GetECSchemaNameOfInstance
        DGNPLATFORM_EXPORT static Utf8CP GetECClassNameOfInstance(ECN::IECInstanceCP instance);

        //! Utility method to load an existing instance of an Item
        //! A subclass may call this as part of its implementation of _LoadProperties
        DGNPLATFORM_EXPORT DgnDbStatus LoadPropertiesIntoInstance(ECN::IECInstancePtr& instance, DgnElementCR el);

    public:
        //! Prepare to insert or update an Item for the specified element
        //! @param el   The host element
        //! @param item The new item to be adopted by the host.
        //! @note \a el will add a reference to \a item and will hold onto it.
        //! @note The item will not actually be inserted into the Db until you call DgnElements::Insert or DgnElements::Update on \a el
        DGNPLATFORM_EXPORT static void SetItem(DgnElementR el, Item& item);

        //! Get read-write access to the Item for the specified element
        //! @param el   The host element
        //! @return The currently cached Item object, or nullptr if the element has no item or if DeleteItem was called.
        //! @note call this method \em only if you plan to \em modify the item
        //! @see Get, GetItem for read-only access
        //! @note The item will not actually be updated in the Db until you call DgnElements::Update on \a el
        DGNPLATFORM_EXPORT static Item* GetItemP(DgnElementR el);

        template<typename T> static T* GetP(DgnElementR el) {return dynamic_cast<T*>(GetItemP(el));}

        //! Get read-only access to the Item for the specified element
        //! @param el   The host element
        //! @return The currently cached Item object, or nullptr if the element has no item or if DeleteItem was called.
        //! @see GetP, GetItemP for read-write access
        DGNPLATFORM_EXPORT static Item const* GetItem(DgnElementCR el);

        template<typename T> static T const* Get(DgnElementCR el) {return dynamic_cast<T const*>(GetItem(el));}

        //! Query the ECClass of item currently stored in the Db for the specified element
        //! @param el   The host element
        //! @return the DgnClassId of the existing item or an invalid ID if the element has no item
        DGNPLATFORM_EXPORT static DgnClassId QueryExistingItemClass(DgnElementCR el);

        //! Invoke the _GenerateElementGeometry method on the item
        //! @param el   The host element
        //! @param reason An indication of why the caller is requesting the element's geometry to be generated.
        DGNPLATFORM_EXPORT static DgnDbStatus GenerateElementGeometry(GeometricElementR el, GenerateReason reason);

        //! Execute the EGA that is specified for this Item.
        //! @param el   The element to be updated
        //! @param origin   The placement origin
        //! @param angles   The placement angles
        //! @param egaInstance The ECInstance that specifies the EGA and supplies any addition input parameters required by the implementation.
        //! @return DgnDbStatus::Success if the EGA was executed and the element's geometry was generated;
        //! DgnDbStatus::NotEnabled if the EGA is not available or cannot be executed; DgnDbStatus::BadArg if properties could not be marshalled from egaInstance; or DgnDbStatus::WriteError if the EGA executed but encountered an error.
        //! @see BentleyApi::Dgn::DgnScript for an explanation of script-based EGAs.
        DGNPLATFORM_EXPORT DgnDbStatus ExecuteEGA(Dgn::DgnElementR el, DPoint3dCR origin, YawPitchRollAnglesCR angles, ECN::IECInstanceCR egaInstance);
    };
#endif

    //! Allows a business key (unique identifier string) from an external system (identified by DgnAuthorityId) to be associated with a DgnElement via a persistent ElementAspect
    struct EXPORT_VTABLE_ATTRIBUTE ExternalKeyAspect : AppData
    {
    private:
        DgnAuthorityId m_authorityId;
        Utf8String m_externalKey;

        ExternalKeyAspect(DgnAuthorityId authorityId, Utf8CP externalKey)
            {
            m_authorityId = authorityId;
            m_externalKey = externalKey;
            }

    protected:
        DGNPLATFORM_EXPORT virtual DropMe _OnInserted(DgnElementCR) override;

    public:
        DGNPLATFORM_EXPORT static Key const& GetAppDataKey();
        DGNPLATFORM_EXPORT static RefCountedPtr<ExternalKeyAspect> Create(DgnAuthorityId authorityId, Utf8CP externalKey);
        DGNPLATFORM_EXPORT static DgnDbStatus Query(Utf8StringR, DgnElementCR, DgnAuthorityId);
        DGNPLATFORM_EXPORT static DgnDbStatus Delete(DgnElementCR, DgnAuthorityId);
        DgnAuthorityId GetAuthorityId() const {return m_authorityId;}
        Utf8CP GetExternalKey() const {return m_externalKey.c_str();}
    };

    typedef RefCountedPtr<ExternalKeyAspect> ExternalKeyAspectPtr;

    //! Allows a description to be associated with a DgnElement via a persistent ElementAspect
    struct EXPORT_VTABLE_ATTRIBUTE DescriptionAspect : AppData
    {
    private:
        Utf8String m_description;

        explicit DescriptionAspect(Utf8CP description)
            {
            m_description.AssignOrClear(description);
            }

    protected:
        DGNPLATFORM_EXPORT virtual DropMe _OnInserted(DgnElementCR) override;

    public:
        DGNPLATFORM_EXPORT static Key const& GetAppDataKey();
        DGNPLATFORM_EXPORT static RefCountedPtr<DescriptionAspect> Create(Utf8CP description);
        DGNPLATFORM_EXPORT static DgnDbStatus Query(Utf8StringR description, DgnElementCR);
        DGNPLATFORM_EXPORT static DgnDbStatus Delete(DgnElementCR);
        Utf8CP GetDescription() const {return m_description.c_str();}
    };

    typedef RefCountedPtr<DescriptionAspect> DescriptionAspectPtr;

    DEFINE_BENTLEY_NEW_DELETE_OPERATORS

private:
    DgnDbStatus BindParams(BeSQLite::EC::ECSqlStatement& statement, bool isForUpdate);
    template<class T> void CallAppData(T const& caller) const;

protected:
    struct Flags
    {
        uint32_t m_persistent:1;
        uint32_t m_lockHeld:1;
        uint32_t m_inSelectionSet:1;
        uint32_t m_hilited:3;
        uint32_t m_undisplayed:1;
        Flags() {memset(this, 0, sizeof(*this));}
    };

    mutable BeAtomic<uint32_t> m_refCount;
    DgnDbR          m_dgndb;
    DgnElementId    m_elementId;
    DgnElementId    m_parentId;
    DgnModelId      m_modelId;
    DgnClassId      m_classId;
    Code            m_code;
    Utf8String      m_label;
    mutable Flags   m_flags;
    mutable bmap<AppData::Key const*, RefCountedPtr<AppData>, std::less<AppData::Key const*>, 8> m_appData;

    virtual Utf8CP _GetECClassName() const {return MyECClassName();}
    virtual Utf8CP _GetSuperECClassName() const {return nullptr;}

    void SetPersistent(bool val) const {m_flags.m_persistent = val;} //!< @private
    void InvalidateElementId() { m_elementId = DgnElementId(); } //!< @private
    void InvalidateCode() { m_code = Code(); } //!< @private
    
    //! Invokes _CopyFrom() in the context of _Clone() or _CloneForImport(), preserving this element's code as specified by the CreateParams supplied to those methods.
    void CopyForCloneFrom(DgnElementCR src);

    DGNPLATFORM_EXPORT virtual ~DgnElement();

    //! Invoked when loading an element from the table, to allow subclasses to extract their property values
    //! from the SELECT statement. The parameters are those which were specified by this elements Handler.
    //! @param[in]      statement    The SELECT statement which selected the data from the table
    //! @param[in]      selectParams The properties selected by the SELECT statement. Use this to obtain an index into the statement.
    //! @return DgnDbStatus::Success if the data was loaded successfully, or else an error status.
    //! @note If you override this method, you @em must first call T_Super::_ExtractSelectParams, forwarding its status.
    //! You should then extract your subclass properties from the supplied ECSqlStatement, using
    //! selectParams.GetParameterIndex() to look up the index of each parameter within the statement.
    virtual DgnDbStatus _ExtractSelectParams(BeSQLite::EC::ECSqlStatement& statement, ECSqlClassParams const& selectParams) { return DgnDbStatus::Success; }

    //! Override this method if your element needs to load additional data from the database when it is loaded (for example,
    //! look up related data in another table).
    //! @note If you override this method, you @em must call T_Super::_LoadFromDb() first, forwarding its status
    DGNPLATFORM_EXPORT virtual DgnDbStatus _LoadFromDb();

    //! Called when an element is about to be inserted into the DgnDb.
    //! @return DgnDbStatus::Success to allow the insert, otherwise it will fail with the returned status.
    //! @note If you override this method, you @em must call T_Super::_OnInsert, forwarding its status.
    DGNPLATFORM_EXPORT virtual DgnDbStatus _OnInsert();

    //! Called to bind the element's property values to the ECSqlStatement when inserting
    //! a new element.  The parameters to bind were the ones specified by this element's Handler.
    //! @note If you override this method, you should bind your subclass properties
    //! to the supplied ECSqlStatement, using statement.GetParameterIndex with your property's name.
    //! Then you @em must call T_Super::_BindInsertParams, forwarding its status.
    DGNPLATFORM_EXPORT virtual DgnDbStatus _BindInsertParams(BeSQLite::EC::ECSqlStatement& statement);

    //! Override this method if your element needs to do additional Inserts into the database (for example,
    //! insert a relationship between the element and something else).
    //! @note If you override this method, you @em must call T_Super::_InsertInDb() first.
    DGNPLATFORM_EXPORT virtual DgnDbStatus _InsertInDb();

    //! Called after a DgnElement was successfully inserted into the database.
    //! @note If you override this method, you @em must call T_Super::_OnInserted.
    DGNPLATFORM_EXPORT virtual void _OnInserted(DgnElementP copiedFrom) const;

    //! Called after a DgnElement was successfully imported into the database.
    virtual void _OnImported(DgnElementCR original, DgnImportContext& importer) const { }
    
    //! Called after a DgnElement that was previously deleted has been reinstated by an undo.
    //! @note If you override this method, you @em must call T_Super::_OnInserted.
    DGNPLATFORM_EXPORT virtual void _OnReversedDelete() const;

    //! Called when this element is about to be replace an original element in the DgnDb.
    //! @param [in] original the original state of this element.
    //! Subclasses may override this method to control whether their instances are updated.
    //! @return DgnDbStatus::Success to allow the update, otherwise the update will fail with the returned status.
    //! @note If you override this method, you @em must call T_Super::_OnUpdate, forwarding its status.
    DGNPLATFORM_EXPORT virtual DgnDbStatus _OnUpdate(DgnElementCR original);

    //! Called to bind the element's property values to the ECSqlStatement when updating
    //! an existing element.  The parameters to bind were the ones specified by this element's Handler
    //! @note If you override this method, you should bind your subclass properties
    //! to the supplied ECSqlStatement, using statement.GetParameterIndex with your property's name.
    //! Then you @em must call T_Super::_BindUpdateParams, forwarding its status.
    DGNPLATFORM_EXPORT virtual DgnDbStatus _BindUpdateParams(BeSQLite::EC::ECSqlStatement& statement);

    //! Called to update a DgnElement in the DgnDb with new values. Override to update subclass properties.
    //! @note This method is called from DgnElements::Update, on the persistent element, after its values have been
    //! copied from the modified version. If the update fails, the original data will be copied back into this DgnElement.  Only
    //! override this method if your element needs to do additional work when updating the element, such as updating
    //! a relationship.
    DGNPLATFORM_EXPORT virtual DgnDbStatus _UpdateInDb();

    //! Called after a DgnElement was successfully updated. The element will be in its post-updated state.
    //! @note If you override this method, you @em must call T_Super::_OnUpdated.
    DGNPLATFORM_EXPORT virtual void _OnUpdated(DgnElementCR original) const;

    //! Called after an update to this DgnElement was reversed by undo. The element will be in its original (pre-change, post-undo) state.
    //! @note If you override this method, you @em must call T_Super::_OnReversedUpdate.
    DGNPLATFORM_EXPORT virtual void _OnReversedUpdate(DgnElementCR changed) const;

    //! Called when an element is about to be deleted from the DgnDb.
    //! Subclasses may override this method to control when/if their instances are deleted.
    //! @return DgnDbStatus::Success to allow the delete, otherwise the delete will fail with the returned status.
    //! @note If you override this method, you @em must call T_Super::_OnDelete, forwarding its status.
    DGNPLATFORM_EXPORT virtual DgnDbStatus _OnDelete() const;

    //! Called to delete a DgnElement from the DgnDb. Override to do any additional processing on delete.
    //! @note If you override this method, you @em must call T_Super::_DeleteInDb, forwarding its status.
    DGNPLATFORM_EXPORT virtual DgnDbStatus _DeleteInDb() const;

    //! Called after a DgnElement was successfully deleted. Note that the element will not be marked as persistent when this is called.
    //! @note If you override this method, you @em must call T_Super::_OnDeleted.
    DGNPLATFORM_EXPORT virtual void _OnDeleted() const;

    //! Called after a DgnElement that was previously added has been removed by undo.
    //! @note If you override this method, you @em must call T_Super::_OnInserted.
    DGNPLATFORM_EXPORT virtual void _OnReversedAdd() const;

    //! Called when a new element is to be inserted into a DgnDb with this element as its parent.
    //! Subclasses may override this method to control which other elements may become children.
    //! @param[in] child the new element that will become a child of this element.
    //! @return DgnDbStatus::Success to allow the child insert, otherwise it will fail with the returned status.
    //! @note implementers should not presume that returning DgnDbStatus::Success means that the element will become a child element,
    //! since the insert may fail for other reasons. Instead, rely on _OnChildInserted for that purpose.
    //! @note If you override this method, you @em must call T_Super::_OnChildInsert, forwarding its status.
    DGNPLATFORM_EXPORT virtual DgnDbStatus _OnChildInsert(DgnElementCR child) const;

    //! Called when an element that has this element as its parent is about to be updated in the DgnDb.
    //! Subclasses may override this method to control modifications to its children.
    //! @param [in] original element in its original state
    //! @param [in] replacement the child element in its modified state
    //! @return DgnDbStatus::Success to allow the child update, otherwise it will fail with the returned status.
    //! @note implementers should not presume that returning DgnDbStatus::Success means that the element was updated,
    //! since the update may fail for other reasons. Instead, rely on _OnChildUpdated for that purpose.
    //! @note If you override this method, you @em must call T_Super::_OnChildUpdate, forwarding its status.
    DGNPLATFORM_EXPORT virtual DgnDbStatus _OnChildUpdate(DgnElementCR original, DgnElementCR replacement) const;

    //! Called when an child element of this element is about to be deleted from the DgnDb.
    //! Subclasses may override this method to block deletion of their children.
    //! @param[in] child that will be deleted.
    //! @return DgnDbStatus::Success to allow the child deletion, otherwise it will fail with the returned status.
    //! @note implementers should not presume that returning DgnDbStatus::Success means that the element was deleted,
    //! since the delete may fail for other reasons. Instead, rely on _OnChildDeleted for that purpose.
    //! @note If you override this method, you @em must call T_Super::_OnChildDelete, forwarding its status.
    DGNPLATFORM_EXPORT virtual DgnDbStatus _OnChildDelete(DgnElementCR child) const;

    //! Called after a new element was inserted with this element as its parent.
    //! @note If you override this method, you @em must call T_Super::_OnChildInserted.
    virtual void _OnChildInserted(DgnElementCR child) const {}

    //! Called after an element, with this element as its parent, was successfully updated.
    //! @note if the parent of an element is changed, this method will @em not be paired with a call to _OnChildUpdate
    //! @note If you override this method, you @em must call T_Super::_OnChildUpdated.
    virtual void _OnChildUpdated(DgnElementCR child) const {}

    //! Called after an element, with this element as its parent, was successfully deleted.
    //! @note If you override this method, you @em must call T_Super::_OnChildDeleted.
    virtual void _OnChildDeleted(DgnElementCR child) const {}

    //! Called when a child element of this element is about to be imported into another DgnDb or model
    //! Subclasses may override this method to block control import of their children.
    //! @param[in] child The original element which is being imported
    //! @param[in] destModel The model into which the child is being imported
    //! @param[in] importer Enables the element to copy the resources that it needs (if copying between DgnDbs) and to remap any references that it holds to things outside itself to the copies of those things.
    virtual DgnDbStatus _OnChildImport(DgnElementCR child, DgnModelR destModel, DgnImportContext& importer) const { return DgnDbStatus::Success; }

    //! Called after an element, with this element as its parent, was successfully imported
    //! @param[in] original The original element which was cloned for import, which is @em not necessarily a child of this element.
    //! @param[in] imported The clone which was imported, which is a child of this element.
    //! @param[in] importer Enables the element to copy the resources that it needs (if copying between DgnDbs) and to remap any references that it holds to things outside itself to the copies of those things.
    virtual void _OnChildImported(DgnElementCR original, DgnElementCR imported, DgnImportContext& importer) const { }

    //! Get the size, in bytes, used by this DgnElement. This is used by the element memory management routines to gauge the "weight" of
    //! each element, so it is not necessary for the value to be 100% accurate.
    //! @note Subclasses of DgnElement that add any member variables should override this method using this template:
    //! uint32_t _GetMemSize() const override {return T_Super::_GetMemSize() + (sizeof(*this) - sizeof(T_Super)) + myAllocedSize;}
    //! where "myAllocedSize" is the number of bytes allocated for this element, held through member variable pointers.
    virtual uint32_t _GetMemSize() const {return sizeof(*this);}

    //! Virtual writeable deep copy method.
    //! @remarks If no CreateParams are supplied, a new Code will be generated for the cloned element - it will \em not be copied from this element's Code.
    DGNPLATFORM_EXPORT DgnElementPtr virtual _Clone(DgnDbStatus* stat=nullptr, DgnElement::CreateParams const* params=nullptr) const;

    //! Virtual assignment method. If your subclass has member variables, it @b must override this method and copy those values from @a source.
    //! @param[in] source The element from which to copy
    //! @note If you override this method, you @b must call T_Super::_CopyFrom, forwarding its status (that is, only return DgnDbStatus::Success if both your
    //! implementation and your superclass succeed.)
    //! @note Implementers should be aware that your element starts in a valid state. Be careful to free existing state before overwriting it. Also note that
    //! @a source is not necessarily the same type as this DgnElement. See notes at CopyFrom.
    //! @note If you hold any IDs, you must also override _RemapIds. Also see _AdjustPlacementForImport
    DGNPLATFORM_EXPORT virtual void _CopyFrom(DgnElementCR source);

    //! Make a (near) duplicate of yourself in memory, in preparation for copying from another element that <em>may be</em> in a different DgnDb.
    //! This base class implementation calls _CopyFrom and then _RemapIds and _AdjustPlacementForImport
    //! @note Do not do any of the following:
    //!     * Do not call Insert and do not attempt to remap your own ElementId. The caller will do those things.
    //!     * Do not deep-copy child elements. The caller must do that (or not).
    //!     * Do not copy ECRelationships or deep-copy related elements. The caller must do that (or not).
    //! @param[out] stat Optional status to describe failures, a valid DgnElementPtr will only be returned if successful.
    //! @param[in] destModel The destination model (which must be in the importer's destination Db).
    //! @param[in] importer Enables the element to copy the resources that it needs (if copying between DgnDbs) and to remap any references that it holds to things outside itself to the copies of those things.
    //! @return In-memory copy of the element
    DGNPLATFORM_EXPORT DgnElementPtr virtual _CloneForImport(DgnDbStatus* stat, DgnModelR destModel, DgnImportContext& importer) const;

    //! Remap any IDs that might refer to elements or resources in the source DgnDb.
    //! @param[in] importer Specifies source and destination DgnDbs and knows how to remap IDs
    DGNPLATFORM_EXPORT virtual void _RemapIds(DgnImportContext& importer);

    //! Apply X,Y offset and Yaw angle adjustment when importing from one DgnDb to another, in the case where source and destination GCSs are compatible but have the Cartesian coordinate system
    //! located at different geo locations and/or have different Azimuth angles.
    //! @param[in] importer Specifies source and destination DgnDbs and knows how to remap IDs
    virtual void _AdjustPlacementForImport(DgnImportContext const& importer) {;}

    //! Get the display label (for use in the GUI) for this DgnElement.
    //! The default implementation returns the label if set or the code if the label is not set.
    //! Override to generate the display label in a different way.
    virtual Utf8String _GetDisplayLabel() const {return HasLabel() ? m_label : GetCode().GetValue();}

    //! Change the parent (owner) of this DgnElement.
    //! The default implementation sets the parent without doing any checking.
    //! @return DgnDbStatus::Success if the parentId was changed, error status otherwise.
    //! Override to validate the parent/child relationship and return a value other than DgnDbStatus::Success to reject proposed new parent.
    DGNPLATFORM_EXPORT virtual DgnDbStatus _SetParentId(DgnElementId parentId);

    //! Change the code of this DgnElement.
    //! The default implementation sets the code without doing any checking.
    //! Override to validate the code.
    //! @return DgnDbStatus::Success if the code was changed, error status otherwise.
    DGNPLATFORM_EXPORT virtual DgnDbStatus _SetCode(Code const& code);

    //! Override to customize how the DgnElement subclass generates its code.
    DGNPLATFORM_EXPORT virtual Code _GenerateDefaultCode();

    virtual GeometrySourceCP _ToGeometrySource() const {return nullptr;}
    virtual PhysicalElementCP _ToPhysicalElement() const {return nullptr;}
    virtual AnnotationElementCP _ToAnnotationElement() const {return nullptr;}
    virtual DrawingElementCP _ToDrawingElement() const {return nullptr;}
    virtual SheetElementCP _ToSheetElement() const {return nullptr;}
    virtual DefinitionElementCP _ToDefinitionElement() const {return nullptr;}
    virtual DictionaryElementCP _ToDictionaryElement() const {return nullptr;}
    virtual IElementGroupCP _ToIElementGroup() const {return nullptr;}
    virtual SystemElementCP _ToSystemElement() const {return nullptr;}

    //! Construct a DgnElement from its params
    DGNPLATFORM_EXPORT explicit DgnElement(CreateParams const& params);

    DGNPLATFORM_EXPORT void ClearAllAppData(); //!< @private

    //! Generate the CreateParams to use for Import
    //! @param destModel Specifies the model into which the element is being cloned
    //! @param importer Specifies source and destination DgnDbs and knows how to remap IDs
    //! @return CreateParams initialized with the element's current data
    //! @remarks The m_id fields are \em not set, as it is never correct for two elements to have the same Id. The m_parentId field is not set,
    //! as it is not clear if the copy should be a child of the same parent as the original. The caller can set this if appropriate.
    //! The m_code field is copied \em only when cloning between two different DgnDbs, as it is never correct for two elements to have the same code.
    CreateParams GetCreateParamsForImport(DgnModelR destModel, DgnImportContext& importer) const;

public:
    static Utf8CP MyECClassName() {return DGN_CLASSNAME_Element;}

    DGNPLATFORM_EXPORT void AddRef() const;  //!< @private
    DGNPLATFORM_EXPORT void Release() const; //!< @private
    uint32_t GetRefCount() const {return m_refCount.load();} //!< Get the current reference count for this DgnElement.

    //! @name Dynamic casting to DgnElement subclasses
    //! @{
    GeometrySourceCP ToGeometrySource() const {return _ToGeometrySource();} //!< more efficient substitute for dynamic_cast<GeometrySourceCP>(el)
    DGNPLATFORM_EXPORT GeometrySource2dCP ToGeometrySource2d() const;
    DGNPLATFORM_EXPORT GeometrySource3dCP ToGeometrySource3d() const;

    DefinitionElementCP ToDefinitionElement() const {return _ToDefinitionElement();} //!< more efficient substitute for dynamic_cast<DefinitionElementCP>(el)
    DictionaryElementCP ToDictionaryElement() const {return _ToDictionaryElement();} //!< more efficient substitute for dynamic_cast<DictionaryElementCP>(el)
    PhysicalElementCP ToPhysicalElement() const {return _ToPhysicalElement();}    //!< more efficient substitute for dynamic_cast<PhysicalElementCP>(el)
    AnnotationElementCP ToAnnotationElement() const {return _ToAnnotationElement();}       //!< more efficient substitute for dynamic_cast<AnnotationElementCP>(el)
    DrawingElementCP ToDrawingElement() const {return _ToDrawingElement();}       //!< more efficient substitute for dynamic_cast<DrawingElementCP>(el)
    SheetElementCP ToSheetElement() const {return _ToSheetElement();}       //!< more efficient substitute for dynamic_cast<SheetElementCP>(el)
    IElementGroupCP ToIElementGroup() const {return _ToIElementGroup();}          //!< more efficient substitute for dynamic_cast<IElementGroup>(el)
    SystemElementCP ToSystemElement() const {return _ToSystemElement();}          //!< more efficient substitute for dynamic_cast<SystemElementCP>(el)
    
    GeometrySourceP ToGeometrySourceP() {return const_cast<GeometrySourceP>(_ToGeometrySource());} //!< more efficient substitute for dynamic_cast<GeometrySourceP>(el)
    GeometrySource2dP ToGeometrySource2dP() {return const_cast<GeometrySource2dP>(ToGeometrySource2d());} //!< more efficient substitute for dynamic_cast<GeometrySource2dP>(el)
    GeometrySource3dP ToGeometrySource3dP() {return const_cast<GeometrySource3dP>(ToGeometrySource3d());} //!< more efficient substitute for dynamic_cast<GeometrySource3dP>(el)

    DefinitionElementP ToDefinitionElementP() {return const_cast<DefinitionElementP>(_ToDefinitionElement());} //!< more efficient substitute for dynamic_cast<DefinitionElementP>(el)
    DictionaryElementP ToDictionaryElementP() {return const_cast<DictionaryElementP>(_ToDictionaryElement());} //!< more efficient substitute for dynamic_cast<DictionaryElementP>(el)
    PhysicalElementP ToPhysicalElementP() {return const_cast<PhysicalElementP>(_ToPhysicalElement());}     //!< more efficient substitute for dynamic_cast<PhysicalElementP>(el)
    AnnotationElementP ToAnnotationElementP() {return const_cast<AnnotationElementP>(_ToAnnotationElement());}         //!< more efficient substitute for dynamic_cast<AnnotationElementP>(el)
    DrawingElementP ToDrawingElementP() {return const_cast<DrawingElementP>(_ToDrawingElement());}         //!< more efficient substitute for dynamic_cast<DrawingElementP>(el)
    SheetElementP ToSheetElementP() {return const_cast<SheetElementP>(_ToSheetElement());}         //!< more efficient substitute for dynamic_cast<SheetElementP>(el)
    SystemElementP ToSystemElementP() {return const_cast<SystemElementP>(_ToSystemElement());}             //!< more efficient substitute for dynamic_cast<SystemElementP>(el)
    //! @}

    bool Is3d() const {return nullptr != ToGeometrySource3d();}                     //!< Determine whether this element is 3d or not
    bool Is2d() const {return nullptr != ToGeometrySource2d();}                     //!< Determine whether this element is 2d or not
    bool IsGeometricElement() const {return nullptr != ToGeometrySource();}         //!< Determine whether this element is geometric or not
    bool IsDefinitionElement() const {return nullptr != ToDefinitionElement();}     //!< Determine whether this element is a definition or not
    bool IsDictionaryElement() const {return nullptr != ToDictionaryElement();}
    bool IsSystemElement() const {return nullptr != ToSystemElement();}             //!< Determine whether this element is a SystemElement or not
    bool IsSameType(DgnElementCR other) {return m_classId == other.m_classId;}      //!< Determine whether this element is the same type (has the same DgnClassId) as another element.

    //! Determine whether this is a copy of the "persistent state" (i.e. an exact copy of what is saved in the DgnDb) of a DgnElement.
    //! @note If this flag is true, this element must be readonly. To modify an element, call CopyForEdit.
    bool IsPersistent() const {return m_flags.m_persistent;}

    //! Create a writeable deep copy of a DgnElement for insert into the same or new model.
    //! @param[out] stat Optional status to describe failures, a valid DgnElementPtr will only be returned if successful.
    //! @param[in] params Optional CreateParams. Might specify a different destination model, etc.
    //! @remarks If no CreateParams are supplied, a new Code will be generated for the cloned element - it will \em not be copied from this element's Code.
    DGNPLATFORM_EXPORT DgnElementPtr Clone(DgnDbStatus* stat=nullptr, DgnElement::CreateParams const* params=nullptr) const;

    //! Copy the content of another DgnElement into this DgnElement.
    //! @param[in] source The other element whose content is copied into this element.
    //! @note This method @b does @b not change the DgnClassId, DgnModel or DgnElementId of this DgnElement. If the type of @a source is different
    //! than this element, then all of the data from subclasses in common are copied and the remaining data on this DgnElement are unchanged.
    void CopyFrom(DgnElementCR source) {_CopyFrom(source);}

    //! Make a writable copy of this DgnElement so that the copy may be edited.
    //! @return a DgnElementPtr that holds the editable copy of this element.
    //! @note This method may only be used on a DgnElement this is the readonly persistent element returned by DgnElements::GetElement, and then
    //! only one editing copy of this element at a time may exist. If another copy is extant, this method will return an invalid DgnElementPtr.
    //! @see MakeCopy, IsPersistent
    DGNPLATFORM_EXPORT DgnElementPtr CopyForEdit() const;

    //! Make a writable copy of this DgnElement so that the copy may be edited.
    //! This is merely a templated shortcut to dynamic_cast the return of CopyForEdit to a subclass of DgnElement.
    template<class T> RefCountedPtr<T> MakeCopy() const {return dynamic_cast<T*>(CopyForEdit().get());}

    //! Create a copy of this DgnElement and all of its extended content in a destination model.
    //! The copied element will be persistent in the destination DgnDb.
    //! @param[out] stat Optional status to describe failures, a valid DgnElementPtr will only be returned if successful.
    //! @param[in] destModel The destination model (which must be in the importer's destination Db).
    //! @param[in] importer Enables the element to copy the resources that it needs (if copying between DgnDbs) and to remap any references that it holds to things outside itself to the copies of those things.
    //! @remarks The element's code will \em not be copied to the copied element if the import is being performed within a single DgnDb, as it is never correct for two elements within the same DgnDb to have the same code.
    //! @return The persistent copy of the element
    DGNPLATFORM_EXPORT DgnElementCPtr Import(DgnDbStatus* stat, DgnModelR destModel, DgnImportContext& importer) const;

    //! Update the persistent state of a DgnElement in the DgnDb from this modified copy of it.
    //! This is merely a shortcut for el.GetDgnDb().Elements().Update(el, stat);
    DGNPLATFORM_EXPORT DgnElementCPtr Update(DgnDbStatus* stat=nullptr);

    //! Insert this DgnElement into the DgnDb.
    //! This is merely a shortcut for el.GetDgnDb().Elements().Insert(el, stat);
    DGNPLATFORM_EXPORT DgnElementCPtr Insert(DgnDbStatus* stat=nullptr);

    //! Delete this DgnElement from the DgnDb,
    //! This is merely a shortcut for el.GetDgnDb().Elements().Delete(el);
    DGNPLATFORM_EXPORT DgnDbStatus Delete() const;

    //! Get the ElementHandler for this DgnElement.
    DGNPLATFORM_EXPORT ElementHandlerR GetElementHandler() const;

    //! @name AppData Management
    //! @{
    //! Get the HeapZone for the DgnDb of this element.
    //! @private
    DGNPLATFORM_EXPORT HeapZoneR GetHeapZone() const;

    //! Add Application Data to this element.
    //! @param[in] key The AppData's key. If AppData with this key already exists on this element, it is dropped and
    //! replaced with \a appData.
    //! @param[in] appData The appData object to attach to this element.
    DGNPLATFORM_EXPORT void AddAppData(AppData::Key const& key, AppData* appData) const;

    //! Drop Application data from this element.
    //! @param[in] key the key for the AppData to drop.
    //! @return SUCCESS if an entry with \a key is found and dropped.
    DGNPLATFORM_EXPORT StatusInt DropAppData(AppData::Key const& key) const;

    //! Find DgnElementAppData on this element by key.
    //! @param[in] key The key for the AppData of interest.
    //! @return the AppData for key \a key, or nullptr.
    DGNPLATFORM_EXPORT AppData* FindAppData(AppData::Key const& key) const;
    //! @}

    DgnModelId GetModelId() const {return m_modelId;}

    //! Get the DgnModel of this DgnElement.
    DGNPLATFORM_EXPORT DgnModelPtr GetModel() const;

    //! Get the DgnDb of this element.
    //! @note This is merely a shortcut for GetDgnModel().GetDgnDb().
    DgnDbR GetDgnDb() const {return m_dgndb;}

    //! Get the DgnElementId of this DgnElement
    DgnElementId GetElementId() const {return m_elementId;}

    //! Get the DgnClassId of this DgnElement.
    DgnClassId GetElementClassId() const {return m_classId;}

    //! DEPRECATED: use GetElementId (preferred) or GetECInstanceKey (for ECRelationships) instead
    //! @private
    DgnElementKey GetElementKey() const {return DgnElementKey(GetElementClassId(), GetElementId());}

    //! Get the ECInstanceKey (the element DgnClassId and DgnElementId) of this DgnElement
    //! @see GetElementClassId, GetElementId
    BeSQLite::EC::ECInstanceKey GetECInstanceKey() const {return BeSQLite::EC::ECInstanceKey(GetElementClassId().GetValue(), GetElementId());}

    //! Get a pointer to the ECClass of this DgnElement.
    DGNPLATFORM_EXPORT ECN::ECClassCP GetElementClass() const;

    //! Get the DgnElementId of the parent of this element.
    //! @see SetParentId
    //! @return Id will be invalid if this element does not have a parent element.
    DgnElementId GetParentId() const {return m_parentId;}
    //! Set the parent (owner) of this DgnElement.
    //! @see GetParentId, _SetParentId
    //! @return DgnDbStatus::Success if the parent was set
    //! @note This call can fail if a DgnElement subclass overrides _SetParentId and rejects the parent.
    DgnDbStatus SetParentId(DgnElementId parentId) {return parentId == GetParentId() ? DgnDbStatus::Success : _SetParentId(parentId);}

    //! Get the code (business key) of this DgnElement.
    Code GetCode() const {return m_code;}
    //! Set the code (business key) of this DgnElement.
    //! @see GetCode, _SetCode
    //! @return DgnDbStatus::Success if the code was set
    //! @note This call can fail if a subclass overrides _SetCode and rejects the code.
    DgnDbStatus SetCode(Code const& code) {return _SetCode(code);}

    //! Query the database for the last modified time of this DgnElement.
    DGNPLATFORM_EXPORT DateTime QueryTimeStamp() const;

    //! Return true if this DgnElement has a label.
    bool HasLabel() const {return !m_label.empty();}
    //! Get the label of this DgnElement.
    //! @note may be nullptr
    Utf8CP GetLabel() const {return m_label.c_str();}
    //! Set the label of this DgnElement.
    void SetLabel(Utf8CP label) {m_label.AssignOrClear(label);}

    //! Get the display label (for use in the GUI) of this DgnElement.
    //! @note The default implementation returns the label if it is set or the code if the label is not set.
    //! @see GetLabel, GetCode, _GetDisplayLabel
    Utf8String GetDisplayLabel() const {return _GetDisplayLabel();}

    //! Query the DgnDb for the children of this DgnElement.
    //! @return DgnElementIdSet containing the DgnElementIds of all child elements of this DgnElement. Will be empty if no children.
    DGNPLATFORM_EXPORT DgnElementIdSet QueryChildren() const;
};

//=======================================================================================
//! A stream of geometry, stored on a DgnElement, created by an ElementGeometryBuilder.
//! @ingroup ElementGeometryGroup
// @bsiclass                                                    Keith.Bentley   12/14
//=======================================================================================
struct GeomStream
{
private:
    uint32_t m_size;
    uint32_t m_allocSize;
    uint8_t* m_data;
    void swap(GeomStream& rhs) {std::swap(m_size,rhs.m_size); std::swap(m_allocSize,rhs.m_allocSize); std::swap(m_data,rhs.m_data);}

public:
    GeomStream() {m_size=m_allocSize=0; m_data=nullptr;}
    DGNPLATFORM_EXPORT GeomStream(GeomStream const&);
    DGNPLATFORM_EXPORT ~GeomStream();
    GeomStream(GeomStream&& rhs) : m_size(rhs.m_size), m_allocSize(rhs.m_allocSize), m_data(rhs.m_data) {rhs.m_size=rhs.m_allocSize=0; rhs.m_data=nullptr;}
    DGNPLATFORM_EXPORT GeomStream& operator=(GeomStream const&);
    GeomStream& operator=(GeomStream&& rhs) {GeomStream(std::move(rhs)).swap(*this); return *this;}

    //! Get the size, in bytes, of the memory allocated for this GeomStream.
    //! @note The allocated size may be larger than the currently used size returned by GetSize.
    uint32_t GetAllocSize() const {return m_allocSize;}
    uint32_t GetSize() const {return m_size;}   //!< Get the size in bytes of the current data in this GeomStream.
    uint8_t const* GetData() const {return m_data;} //!< Get a const pointer to the GeomStream.
    uint8_t* GetDataR() const {return m_data;}      //!< Get a writable pointer to the GeomStream.
    bool HasGeometry() const {return 0 != m_size;}  //!< return false if this GeomStream is empty.
    void Clear() {FREE_AND_CLEAR(m_data); m_size = m_allocSize = 0;} //!< Return this object to an empty/uninitialized state.

    //! Reserve memory for this GeomStream.
    //! @param[in] size the number of bytes to reserve
    DGNPLATFORM_EXPORT void ReserveMemory(uint32_t size);
    //! Save a stream of geometry into this GeomStream.
    //! @param[in] data the data to save
    //! @param[in] size number of bytes in data
    DGNPLATFORM_EXPORT void SaveData(uint8_t const* data, uint32_t size);

    DgnDbStatus WriteGeomStreamAndStep(DgnDbR dgnDb, Utf8CP table, Utf8CP colname, uint64_t rowId, BeSQLite::Statement& stmt, int stmtcolidx) const;
    DgnDbStatus ReadGeomStream(DgnDbR dgnDb, Utf8CP table, Utf8CP colname, uint64_t rowId);
};

//=======================================================================================
//! The position, orientation, and size of a 3d element.
// @bsiclass                                                    Keith.Bentley   06/14
//=======================================================================================
struct Placement3d
{
protected:
    DPoint3d            m_origin;
    YawPitchRollAngles  m_angles;
    ElementAlignedBox3d m_boundingBox;

public:
    Placement3d() : m_origin(DPoint3d::FromZero())  {}
    Placement3d(DPoint3dCR origin, YawPitchRollAngles angles, ElementAlignedBox3dCR box): m_origin(origin), m_angles(angles), m_boundingBox(box) {}
    Placement3d(Placement3d const& rhs) : m_origin(rhs.m_origin), m_angles(rhs.m_angles), m_boundingBox(rhs.m_boundingBox) {}
    Placement3d(Placement3d&& rhs) : m_origin(rhs.m_origin), m_angles(rhs.m_angles), m_boundingBox(rhs.m_boundingBox) {}
    Placement3d& operator=(Placement3d&& rhs) {m_origin=rhs.m_origin; m_angles=rhs.m_angles; m_boundingBox=rhs.m_boundingBox; return *this;}
    Placement3d& operator=(Placement3d const& rhs) {m_origin=rhs.m_origin; m_angles=rhs.m_angles; m_boundingBox=rhs.m_boundingBox; return *this;}

    //! Get the origin of this Placement3d.
    DPoint3dCR GetOrigin() const {return m_origin;}

    //! Get a writable reference to the origin of this Placement3d.
    DPoint3dR GetOriginR() {return m_origin;}

    //! Get the YawPitchRollAngles of this Placement3d.
    YawPitchRollAnglesCR GetAngles() const {return m_angles;}

    //! Get a writable reference to the YawPitchRollAngles of this Placement3d.
    YawPitchRollAnglesR GetAnglesR() {return m_angles;}

    //! Get the ElementAlignedBox3d of this Placement3d.
    ElementAlignedBox3d const& GetElementBox() const {return m_boundingBox;}

    //! Get a writable reference to the ElementAlignedBox3d of this Placement3d.
    ElementAlignedBox3d& GetElementBoxR() {return m_boundingBox;}

    //! Convert the origin and YawPitchRollAngles of this Placement3d into a Transform.
    Transform GetTransform() const {return m_angles.ToTransform(m_origin);}

    //! Calculate the AxisAlignedBox3d of this Placement3d.
    DGNPLATFORM_EXPORT AxisAlignedBox3d CalculateRange() const;

    //! Determine whether this Placement3d is valid.
    bool IsValid() const {return m_boundingBox.IsValid();}
};

//=======================================================================================
//! The position, rotation angle, and bounding box for a 2-dimensional element.
// @bsiclass                                                    Keith.Bentley   06/14
//=======================================================================================
struct Placement2d
{
protected:
    DPoint2d            m_origin;
    AngleInDegrees      m_angle;
    ElementAlignedBox2d m_boundingBox;

public:
    Placement2d() : m_origin(DPoint2d::FromZero()) {}
    Placement2d(DPoint2dCR origin, AngleInDegrees const& angle, ElementAlignedBox2dCR box) : m_origin(origin), m_angle(angle), m_boundingBox(box){}
    Placement2d(Placement2d const& rhs) : m_origin(rhs.m_origin), m_angle(rhs.m_angle), m_boundingBox(rhs.m_boundingBox) {}
    Placement2d(Placement2d&& rhs) : m_origin(rhs.m_origin), m_angle(rhs.m_angle), m_boundingBox(rhs.m_boundingBox) {}
    Placement2d& operator=(Placement2d&& rhs) {m_origin=rhs.m_origin; m_angle=rhs.m_angle; m_boundingBox=rhs.m_boundingBox; return *this;}
    Placement2d& operator=(Placement2d const& rhs) {m_origin=rhs.m_origin; m_angle=rhs.m_angle; m_boundingBox=rhs.m_boundingBox; return *this;}

    //! Get the origin of this Placement2d.
    DPoint2dCR GetOrigin() const {return m_origin;}

    //! Get a writable reference to the origin of this Placement2d.
    DPoint2dR GetOriginR() {return m_origin;}

    //! Get the angle of this Placement2d
    AngleInDegrees GetAngle() const {return m_angle;}

    //! Get a writable reference to the angle of this Placement2d.
    AngleInDegrees& GetAngleR() {return m_angle;}

    //! Get the ElementAlignedBox2d of this Placement2d.
    ElementAlignedBox2d const& GetElementBox() const {return m_boundingBox;}

    //! Get a writable reference to the ElementAlignedBox2d of this Placement2d.
    ElementAlignedBox2d& GetElementBoxR() {return m_boundingBox;}

    //! Convert the origin and angle of this Placement2d into a Transform.
    Transform GetTransform() const {Transform t; t.InitFromOriginAngleAndLengths(m_origin, m_angle.Radians(), 1.0, 1.0); return t;}

    //! Calculate an AxisAlignedBox3d for this Placement2d.
    //! @note the z values are arbitrarily set to +-.5 mm.
    DGNPLATFORM_EXPORT AxisAlignedBox3d CalculateRange() const;

    //! Determine whether this Placement2d is valid
    bool IsValid() const {return m_boundingBox.IsValid();}
};

//=======================================================================================
// @bsiclass                                                    Brien.Bastings  11/15
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE GeometrySource
{
//__PUBLISH_SECTION_END__
    friend struct ElementGeometryBuilder;

//__PUBLISH_SECTION_START__
protected:

virtual DgnDbR _GetSourceDgnDb() const = 0;
virtual DgnElementCP _ToElement() const = 0;
virtual GeometrySource2dCP _ToGeometrySource2d() const = 0; // Either this method or _ToGeometrySource3d must return non-null.
virtual GeometrySource3dCP _ToGeometrySource3d() const = 0; // Either this method or _ToGeometrySource2d must return non-null.

virtual DgnCategoryId _GetCategoryId() const = 0;
virtual DgnDbStatus _SetCategoryId(DgnCategoryId categoryId) = 0;
virtual GeomStreamCR _GetGeomStream() const = 0;
virtual bool _HasGeometry() const {return _GetGeomStream().HasGeometry();}
virtual AxisAlignedBox3d _CalculateRange3d() const = 0;

DGNPLATFORM_EXPORT virtual void _Draw(ViewContextR) const;
DGNPLATFORM_EXPORT virtual bool _DrawHit(HitDetailCR, ViewContextR) const;
DGNPLATFORM_EXPORT virtual void _GetInfoString(HitDetailCR, Utf8StringR descr, Utf8CP delimiter) const;
DGNPLATFORM_EXPORT virtual SnapStatus _OnSnap(SnapContextR) const;

GeomStreamR GetGeomStreamR() {return const_cast<GeomStreamR>(_GetGeomStream());} // Only ElementGeometryBuilder should have write access to the GeomStream...

public:

DgnDbR GetSourceDgnDb() const {return _GetSourceDgnDb();}
DgnElementCP ToElement() const {return _ToElement();} //! Caller must be prepared to this to return nullptr.
DgnElementP ToElementP() {return const_cast<DgnElementP>(_ToElement());} //! Caller must be prepared to this to return nullptr.

GeometrySource2dCP ToGeometrySource2d() const {return _ToGeometrySource2d();}
GeometrySource2dP ToGeometrySource2dP() {return const_cast<GeometrySource2dP>(_ToGeometrySource2d());}

GeometrySource3dCP ToGeometrySource3d() const {return _ToGeometrySource3d();}
GeometrySource3dP ToGeometrySource3dP() {return const_cast<GeometrySource3dP>(_ToGeometrySource3d());}

DgnCategoryId GetCategoryId() const {return _GetCategoryId();}
DgnDbStatus SetCategoryId(DgnCategoryId categoryId) {return _SetCategoryId(categoryId);}

GeomStreamCR GetGeomStream() const {return _GetGeomStream();}
bool HasGeometry() const {return _HasGeometry();} //!< return false if this geometry source currently has no geometry (is empty).
AxisAlignedBox3d CalculateRange3d() const {return _CalculateRange3d();}
DGNPLATFORM_EXPORT Transform GetPlacementTransform() const;

// NOT_NOW_GEOMETRY_SOURCE - Make hilite/undisplayed virtual so transients can use them...
bool IsUndisplayed() const {if (nullptr == ToElement()) return false; return ToElement()->m_flags.m_undisplayed;}
DgnElement::Hilited IsHilited() const {if (nullptr == ToElement()) return DgnElement::Hilited::None; return (DgnElement::Hilited) ToElement()->m_flags.m_hilited;} //!< Get the current Hilited state of this element
bool IsInSelectionSet() const {if (nullptr == ToElement()) return false; return ToElement()->m_flags.m_inSelectionSet;}

DGNPLATFORM_EXPORT void SetUndisplayed(bool yesNo) const;
DGNPLATFORM_EXPORT void SetHilited(DgnElement::Hilited newState) const; //!< Change the current Hilited state of this element
DGNPLATFORM_EXPORT void SetInSelectionSet(bool yesNo) const; //!< @private

void Draw(ViewContextR context) const {_Draw(context);}
bool DrawHit(HitDetailCR hit, ViewContextR context) const {return _DrawHit(hit, context);}
void GetInfoString(HitDetailCR hit, Utf8StringR descr, Utf8CP delimiter) const {_GetInfoString(hit, descr, delimiter);}
SnapStatus OnSnap(SnapContextR context) const {return _OnSnap(context);}

}; // GeometrySource

//=======================================================================================
// @bsiclass                                                    Brien.Bastings  11/15
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE GeometrySource3d : GeometrySource
{
protected:

virtual GeometrySource2dCP _ToGeometrySource2d() const override final {return nullptr;}
virtual AxisAlignedBox3d _CalculateRange3d() const override final {return _GetPlacement().CalculateRange();}
virtual Placement3dCR _GetPlacement() const = 0;
virtual DgnDbStatus _SetPlacement(Placement3dCR placement) = 0;

DGNPLATFORM_EXPORT DgnDbStatus InsertGeomSourceInDb();
DGNPLATFORM_EXPORT DgnDbStatus UpdateGeomSourceInDb();

public:

Placement3dCR GetPlacement() const {return _GetPlacement();} //!< Get the Placement3d of this element
DgnDbStatus SetPlacement(Placement3dCR placement) {return _SetPlacement(placement);} //!< Change the Placement3d for this element

}; // GeometrySource3d

//=======================================================================================
// @bsiclass                                                    Brien.Bastings  11/15
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE GeometrySource2d : GeometrySource
{
protected:

virtual GeometrySource3dCP _ToGeometrySource3d() const override final {return nullptr;}
virtual AxisAlignedBox3d _CalculateRange3d() const override final {return _GetPlacement().CalculateRange();}
virtual Placement2dCR _GetPlacement() const = 0;
virtual DgnDbStatus _SetPlacement(Placement2dCR placement) = 0;

DGNPLATFORM_EXPORT DgnDbStatus InsertGeomSourceInDb();
DGNPLATFORM_EXPORT DgnDbStatus UpdateGeomSourceInDb();

public:

Placement2dCR GetPlacement() const {return _GetPlacement();} //!< Get the Placement2d of this element
DgnDbStatus SetPlacement(Placement2dCR placement) {return _SetPlacement(placement);} //!< Change the Placement2d for this element

}; // GeometrySource2d

//=======================================================================================
//! Data associated with a geometric element.
//! @private
// @bsiclass                                                    Paul.Connelly   12/15
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ElementGeomData
{
protected:
    DgnCategoryId   m_categoryId;
    GeomStream      m_geom;

    explicit ElementGeomData(DgnCategoryId category) : m_categoryId(category) { }

    virtual bool _IsPlacementValid() const = 0;
    virtual void _SetPlacement(void const* placement) = 0;
public:
    DgnCategoryId GetCategoryId() const { return m_categoryId; }
    GeomStreamCR GetGeomStream() const { return m_geom; }

    DGNPLATFORM_EXPORT DgnDbStatus SetCategoryId(DgnCategoryId catId, DgnElementCR el);

    DGNPLATFORM_EXPORT DgnDbStatus Validate() const;
    DGNPLATFORM_EXPORT DgnDbStatus LoadFromDb(DgnElementId elemId, DgnDbR db);
    DGNPLATFORM_EXPORT void RemapIds(DgnImportContext& importer);
};

//=======================================================================================
//! Geometric data associated with a 3d geometric element.
//! @private
// @bsiclass                                                    Paul.Connelly   12/15
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ElementGeom3d : ElementGeomData
{
    typedef Placement3d T_Placement;
    typedef GeometrySource3d T_GeometrySource;
private:
    Placement3d     m_placement;

    virtual bool _IsPlacementValid() const override { return m_placement.IsValid(); }
    DGNPLATFORM_EXPORT virtual void _SetPlacement(void const* placement) override;
public:
    explicit ElementGeom3d(DgnCategoryId category=DgnCategoryId(), Placement3dCR placement=Placement3d()) : ElementGeomData(category), m_placement(placement) { }

    Placement3dCR GetPlacement() const { return m_placement; }

    DGNPLATFORM_EXPORT DgnDbStatus SetPlacement(Placement3dCR placement, DgnElementCR el);

    DGNPLATFORM_EXPORT void AdjustPlacementForImport(DgnImportContext const& importer);
    DGNPLATFORM_EXPORT void CopyFrom(GeometrySource3dCP geomSource);
};

//=======================================================================================
//! Parameterized base class for direct or indirect subclasses of DgnElement with
//! 2D or 3D geometry.
//! This template allows any subclass of DgnElement to introduce geometry regardless of
//! its position within the class hierarchy.
//! Specializations exist for 2d and 3d geometry (GeometricElement2d, GeometricElement3d).
//! @ingroup DgnElementGroup
// @bsiclass                                                    Paul.Connelly   12/15
//=======================================================================================
template<typename T_Base, typename T_Geom> struct EXPORT_VTABLE_ATTRIBUTE GeometricElement : T_Base, T_Geom::T_GeometrySource
{
protected:
    T_Geom m_geom;

    typedef typename T_Base::CreateParams CreateParams;
    typedef typename T_Geom::T_Placement T_Placement;

    explicit GeometricElement(CreateParams const& params, DgnCategoryId category=DgnCategoryId(), T_Placement const& placement=T_Placement())
        : T_Base(params), m_geom(category, placement) { }

    virtual DgnDbR _GetSourceDgnDb() const override final {return this->GetDgnDb();}
    virtual DgnElementCP _ToElement() const override final {return this;}
    virtual GeometrySourceCP _ToGeometrySource() const override final {return this;}

    virtual DgnCategoryId _GetCategoryId() const override final {return m_geom.GetCategoryId();}
    virtual DgnDbStatus _SetCategoryId(DgnCategoryId categoryId) override { return m_geom.SetCategoryId(categoryId, *this); }
    virtual GeomStreamCR _GetGeomStream() const override final {return m_geom.GetGeomStream();}

    virtual T_Placement const& _GetPlacement() const override final {return m_geom.GetPlacement();}
    virtual DgnDbStatus _SetPlacement(T_Placement const& placement) override { return m_geom.SetPlacement(placement, *this); }
    virtual void _AdjustPlacementForImport(DgnImportContext const& importer) override { m_geom.AdjustPlacementForImport(importer); }

    virtual DgnDbStatus _OnInsert() override { auto status = m_geom.Validate(); return DgnDbStatus::Success == status ? T_Base::_OnInsert() : status; }
    virtual DgnDbStatus _OnUpdate(DgnElementCR original) override { auto status = m_geom.Validate(); return DgnDbStatus::Success == status ? T_Base::_OnUpdate(original) : status; }

    virtual DgnDbStatus _LoadFromDb() override { auto status = T_Base::_LoadFromDb(); return DgnDbStatus::Success == status ? m_geom.LoadFromDb(this->GetElementId(), this->GetDgnDb()) : status; }
    virtual DgnDbStatus _InsertInDb() override { auto status = T_Base::_InsertInDb(); return DgnDbStatus::Success == status ? this->InsertGeomSourceInDb() : status; }
    virtual DgnDbStatus _UpdateInDb() override { auto status = T_Base::_UpdateInDb(); return DgnDbStatus::Success == status ? this->UpdateGeomSourceInDb() : status; }

    virtual void _RemapIds(DgnImportContext& importer) override { T_Base::_RemapIds(importer); m_geom.RemapIds(importer); }
    virtual uint32_t _GetMemSize() const override {return T_Base::_GetMemSize() + static_cast<uint32_t>(sizeof(m_geom));}
};

//=======================================================================================
//! Parameterized base class for direct or indirect subclasses of DgnElement with
//! 3D geometry.
//! @ingroup DgnElementGroup
// @bsiclass                                                    Paul.Connelly   12/15
//=======================================================================================
template<typename T_Base> struct EXPORT_VTABLE_ATTRIBUTE GeometricElement3d : GeometricElement<T_Base, ElementGeom3d>
{
protected:
    typedef typename T_Base::CreateParams CreateParams;

    explicit GeometricElement3d(CreateParams const& params, DgnCategoryId category=DgnCategoryId(), Placement3dCR placement=Placement3d())
        : GeometricElement<T_Base, ElementGeom3d>(params, category, placement) { }

    virtual GeometrySource3dCP _ToGeometrySource3d() const override final {return this;}
    virtual void _CopyFrom(DgnElementCR rhs) override { T_Base::_CopyFrom(rhs); this->m_geom.CopyFrom(rhs.ToGeometrySource3d()); }
};

//! Specialization of GeometricElement3d deriving directly from the dgn:Element ECClass.
typedef GeometricElement3d<DgnElement> DgnElement3d;

//=======================================================================================
//! CreateParams used for constructing geometric elements
//! @ingroup DgnElementGroup
// @bsiclass                                                    Paul.Connelly   12/15
//=======================================================================================
template<typename T_Placement> struct GeometricElementCreateParams : DgnElement::CreateParams
{
    DEFINE_T_SUPER(DgnElement::CreateParams);

    DgnCategoryId m_categoryId;
    T_Placement const& m_placement;

    GeometricElementCreateParams(DgnDbR db, DgnModelId modelId, DgnClassId classId, DgnCategoryId category, T_Placement const& placement=T_Placement(), DgnElement::Code const& code=DgnElement::Code(), Utf8CP label=nullptr, DgnElementId parent=DgnElementId()) :
        T_Super(db, modelId, classId, code, label, parent), m_categoryId(category), m_placement(placement) {}

    explicit GeometricElementCreateParams(T_Super const& params, DgnCategoryId category=DgnCategoryId(), T_Placement const& placement=T_Placement()) : T_Super(params), m_categoryId(category), m_placement(placement) {}
    GeometricElementCreateParams(GeometricElementCreateParams const& params) : T_Super(params), m_categoryId(params.m_categoryId), m_placement(params.m_placement) {}
};

//! CreateParams used for constructing direct subclasses of GeometricElement2d<DgnElement>
typedef GeometricElementCreateParams<Placement2d> ElementCreateParams2d;

//! CreateParams used for constructing direct subclasses of GeometricElement3d<DgnElement>
typedef GeometricElementCreateParams<Placement3d> ElementCreateParams3d;

//=======================================================================================
//! A 3d element that exists in the physical coordinate space of a DgnDb.
//! @ingroup DgnElementGroup
// @bsiclass                                                    Keith.Bentley   04/15
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE PhysicalElement : DgnElement3d
{
    DGNELEMENT_DECLARE_MEMBERS(DGN_CLASSNAME_PhysicalElement, DgnElement3d)

protected:
    PhysicalElementCP _ToPhysicalElement() const override {return this;}

public:
    typedef ElementCreateParams3d CreateParams;

    explicit PhysicalElement(CreateParams const& params) : T_Super(params, params.m_categoryId, params.m_placement) {}

    //! Create an instance of a PhysicalElement from a CreateParams.
    //! @note This is a static method that creates an instance of the PhysicalElement class. To create subclasses, use static methods on the appropriate class.
    static PhysicalElementPtr Create(CreateParams const& params) {return new PhysicalElement(params);}

    //! Create an instance of a PhysicalElement from a model and DgnCategoryId, using the default values for all other parameters.
    //! @param[in] model The PhysicalModel for the new PhysicalElement.
    //! @param[in] categoryId The category for the new PhysicalElement.
    DGNPLATFORM_EXPORT static PhysicalElementPtr Create(PhysicalModelR model, DgnCategoryId categoryId);
};

//=======================================================================================
//! Geometric data associated with a 2d geometric element.
//! @private
// @bsiclass                                                    Paul.Connelly   12/15
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ElementGeom2d : ElementGeomData
{
    typedef Placement2d T_Placement;
    typedef GeometrySource2d T_GeometrySource;
private:
    Placement2d     m_placement;

    virtual bool _IsPlacementValid() const override { return m_placement.IsValid(); }
    DGNPLATFORM_EXPORT virtual void _SetPlacement(void const* placement) override;
public:
    explicit ElementGeom2d(DgnCategoryId category=DgnCategoryId(), Placement2dCR placement=Placement2d()) : ElementGeomData(category), m_placement(placement) { }

    Placement2dCR GetPlacement() const { return m_placement; }

    DGNPLATFORM_EXPORT DgnDbStatus SetPlacement(Placement2dCR placement, DgnElementCR el);

    DGNPLATFORM_EXPORT void AdjustPlacementForImport(DgnImportContext const& importer);
    DGNPLATFORM_EXPORT void CopyFrom(GeometrySource2dCP geomSource);
};

//=======================================================================================
//! Parameterized base class for direct or indirect subclasses of DgnElement with
//! 2D geometry.
//! @ingroup DgnElementGroup
// @bsiclass                                                    Paul.Connelly   12/15
//=======================================================================================
template<typename T_Base> struct EXPORT_VTABLE_ATTRIBUTE GeometricElement2d : GeometricElement<T_Base, ElementGeom2d>
{
protected:
    typedef typename T_Base::CreateParams CreateParams;

    explicit GeometricElement2d(CreateParams const& params, DgnCategoryId category=DgnCategoryId(), Placement2dCR placement=Placement2d())
        : GeometricElement<T_Base, ElementGeom2d>(params, category, placement) { }

    virtual GeometrySource2dCP _ToGeometrySource2d() const override final {return this;}
    virtual void _CopyFrom(DgnElementCR rhs) override { T_Base::_CopyFrom(rhs); this->m_geom.CopyFrom(rhs.ToGeometrySource2d()); }
};

//! Specialization of GeometricElement2d deriving directly from the dgn:Element ECClass.
typedef GeometricElement2d<DgnElement> DgnElement2d;

//=======================================================================================
//! A 2-dimensional geometric element used to annotate drawings and sheets.
//! @ingroup DgnElementGroup
// @bsiclass                                                    Paul.Connelly   12/15
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE AnnotationElement : DgnElement2d
{
    DGNELEMENT_DECLARE_MEMBERS(DGN_CLASSNAME_AnnotationElement, DgnElement2d)
    friend struct dgn_ElementHandler::Annotation;
public:
    typedef ElementCreateParams2d CreateParams;

    //! Create a AnnotationElement from CreateParams.
    static AnnotationElementPtr Create(CreateParams const& params) {return new AnnotationElement(params);}
protected:
    virtual AnnotationElementCP _ToAnnotationElement() const override final {return this;}

    explicit AnnotationElement(CreateParams const& params) : T_Super(params, params.m_categoryId, params.m_placement) { }
}; // AnnotationElement

//=======================================================================================
//! A 2-dimensional geometric element used in drawings
//! @ingroup DgnElementGroup
// @bsiclass                                                    Paul.Connelly   12/15
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE DrawingElement : DgnElement2d
{
    DGNELEMENT_DECLARE_MEMBERS(DGN_CLASSNAME_DrawingElement, DgnElement2d)
    friend struct dgn_ElementHandler::Drawing;
public:
    typedef ElementCreateParams2d CreateParams;

    //! Create a DrawingElement from CreateParams.
    static DrawingElementPtr Create(CreateParams const& params) {return new DrawingElement(params);}
protected:
    virtual DrawingElementCP _ToDrawingElement() const override final {return this;}

    explicit DrawingElement(CreateParams const& params) : T_Super(params, params.m_categoryId, params.m_placement) { }
}; // DrawingElement

//=======================================================================================
//! A 2-dimensional geometric element used in sheets
//! @ingroup DgnElementGroup
// @bsiclass                                                    Paul.Connelly   12/15
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE SheetElement : DgnElement2d
{
    DGNELEMENT_DECLARE_MEMBERS(DGN_CLASSNAME_SheetElement, DgnElement2d)
    friend struct dgn_ElementHandler::Sheet;
public:
    typedef ElementCreateParams2d CreateParams;

    //! Create a SheetElement from CreateParams.
    static SheetElementPtr Create(CreateParams const& params) {return new SheetElement(params);}
protected:
    virtual SheetElementCP _ToSheetElement() const override final {return this;}

    explicit SheetElement(CreateParams const& params) : T_Super(params, params.m_categoryId, params.m_placement) { }
}; // SheetElement

//=======================================================================================
//! A SystemElement is used to model functional systems
//! @see SystemModel
//! @ingroup DgnElementGroup
// @bsiclass                                                    Shaun.Sewall    12/25
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE SystemElement : DgnElement
{
    DEFINE_T_SUPER(DgnElement);

protected:
    virtual SystemElementCP _ToSystemElement() const override final {return this;}
    explicit SystemElement(CreateParams const& params) : T_Super(params) {}
};

//=======================================================================================
//! Helper class for maintaining and querying the ElementGroupsMembers relationship
//! @see IElementGroup
//! @private
// @bsiclass                                                    Shaun.Sewall    10/15
//=======================================================================================
struct ElementGroupsMembers : NonCopyableClass
{
public:
    DGNPLATFORM_EXPORT static DgnDbStatus Insert(DgnElementCR group, DgnElementCR member);
    DGNPLATFORM_EXPORT static DgnDbStatus Delete(DgnElementCR group, DgnElementCR member);
    DGNPLATFORM_EXPORT static bool HasMember(DgnElementCR group, DgnElementCR member);
    DGNPLATFORM_EXPORT static DgnElementIdSet QueryMembers(DgnElementCR group);
    DGNPLATFORM_EXPORT static DgnElementIdSet QueryGroups(DgnElementCR member);
};

//=======================================================================================
//! Base interface to query a group (element) that has other elements as members
//! @see IElementGroupOf
//! @ingroup DgnElementGroup
// @bsiclass                                                    Shaun.Sewall    11/15
//=======================================================================================
struct IElementGroup
{
protected:
    //! Override to return the <em>this</em> pointer of the group DgnElement
    virtual DgnElementCP _ToGroupElement() const = 0;

public:
    //! Query for the members of this group
    DgnElementIdSet QueryMembers() const {return ElementGroupsMembers::QueryMembers(*_ToGroupElement());}
    //! Returns true if this group has the specified member
    bool HasMemberElement(DgnElementCR member) const {return ElementGroupsMembers::HasMember(*_ToGroupElement(), member);}
};

//=======================================================================================
//! Templated class used for an element to group other member elements and manage the
//! members of the group in a type-safe way.
//! @note Template type T must be a subclass of DgnElement.
//! @note The class that implements this interface must also be an element.
//! @ingroup DgnElementGroup
// @bsiclass                                                    Shaun.Sewall    10/15
//=======================================================================================
template<class T> class IElementGroupOf : public IElementGroup
{
protected:
    //! Called prior to member being added to group
    virtual DgnDbStatus _OnMemberAdd(T const& member) const {return DgnDbStatus::Success;}
    //! Called after member is added to group
    virtual void _OnMemberAdded(T const& member) const {}

    //! Called prior to member being removed from group
    virtual DgnDbStatus _OnMemberRemove(T const& member) const {return DgnDbStatus::Success;}
    //! Called after member removed from group
    virtual void _OnMemberRemoved(T const& member) const {}

    IElementGroupOf()
        {
        static_assert(std::is_base_of<DgnElement, T>::value, "IElementGroupOf can only group subclasses of DgnElement");
        }

public:
    //! Add a member to this group
    DgnDbStatus AddMember(T const& member) const
        {
        DgnElementCR groupElement = *_ToGroupElement();
        DgnElementCR memberElement = static_cast<DgnElementCR>(member); // see static_assert in constructor

        DgnDbStatus status = _OnMemberAdd(member);
        if (DgnDbStatus::Success != status)
            return status;

        status = ElementGroupsMembers::Insert(groupElement, memberElement);
        if (DgnDbStatus::Success != status)
            return status;

        _OnMemberAdded(member);
        return DgnDbStatus::Success;
        }

    //! Remove a member from this group
    DgnDbStatus RemoveMember(T const& member) const
        {
        DgnElementCR groupElement = *_ToGroupElement();
        DgnElementCR memberElement = static_cast<DgnElementCR>(member); // see static_assert in constructor

        DgnDbStatus status = _OnMemberRemove(member);
        if (DgnDbStatus::Success != status)
            return status;

        status = ElementGroupsMembers::Delete(groupElement, memberElement);
        if (DgnDbStatus::Success != status)
            return status;

        _OnMemberRemoved(member);
        return DgnDbStatus::Success;
        }
    
    //! Returns true if this group has the specified member
    bool HasMember(T const& member) const
        {
        DgnElementCR memberElement = static_cast<DgnElementCR>(member); // see static_assert in constructor
        return HasMemberElement(memberElement);
        }

    //! Query for the groups that the specified element is a member of
    static DgnElementIdSet QueryGroups(T const& member)
        {
        DgnElementCR memberElement = static_cast<DgnElementCR>(member); // see static_assert in constructor
        return ElementGroupsMembers::QueryGroups(memberElement);
        }
};

//=======================================================================================
//! @note Instead of using this class, create own subclass (somewhere in DgnElement hierarchy)
//! and use the IElementGroupOf interface to add grouping behavior to that subclass.
//! @note This class is only temporarily here to support DgnV8 conversion
//! @private
// @bsiclass                                                    Shaun.Sewall    05/15
//=======================================================================================
// WIP: Obsolete. Replaced by IElementGroup
struct EXPORT_VTABLE_ATTRIBUTE ElementGroup : DgnElement, IElementGroupOf<DgnElement>
{
    DGNELEMENT_DECLARE_MEMBERS(DGN_CLASSNAME_ElementGroup, DgnElement)

protected:
    Dgn::IElementGroupCP _ToIElementGroup() const override final {return this;}
    virtual Dgn::DgnElementCP _ToGroupElement() const override final {return this;}

public:
    explicit ElementGroup(CreateParams const& params) : T_Super(params) {}
};

//=======================================================================================
//! A DefinitionElement which resides in (and only in) a DefinitionModel.
//! @ingroup DgnElementGroup
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE DefinitionElement : DgnElement
{
    DEFINE_T_SUPER(DgnElement);

protected:
    virtual DefinitionElementCP _ToDefinitionElement() const override final {return this;}
    explicit DefinitionElement(CreateParams const& params) : T_Super(params) {}
};

//=======================================================================================
//! A DefinitionElement which resides in (and only in) the dictionary model.
//! Typically represents a style or similar resource used by other elements throughout
//! the DgnDb.
//! @ingroup DgnElementGroup
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE DictionaryElement : DefinitionElement
{
    DEFINE_T_SUPER(DefinitionElement);
public:
    //! Parameters used to construct a DictionaryElement
    struct CreateParams : T_Super::CreateParams
    {
        DEFINE_T_SUPER(DictionaryElement::T_Super::CreateParams);

        //! Constructs parameters for a dictionary element.
        //! @param[in] db       The DgnDb in which the element is to reside.
        //! @param[in] classId  The ID of the ECClass representing this element.
        //! @param[in] code     The element's unique code.
        //! @param[in] label    The element's label
        //! @param[in] parentId The ID of the element's parent element.
        DGNPLATFORM_EXPORT CreateParams(DgnDbR db, DgnClassId classId, Code const& code, Utf8CP label=nullptr, DgnElementId parentId=DgnElementId());

        //! Constructor from base class. Primarily for internal use.
        explicit CreateParams(DgnElement::CreateParams const& params) : T_Super(params) { }

        //! Constructs parameters for a dictionary element with the specified values. Chiefly for internal use.
        CreateParams(DgnDbR db, DgnModelId modelId, DgnClassId classId, Code code, Utf8CP label=nullptr, DgnElementId parent=DgnElementId())
            : T_Super(db, modelId, classId, code, label, parent) { }
    };
protected:
    DGNPLATFORM_EXPORT virtual DgnDbStatus _OnInsert() override;
    virtual DictionaryElementCP _ToDictionaryElement() const override final { return this; }

    explicit DictionaryElement(CreateParams const& params) : T_Super(params) { }
};

struct ECSqlClassInfo;

//=======================================================================================
//! The DgnElements for a DgnDb.
//! This class holds a cache of reference-counted DgnElements. All in-memory DgnElements for a DgnDb are held in its DgnElements member.
//! When the reference count of an element goes to zero, it is not immediately freed. Instead, it is held by this class
//! and may be "reclaimed" later if/when it is needed again. The memory held by DgnElements is not actually freed until
//! their reference count goes to 0 and the cache is subsequently purged.
//! @see DgnDb::Elements
//! @ingroup DgnElementGroup
//=======================================================================================
struct DgnElements : DgnDbTable, IMemoryConsumer
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

private:
    struct ElementSelectStatement
    {
        BeSQLite::EC::CachedECSqlStatementPtr m_statement;
        ECSqlClassParams const& m_params;

        ElementSelectStatement(BeSQLite::EC::CachedECSqlStatement* stmt, ECSqlClassParams const& params) : m_statement(stmt), m_params(params) { }
    };

    struct HandlerStatementCache
    {
    private:
        struct Entry
        {
            ElementHandlerP m_handler;
            BeSQLite::EC::CachedECSqlStatementPtr m_select;

            explicit Entry(ElementHandlerP handler=nullptr) : m_handler(handler) { }
        };

        typedef bvector<Entry> Entries;

        mutable Entries m_entries;

        Entry* FindEntry(ElementHandlerR handler) const;
    public:
        ElementSelectStatement GetPreparedSelectStatement(DgnElementR el, ElementHandlerR handler, ECSqlClassInfo const& classInfo) const;
        void Empty();
    };

    DgnElementId  m_nextAvailableId;
    struct ElemIdTree* m_tree;
    HeapZone m_heapZone;
    BeSQLite::StatementCache m_stmts;
    BeSQLite::SnappyFromBlob m_snappyFrom;
    BeSQLite::SnappyToBlob m_snappyTo;
    DgnElementIdSet m_selectionSet;
    mutable BeSQLite::BeDbMutex m_mutex;
    HandlerStatementCache m_handlerStmts;

    void OnReclaimed(DgnElementCR);
    void OnUnreferenced(DgnElementCR);
    void Destroy();
    void AddToPool(DgnElementCR) const;
    void DropFromPool(DgnElementCR) const;
    void SendOnLoadedEvent(DgnElementR elRef) const;
    void FinishUpdate(DgnElementCR replacement, DgnElementCR original);
    DgnElementCPtr LoadElement(DgnElement::CreateParams const& params, bool makePersistent) const;
    DgnElementCPtr LoadElement(DgnElementId elementId, bool makePersistent) const;
    void InitNextId();
    DgnElementCPtr PerformInsert(DgnElementR element, DgnDbStatus&);
    DgnDbStatus PerformDelete(DgnElementCR);
    explicit DgnElements(DgnDbR db);
    ~DgnElements();

    DGNPLATFORM_EXPORT DgnElementCPtr InsertElement(DgnElementR element, DgnDbStatus* stat);
    DGNPLATFORM_EXPORT DgnElementCPtr UpdateElement(DgnElementR element, DgnDbStatus* stat);

    ElementSelectStatement GetPreparedSelectStatement(DgnElementR el) const;
    BeSQLite::EC::CachedECSqlStatementPtr GetPreparedInsertStatement(DgnElementR el) const;
    BeSQLite::EC::CachedECSqlStatementPtr GetPreparedUpdateStatement(DgnElementR el) const;

    virtual int64_t _CalculateBytesConsumed() const override { return GetTotalAllocated(); }
    virtual int64_t _Purge(int64_t memTarget) override;
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
    DGNPLATFORM_EXPORT DgnElementId QueryElementIdByCode(DgnElement::Code const& code) const;

    //! Query for the DgnElementId of the element that has the specified code
    DGNPLATFORM_EXPORT DgnElementId QueryElementIdByCode(DgnAuthorityId codeAuthorityId, Utf8StringCR codeValue, Utf8StringCR nameSpace="") const;

    //! Query for the DgnElementId of the element that has the specified code
    DGNPLATFORM_EXPORT DgnElementId QueryElementIdByCode(Utf8CP codeAuthorityName, Utf8StringCR codeValue, Utf8StringCR nameSpace="") const;

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
    //! @note The element's code must be unique among all elements within the DgnDb, or this method will fail with DgnDbStatus::DuplicateCode.
    template<class T> RefCountedCPtr<T> Insert(T& element, DgnDbStatus* stat=nullptr) {return (T const*) InsertElement(element, stat).get();}

    //! Update the original persistent DgnElement from which the supplied DgnElement was copied.
    //! @param[in] modifiedElement The modified copy of the DgnElement to update.
    //! @param[in] stat An optional status value. Will be DgnDbStatus::Success if the update was successful, error status otherwise.
    //! @return RefCountedCPtr to the modified persistent element. Will be invalid if the update failed.
    //! @note This call returns a RefCountedCPtr to the *original* peristent element (which has now been updated to reflect the changes from
    //! modifiedElement). modifiedElement does *not* become persistent from this call.
    template<class T> RefCountedCPtr<T> Update(T& modifiedElement, DgnDbStatus* stat=nullptr) {return (T const*) UpdateElement(modifiedElement, stat).get();}

    //! Delete a DgnElement from this DgnDb.
    //! @param[in] element The element to delete.
    //! @return DgnDbStatus::Success if the element was deleted, error status otherwise.
    DGNPLATFORM_EXPORT DgnDbStatus Delete(DgnElementCR element);

    //! Delete a DgnElement from this DgnDb by DgnElementId.
    //! @return DgnDbStatus::Success if the element was deleted, error status otherwise.
    //! @note This method is merely a shortcut to #GetElement and then #Delete
    DgnDbStatus Delete(DgnElementId id) {auto el=GetElement(id); return el.IsValid() ? Delete(*el) : DgnDbStatus::NotFound;}

    //! Get the Heapzone for this DgnDb.
    HeapZone& GetHeapZone() {return m_heapZone;}

    DgnElementIdSet const& GetSelectionSet() const {return m_selectionSet;}
    DgnElementIdSet& GetSelectionSetR() {return m_selectionSet;}
};

//=======================================================================================
//! Can be used as the base class for an Item that uses an IECInstance to cache its properties in memory.
//! @note There is no AspectHandler for InstanceBackedItem. A class that derives from InstanceBackedItem must
//! register its own handler.
// @bsiclass                                                BentleySystems
//=======================================================================================
#ifdef WIP_ELEMENT_ITEM // *** pending redesign
struct InstanceBackedItem : DgnElement::Item
{
    ECN::IECInstancePtr m_instance;

    Utf8CP _GetECSchemaName() const override {return m_instance->GetClass().GetSchema().GetName().c_str();}
    Utf8CP _GetECClassName() const override {return m_instance->GetClass().GetName().c_str();}
    DGNPLATFORM_EXPORT DgnDbStatus _LoadProperties(DgnElementCR) override;
    DGNPLATFORM_EXPORT DgnDbStatus _UpdateProperties(DgnElementCR) override;
    DGNPLATFORM_EXPORT DgnDbStatus _GenerateElementGeometry(GeometricElementR el, GenerateReason) override;

    InstanceBackedItem() {}

    void SetInstanceId(BeSQLite::EC::ECInstanceId eid);
};
#endif

#ifdef WIP_COMPONENT_MODEL // *** Pending redesign
// *** WIP_ELEMENT_ITEM - move this back into ComponentSolution after making ElementItem a top-level class
DgnDbStatus ExecuteComponentSolutionEGA(DgnElementR el, DPoint3dCR origin, YawPitchRollAnglesCR angles, ECN::IECInstanceCR itemInstance, Utf8StringCR cmName, Utf8StringCR paramNames, DgnElement::Item& item);
#endif

//=======================================================================================
//! The basic element copier. Makes a persistent copy of elements and their children.
// @bsiclass                                                BentleySystems
//=======================================================================================
struct ElementCopier
{
protected:
    DgnCloneContext& m_context;
    bool m_copyChildren;

public:
    DGNPLATFORM_EXPORT ElementCopier(DgnCloneContext& c);

    DgnCloneContext& GetCloneContext() {return m_context;}

    //! Specify if children should be deep-copied or not. The default is yes, deep-copy children.
    void SetCopyChildren(bool b) {m_copyChildren=b;}

    //! Make a persistent copy of a specified Physical element, along with all of its children.
    //! @param[out] stat        Optional. If not null, then an error code is stored here in case the copy fails.
    //! @param[in] targetModel  The model where the instance is to be inserted
    //! @param[in] sourceElement The element that is to be copied
    //! @param[in] code         The code to assign to the new element. If invalid, then a code will be generated by the sourceElement's CodeAuthority
    //! @param[in] newParentId  Optional. The element that should be the parent of the new element. If not specified, then the parent of the new element
    //!                             will either be the parent of the source element or the element to which the source parent has been remapped. See DgnCloneContext.
    //! @return a new element if successful
    DGNPLATFORM_EXPORT DgnElementCPtr MakeCopy(DgnDbStatus* stat, DgnModelR targetModel, DgnElementCR sourceElement, DgnElement::Code const& code, DgnElementId newParentId = DgnElementId());
};

//=======================================================================================
//! Utility to collect editable elements.
//! Order is \em not preserved.
//! The collection holds only one copy of an element with a given ElementId.
// @bsiclass                                                BentleySystems
//=======================================================================================
struct DgnEditElementCollector
{
protected:
     bset<DgnElementP> m_elements; // The editable elements in the set. We manage their refcounts as we add and remove them 
     bmap<DgnElementId,DgnElementP> m_ids; // The Elements in the set that have IDs. Child elements will always have IDs. Some top-level elements may not have an ID.

public:
    DGNPLATFORM_EXPORT DgnEditElementCollector();
    DGNPLATFORM_EXPORT ~DgnEditElementCollector();

    //! Add the specified editable copy of an element to the collection. 
    //! @param el  The editable copy to be added
    //! @return The element that is in the collection. 
    DGNPLATFORM_EXPORT DgnElementPtr AddElement(DgnElementR el);

    //! Add editable copies of the children of an element to the collection. 
    //! @param el  The parent element to be queried
    //! @param maxDepth The levels of child elements to add. Pass 1 to add only the immediate children.
    DGNPLATFORM_EXPORT void AddChildren(DgnElementCR el, size_t maxDepth = std::numeric_limits<size_t>::max());
    
    //! Add an element and editable copies of its children to the collection
    //! @param el       The parent element to be added and queried
    //! @param maxDepth The levels of child elements to add. Pass 1 to add only the immediate children.
    DGNPLATFORM_EXPORT void AddAssembly(DgnElement& el, size_t maxDepth = std::numeric_limits<size_t>::max()) {AddElement(el); AddChildren(el, maxDepth);}

    //! Add an editable copy of the specified element to the collection.
    //! If the collection already contains an element with the same ElementId, then \a el is not added and the existing element is returned.
    //! @param el  The element to be edited
    //! @return The element that is in the collection or nullptr if the element could not be copied for edit.
    DGNPLATFORM_EXPORT DgnElementPtr EditElement(DgnElementCR el) {auto ee = el.CopyForEdit(); if (ee.IsValid()) return AddElement(*ee); else return nullptr;}
    
    //! Add an editable copy of the specified element and its children to the collection.
    //! @param el       The element to be added
    //! @param maxDepth The levels of child elements to add. Pass 1 to add only the immediate children.
    DGNPLATFORM_EXPORT void EditAssembly(DgnElementCR el, size_t maxDepth = std::numeric_limits<size_t>::max()) {auto ee = el.CopyForEdit(); if (ee.IsValid()) AddAssembly(*ee, maxDepth);}

    //! Look up the editable copy of an element in the collection by its ElementId.
    //! @return The element that is in the collection or nullptr if not found.
    DGNPLATFORM_EXPORT DgnElementPtr FindElementById(DgnElementId);

    //! Remove the specified editable copy of an element from the collection.
    //! @param el  The editable copy of an element in the collection
    //! @note \a el must be the editable copy of the element that is in this collection.
    //! @see FindElementById 
    DGNPLATFORM_EXPORT void RemoveElement(DgnElementR el);

    //! Remove an element's children (by ID) from the collection.
    //! @param el  The parent element to query.
    //! @param maxDepth The levels of child elements to add. Pass 1 to add only the immediate children.
    DGNPLATFORM_EXPORT void RemoveChildren(DgnElementCR el, size_t maxDepth = std::numeric_limits<size_t>::max());
    
    //! Remove the specified editable copy of an element and its children (by ID) from the collection.
    //! @param el  The editable copy of the parent element to remove and to query.
    //! @param maxDepth The levels of child elements to add. Pass 1 to add only the immediate children.
    DGNPLATFORM_EXPORT void RemoveAssembly(DgnElementR el, size_t maxDepth = std::numeric_limits<size_t>::max()) {RemoveElement(el); RemoveChildren(el, maxDepth);}

    //! Get the number of elements currently in the collection
    size_t size() {return m_elements.size();}

    //! Get an iterator pointing to the beginning of the collection
    bset<DgnElementP>::const_iterator begin() const {return m_elements.begin();}

    //! Get an iterator pointing to the end of the collection
    bset<DgnElementP>::const_iterator end() const {return m_elements.end();}

    //! Insert or update all elements in the collection. Elements with valid ElementIds are updated. Elements with no ElementIds are inserted. 
    //! @return non-zero error status if any insert or update fails. In that case some elements in the collection may not be written.
    DGNPLATFORM_EXPORT DgnDbStatus Write();
};

//=======================================================================================
//! Applies a transform one or more  elements
// @bsiclass                                                BentleySystems
//=======================================================================================
struct DgnElementTransformer
{
    DGNPLATFORM_EXPORT static DgnDbStatus ApplyTransformTo(DgnElementR el, Transform const& t); 

    template<typename COLL>
    static DgnDbStatus ApplyTransformToAll(COLL& collection, Transform const& t) 
        {
        for (auto& item : collection)
            {
            DgnDbStatus status = ApplyTransformTo(*item, t);
            if (DgnDbStatus::Success != status)
                return status;
            }
        return DgnDbStatus::Success;
        }
};

END_BENTLEY_DGNPLATFORM_NAMESPACE
