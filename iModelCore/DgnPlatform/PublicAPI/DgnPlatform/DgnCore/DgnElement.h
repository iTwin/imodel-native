/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/DgnElement.h $
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

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

namespace dgn_ElementHandler {struct Element; struct Physical;};
namespace dgn_TxnTable {struct Element;};

struct MultiAspectMux;

typedef RefCountedPtr<ElementGeometry> ElementGeometryPtr;

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
    friend struct EditElementHandle;
    friend struct dgn_ElementHandler::Element;
    friend struct dgn_TxnTable::Element;

    //! Parameters for creating new DgnElements
    struct CreateParams
    {
        DgnModelR       m_model;
        DgnClassId      m_classId;
        DgnCategoryId   m_categoryId;
        Utf8CP          m_code;
        Utf8CP          m_label;
        DgnElementId    m_id;
        DgnElementId    m_parentId;
        double          m_lastModTime;
        CreateParams(DgnModelR model, DgnClassId classId, DgnCategoryId category, Utf8CP label=nullptr, Utf8CP code=nullptr, DgnElementId id=DgnElementId(), 
                     DgnElementId parent=DgnElementId(), double lastModTime=0.0) :
                    m_model(model), m_classId(classId), m_categoryId(category), m_label(label), m_code(code), m_id(id), m_parentId(parent), m_lastModTime(lastModTime) {}

        void SetLabel(Utf8CP label) {m_label = label;}  //!< Set the label for DgnElements created with this CreateParams
        void SetCode(Utf8CP code) {m_code = code;}      //!< Set the code for DgnElements created with this CreateParams
        void SetParentId(DgnElementId parent) {m_parentId=parent;} //!< Set the ParentId for DgnElements created with this CreateParams
    };

    //! The Hilite state of a DgnElement. If an element is "hilited", its appearance is changed to call attention to it.
    enum class Hilited : uint8_t
    {
        None         = 0, //!< the element is displayed normally (not hilited)
        Normal       = 1, //!< the element is displayed using the normal hilite appearance
        Dashed       = 2,
        Background   = 3, //!< the element is displayed with the background color
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

        //! Called after the element was Deleted.
        //! @param[in]  el the DgnElement that was deleted
        //! @return true to drop this appData, false to leave it attached to the DgnElement.
        virtual DropMe _OnDeleted(DgnElementCR el) {return DropMe::Yes;}
    };

    //! Buffers changes to a dgn.ElementAspect in memory and writes out the changes when the host DgnElement is inserted or updated.
    //! A domain that defines an subclass of dgn.ElementItem or ElementAspect in the schema should normally also define a subclass of Aspect to manage changes to the instances.
    //! A domain will normally subclass one of the following more specific subclasses:
    //!     * Item - a domain that defines a subclass of dgn.ElementItem should define a subclass of Item.
    //!     * UniqueAspect - a domain that defines a subclass of dgn.ElementAspect that must be 1:1 with the host element should define a subclass of UniqueAspect.
    //!     * MultiAspect - a domain that defines a subclass of dgn.ElementAspect that can associate multiple instances of the class with the host element should define a subclass of MultiAspect.
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
        DGNPLATFORM_EXPORT DgnClassId GetThisECClassId(DgnDbR);
        ECN::ECClassCP GetThisECClass(DgnDbR);
        Utf8String  GetFullEcSqlClassName() {return _GetECSchemaName().append(".").append(_GetECClassName());}

        DGNPLATFORM_EXPORT Aspect();

        //! The subclass must implement this method to return the full class name (in ECSql schema.class format) of the instance.
        virtual Utf8String _GetECSchemaName() = 0;

        //! The subclass must implement this method to return the full class name (in ECSql schema.class format) of the instance.
        virtual Utf8String _GetECClassName() = 0;

        //! The subclass must implement this method to report an existing instance on the host element that this instance will replace.
        virtual BeSQLite::EC::ECInstanceKey _QueryExistingInstanceKey(DgnElementCR) = 0;

        //! The subclass must override this method in order to insert an empty instance into the Db and associates it with the host element.
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
        DGNPLATFORM_EXPORT void Delete() {m_changeType = ChangeType::Delete;}
    };

    //! Base class dgn.ElementAspects other than the dgn.ElementItem
    struct EXPORT_VTABLE_ATTRIBUTE NonItemAspect : Aspect
    {
    protected:
        BeSQLite::EC::ECInstanceId m_instanceId;

        DGNPLATFORM_EXPORT DgnDbStatus _DeleteInstance(DgnElementCR el) override final;
        DGNPLATFORM_EXPORT DgnDbStatus _InsertInstance(DgnElementCR el) override final;

    public:
        //! Get the ID of this aspect
        BeSQLite::EC::ECInstanceId GetInstanceId() const {return m_instanceId;}
    };

    //! Represents a non-Item ElementAspect subclass for the case where the host Element can have only 1 instance of the subclass.
    //! A subclass of UniqueAspect must override the following methods:
    //!     * _GetECSchemaName
    //!     * _GetECClassName
    //!     * _UpdateProperties
    //!     * _LoadProperties
    //! @see Item
    //! @see MultiAspect
    //! @note A domain that defines a subclass of UniqueAspect must also define a subclass of ElementAspectHandler to load it.
    struct EXPORT_VTABLE_ATTRIBUTE UniqueAspect : NonItemAspect
    {
    private:
        static Key& GetKey(ECN::ECClassCR cls) {return *(Key*)&cls;}
        Key& GetKey(DgnDbR db) {return GetKey(*GetThisECClass(db));}
        static UniqueAspect* Find(DgnElementCR, ECN::ECClassCR);
        static UniqueAspect* Load(DgnElementCR, ECN::ECClassCR);
        DGNPLATFORM_EXPORT BeSQLite::EC::ECInstanceKey _QueryExistingInstanceKey(DgnElementCR) override final;
        static void SetAspect0(DgnElementCR el, UniqueAspect& aspect);
        
    public:
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

        //! Query for an existing instance of the specified aspect class that is associated with the specified element.
        //! @param el   The host element
        //! @param ecclass The class of ElementAspect to look for
        //! @return the ID of the existing instance or an invalid ID if the host element has no instance of this aspect class
        static BeSQLite::EC::ECInstanceId QueryExistingInstanceId(DgnElementCR el, DgnClassId ecclass);
        };

    //! Represents a non-Item ElementAspect subclass in the case where the host Element can have multiple instances of the subclass.
    //! Use ECSql to query existing instances and their properties. Use GetAspectP or GetP to buffer changes to a particular instance.
    //! <p>A subclass of MultiAspect must override the following methods:
    //!     * _GetECSchemaName
    //!     * _GetECClassName
    //!     * _UpdateProperties
    //!     * _LoadProperties
    //! @see Item, UniqueAspect
    //! (Note: This is not stored directly as AppData, but is held by an AppData that aggregates instances for this class.)
    //! @note A domain that defines a subclass of MultiAspect must also define a subclass of ElementAspectHandler to load it.
    struct EXPORT_VTABLE_ATTRIBUTE MultiAspect : NonItemAspect
    {
    private:
        DGNPLATFORM_EXPORT BeSQLite::EC::ECInstanceKey _QueryExistingInstanceKey(DgnElementCR) override final;
    public:
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

    //! Buffers changes to a dgn.ElementItem.
    //! dgn.ElementItem is a special kind of dgn.ElementAspect. A dgn.Element can have 0 or 1 dgn.ElementItems, and the dgn.ElementItem always has the ID as the host dgn.Element.
    //! Note that the item's actual class can vary, as long as it is a subclass of dgn.ElementItem.
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
    //! @see UniqueAspect, MultiAspect
    //! @note A domain that defines a subclass of Item must also define a subclass of ElementAspectHandler to load it.
    struct EXPORT_VTABLE_ATTRIBUTE Item : Aspect
    {
    private:
        static Key s_key;

        static Item* Find(DgnElementCR);
        static Item* Load(DgnElementCR);

        DGNPLATFORM_EXPORT DgnDbStatus _DeleteInstance(DgnElementCR el) override final;
        DGNPLATFORM_EXPORT DgnDbStatus _InsertInstance(DgnElementCR el) override final;
        DGNPLATFORM_EXPORT BeSQLite::EC::ECInstanceKey _QueryExistingInstanceKey(DgnElementCR) override final;
        DGNPLATFORM_EXPORT DgnDbStatus _OnInsert(DgnElementR el) override final {return CallGenerateElementGeometry(el);}
        DGNPLATFORM_EXPORT DgnDbStatus _OnUpdate(DgnElementR el, DgnElementCR original) override final {return CallGenerateElementGeometry(el);}
        static void SetItem0(DgnElementCR el, Item& item);
        DgnDbStatus CallGenerateElementGeometry(DgnElementR);

    protected:
        //! The subclass must implement this method to generate geometry and store it on \a el.
        //! The platform invokes _GenerateElementGeometry just \em before an element is inserted and/or updated.
        //! @param el   The element to be updated.
        virtual DgnDbStatus _GenerateElementGeometry(GeometricElementR el) = 0;

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
        DGNPLATFORM_EXPORT static DgnDbStatus GenerateElementGeometry(GeometricElementR el);
    };

    DEFINE_BENTLEY_NEW_DELETE_OPERATORS

private:
    void UpdateLastModTime();
    template<class T> void CallAppData(T const& caller) const;

protected:
    struct Flags
        {
        uint32_t m_persistent:1;
        uint32_t m_editable:1;
        uint32_t m_lockHeld:1;
        uint32_t m_inSelectionSet:1;
        uint32_t m_hilited:3;
        uint32_t m_undisplayed:1;
        Flags() {memset(this, 0, sizeof(*this));}
        };

    mutable BeAtomic<uint32_t> m_refCount;
    DgnElementId    m_elementId;
    DgnElementId    m_parentId;
    DgnModelR       m_dgnModel;
    DgnClassId      m_classId;
    DgnCategoryId   m_categoryId;
    Utf8String      m_code;
    Utf8String      m_label;
    double          m_lastModTime;
    mutable Flags   m_flags;
    mutable bmap<AppData::Key const*, RefCountedPtr<AppData>, std::less<AppData::Key const*>, 8> m_appData;

    friend struct MultiAspect;

    void SetPersistent(bool val) const {m_flags.m_persistent = val;} //!< @private

    DGNPLATFORM_EXPORT virtual ~DgnElement();

    //! Called to load properties of a DgnElement from the DgnDb. Override to load subclass properties.
    //! @note If you override this method, you @em must call T_Super::_LoadFromDb, forwarding its status.
    virtual DgnDbStatus _LoadFromDb() {return DgnDbStatus::Success;}

    //! Called when an element is about to be inserted into the DgnDb.
    //! @return DgnDbStatus::Success to allow the insert, otherwise it will fail with the returned status.
    //! @note If you override this method, you @em must call T_Super::_OnInsert, forwarding its status.
    DGNPLATFORM_EXPORT virtual DgnDbStatus _OnInsert();

    //! Called to insert a new DgnElement into the DgnDb. Override to save subclass properties.
    //! @note If you override this method, you @em must call T_Super::_InsertInDb, forwarding its status.
    DGNPLATFORM_EXPORT virtual DgnDbStatus _InsertInDb();

    //! Called after a DgnElement was successfully inserted into the database.
    //! @note If you override this method, you @em must call T_Super::_OnInserted.
    DGNPLATFORM_EXPORT virtual void _OnInserted(DgnElementP copiedFrom) const;

    //! Called when this element is about to be replace an original element in the DgnDb.
    //! @param [in] original the original state of this element.
    //! Subclasses may override this method to control whether their instances are updated.
    //! @return DgnDbStatus::Success to allow the update, otherwise the update will fail with the returned status.
    //! @note If you override this method, you @em must call T_Super::_OnUpdate, forwarding its status.
    DGNPLATFORM_EXPORT virtual DgnDbStatus _OnUpdate(DgnElementCR original);

    //! Called to update a DgnElement in the DgnDb with new values. Override to update subclass properties.
    //! @note This method is called from DgnElements::Update, on the persistent element, after its values have been
    //! copied from the modified version. If the update fails, the original data will be copied back into this DgnElement.
    //! @note If you override this method, you @em must call T_Super::_UpdateInDb, forwarding its status.
    DGNPLATFORM_EXPORT virtual DgnDbStatus _UpdateInDb();

    //! Called after a DgnElement was successfully updated. The element will be in its post-updated state.
    //! @note If you override this method, you @em must call T_Super::_OnUpdated.
    DGNPLATFORM_EXPORT virtual void _OnUpdated(DgnElementCR original) const;

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

    //! Called when a new element is to be inserted into a DgnDb with this element as its parent.
    //! Subclasses may override this method to control which other elements may become children.
    //! @param[in] child the new element that will become a child of this element.
    //! @return DgnDbStatus::Success to allow the child insert, otherwise it will fail with the returned status.
    //! @note implementers should not presume that returning DgnDbStatus::Success means that the element will become a child element,
    //! since the insert may fail for other reasons. Instead, rely on _OnChildInserted for that purpose.
    //! @note If you override this method, you @em must call T_Super::_OnChildInsert, forwarding its status.
    virtual DgnDbStatus _OnChildInsert(DgnElementCR child) const {return DgnDbStatus::Success;}

    //! Called when an element that has this element as its parent is about to be updated in the DgnDb.
    //! Subclasses may override this method to control modifications to its children.
    //! @param [in] original element in its original state
    //! @param [in] replacement the child element in its modified state
    //! @return DgnDbStatus::Success to allow the child update, otherwise it will fail with the returned status.
    //! @note implementers should not presume that returning DgnDbStatus::Success means that the element was updated,
    //! since the update may fail for other reasons. Instead, rely on _OnChildUpdated for that purpose.
    //! @note If you override this method, you @em must call T_Super::_OnChildUpdate, forwarding its status.
    virtual DgnDbStatus _OnChildUpdate(DgnElementCR original, DgnElementCR replacement) const {return DgnDbStatus::Success;}

    //! Called when an child element of this element is about to be deleted from the DgnDb.
    //! Subclasses may override this method to block deletion of their children.
    //! @param[in] child that will be deleted.
    //! @return DgnDbStatus::Success to allow the child deletion, otherwise it will fail with the returned status.
    //! @note implementers should not presume that returning DgnDbStatus::Success means that the element was deleted,
    //! since the delete may fail for other reasons. Instead, rely on _OnChildDeleted for that purpose.
    //! @note If you override this method, you @em must call T_Super::_OnChildDelete, forwarding its status.
    virtual DgnDbStatus _OnChildDelete(DgnElementCR child) const {return DgnDbStatus::Success;}

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

    //! Get the size, in bytes, used by this DgnElement. This is used by the element memory management routines to gauge the "weight" of
    //! each element, so it is not necessary for the value to be 100% accurate.
    //! @note Subclasses of DgnElement that add any member variables should override this method using this template:
    //! uint32_t _GetMemSize() const override {return T_Super::_GetMemSize() + (sizeof(*this) - sizeof(T_Super)) + myAllocedSize;}
    //! where "myAllocedSize" is the number of bytes allocated for this element, held through member variable pointers.
    virtual uint32_t _GetMemSize() const {return sizeof(*this);}

    //! Virtual assignment method. If your subclass has member variables, it @b must override this method and copy those values from @a source.
    //! @param[in] source The element from which to copy
    //! @note If you override this method, you @b must call T_Super::_CopyFrom, forwarding its status (that is, only return DgnDbStatus::Success if both your
    //! implementation and your superclass succeed.)
    //! @note Implementers should be aware that your element starts in a valid state. Be careful to free existing state before overwriting it. Also note that
    //! @a source is not necessarily the same type as this DgnElement. See notes at CopyFrom.
    DGNPLATFORM_EXPORT virtual void _CopyFrom(DgnElementCR source);

    //! Get the display label (for use in the GUI) for this DgnElement.
    //! The default implementation returns the label if set or the code if the label is not set.
    //! Override to generate the display label in a different way.
    virtual Utf8String _GetDisplayLabel() const {return GetLabel() ? GetLabel() : GetCode();}

    //! Change the parent (owner) of this DgnElement.
    //! The default implementation sets the parent without doing any checking.
    //! @return DgnDbStatus::Success if the parentId was changed, error status otherwise.
    //! Override to validate the parent/child relationship and return a value other than DgnDbStatus::Success to reject proposed new parent.
    virtual DgnDbStatus _SetParentId(DgnElementId parentId) {m_parentId = parentId; return DgnDbStatus::Success;}

    //! Change the category of this DgnElement.
    //! The default implementation sets the category without doing any checking.
    //! Override to validate the category.
    //! @return DgnDbStatus::Success if the categoryId was changed, error status otherwise.
    virtual DgnDbStatus _SetCategoryId(DgnCategoryId categoryId) {m_categoryId = categoryId; return DgnDbStatus::Success;}

    //! Change the code of this DgnElement.
    //! The default implementation sets the code without doing any checking.
    //! Override to validate the code.
    //! @return DgnDbStatus::Success if the code was changed, error status otherwise.
    virtual DgnDbStatus _SetCode(Utf8CP code) {m_code.AssignOrClear(code); return DgnDbStatus::Success;}

    //! Override to customize how the DgnElement subclass generates its code.
    DGNPLATFORM_EXPORT virtual Utf8String _GenerateDefaultCode();

    virtual GeometricElementCP _ToGeometricElement() const {return nullptr;}
    virtual DgnElement3dCP _ToElement3d() const {return nullptr;}
    virtual DgnElement2dCP _ToElement2d() const {return nullptr;}
    virtual PhysicalElementCP _ToPhysicalElement() const {return nullptr;}
    virtual DrawingElementCP _ToDrawingElement() const {return nullptr;}
    virtual ElementGroupCP _ToElementGroup() const {return nullptr;}

    //! Construct a DgnElement from its params
    DGNPLATFORM_EXPORT explicit DgnElement(CreateParams const& params);

    DGNPLATFORM_EXPORT void ClearAllAppData(); //!< @private

public:
    DGNPLATFORM_EXPORT void SetInSelectionSet(bool yesNo) const; //!< @private

    DGNPLATFORM_EXPORT void AddRef() const;  //!< @private
    DGNPLATFORM_EXPORT void Release() const; //!< @private
    uint32_t GetRefCount() const {return m_refCount.load();} //!< Get the current reference count for this DgnElement.

    GeometricElementCP ToGeometricElement() const {return _ToGeometricElement();} //!< more efficient substitute for dynamic_cast<GeometricElementCP>(el)
    DgnElement3dCP ToElement3d() const {return _ToElement3d();}                   //!< more efficient substitute for dynamic_cast<DgnElement3dCP>(el)
    DgnElement2dCP ToElement2d() const {return _ToElement2d();}                   //!< more efficient substitute for dynamic_cast<DgnElement2dCP>(el)
    PhysicalElementCP ToPhysicalElement() const {return _ToPhysicalElement();}    //!< more efficient substitute for dynamic_cast<PhysicalElementCP>(el)
    DrawingElementCP ToDrawingElement() const {return _ToDrawingElement();}       //!< more efficient substitute for dynamic_cast<DrawingElementCP>(el)
    ElementGroupCP ToElementGroup() const {return _ToElementGroup();}             //!< more efficient substitute for dynamic_cast<ElementGroupCP>(el)

    GeometricElementP ToGeometricElementP() {return const_cast<GeometricElementP>(_ToGeometricElement());} //!< more efficient substitute for dynamic_cast<GeometricElementP>(el)
    DgnElement3dP ToElement3dP() {return const_cast<DgnElement3dP>(_ToElement3d());}                       //!< more efficient substitute for dynamic_cast<DgnElement3dP>(el)
    DgnElement2dP ToElement2dP() {return const_cast<DgnElement2dP>(_ToElement2d());}                       //!< more efficient substitute for dynamic_cast<DgnElement2dP>(el)
    PhysicalElementP ToPhysicalElementP() {return const_cast<PhysicalElementP>(_ToPhysicalElement());}     //!< more efficient substitute for dynamic_cast<PhysicalElementP>(el)
    DrawingElementP ToDrawingElementP() {return const_cast<DrawingElementP>(_ToDrawingElement());}         //!< more efficient substitute for dynamic_cast<DrawingElementP>(el)
    ElementGroupP ToElementGroupP() {return const_cast<ElementGroupP>(_ToElementGroup());}                 //!< more efficient substitute for dynamic_cast<ElementGroupP>(el)

    bool Is3d() const {return nullptr != _ToElement3d();} //!< Determine whether this element is 3d or not
    bool IsSameType(DgnElementCR other) {return m_classId == other.m_classId;}//!< Determine whether this element is the same type (has the same DgnClassId) as another element.

    Hilited IsHilited() const {return (Hilited) m_flags.m_hilited;} //!< Get the current Hilited state of this element
    void SetHilited(Hilited newState) const {m_flags.m_hilited = (uint8_t) newState;} //!< Change the current Hilited state of this element

    //! Determine whether this is a copy of the "persistent state" (i.e. an exact copy of what is saved in the DgnDb) of a DgnElement.
    //! @note If this flag is true, this element must be readonly. To modify an element, call CopyForEdit.
    bool IsPersistent() const {return m_flags.m_persistent;}

    //! Test if the element is currently in the selection set.
    bool IsInSelectionSet() const {return m_flags.m_inSelectionSet;}

    //! Test if the element is not displayed.
    bool IsUndisplayed() const {return m_flags.m_undisplayed;}

    //! Set this element's undisplayed flag
    void SetUndisplayedFlag(bool yesNo) {m_flags.m_undisplayed = yesNo;}

    //! Copy the content of another DgnElement into this DgnElement.
    //! @param[in] source The other element whose content is copied into this element.
    //! @note This method @b does @b not change the DgnClassId, DgnModel or DgnElementId of this DgnElement. If the type of @a source is different
    //! than this element, then all of the data from subclasses in common are copied and the remaining data on this DgnElement are unchanged.
    void CopyFrom(DgnElementCR source) {_CopyFrom(source);}

    //! Make a writable copy of this DgnElement so that the copy may be edited.
    //! @return a DgnElementPtr that holds the editable copy of this element.
    //! @note This method may only be used on a DgnElement this is the readonly persistent element returned by DgnElements::GetElement, and then
    //! only one editing copy of this element at a time may exist. If another copy is extant, this method will return an invalid DgnElementPtr.
    //! @see MakeCopy IsPersistent
    DGNPLATFORM_EXPORT DgnElementPtr CopyForEdit() const;

    //! Make a writable copy of this DgnElement so that the copy may be edited.
    //! This is merely a templated shortcut to dynamic_cast the return of CopyForEdit to a subclass of DgnElement.
    template<class T> RefCountedPtr<T> MakeCopy() const {return dynamic_cast<T*>(CopyForEdit().get());}

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

/** @name AppData Management */
/** @{ */
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
/** @} */

    //! Get the DgnModel of this DgnElement.
    DgnModelR GetDgnModel() const {return m_dgnModel;}

    //! Get the DgnDb of this element.
    //! @note This is merely a shortcut for GetDgnModel().GetDgnDb().
    DGNPLATFORM_EXPORT DgnDbR GetDgnDb() const;

    //! Get the DgnElementId of this DgnElement
    DgnElementId GetElementId() const {return m_elementId;}

    //! Invalidate the ElementId of this element.
    //! This can be used to clear the ElementId of this element before inserting a copy of it (otherwise Insert on the copy will fail.)
    void InvalidateElementId() {m_elementId = DgnElementId();}

    //! Get the DgnClassId of this DgnElement.
    DgnClassId GetElementClassId() const {return m_classId;}

    //! Get the DgnElementKey (the element DgnClassId and DgnElementId) of this DgnElement
    DgnElementKey GetElementKey() const {return DgnElementKey(GetElementClassId(), GetElementId());}

    //! Get a pointer to the ECClass of this DgnElement.
    DGNPLATFORM_EXPORT ECN::ECClassCP GetElementClass() const;

    //! Static method to Query the DgnClassId of the dgn.Element ECClass in the specified DgnDb.
    //! @note This is a static method that always returns the DgnClassId of the dgn.Element class - it does @em not return the class of a specific instance.
    //! @see GetElementClassId
    DGNPLATFORM_EXPORT static DgnClassId QueryClassId(DgnDbR db);

    //! Get the DgnElementId of the parent of this element.
    //! @see SetParentId
    //! @return Id will be invalid if this element does not have a parent element.
    DgnElementId GetParentId() const {return m_parentId;}

    //! Set the parent (owner) of this DgnElement.
    //! @see GetParentId, _SetParentId
    //! @return DgnDbStatus::Success if the parent was set
    //! @note This call can fail if a DgnElement subclass overrides _SetParentId and rejects the parent.
    DgnDbStatus SetParentId(DgnElementId parentId) {return _SetParentId(parentId);}

    //! Get the category of this DgnElement.
    //! @see SetCategoryId
    DgnCategoryId GetCategoryId() const {return m_categoryId;}

    //! Set the category of this DgnElement.
    //! @see GetCategoryId, _SetCategoryId
    //! @return DgnDbStatus::Success if the category was set
    //! @note This call can fail if a subclass overrides _SetCategoryId and rejects the category.
    DgnDbStatus SetCategoryId(DgnCategoryId categoryId) {return _SetCategoryId(categoryId);}

    //! Get the code (business key) of this DgnElement.
    Utf8CP GetCode() const {return m_code.c_str();}

    //! Set the code (business key) of this DgnElement.
    //! @see GetCode, _SetCode
    //! @return DgnDbStatus::Success if the code was set
    //! @note This call can fail if a subclass overrides _SetCode and rejects the code.
    DgnDbStatus SetCode(Utf8CP code) {return _SetCode(code);}

    //! Get the optional label (user-friendly name) of this DgnElement.
    Utf8CP GetLabel() const {return m_label.c_str();}

    //! Set the label (user-friendly name) of this DgnElement.
    void SetLabel(Utf8CP label) {m_label.AssignOrClear(label);}

    //! Get the time this element was last modified.
    //! @note the time is in UTC Julian days.
    double GetLastModifiedTime() const {return m_lastModTime;}

    //! Get the last modified time as a DateTime timestamp
    DateTime GetTimeStamp() const {DateTime timestamp; DateTime::FromJulianDay(timestamp, m_lastModTime, DateTime::Info(DateTime::Kind::Utc)); return timestamp;}

    //! Get the display label (for use in the GUI) of this DgnElement.
    //! @note The default implementation returns the label if it is set or the code if the label is not set.
    //! @see GetLabel, GetCode, _GetDisplayLabel
    Utf8String GetDisplayLabel() const {return _GetDisplayLabel();}

    //! Query the DgnDb for the children of this DgnElement.
    //! @return DgnElementIdSet containing the DgnElementIds of all child elements of this DgnElement. Will be empty if no children.
    DGNPLATFORM_EXPORT DgnElementIdSet QueryChildren() const;

    //! Create a new instance of a DgnElement using the specified CreateParams.
    //! @note This is a static method that only creates instances of the DgnElement class. To create instances of subclasses,
    //! use a static method on the subclass.
    static DgnElementPtr Create(CreateParams const& params) {return new DgnElement(params);}

};

//=======================================================================================
//! A stream of geometry, stored on a DgnElement, created by an ElementGeometryBuilder.
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

    //! Reserve memory for this GeomStream.
    //! @param[in] size the number of bytes to reserve
    DGNPLATFORM_EXPORT void ReserveMemory(uint32_t size);
    //! Save a stream of geometry into this GeomStream.
    //! @param[in] data the data to save
    //! @param[in] size number of bytes in data
    DGNPLATFORM_EXPORT void SaveData(uint8_t const* data, uint32_t size);
};

//=======================================================================================
//! The position, orientation, and size of a DgnElement3d.
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
//! The position, rotation angle, and size of a DgnElement2d.
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
//! A DgnElement that has a Geometry Aspect.
//! @note This an abstract class. Subclasses DgnElement2d and DgnElement3d provide concrete implementations.
//! @ingroup DgnElementGroup
// @bsiclass                                                    Keith.Bentley   04/15
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE GeometricElement : DgnElement
{
    DEFINE_T_SUPER(DgnElement);

protected:
    GeomStream m_geom;

    DGNPLATFORM_EXPORT DgnDbStatus _LoadFromDb() override;
    DGNPLATFORM_EXPORT DgnDbStatus _InsertInDb() override;
    DGNPLATFORM_EXPORT DgnDbStatus _UpdateInDb() override;
    DGNPLATFORM_EXPORT void _CopyFrom(DgnElementCR) override;
    virtual DgnDbStatus _BindPlacement(BeSQLite::Statement&) = 0;
    GeometricElementCP _ToGeometricElement() const override {return this;}
    DgnDbStatus WriteGeomStream(BeSQLite::Statement&, DgnDbR);
    explicit GeometricElement(CreateParams const& params) : T_Super(params) {}
    uint32_t _GetMemSize() const override {return T_Super::_GetMemSize() +(sizeof(*this) - sizeof(T_Super)) + m_geom.GetAllocSize();}
    virtual AxisAlignedBox3d _CalculateRange3d() const = 0;

public:
    DGNPLATFORM_EXPORT QvCache* GetMyQvCache() const;
    DGNPLATFORM_EXPORT QvElem* GetQvElem(uint32_t index) const;
    DGNPLATFORM_EXPORT bool SetQvElem(QvElem* qvElem, uint32_t index);
    T_QvElemSet* GetQvElems(bool createIfNotPresent) const;
    DGNPLATFORM_EXPORT void SaveGeomStream(GeomStreamCP);
    DGNPLATFORM_EXPORT virtual void _Draw(ViewContextR) const;
    DGNPLATFORM_EXPORT virtual bool _DrawHit(HitDetailCR, ViewContextR) const;
    bool HasGeometry() const {return m_geom.HasGeometry();}  //!< return false if this GeometricElement currently has no geometry (is empty).
    AxisAlignedBox3d CalculateRange3d() const {return _CalculateRange3d();}

    //! Get the GeomStream for this GeometricElement.
    GeomStreamCR GetGeomStream() const {return m_geom;}

    //! Get a writable reference to the GeomStream for this GeometricElement.
    GeomStreamR GetGeomStreamR() {return m_geom;}
};

//=======================================================================================
//! A 3-dimensional GeometricElement.
//! @ingroup DgnElementGroup
// @bsiclass                                                    Keith.Bentley   04/15
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE DgnElement3d : GeometricElement
{
    DEFINE_T_SUPER(GeometricElement);

    struct CreateParams : DgnElement::CreateParams
    {
    DEFINE_T_SUPER(DgnElement::CreateParams);

    Placement3dCR m_placement;
    CreateParams(DgnModelR model, DgnClassId classId, DgnCategoryId category, Placement3dCR placement=Placement3d(), Utf8CP label=nullptr, Utf8CP code=nullptr, DgnElementId id=DgnElementId(), DgnElementId parent=DgnElementId()) :
        T_Super(model, classId, category, label, code, id, parent), m_placement(placement) {}

    explicit CreateParams(DgnElement::CreateParams const& params, Placement3dCR placement=Placement3d()) : T_Super(params), m_placement(placement){}
    };

protected:
    Placement3d m_placement;
    DGNPLATFORM_EXPORT DgnDbStatus _LoadFromDb() override;
    DGNPLATFORM_EXPORT DgnDbStatus _BindPlacement(BeSQLite::Statement&) override;
    DGNPLATFORM_EXPORT void _CopyFrom(DgnElementCR) override;
    uint32_t _GetMemSize() const override {return T_Super::_GetMemSize() + (sizeof(*this) - sizeof(T_Super));}
    explicit DgnElement3d(CreateParams const& params) : T_Super(params), m_placement(params.m_placement) {}
    DgnElement3dCP _ToElement3d() const override {return this;}
    AxisAlignedBox3d _CalculateRange3d() const override {return m_placement.CalculateRange();}

public:
    Placement3dCR GetPlacement() const {return m_placement;} //!< Get the Placement3d of this DgnElement3d
    void SetPlacement(Placement3dCR placement) {m_placement = placement;} //!< Change the Placement3d for this DgnElement3d
};

//=======================================================================================
//! A DgnElement3d that exists in the physical coordinate space of a DgnDb.
//! @ingroup DgnElementGroup
// @bsiclass                                                    Keith.Bentley   04/15
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE PhysicalElement : DgnElement3d
{
    DEFINE_T_SUPER(DgnElement3d);
protected:
    PhysicalElementCP _ToPhysicalElement() const override {return this;}

public:
    explicit PhysicalElement(CreateParams const& params) : T_Super(params) {}

    //! Create an instance of a PhysicalElement from a CreateParams.
    //! @note This is a static method that creates an instance of the PhysicalElement class. To create subclasses, use static methods on the appropriate class.
    static PhysicalElementPtr Create(CreateParams const& params) {return new PhysicalElement(params);}

    //! Create an instance of a PhysicalElement from a model and DgnCategoryId, using the default values for all other parameters.
    //! @param[in] model The PhysicalModel for the new PhysicalElement.
    //! @param[in] categoryId The category for the new PhysicalElement.
    DGNPLATFORM_EXPORT static PhysicalElementPtr Create(PhysicalModelR model, DgnCategoryId categoryId);

    //! Query the DgnClassId for the dgn.PhysicalElement class in the specified DgnDb.
    //! @note This is a static method that always returns the DgnClassId of the dgn.PhysicalElement class - it does @b not return the class of a specific instance.
    DGNPLATFORM_EXPORT static DgnClassId QueryClassId(DgnDbR db);
};

//=======================================================================================
//! A 2-dimensional GeometricElement.
//! @ingroup DgnElementGroup
// @bsiclass                                                    Keith.Bentley   04/15
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE DgnElement2d : GeometricElement
{
    DEFINE_T_SUPER(GeometricElement);
    struct CreateParams : DgnElement::CreateParams
    {
    DEFINE_T_SUPER(DgnElement::CreateParams);

    Placement2dCR m_placement;
    CreateParams(DgnModelR model, DgnClassId classId, DgnCategoryId category, Placement2dCR placement=Placement2d(), Utf8CP label=nullptr, Utf8CP code=nullptr, DgnElementId id=DgnElementId(), DgnElementId parent=DgnElementId()) :
        T_Super(model, classId, category, label, code, id, parent), m_placement(placement) {}

    explicit CreateParams(DgnElement::CreateParams const& params, Placement2dCR placement=Placement2d()) : T_Super(params), m_placement(placement){}
    };

protected:
    Placement2d m_placement;
    DGNPLATFORM_EXPORT DgnDbStatus _LoadFromDb() override;
    DGNPLATFORM_EXPORT DgnDbStatus _BindPlacement(BeSQLite::Statement&) override;
    DGNPLATFORM_EXPORT void _CopyFrom(DgnElementCR) override;
    DgnElement2dCP _ToElement2d() const override {return this;}
    AxisAlignedBox3d _CalculateRange3d() const override {return m_placement.CalculateRange();}
    uint32_t _GetMemSize() const override {return T_Super::_GetMemSize() +(sizeof(*this) - sizeof(T_Super));}
    explicit DgnElement2d(CreateParams const& params) : T_Super(params) {}

public:
    Placement2dCR GetPlacement() const {return m_placement;}     //!< Get the Placement2d for this DgnElement2d
    void SetPlacement(Placement2dCR placement) {m_placement=placement;} //!< Change the Placement2d for this Dgnele
};

//=======================================================================================
//! A DgnElement2d that holds geometry in a DrawingModel
//! @ingroup DgnElementGroup
// @bsiclass                                                    Keith.Bentley   04/15
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE DrawingElement : DgnElement2d
{
    DEFINE_T_SUPER(DgnElement2d);
protected:
    DrawingElementCP _ToDrawingElement() const override {return this;}

public:
    explicit DrawingElement(CreateParams const& params) : T_Super(params) {}
    //! Create a DrawingElement from CreateParams.
    static DrawingElementPtr Create(CreateParams const& params) {return new DrawingElement(params);}

    //! Query the DgnClassId for the dgn.DrawingElement class in the specified DgnDb.
    //! @note This is a static method that always returns the DgnClassId of the dgn.DrawingElement class - it does @b not return the class of a specific instance.
    DGNPLATFORM_EXPORT static DgnClassId QueryClassId(DgnDbR db);
};

//=======================================================================================
//! A "logical Group" of elements.
//! "Logical" groups hold a referencing (not an owning) relationship with their members.
//! ElementGroup can be subclassed for custom grouping behavior.
//! @ingroup DgnElementGroup
// @bsiclass                                                    Shaun.Sewall    05/15
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ElementGroup : DgnElement
{
    DEFINE_T_SUPER(DgnElement);

protected:
    ElementGroupCP _ToElementGroup() const override {return this;}

    //! Called when a member is about to be inserted into this ElementGroup
    //! Override and return something other than DgnDbStatus::Success to prevent the member from being inserted into this ElementGroup.
    virtual DgnDbStatus _OnMemberInsert(DgnElementCR member) const {return DgnDbStatus::Success;}
    //! Called after a member has been inserted into this ElementGroup
    //! Override if additional processing is required after a member was inserted.
    virtual void _OnMemberInserted(DgnElementCR member) const {}

    //! Called when a member is about to be deleted from this ElementGroup
    //! Override and return something other than DgnDbStatus::Success to prevent the member from being deleted from this ElementGroup.
    virtual DgnDbStatus _OnMemberDelete(DgnElementCR member) const {return DgnDbStatus::Success;}
    //! Called after a member has been deleted from this ElementGroup
    //! Override if additional processing is required after a member was deleted.
    virtual void _OnMemberDeleted(DgnElementCR member) const {}

    //! Called when members of the group are queried
    DGNPLATFORM_EXPORT virtual DgnElementIdSet _QueryMembers() const;

public:
    explicit ElementGroup(CreateParams const& params) : T_Super(params) {}

    //! Create a new instance of a DgnElement using the specified CreateParams.
    //! @note This is a static method that only creates instances of the ElementGroup class. To create instances of subclasses,
    //! use a static method on the subclass.
    static ElementGroupPtr Create(CreateParams const& params) { return new ElementGroup(params); }

    //! Query the DgnClassId for the dgn.ElementGroup class in the specified DgnDb.
    //! @note This is a static method that always returns the DgnClassId of the dgn.ElementGroup class - it does @b not return the class of a specific instance.
    DGNPLATFORM_EXPORT static DgnClassId QueryClassId(DgnDbR db);

    //! Insert a member into this ElementGroup. This creates an ElementGroupHasMembers ECRelationship between this ElementGroup and the specified DgnElement
    //! @param[in] member The element to become a member of this ElementGroup.
    //! @note The ElementGroup and the specified DgnElement must have already been inserted (have valid DgnElementIds)
    //! @note This only affects the ElementGroupHasMembers ECRelationship (stored as a database link table).
    DGNPLATFORM_EXPORT DgnDbStatus InsertMember(DgnElementCR member) const;

    //! Deletes the ElementGroupHasMembers ECRelationship between this ElementGroup and the specified DgnElement
    //! @param[in] member The element to remove from this ElementGroup.
    //! @note This only affects the ElementGroupHasMembers ECRelationship (stored as a database link table).
    DGNPLATFORM_EXPORT DgnDbStatus DeleteMember(DgnElementCR member) const;

    //! Query for the set of members of this ElementGroup
    //! @see QueryFromMember
    DgnElementIdSet QueryMembers() const { return _QueryMembers(); }

    //! Query an ElementGroup from a member element.
    //! @param[in] db the DgnDb to query
    //! @param[in] groupClassId specify the type of ElementGroup to consider as a DgnElement could be in more than one ElementGroup.
    //! @param[in] memberElementId the DgnElementId of the member element
    //! @return the DgnElementId of the ElementGroup.  Will be invalid if not found.
    //! @see QueryMembers
    DGNPLATFORM_EXPORT static DgnElementId QueryFromMember(DgnDbR db, DgnClassId groupClassId, DgnElementId memberElementId);
};

END_BENTLEY_DGNPLATFORM_NAMESPACE
