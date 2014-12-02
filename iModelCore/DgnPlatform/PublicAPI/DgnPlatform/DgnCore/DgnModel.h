/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/DgnModel.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include "DgnCore.h"
#include "DgnProjectTables.h"
#include "MSElementDescr.h"
#include "ElementRef.h"
#include "ModelInfo.h"
#include <DgnPlatform/DgnProperties.h>

//__PUBLISH_SECTION_END__
#include <Bentley/WString.h>

#define DGN_TABLE_LEVEL_FOR_MODEL(m)  (m)->GetDgnProject().Levels()

DGNPLATFORM_TYPEDEFS (ElemRangeIndex)
DGNPLATFORM_TYPEDEFS (SectioningViewController)
DGNPLATFORM_REF_COUNTED_PTR(SectioningViewController)
FOREIGNFORMAT_DGNV8_TYPEDEFS (DgnV8ModelReader)

//__PUBLISH_SECTION_START__
DGNPLATFORM_TYPEDEFS (ICheckStop)

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

typedef bset<ElementRefP> T_StdElementRefSet;
struct PersistentElementRefList;

//__PUBLISH_SECTION_END__

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   06/11
//=======================================================================================
struct AddElementOptions
{
    bool m_newElement; // true = created this session, false means loaded from file
    DgnModelR m_model;

    AddElementOptions (bool isNew, DgnModelR model) : m_newElement(isNew), m_model(model){}

    bool IsNewElement() const {return m_newElement;}
    bool IsLoadFromFile() const {return !IsNewElement();}

    DGNPLATFORM_EXPORT virtual DgnModelStatus _VerifyElemDescr(ElementListHandler& list, MSElementDescrR descr) const;
    DGNPLATFORM_EXPORT virtual void _ResolveHandler (MSElementDescrR descr) const;
    DGNPLATFORM_EXPORT virtual void _ValidateIds(DgnProjectR dgnFile, MSElementDescrR descr) const;
    DGNPLATFORM_EXPORT virtual void _OnAdded(PersistentElementRefR, bool isGraphicsList) const;
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   06/11
//=======================================================================================
struct AddFromUndoOptions : AddElementOptions
{
    DEFINE_T_SUPER(AddElementOptions)
    AddFromUndoOptions(DgnModelR model) : AddElementOptions(false, model) {}
    virtual DgnModelStatus _VerifyElemDescr(ElementListHandler& list, MSElementDescrR descr) const override {return DGNMODEL_STATUS_Success;}
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   06/11
//=======================================================================================
struct AddNewElementOptions : AddElementOptions
{
    DEFINE_T_SUPER(AddElementOptions)
    double m_createTime;
    DGNPLATFORM_EXPORT AddNewElementOptions(DgnModelR);

    DGNPLATFORM_EXPORT virtual void _ValidateIds(DgnProjectR dgnFile, MSElementDescrR descr) const override;
    DGNPLATFORM_EXPORT virtual void _OnAdded(PersistentElementRefR, bool isGraphicsList) const override;
};

//__PUBLISH_SECTION_START__
struct ElementRefMap : bmap<ElementId, PersistentElementRefPtr>{};
struct ElementRefVec : bvector<PersistentElementRefP>{};

//=======================================================================================
//! Iterate over the elements in a PersistentElementRefList.
//! @note It is <b>not legal to add or delete elements to the list while iterating it</b>. To do that, first make a
//! ElementRefVec by calling PersistentElementRefList::MakeElementRefVec.
//! @see PersistentElementRefList
//! @bsiclass                                                     KeithBentley    10/00
//=======================================================================================
struct PersistentElementRefListIterator : std::iterator<std::input_iterator_tag, PersistentElementRefP const>
{
private:
    ElementRefMap*                m_map;
    ElementRefMap::const_iterator m_it;
public:

    //! Construct a blank PersistentElementRefListIterator.
    PersistentElementRefListIterator () {Invalidate();}
    void Invalidate() {m_map = NULL;}
    bool IsValid() const {return NULL != m_map && m_it != m_map->end();}

    PersistentElementRefP GetCurrentElementRef() const {return IsValid() ? m_it->second.get() : NULL;}

    //! Change the current ElementRefP pointed to by this iterator to toElm.
    DGNPLATFORM_EXPORT PersistentElementRefP SetCurrentElementRef (PersistentElementRefP toElm);

//__PUBLISH_SECTION_END__
    //! Set the current position of this iterator to the first element in elmList
    DGNPLATFORM_EXPORT PersistentElementRefP GetFirstElementRef (PersistentElementRefList* elmList, bool wantDeleted=false);

    //! Set the current position of this iterator to the next element in the PersistentElementRefList. If the iterator is currently
    //! at the end of the list, then CurrElm() will return NULL.
    //! @param[in] wantDeleted if false, the iterator will skip deleted elements.
    //! @param[in] wantChildren if false, the iterator will skip the children of complex elements and only return non-complex elements.
    DGNPLATFORM_EXPORT PersistentElementRefP GetNextElementRef (bool wantDeleted=false);

//__PUBLISH_SECTION_START__
public:
    DGNPLATFORM_EXPORT PersistentElementRefListIterator& operator++();
    DGNPLATFORM_EXPORT bool operator==(PersistentElementRefListIterator const& rhs) const;
    bool operator!=(PersistentElementRefListIterator const& rhs) const {return !(*this == rhs);}

    //! Access the element data
    PersistentElementRefP operator*() const {return GetCurrentElementRef();}
};

//=======================================================================================
// Values for the Owner column of DGNELEMENT_TABLE_Data
// @bsiclass                                                    Keith.Bentley   07/11
//=======================================================================================
enum ElementOwnerType
    {
    OWNER_ChildElem  = 1,
    OWNER_Physical   = 4,
    OWNER_Drawing    = 5,
    OWNER_Component  = 6,
    };

//__PUBLISH_SECTION_END__
//=======================================================================================
// @bsiclass                                                    Keith.Bentley   07/11
//=======================================================================================
struct ElementListHandler
{
    virtual UInt32 _BindGraphicColumns(BeSQLiteStatementP, DgnElementCR) const {return sizeof(DgnElementHeader);}
    virtual ElementOwnerType _GetElementOwnerType() const = 0;
    virtual UInt32 _GetHeaderSize() const = 0;
    virtual void _LoadGraphicColumns(BeSQLiteStatementR, DgnElementP) const {}
    virtual PersistentElementRefList* _CreateList(DgnModelP) = 0;

    virtual bool _VerifyDimensionality (DgnElementCR) {return true;}
    virtual DgnModelStatus _VerifyElemDescr (MSElementDescrR elemDescr);

    DgnElementRef& CreateNewElementRef (DgnModelR, MSElementDescrR descr, AddElementOptions const& opts);
    DgnModelStatus CheckElementIntegrity (DgnElementR);

    DGNPLATFORM_EXPORT DgnModelStatus AddElementToModel (DgnModelR, MSElementDescrR, AddElementOptions const&);
    DGNPLATFORM_EXPORT DgnModelStatus ReplaceElement (MSElementDescrR newDescr, PersistentElementRefR oldElm, AddElementOptions const& opts);
};

//__PUBLISH_SECTION_START__
//=======================================================================================
//! A iterable list of PersistentElementRefP.
//! @bsiclass                                                     KeithBentley    10/00
//=======================================================================================
struct PersistentElementRefList
{
//__PUBLISH_SECTION_END__
    friend struct ElementListHandler;
    friend struct PersistentElementRefListIterator;
    friend struct PersistentElementRef;
    friend struct DgnModel;
    friend struct AddElementOptions;
    friend struct DbElementListReader;
    friend struct DbElementReloader;
    friend struct ForeignFormat::DgnV8::DgnV8ModelReader;

protected:
    ElementRefMap       m_ownedElems;
    DgnModelP           m_dgnModel;
    ElementListHandlerR m_listHandler;
    bool                m_wasFilled; // true if the list was filled from db

    virtual void _EmptyList() {ReleaseAllElements();}
    virtual void _SetFilled () {m_wasFilled = true;}
    virtual StatusInt _OnElementAdded (PersistentElementRefR elRef) {return RegisterId(elRef);}
    virtual void _OnElementModify (PersistentElementRefR elRef) {}
    virtual void _OnElementModified (PersistentElementRefR elRef) {}
    virtual void _OnElementDeletedFromDb (PersistentElementRefR, bool canceled);

    DGNPLATFORM_EXPORT void ReleaseAllElements ();

public:
    PersistentElementRefList (ElementListHandlerR, DgnModelP);
    virtual ~PersistentElementRefList ();
    void EmptyList() {_EmptyList();}
    DGNPLATFORM_EXPORT ElementRefVec MakeElementRefVec() const;
    DGNPLATFORM_EXPORT UInt32 CountElements() const;

    ElementListHandlerR GetListHandler() const {return m_listHandler;}
    PersistentElementRefP FindElementById (ElementId id);
    DGNPLATFORM_EXPORT StatusInt RegisterId (PersistentElementRef& elRef);

    bool WasFilled () const {return m_wasFilled;}
    DgnModelP MyModel () const {return m_dgnModel;}
    DGNPLATFORM_EXPORT DgnProjectR GetDgnProject() const;

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__
public:
    typedef PersistentElementRefListIterator const_iterator;
    typedef const_iterator iterator;    //!< only const iteration is possible

    bool IsEmpty () const {return (begin() != end());}
    DGNPLATFORM_EXPORT const_iterator begin () const;
    DGNPLATFORM_EXPORT const_iterator end () const;
    DGNPLATFORM_EXPORT DgnModelP GetDgnModelP() const;
};

//=======================================================================================
//! An iterable list of graphical elements that may have a range index
// @bsiclass                                                    Keith.Bentley   01/12
//=======================================================================================
struct GraphicElementRefList : public PersistentElementRefList
{
//__PUBLISH_SECTION_END__
    DEFINE_T_SUPER(PersistentElementRefList)
    friend struct DgnModel;
    friend struct QueryModel;

protected:
    ElemRangeIndexP  m_rangeIndex;               // range tree information
    virtual void _EmptyList() override {T_Super::_EmptyList(); ClearRangeIndex();}
    virtual StatusInt _OnElementAdded (PersistentElementRefR elRef) override {if (SUCCESS!=T_Super::_OnElementAdded(elRef)) return ERROR; InsertRangeElement(&elRef,false); return SUCCESS;}
    virtual void _OnElementModify (PersistentElementRefR elRef) override { T_Super::_OnElementModify(elRef); RemoveRangeElement (elRef);}
    virtual void _OnElementModified (PersistentElementRefR elRef) override {T_Super::_OnElementModified(elRef); InsertRangeElement(&elRef,false);}
    virtual void _SetFilled () override {T_Super::_SetFilled(); AllocateRangeIndex();}
    DGNPLATFORM_EXPORT virtual void _OnElementDeletedFromDb (PersistentElementRefR, bool canceled) override;

    GraphicElementRefList(ElementListHandlerR ListHandler, DgnModelP dgnModel) : PersistentElementRefList (ListHandler,dgnModel) {m_rangeIndex = NULL;}
    ~GraphicElementRefList() {ClearRangeIndex();}

    DGNPLATFORM_EXPORT void AllocateRangeIndex();
    DGNPLATFORM_EXPORT void ClearRangeIndex();
    DGNPLATFORM_EXPORT int GetRangeStamp();
    void RemoveRangeElement (PersistentElementRefR elemRef);

public:
    ElemRangeIndexP GetRangeIndexP(bool createIfNotPresent);
    StatusInt InsertRangeElement (PersistentElementRefP, bool updateDerivedRangeFirst);
//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__
};

//========================================================================================
//! Application-defined object that is stored with a DgnModel. This object is notified as significant events occur
//! for its "host" DgnModel. By creating a subclass of this class, applications can maintain relevant information about
//! a DgnModel with the DgnModel for efficient lookup and lifecycle management.
//! @see DgnModel::AddAppData
//! @bsiclass
//=======================================================================================
struct DgnModelAppData
    {
    virtual ~DgnModelAppData() {}

    //! A unique Key to identify each subclass of DgnModelAppData.
    struct Key : BeSQLite::AppDataKey {};

    //! Override this method to be notified when host DgnModel is about to be deleted from memory or when this appdata is being dropped from the host model.
    //! @note The persistent DgnModel is not being deleted, just this in-memory copy of it.
    virtual void _OnCleanup (DgnModelR host) = 0;

    //! Override this method to be notified after host DgnModel has been filled.
    virtual void _OnFilled (DgnModelR host) {}

    //! Override this method to be notified when host DgnModel is about to be emptied.
    //! @return true to be dropped from host (_OnCleanup will be called.)
    virtual bool _OnEmpty (DgnModelR host) {return true;}

    //! Override this method to be notified after host DgnModel has been emptied. Won't be called unless #_OnEmpty returns false.
    virtual bool _OnEmptied (DgnModelR host) {return false;}

    //! Override this method to be notified before the persistent MicroStationModel is marked for delete. The actual delete happens when the DgnFile
    //! holding the DgnModel is closed.
    virtual void _OnModelDelete (DgnModelR host) {}

    //! Override this method to be notified after the persistent MicroStationModel is undeleted.
    virtual void _OnModelUnDelete (DgnModelR host) {}

    //! Override this method to be notified when a (top-level) element is added to the host DgnModel.
    //! @remarks This is called before the element is added to the range index.
    //! @param host         The host model.
    //! @param elem         The element that was just added to the host model. Note that only top-level elements are reported.
    //! @param isGraphicsList Is this element in the graphics list of the host model?
    virtual void _OnElementAdded (DgnModelR host, PersistentElementRef& elem, bool isGraphicsList) {;}

    //! Override this method to be notified when a model's properties are changed.
    //! @param host         The host model.
    //! @param original     The model's original properties
    virtual void _OnSaveModelProperties (DgnModelR host, ModelInfoCR original) {;}
    };

//=======================================================================================
//! A DgnModel represents a model in memory and holds the elements that belong to the model, if filled.
//!
//! <h3>Filling</h3>
//! Opening a DgnProject does not cause it to load any element data. To access elements, you can either
//! access them by ElementId, or get a DgnModel and then "fill" it.
//!   \li Call #FillSections to fill a model.
//!   \li Call #IsFilled to test if a model is filled.
//!
//! A DgnModel contains two DgnElmLists of elements:
//! the GraphicElm list holds all displayable elements.
//! Each of these lists is a collection.
//! If you wish to find only graphic elements, the most efficient way is to call #GetGraphicElementsP and then iterate.
//! @ingroup DgnFileGroup
//! @bsiclass                                                     KeithBentley    10/00
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE DgnModel
    {
//__PUBLISH_SECTION_END__

    friend struct DgnModels;
    friend struct DgnElementRef;
    friend struct PersistentElementRef;
    friend struct PersistentElementRefList;
    friend struct DerivedElementRangeUtils;
    friend struct DbElementReader;
    friend struct ModelInfoAccess;

protected:
    typedef BeSQLite::AppDataList<DgnModelAppData, DgnModelAppData::Key, DgnModelR> T_AppDataList;

    DgnProjectR     m_project;
    DgnModelId      m_modelId;
    T_AppDataList   m_appData;
    Utf8String      m_name;
    ModelInfo       m_modelInfo;
    GraphicElementRefList*    m_graphicElems;  // elements that have their "isGraphics" flag on
    bool  m_readonly;                 // true if this model is from a read-only file.
    bool  m_isGeoReprojected;         // if true, model has been transformed by application, make sure reference to it is not scaled.
    bool  m_mark;                     // "mark" that can be used by applications on a temporary basis.

private:
    void UpdateElmDscrInPlace (PersistentElementRefP, MSElementDescrP newEL, ElementRefP parent, bool directWrite);
    ElementListHandlerR DetermineListHandler (MSElementDescrR);

protected:
    DGNPLATFORM_EXPORT DgnModel (DgnProjectR project, DgnModelId modelID, Utf8CP name);
    DGNPLATFORM_EXPORT virtual ~DgnModel ();

    virtual DgnModelType _GetModelType() const = 0;
    virtual BeSQLite::DbResult _QueryModelRange (DRange3dR range) {return BeSQLite::BE_SQLITE_ERROR;}
    DGNPLATFORM_EXPORT virtual void _Dump (int nestLevel, bool brief) const;
    virtual ElementListHandlerR _GetGraphicElementHandler() const = 0;
    virtual bool _Is3d() const = 0;
    DGNPLATFORM_EXPORT virtual void _ToSettingsJson(Json::Value&) const;
    DGNPLATFORM_EXPORT virtual void _FromSettingsJson(Json::Value const&);
    DGNPLATFORM_EXPORT virtual void _ToPropertiesJson(Json::Value&) const;
    DGNPLATFORM_EXPORT virtual void _FromPropertiesJson(Json::Value const&);
    DGNPLATFORM_EXPORT virtual DPoint3d _GetGlobalOrigin() const ;
    DGNPLATFORM_EXPORT virtual BentleyStatus _LoadFromDb();

public:
    DGNPLATFORM_EXPORT void Dump (int nestLevel, bool brief) const;
    DGNPLATFORM_EXPORT bool NotifyOnEmpty ();

    void ModelFillComplete();
    void SetIsGeographicReprojected (bool newState) {m_isGeoReprojected = newState;}
    bool IsGeographicReprojected () {return m_isGeoReprojected;}
    void ChangeToReadOnly ();
    DGNPLATFORM_EXPORT bool SetReadOnly (bool newState);
    void ElementChanged(DgnElementRef&, ElemRefChangeReason);
    void ElementAdded(PersistentElementRef&, bool isGraphicsList);
    void OnElementDeletedFromDb(PersistentElementRefR, bool canceled);
    DGNPLATFORM_EXPORT ElemRangeIndexP GetRangeIndexP(bool create) const;
    StatusInt GetRangeIfKnown (DRange3dR pRange);
    BentleyStatus LoadFromDb();
    DGNPLATFORM_EXPORT BeSQLite::DbResult SaveProperties();
    DGNPLATFORM_EXPORT BeSQLite::DbResult SaveSettings();

    bool IsReadOnly () {return m_readonly;}
    DGNPLATFORM_EXPORT bool IsLocked () const; // == GetModelInfo().GetIsLocked()
    bool IsReadonlyOrLocked () const {return IsReadOnly() || IsLocked();}
    DGNPLATFORM_EXPORT void ClearAllDirtyFlags ();
    QvCache* GetQvCache ();
    void ClearAllQvElems ();
    DGNPLATFORM_EXPORT void SetFilled ();
    DGNPLATFORM_EXPORT BeSQLite::DbResult ChangeElementHandlerInDb (ElementId, ElementHandlerId);
    DGNPLATFORM_EXPORT BeSQLite::DbResult ChangeElementLevelInDb (ElementId elementId, LevelId levelId);

    bool GetMark () { return m_mark; }
    void SetMark (bool mark) { m_mark = mark; }
    DGNPLATFORM_EXPORT double GetLineStyleScale () const;
    DGNPLATFORM_EXPORT double GetBRepScaleToDestination (DgnModelR dst);

    //! Direct element I/O. To be called only by Itxn!
    //! Add a new element to this model.
    //! @param[in,out] newEl The new element to add. The "elementID" and "elementRef" fields of the header
    //!                       are updated to reflect the new persistent state of the element.
    //! @return SUCCESS if the element was successfully added to the DgnModel.
    //! @see #PersistentElementRefP::DeleteElement for how to delete an element.
    DGNPLATFORM_EXPORT DgnModelStatus AddElementDescr (MSElementDescrR newEl, AddElementOptions const&);

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__
public:
    DGNPLATFORM_EXPORT double GetMillimetersPerMaster() const;
    DGNPLATFORM_EXPORT double GetSubPerMaster () const;
    DGNPLATFORM_EXPORT DPoint3d GetGlobalOrigin() const;
    DGNPLATFORM_EXPORT BeSQLite::DbResult CopyPropertiesFrom(DgnModelCR);

/** @name Filling */
/** @{ */
    //! Empty the contents of this model.
    DGNPLATFORM_EXPORT void Empty ();

    //! Make sure this model is filled.
    DGNPLATFORM_EXPORT DgnFileStatus FillModel();

    //! Determine whether this model has been "filled" from disk or not.
    //! @return true if the model is filled.
    DGNPLATFORM_EXPORT bool IsFilled() const;

    //! Get the number of loaded elements in this model.
    //! @return the number of elements in this model.
    //! @note The model must be filled before calling this method. 
    //! @see FillSections
    DGNPLATFORM_EXPORT UInt32 GetElementCount() const;
/** @} */

/** @name Finding Elements */
/** @{ */
    DGNPLATFORM_EXPORT GraphicElementRefList* GetGraphicElementsP(bool createIfNotPresent=false) const;

    //=======================================================================================
    //! Iterator over the elements in a DgnModel.
    //! @bsiclass                                                     KeithBentley    10/00
    //=======================================================================================
    struct ElementRefIterator : std::iterator<std::forward_iterator_tag, PersistentElementRefP const>
    {
    private:
    //__PUBLISH_SECTION_END__
    public:
    //__PUBLISH_SECTION_START__
        enum IteratorState {ITERATING_GraphicElms = 2, ITERATING_HitEOF = 3};

    private:
        PersistentElementRefListIterator m_iter;
        DgnModelP                       m_model;
        IteratorState                   m_state;

    private:
    //__PUBLISH_SECTION_END__
    public:                                  // Make sure this is not constructible in the published API
    //__PUBLISH_SECTION_START__
        ElementRefIterator () {m_model = NULL; m_state = ITERATING_GraphicElms;}
        DGNPLATFORM_EXPORT ElementRefIterator (DgnModel* pModel);    // iterate this model only
        DGNPLATFORM_EXPORT ElementRefIterator (ElementRefIterator *source);

    //__PUBLISH_SECTION_END__
    public:
        PersistentElementRefP GetCurrentElementRef() {return m_iter.GetCurrentElementRef();}
        IteratorState GetState () {return m_state;}
        DgnModelP GetModel () {return m_model;}

        DGNPLATFORM_EXPORT PersistentElementRefP GetFirstElementRef (DgnModelP pModel, bool wantDeleted=false);
                       PersistentElementRefP GetFirstElementRef () { return GetFirstElementRef (m_model, false); }
        DGNPLATFORM_EXPORT PersistentElementRefP GetNextElementRef (bool wantDeleted=false);
                       void SetModel (DgnModelP dgnModel) {m_model = dgnModel; m_iter.Invalidate();}
        DGNPLATFORM_EXPORT bool SetCurrentElm (PersistentElementRefP toElm);
        DGNPLATFORM_EXPORT void SetAtEOF ();
        bool HitEOF () const {return m_state == ITERATING_HitEOF;}
    //__PUBLISH_SECTION_START__
    public:
        DGNPLATFORM_EXPORT ElementRefIterator& operator++();
        DGNPLATFORM_EXPORT bool operator!=(ElementRefIterator const& rhs) const;
        bool operator==(ElementRefIterator const& rhs) const {return !(*this != rhs);}

        //! Access the element data
        DGNPLATFORM_EXPORT PersistentElementRefP operator* () const;
    };

#if defined (NEEDS_WORK_DGNITEM)
    //=======================================================================================
    //! Collection of all elements in a model
    //! @bsiclass                                                 Sam.Wilson      05/2009
    //=======================================================================================
    struct      ElementsCollection
    {
    private:
        friend struct  DgnModel;
        DgnModel const& m_model;
        ElementsCollection (DgnModelCR model) : m_model(model) {}

    public:
        typedef ElementRefIterator const_iterator;
        typedef const_iterator iterator;    //!< only const iteration is possible
        DGNPLATFORM_EXPORT const_iterator begin () const;
        DGNPLATFORM_EXPORT const_iterator end () const;
    };

    //!Get the collection of all elements in the model, including both control and graphics elements.
    //!   Example:
    //!   \code
    //!   for (PersistentElementRefP ref : model.GetElementsCollection ())
    //!       {
    //!       ...
    //!       }
    //!   \endcode
    //! @see GetControlElementsP, GetGraphicElementsP
    ElementsCollection GetElementsCollection () const {return ElementsCollection (*this);}
#endif

    //! Find a PersistentElementRefP in this model by ElementId.
    //! @return PersistentElementRefP of element with \a id, or NULL.
    DGNPLATFORM_EXPORT PersistentElementRefP FindElementById (ElementId id);
/** @} */

/** @name Model properties */
/** @{ */
    typedef DgnModelProperty::Spec const& DgnModelPropertySpecCR;
    DGNPLATFORM_EXPORT BeSQLite::DbResult SaveProperty (DgnModelPropertySpecCR , void const* value, UInt32 size, UInt64 id=0);
    DGNPLATFORM_EXPORT BeSQLite::DbResult SavePropertyString (DgnModelPropertySpecCR , Utf8StringCR, UInt64 id=0);
    DGNPLATFORM_EXPORT BeSQLite::DbResult QueryProperty (void* value, UInt32 size, DgnModelPropertySpecCR , UInt64 id=0);
    DGNPLATFORM_EXPORT BeSQLite::DbResult QueryProperty (Utf8StringR, DgnModelPropertySpecCR, UInt64 id=0);
    DGNPLATFORM_EXPORT BeSQLite::DbResult QueryPropertySize (UInt32& size, DgnModelPropertySpecCR spec, UInt64 id);
    DGNPLATFORM_EXPORT BeSQLite::DbResult DeleteProperty (UInt32& size, DgnModelPropertySpecCR spec, UInt64 id);

    //! Get the ModelId for this model.
    DGNPLATFORM_EXPORT DgnModelId GetModelId () const;

    //! Query if this is a 3D model
    DGNPLATFORM_EXPORT bool Is3d () const;

    //! Get the range of all graphical elements in the model.
    DGNPLATFORM_EXPORT BeSQLite::DbResult QueryModelRange (DRange3dR range);

//__PUBLISH_SECTION_END__
    //! Get the ModelInfo for this model.
    //! This should only be used internally. ModelInfo should NOT be modified directly because the values need to be verified for the given model.
    DGNPLATFORM_EXPORT ModelInfoR GetModelInfoR ();

//__PUBLISH_SECTION_START__
public:
    //! Get the ModelInfo for this model.
    DGNPLATFORM_EXPORT ModelInfoCR GetModelInfo() const;

    //! Get the name of this model
    DGNPLATFORM_EXPORT Utf8CP GetModelName () const;

    DGNPLATFORM_EXPORT DgnModelType GetModelType() const;

    //! Determine whether this is a readonly DgnModel or not.
    DGNPLATFORM_EXPORT bool IsReadOnly () const;

    //! Get the project that contains this model.
    //! @return the DgnProject that contains this model.
    DGNPLATFORM_EXPORT DgnProjectR GetDgnProject() const;

    //! Get the spatial extent of all elements in this model.
    //! @param[out]     range      Filled with the union of all of the ranges of the elements in model.
    DGNPLATFORM_EXPORT StatusInt GetRange(DRange3dR range);

/** @} */

/** @name DgnModelAppData */
/** @{ */
    //! Add (or replace) appData to this model.
    //! @return SUCCESS if appData was successfully added. Note that it is illegal to add or remove AppData from within
    //! any of the AppData "_On" methods. If an entry with \a key already exists, it will be dropped and replaced with \a appData.
    DGNPLATFORM_EXPORT StatusInt AddAppData (DgnModelAppData::Key const& key, DgnModelAppData* appData);
    //! @return SUCCESS if appData with key is found and was dropped.
    //! @remarks Calls the app data object's _OnCleanup method.
    DGNPLATFORM_EXPORT StatusInt DropAppData (DgnModelAppData::Key const& key);

    //! Search for appData on this model that was added with the specified key.
    //! @return the DgnModelAppData with \a key, or NULL.
    DGNPLATFORM_EXPORT DgnModelAppData* FindAppData (DgnModelAppData::Key const& key) const;
/** @} */
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   10/11
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE PhysicalModel : DgnModel
{
    DEFINE_T_SUPER(DgnModel)
//__PUBLISH_SECTION_END__
protected:
    virtual DgnModelType _GetModelType() const override {return DgnModelType::Physical;}
    virtual ElementListHandlerR _GetGraphicElementHandler() const override;
    virtual bool _Is3d() const override {return true;}
    DGNPLATFORM_EXPORT virtual BeSQLite::DbResult _QueryModelRange (DRange3dR range) override;

public:
    PhysicalModel (DgnProjectR project, DgnModelId modelId, Utf8CP name) : DgnModel (project, modelId, name) {}

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__
    //! Create a PhysicalGraphics associated with this model on the specified level. 
    //! @param[in] level The level to create this PhysicalGraphics on.
    //! @return The newly created PhysicalGraphics. 
    //! @see DgnGraphicsGroup
    DGNPLATFORM_EXPORT PhysicalGraphicsPtr CreatePhysicalGraphics (LevelId level = LEVEL_DEFAULT_LEVEL_ID);

    //! Create a PhysicalGraphics and populate it from the element in this DgnModel with the specified id.
    //! @param[in] id The ElementId of the element to create this PhysicalGraphics from.
    //! @return The newly created PhysicalGraphics. 
    //! @see DgnGraphicsGroup
    DGNPLATFORM_EXPORT PhysicalGraphicsPtr ReadPhysicalGraphics (ElementId id);
};

//=======================================================================================
//! @private
// @bsiclass                                                    Keith.Bentley   10/11
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ComponentModel : PhysicalModel
{
    DEFINE_T_SUPER(PhysicalModel)
//__PUBLISH_SECTION_END__
protected:
    virtual DgnModelType _GetModelType() const override {return DgnModelType::Component;}
    virtual DPoint3d _GetGlobalOrigin() const override {return DPoint3d::FromZero();}

public:
    ComponentModel (DgnProjectR project, DgnModelId modelId, Utf8CP name) : PhysicalModel (project, modelId, name) {}
//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__
};

//=======================================================================================
//! A Drawing Model is a infinite planar model.
// @bsiclass                                                    Keith.Bentley   10/11
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE DgnModel2d : DgnModel
    {
    DEFINE_T_SUPER(DgnModel)

//__PUBLISH_SECTION_END__
protected:
    DPoint2d m_globalOrigin;       //!< Global Origin - in millimeters

    DGNPLATFORM_EXPORT virtual ElementListHandlerR _GetGraphicElementHandler() const override;
    virtual bool _Is3d() const override {return false;}
    DGNPLATFORM_EXPORT virtual void _ToPropertiesJson(Json::Value&) const override;
    DGNPLATFORM_EXPORT virtual void _FromPropertiesJson(Json::Value const&) override;
    virtual DPoint3d _GetGlobalOrigin() const override {return DPoint3d::From(m_globalOrigin);}

    DGNPLATFORM_EXPORT virtual BeSQLite::DbResult _QueryModelRange (DRange3dR range) override;

public:
    void SetGlobalOrigin(DPoint2dCR org) {m_globalOrigin = org;}

    DgnModel2d (DgnProjectR project, DgnModelId modelId, Utf8CP name) : DgnModel (project, modelId, name) {m_globalOrigin.Zero();}

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__

    //! Create a DrawingGraphics associated with this drawing model on the specified level.
    //! @param[in] level The level to create this DrawingGraphics on.
    //! @return The newly created DrawingGraphics.
    //! @see DgnGraphicsGroup
    DGNPLATFORM_EXPORT DrawingGraphicsPtr Create2dGraphics (LevelId level = LEVEL_DEFAULT_LEVEL_ID);

    //! Create a DrawingGraphics and populate it from the element in this DgnModel with the specified id.
    //! @param[in] id The ElementId of the element to create this DrawingGraphics from.
    //! @return The newly created DrawingGraphics. 
    //! @see DgnGraphicsGroup
    DGNPLATFORM_EXPORT DrawingGraphicsPtr Read2dGraphics (ElementId id);
    };

//=======================================================================================
//! A Drawing Model is a infinite planar model.
// @bsiclass                                                    Keith.Bentley   10/11
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE DrawingModel : DgnModel2d
{
    DEFINE_T_SUPER(DgnModel2d)

//__PUBLISH_SECTION_END__
protected:
    virtual DgnModelType _GetModelType() const override {return DgnModelType::Drawing;}

public:
    DrawingModel (DgnProjectR project, DgnModelId modelId, Utf8CP name) : DgnModel2d (project, modelId, name) {}

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__
};

//=======================================================================================
//! A SectionDrawingModel represents a plane in physical space. The SectionDrawingModel contains 
//! graphics that were computed in some way by intersecting physical models with the drawing model plane.
//! SectionDrawingModel can have one or more convex clip plane sets that can be used to clip a
//! physical view in order to display the drawing.
//! SectionDrawingModels can also contain 2-D annotation elements.
// @bsiclass                                                    Keith.Bentley   10/11
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE SectionDrawingModel : DrawingModel
    {
    DEFINE_T_SUPER(DrawingModel)

//__PUBLISH_SECTION_END__
protected:
    Transform m_transformFromDrawingLCS;  // Equal to m_transformFromCutPlaneLCS, plus additional scaling and possibly additional translation.
    DRange1d m_zrange; // range of Z values of non-planar proxy graphics such as "forward" visible edges in drawing's LCS.
    double m_annotationScale; // the intended viewing scale of annotations in this drawing

    virtual void _FromPropertiesJson(Json::Value const&) override;
    virtual void _ToPropertiesJson(Json::Value&) const override;
    DRange1d GetZRange() const {return m_zrange;}

public:
    SectionDrawingModel (DgnProjectR project, DgnModelId modelId, Utf8CP name) : DrawingModel (project, modelId, name) 
        {
        m_transformFromDrawingLCS.InitIdentity();
        m_zrange.InitNull();
        m_annotationScale = 1.0;
        }

    //! Query and load the sectioning view that was used to generate this drawing.
    //! @return an invalid pointer if this drawing was not generated or if the section view cannot be found or loaded.
    DGNPLATFORM_EXPORT SectioningViewControllerPtr GetSourceView();

public:
    Transform GetTransformToWorld() const {return m_transformFromDrawingLCS;} //!< Returns the transform FROM a local coordinate system that lies in one of the cut planes TO world/project coordinates.
    void SetTransformFromDrawingLCS (TransformCR t) {m_transformFromDrawingLCS=t;} //!< Stores the transform FROM a local coordinate system that lies in one of the cut planes TO world/project coordinates.

    Transform GetFlatteningMatrix (double zdelta = 0.0) const; //!< Get the matrix to use when viewing this model. It ensures that all elements lie in the LCS plane.
    void AddToZRange (double z) {m_zrange.Extend(z);} //!< Report the Z value of non-planar proxy graphics such as "forward" visible edges in drawing's LCS.

    //! Set the scale of annotations in this drawing
    void SetAnnotationScale (double s) {m_annotationScale = s;}

    //! Get the scale of annotations in this drawing 
    double GetAnnotationScale() const {return m_annotationScale;}

//__PUBLISH_SECTION_START__
    };

//=======================================================================================
//! @private
// @bsiclass                                                    Keith.Bentley   10/11
//=======================================================================================
struct SheetModel : DgnModel2d
    {
    DEFINE_T_SUPER(DgnModel2d)
//__PUBLISH_SECTION_END__
protected:
    virtual DgnModelType _GetModelType() const override {return DgnModelType::Sheet;}

public:
    SheetModel (DgnProjectR project, DgnModelId modelId, Utf8CP name) : DgnModel2d (project, modelId, name) {}
//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__
    };

//=======================================================================================
//! A DgnModelList is-a bvector of DgnModel pointers.
//! @deprecated
//! @private
//=======================================================================================
struct DgnModelList : bvector<DgnModelP>
    {
    bool IsFound (DgnModelP modelRef) const {return std::find (begin(), end(), modelRef) != end();}
    };

END_BENTLEY_DGNPLATFORM_NAMESPACE

