/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/ITxnManager.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include    "DgnFile.h"

//__PUBLISH_SECTION_START__
/** @cond BENTLEY_SDK_Internal */

DGNPLATFORM_TYPEDEFS (TxnMonitor)

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

/*=================================================================================**//**
 @addtogroup TxnMgr
 <h1>Transaction Manager</h1>
 The Transaction Manager enables you to save changes, journal transactionable changes during a session, and to organize
 changes into units called transactions.

 <h2>Making and Saving Changes</h2>
 Use the Transaction Manager in order to add, save changes to, or to delete elements, XAttributes or models.

 Here are a few common patterns:

 <h3>To add an element:</h3>
 -# Create an EditElementHandle. Initially, the handle is invalid.
 -# Call a static handler method to define the element. For example, LineHandler::CreateLineElement
 -# Call EditElementHandle::AddToModel to add the new element to a model.

 The element is now persistent in the specified model.

 <h3>To change an element:</h3>
 -# Obtain an ElementRefP that refers to the persistent element in the model.
 -# Create an EditElementHandle that can be used to access the element's data.
 -# Call handler methods such as Handler::ApplyTransform to modify the element's data.
 -# Call EditElementHandle::ReplaceInModel to update the persistent element's data to reflect the change.

 Note that you could make multiple changes to the EditElementHandle before writing to the model.

 <h3>To delete an element:</h3>
 -# Obtain an ElementRefP that refers to the persistent element in the model.
 -# Create an EditElementHandle from the ElementRefP and then call EditElementHandle::DeleteFromModel to delete the element.

 <h3>To add an XAttribute to an element:</h3>
 -# Obtain an ElementRefP that refers to the persistent element in the model.
 -# Define the XAttribute's data and decide what XAttributeHandlerId to use.
 -# Call ITxn::AddXAttribute to add the XAttribute to the persistent element in the model.

 The XAttribute is now persistent in the element's model.

 <h3>To modify an XAttribute on an element:</h3>
 -# Obtain an ElementRefP that refers to the persistent element in the model.
 -# Use XAttributeHandle to locate the XAttribute on the element.
 -# Decide how you want to modify the XAttribute's data.
 -# Call ITxn::ModifyXAttributeData to update the persistent XAttribute data in the model.

 Or, you could call ITxn::ReplaceXAttributeData to update the persistent XAttribute to store completely different data.

 <h3>To delete an XAttribute:</h3>
 -# Obtain an ElementRefP that refers to the persistent element in the model.
 -# Use XAttributeHandle to locate the XAttribute on the element.
 -# Call ITxn::DeleteXAttribute to remove the XAttribute from the persistent element in the model.

 <h2>Transactions</h2>
 A transaction is defined as a set of changes that are treated as a unit. All changes and deletions are made as part of a transaction.
 Most transactions are undoable and can be reversed or reinstated as a unit. Undoable transactions support \ref TxnMonitor listeners.

 The Transaction Manager implements the ITxnManager interface. This interface has methods to query the current transaction, and to reverse, cancel, or reinstate transactions.

 All changes are made in the context of the current transaction. There is always a current transaction running. You don't have to start
 or create one. ITxnManager::GetCurrentTxn returns the current transaction.

 ITxnManager::CloseCurrentTxn defines a boundary that ends the current transaction and starts a new one.
 Setting a transaction boundary validates the preceding changes (ITxnManager::ValidateCurrentTxn). Validating an undoable transaction also notifies \ref TxnMonitor listeners.
 For an undoable transaction, setting a transaction boundary defines an undo point.

 The current transaction implements the ITxn interface. The ITxn interface provides methods to write changes to elements, XAttributes, and models.
 The EditElementHandle methods to add, replace, and delete elements turn around and call the appropriate methods on the current transaction.

 <h2>Example</h2>
 Here is a simple example of using the transaction manager to add two elements to a model and then close the transaction.

 \code
    ...
   EditElementHandle eh1, eh2;

   LineHandler::CreateLineElement (eh1, ...., model1);
   ITxnManager::GetCurrentTxn().AddElement (eh1);

   LineHandler::CreateLineElement (eh2, ...., model1);
   ITxnManager::GetCurrentTxn().AddElement (eh2);

   ITxnManager::CloseCurrentTxn(); // Call TxnMonitors and set transaction boundary.
 \endcode

 These two adds can now be reversed by calling ITxnManager::ReverseSingleTxn.

 @bsiclass
+===============+===============+===============+===============+===============+======*/
enum TxnApplyDirection {APPLY_FOR_Undo=0, APPLY_FOR_Redo=1};
enum TxnApplyStage     {APPLY_Pre=0, APPLY_Post=1};

//=======================================================================================
//! A 32 bit value to identify the group of entries that form a single transaction.
// @bsiclass                                                      Keith.Bentley   02/04
//=======================================================================================
struct TxnId
{
    Int32  m_value;

public:
    TxnId ()  {m_value = -1;}
    explicit TxnId (Int32 val) {m_value = val;}
//__PUBLISH_SECTION_END__
    void Init() {m_value = 0;}
    void Next() {++m_value;}
    void Prev() {--m_value;}
//__PUBLISH_SECTION_START__
    bool IsValid() const {return -1 != m_value;}
    Int32 GetValue() const {return m_value;}
    operator Int32() const {return m_value;}
};

//=======================================================================================
//! A summary of all of the element-based changes that occurred during a Txn. TxnMonitors are supplied with a
//! TxnSummary so they can determine what elements were affected by a Txn.
//! A TxnSummary includes information about the Txn it summarizes, plus the following sets:
//!    -# DgnElement Adds
//!    -# DgnElement Deletes
//!    -# DgnElement Modifies
//!    -# XAttribute Adds
//!    -# XAttribute Deletes
//!    -# XAttribute Modifies
// @bsiclass                                                    Keith.Bentley   07/13
//=======================================================================================
struct TxnSummary
{
    //=======================================================================================
    //! Records the fact that an element was added or deleted during the transaction
    // @bsiclass                                                    Keith.Bentley   07/13
    //=======================================================================================
    struct LifecycleChange
    {
    private:
        ElementId         m_elId;
        ElementHandlerId  m_handlerId;

    public:
        LifecycleChange() {}
        LifecycleChange(ElementId elId, ElementHandlerId handlerId) : m_elId(elId), m_handlerId(handlerId) {}
        bool operator< (LifecycleChange const& other) const {return m_elId < other.m_elId; }
        ElementId GetElementId() const {return m_elId;}
        ElementHandlerId GetHandlerId() const {return m_handlerId;}
        PersistentElementRefPtr GetElement(DgnProjectR project) const {return project.Models().GetElementById(m_elId);}
    };

    //=======================================================================================
    //! A single change to an XAttribute
    // @bsiclass                                                    Keith.Bentley   07/13
    //=======================================================================================
    struct XAttrChange
    {
    private:
        ElementId           m_elId;
        XAttributeHandlerId m_handlerId;
        UInt32              m_xAttrId;

    public:
        XAttrChange() {}
        XAttrChange(ElementId elId, XAttributeHandlerId handlerId, UInt32 xAttrId) : m_elId(elId), m_handlerId(handlerId), m_xAttrId(xAttrId) {}
        bool operator< (XAttrChange const& other) const
            {
            if (m_elId < other.m_elId) return true;
            if (m_elId > other.m_elId) return false;
            if (m_handlerId < other.m_handlerId) return true;
            if (m_handlerId > other.m_handlerId) return false;
            return m_xAttrId < other.m_xAttrId;
            }
        ElementId GetElementId() const {return m_elId;}
        XAttributeHandlerId GetHandlerId() const {return m_handlerId;}
        UInt32 GetXAttrId() const {return m_xAttrId;}
        PersistentElementRefPtr GetElement(DgnProjectR project) const {return project.Models().GetElementById(m_elId);}
    };

    typedef bset<XAttrChange> TxnXAttrSet;
    typedef bset<LifecycleChange> TxnLifecycleSet;
    typedef bset<PersistentElementRefP> TxnModifySet;

private:
    TxnSummary (); // internal only

//__PUBLISH_SECTION_END__
    DgnProjectR     m_project;
    TxnId           m_txnId;
    Utf8String      m_txnDescr;
    UInt64          m_txnSource;
    DRange3d        m_physicalRange;
    DRange2d        m_drawingRange;
    TxnLifecycleSet m_added;
    TxnLifecycleSet m_deleted;
    TxnModifySet    m_modified;
    TxnXAttrSet     m_xattAdded;
    TxnXAttrSet     m_xattDeleted;
    TxnXAttrSet     m_xattModified;

    friend struct ITxnManager;
    friend struct ElementTableHandler;
    friend struct XAttTableHandler;

public:
    DRange3dR GetPhysicalRangeR() {return m_physicalRange;}
    DRange2dR GetDrawingRangeR() {return m_drawingRange;}
    void CallHandlers_Boundary() const;
    void CallHandlers_Reverse(bool isUndo) const;
    void CallHandlers_Reversed(bool isUndo) const;

    TxnSummary (DgnProjectR, TxnId, Utf8String, UInt64, BeSQLite::ChangeSet&);

//__PUBLISH_SECTION_START__
public:
    //! Get the TxnLifecycleSet that holds the Added elements for this TxnSummary
    DGNPLATFORM_EXPORT TxnLifecycleSet const& GetElementAdds() const;

    //! Get the TxnLifecycleSet that holds the deleted elements for this TxnSummary
    DGNPLATFORM_EXPORT TxnLifecycleSet const& GetElementDeletes() const;

    //! Get the TxnModifySet that holds the Modified elements for this TxnSummary
    DGNPLATFORM_EXPORT TxnModifySet const& GetElementModifies() const;

    //! Get the TxnXAttrSet that holds the Added XAttributes efor this TxnSummary
    DGNPLATFORM_EXPORT TxnXAttrSet const& GetXAttributeAdds() const;

    //! Get the TxnXAttrSet that holds the Deleted XAttributes for this TxnSummary
    DGNPLATFORM_EXPORT TxnXAttrSet const& GetXAttributeDeletes() const;

    //! Get the TxnXAttrSet that holds the Modified XAttributes for this TxnSummary
    DGNPLATFORM_EXPORT TxnXAttrSet const& GetXAttributeModifies() const;

    //! Get the union of the range of all affected physical elements. For all Adds in the TxnSummary, this range
    //! contains the new elements' range. For all Deletes, this range contains all of the original elements' range. For all modifies,
    //! this range includes both the pre- and post-changed ranges.
    DGNPLATFORM_EXPORT DRange3dCR GetPhysicalRange() const;

    //! Get the union of the range of all affected drawing elements. For all Adds in the TxnSummary, this range
    //! contains the new elements' range. For all Deletes, this range contains all of the original elements' range. For all modifies,
    //! this range includes both the pre- and post-changed ranges.
    DGNPLATFORM_EXPORT DRange2dCR GetDrawingRange() const;

    //! Get the DgnProject for this TxnSummary
    DGNPLATFORM_EXPORT DgnProjectR GetDgnProject() const;

    //! Get the TxnId of this TxnSummary
    DGNPLATFORM_EXPORT TxnId GetTxnId() const;

    //! Get the description of this TxnSummary
    //! @see ITxnManager::SetTxnDescription
    DGNPLATFORM_EXPORT Utf8StringCR GetTxnDescription() const;

    //! Get the source for this TxnSummary
    //! @see ITxnManager::SetTxnSource
    DGNPLATFORM_EXPORT UInt64 GetTxnSource() const;
};

//=======================================================================================
//! Interface to be implemented to monitor changes to elements.
//! @ingroup TxnMgr
// @bsiclass                                                      Keith.Bentley   10/07
//=======================================================================================
struct TxnMonitor
{
    virtual void _OnTxnBoundary (TxnSummaryCR) = 0;
    virtual void _OnTxnReverse (TxnSummaryCR, bool isUndo) = 0;
    virtual void _OnTxnReversed (TxnSummaryCR, bool isUndo) = 0;
    virtual void _OnUndoRedoFinished (DgnProjectR, bool isUndo) {}
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   02/11
//=======================================================================================
struct ITxnOptions
    {
    bool m_clearReversedTxns;
    bool m_writesIllegal;

    ITxnOptions()
        {
        m_clearReversedTxns = true;
        m_writesIllegal = false;
        }
    };

//=======================================================================================
// @bsiclass
//=======================================================================================
struct ITxn : NonCopyableClass
{
//__PUBLISH_SECTION_END__
private:
    StatusInt CheckElementForWrite (ElementRefP);

protected:
    ITxnOptions m_opts;
    ITxn(ITxnOptions opts = ITxnOptions()) : m_opts (opts) {}
    virtual ~ITxn() {}

    void ClearReversedTxns (DgnProjectR);
    virtual StatusInt _CheckDgnModelForWrite (DgnModelP) {return SUCCESS;}

public:
    DGNPLATFORM_EXPORT StatusInt WriteXaChanges (MSElementDescrP ed, bool isAdd);

    /// @name DeprecatedDescrBasedTxn
    //@{
    // Proper usage of these methods is very tricky. They are provided for compatibility only, and in all new code you should use
    // "AddElement" and "ReplaceElement". The problem with these methods is that they take an MSElementDescrP and not MSElementDescrH.
    // After the call, the elDescr supplied as input is NOT guaranteed to reflect the actual element that was added/modified. That's because both
    // element handlers and write-to-file hooks are permitted to "replace" the supplied element with a different one. But, since
    // the signature of this method doesn't allow a way to return the replaced element descriptor, this method must be used carefully.
    // Basically, after this call the caller must immediately free "elDescr", since the only reliable information in it will be the
    // elementRef value. Understanding all of this is tedious and unnecessary though, since AddElement and ReplaceElement do not
    // suffer from this problem. But converting existing callers can be difficult, which is the only reason these methods exist.
    DGNPLATFORM_EXPORT StatusInt AddElemDescr (ElementRefP& newElemRef, DgnModelP, MSElementDescrP elDescr);
    DGNPLATFORM_EXPORT StatusInt ReplaceElementDescr (ElementRefP& out, ElementRefP inRef, MSElementDescrP newElemDscr);
    //@}

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__
public:

    /// @name XAttributes
    //@{
    //! Add a new XAttribute to an existing element.
    //! @param[in]          elRef           The ElementRefP of the element.
    //! @param[in]          handlerId       The XAttributeHandlerId of the new XAttribute.
    //! @param[in]          xAttrId         The XAttributeId of the new XAttribute. If INVALID_XATTR_ID, then a new, unique, id will be
    //!                                         assigned. The value of the new id will be returned in \a outXAttrId.
    //! @param[in]          xAttrData       A buffer of at least \a length bytes long that holds the value to be saved in the new XAttribute.
    //! @param[in]          length          The number of bytes in xAttrData.
    //! @param[out]         outXAttrId      Optional, the id of the new XAttribute. This is necessary only if \a xAttrId is INVALID_XATTR_ID, in which case
    //!                                         the new XAttribute is assigned a new unique id.
    //! @param[in] flags flags to control compression of data
    //! Remarks Implementation must use DependencyMgrXAttributeChangeTracker
    //! @return SUCCESS if the XAttribute was added to the ElementRefP.
    DGNPLATFORM_EXPORT StatusInt AddXAttribute (ElementRefP elRef, XAttributeHandlerId handlerId, UInt32 xAttrId, void const* xAttrData, UInt32 length, UInt32* outXAttrId=NULL, DgnModels::XAttributeFlags flags=DgnModels::XAttributeFlags());

    //! Delete an existing XAttribute from an element.
    //! @param[in]          xAttr           An XAttributeHandle that references the XAttribute to delete.
    //! @return SUCCESS if \a xAttr is valid and the XAttribute was deleted from the element. DGNMODEL_STATUS_InvalidXattribute if xAttr was not valid.
    //! Remarks Implementation must use DependencyMgrXAttributeChangeTracker
    DGNPLATFORM_EXPORT StatusInt DeleteXAttribute (XAttributeHandleR xAttr);

    //! Modify all or part of an existing XAttribute. The size of the XAttribute cannot be changed.
    //! @param[in]          xAttr           An XAttributeHandle that references the XAttribute to modify.
    //! @param[in]          data            The new data to save in the XAttribute.
    //! @param[in]          start           The starting byte number to replace with \a data.
    //! @param[in]          length          The number of bytes to replace with \a data.
    //! @param[in] flags flags to control compression of data
    //! @return SUCCESS if the data in \a xAttr was modified. DGNMODEL_STATUS_InvalidXattribute if xAttr was not valid. ERROR if attempting to write
    //!                 more data than exists in xAttr.
    //! Remarks Implementation must use DependencyMgrXAttributeChangeTracker
    DGNPLATFORM_EXPORT StatusInt ModifyXAttributeData (XAttributeHandleR xAttr, void const* data, UInt32 start, UInt32 length, DgnModels::XAttributeFlags flags=DgnModels::XAttributeFlags());

    //! Replace an existing XAttribute with a new value. The size of the XAttribute \em can change with this method.
    //! @param[in]          xAttr           An XAttributeHandle that references the XAttribute to replace.
    //! @param[in]          data            The new data to save in the XAttribute.
    //! @param[in]          newSize         The number of bytes in \a data.
    //! @param[in] flags flags to control compression of data
    //! @return SUCCESS if the data in \a xAttr was replaced. DGNMODEL_STATUS_InvalidXattribute if xAttr was not valid.
    //! Remarks Implementation must use DependencyMgrXAttributeChangeTracker
    DGNPLATFORM_EXPORT StatusInt ReplaceXAttributeData (XAttributeHandleR xAttr, void const* data, UInt32 newSize = 0, DgnModels::XAttributeFlags flags=DgnModels::XAttributeFlags());
    //@}

    /// @name Element I/O
    //@{

    //! Delete an element from a model.
    //! @param[in]          elem            The ElementRefP of the element to be deleted. Must be a valid existing element.
    //! @return SUCCESS if the element was deleted.
    DGNPLATFORM_EXPORT StatusInt DeleteElement (ElementRefP elem);

    //! Add a new element to a model.
    //! @remarks \a newEl must already be associated with a model.
    //! @param[in,out]      newEl           The element to be added. Must hold an MSElementDescr.
    //! @return SUCCESS if the element was added or non-zero indicating failure. Possible reasons for failure include:
    //! -- \a newEl is not associated with a model,
    //! -- the model is not writable,
    //! -- the element handler blocked the add,
    //! -- the element could not be assigned an ElementId,
    //! -- any scheduled XAttribute change failed.
    DGNPLATFORM_EXPORT StatusInt AddElement (EditElementHandleR newEl);

    //! Replace an existing element in a model with a different one.
    //! @param[in,out] el The element to be replaced.
    //! @param[in]  in The ElementRefP of the element to be replaced. Must be a valid existing element.
    //! @return SUCCESS if the element was replaced and out is non-NULL.
    DGNPLATFORM_EXPORT StatusInt ReplaceElement (EditElementHandleR el, ElementRefP in);
    //@}
};

//__PUBLISH_SECTION_END__

#if !defined (DOCUMENTATION_GENERATOR)
//=======================================================================================
//! The first and last entry number that forms a single transaction.
// @bsiclass                                                      Keith.Bentley   02/04
//=======================================================================================
class TxnRange
{
    TxnId     m_first;
    TxnId     m_last;

public:
    TxnRange (TxnId first, TxnId last) : m_first(first), m_last(last) {}

    TxnId GetFirst () {return m_first;}
    TxnId GetLast () {return m_last;}
};

//=======================================================================================
//! To reinstate a reversed transaction, we need to know the first and last entry number, PLUS the
//! current value of the end of transaction data (before the transaction was reversed.)
// @bsiclass                                                      Keith.Bentley   02/04
//=======================================================================================
struct RevTxn
{
    TxnRange    m_range;
    bool        m_multiStep;
    RevTxn (TxnRange& range, bool multiStep) : m_range (range) {m_multiStep = multiStep;}
};
#endif

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   07/11
//=======================================================================================
struct UndoDb : BeSQLite::Db
{
    BeSQLite::DbResult Open();
    void Empty();
    void TruncateChanges (TxnId id);
    BeSQLite::DbResult SaveEntry(TxnId id, UInt64 source, Utf8StringCR descr);
    BeSQLite::DbResult ReadEntry(TxnId id, UInt64& source, Utf8StringR cmdName);
    BeSQLite::DbResult SaveChange(TxnId id, BeSQLite::ChangeSet& changeset);
    BeSQLite::DbResult SaveMark(TxnId id, Utf8CP);
    TxnId FindMark(Utf8StringR, TxnId before);

    struct ChangedFiles
        {
        UndoDb& m_db;
        TxnId   m_id;
        mutable BeSQLite::Statement m_sql;
        ChangedFiles (UndoDb& db, TxnId id) : m_db(db), m_id(id){}

        struct Entry : std::iterator<std::input_iterator_tag, Entry const>
            {
            friend struct  ChangedFiles;
        private:
            bool  m_isValid;
            BeSQLiteStatementP m_sql;
            Entry (BeSQLiteStatementP sql, bool isValid) {m_sql = sql; m_isValid = isValid;}

        public:
            bool IsValid() const {return m_isValid;}
            Entry& operator++() {m_isValid = (BeSQLite::BE_SQLITE_ROW == m_sql->Step()); return *this;}
            Entry const& operator* () const {return *this;}
            bool operator!=(Entry const& rhs) const {return (m_isValid != rhs.m_isValid);}
            bool operator==(Entry const& rhs) const {return (m_isValid == rhs.m_isValid);}

            void GetChangeSet(BeSQLite::ChangeSet& changeSet, bool invert) const;
            };

        typedef Entry const_iterator;
        const_iterator begin() const;
        const_iterator end() const {return Entry (&m_sql, false);}
        };
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   07/11
//=======================================================================================
struct TxnTableHandler
{
    virtual Utf8CP _GetTableName() const = 0;
    virtual void _OnAdd(TxnSummary&, BeSQLite::Changes::Change const&) const= 0;
    virtual void _OnDelete(TxnSummary&, BeSQLite::Changes::Change const&) const= 0;
    virtual void _OnUpdate(TxnSummary&, BeSQLite::Changes::Change const&) const= 0;
};

//__PUBLISH_SECTION_START__

//=======================================================================================
//! This class provides a transaction mechanism for handling changes to elements in DgnProjects
//! @ingroup TxnMgr
// @bsiclass
//=======================================================================================
struct ITxnManager
{
//__PUBLISH_SECTION_END__

    friend struct TxnIter;
    friend struct ITxn;
    friend struct DgnCacheTxn;

protected:
    DgnProjectR     m_project;
    UndoDb          m_db;
    bvector<TxnId>  m_txnGroup;
    bvector<RevTxn> m_reversedTxn;
    Utf8String      m_txnDescr;
    UInt64          m_txnSource;
    ITxn*           m_currTxn;
    TxnId           m_firstTxn;
    TxnId           m_currentTxnID;
    bool            m_isActive;
    bool            m_boundaryMarked;
    bool            m_undoInProgress;
    bool            m_callRestartFunc;
    bool            m_inDynamics;

    void SetActive (bool newValue) {m_isActive = newValue;}
    void SetUndoInProgress(bool);
    TxnId GetFirstTxnId () {return m_firstTxn;}
    void ReverseTxnRange (TxnRange& txnRange, Utf8StringP, bool);
    void CheckTxnBoundary ();
    void ReinstateTxn (TxnRange&, Utf8StringP redoStr);
    bool HasAnyChanges();
    void ApplyChanges (TxnId, TxnApplyDirection);
    void CancelChanges (TxnId txnId);
    StatusInt ReinstateActions (RevTxn& revTxn);
    bool PrepareForUndo();
    StatusInt ReverseActions (TxnRange& txnRange, bool multiStep, bool showMsg);

public:
    DGNPLATFORM_EXPORT static void AddTableHandler(TxnTableHandler&);
    static TxnTableHandler* FindTableHandler(Utf8CP);

    DGNPLATFORM_EXPORT BentleyStatus SaveUndoMark(Utf8CP name);
    DGNPLATFORM_EXPORT void GetUndoString (Utf8StringR);
    DGNPLATFORM_EXPORT void GetRedoString (Utf8StringR);
    DGNPLATFORM_EXPORT void DumpTxnHistory (int maxChanges);

    DGNPLATFORM_EXPORT ITxnManager (DgnProjectR);

    //! Apply a changeset and then clean up the screen, reload elements, refresh other cached data, and notify txn listeners.
    //! @param changeset the changeset to apply
    //! @param txnId     the transaction id to pass to monitors
    //! @param txnDescr  the transaction description to pass to monitors
    //! @param txnSource the transaction source to pass to monitors
    //! @param isUndo    the undo/redo flag to pass to monitors
    //! @return the result of calling changeset.ApplyChanges
    DGNPLATFORM_EXPORT BeSQLite::DbResult ApplyChangeSet (BeSQLite::ChangeSet& changeset, TxnId txnId, Utf8StringCR txnDescr, UInt64 txnSource, bool isUndo);

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__
public:

    //! @name The Current Transaction
    //@{

    //! End the current transaction, either committing or cancelling the changes made.
    //! If changes are to be discarded, then this function will call CancelToPos.
    //! If changes are to be retained, then this function will invoke TxnMonitor callbacks and then set a closing mark.
    //! The closing mark designates that all of the preceding changes are to be undone together as a single operation, and that future
    //! changes are to be undone separately.
    //! @remarks If there is a Transaction Group active, the effect of this method is only to validate the transaction, if requested.
    DGNPLATFORM_EXPORT void CloseCurrentTxn ();

    //! Get the current transaction
    DGNPLATFORM_EXPORT ITxn& GetCurrentTxn();

    //! Set the current ITxn
    //! @param newTxn   the ITxn implementation that is to handle changes in the current transaction
    DGNPLATFORM_EXPORT ITxn& SetCurrentTxn (ITxn& newTxn);
    //@}

    //! @name Start and Stop Journaling Changes for Undo
    //@{
    //! Query if the Transaction Manager is currently active
    //! @return True if the Transaction Manager is currently active.
    //! @see Activate
    DGNPLATFORM_EXPORT bool IsActive();

    //! Turn on the Transaction Manager. After this, all changes to elements in transactable files will be journaled.
    //! @See DgnFile::SetTransactable
    //! @see Deactivate
    DGNPLATFORM_EXPORT void Activate();

    //! Turn off the Transaction Manager. After this, changes to the database will not be journaled for undo.
    //! Turning off the Transaction Manager also permanently clears any journaled changes. Therefore, turning the Transaction Manager
    //! off and then back on has the effect of clearing it.
    //! @see Activate
    DGNPLATFORM_EXPORT void Deactivate();
    //@}

    //! @name Transaction Groups
    //@{

    //! Start a new Transaction Group. Transaction Groups are used to cause changes that would normally
    //! be considered separate actions to be "grouped" into a single action. This means that when the user issues the "undo"
    //! command, the entire group of changes is undone as a single action. Groups can be nested, and until the outermost group is closed,
    //! all element changes constitute a single transaction.
    //! @param[in] startNewTxn if true, and if there is no current Transaction Group, and if there are pending changes, then start a new group. Otherwise
    //!                       changes from this group are included with the previous changes.
    //! @remarks This method should \e always be paired with a call to EndTxnGroup.
    //! @remarks This method cancels all pending nested transactions.
    DGNPLATFORM_EXPORT void StartTxnGroup (bool startNewTxn);

    //! Close the current Transaction Group.
    DGNPLATFORM_EXPORT void EndTxnGroup();

    //! Return the number of entries in the current Transaction Group.
    DGNPLATFORM_EXPORT size_t GetTxnGroupCount ();

    //! @return The TxnId of the beginning of the innermost Transaction Group. If no Transaction Group is active, the TxnId will be zero.
    DGNPLATFORM_EXPORT TxnId GetCurrGroupStart();
    //@}

    //! @name Reversing and Reinstating Transactions
    //@{

    //! Query if there are currently any reversible (undoable) changes in the Transaction Manager
    //! @return true if there are currently any reversible (undoable) changes in the Transaction Manager.
    DGNPLATFORM_EXPORT bool HasEntries ();

    //! Query if there are currently any reinstateable (redoable) changes in the Transaction Manager
    //! @return True if there are currently any reinstateable (redoable) changes in the Transaction Manager.
    DGNPLATFORM_EXPORT bool RedoIsPossible ();

    //! Reverse (undo) the most recent transaction(s).
    //! @param[in] numActions the number of transactions to reverse. If numActions is greater than 1, the entire set of transactions will
    //!       be reinstated together when/if ReinstateTxn is called (e.g., the user issues the "REDO" command.)
    //! @remarks  Reversed Transactions can be reinstated by calling ReinstateTxn. To completely remove all vestiges of (including the memory
    //!           used by) a transaction, call ClearReversedTxns.
    //! @see ReinstateTxn ClearReversedTxns
    //! @remarks This method cancels all pending nested transactions.
    DGNPLATFORM_EXPORT void ReverseTxns (int numActions);

    //! Reverse (undo) the most recent transaction.
    //! @param[in] callRestartFunc whether to restart the current tool afterwards, only the current tool should pass false.
    //! @remarks This method cancels all pending nested transactions.
    DGNPLATFORM_EXPORT void ReverseSingleTxn (bool callRestartFunc=true);

    //! Reverse all element changes back to the most recent Mark. Marks are created by calling SaveUndoMark.
    //! @param[out] name of mark undone.
    //! @remarks This method cancels all pending nested transactions.
    DGNPLATFORM_EXPORT void ReverseToMark (Utf8StringR name);

    //! Reverse all element changes back to the beginning of the session.
    //! @param[in] prompt display a dialog warning the user of the severity of this action and giving an opportunity to cancel.
    //! @remarks This method cancels all pending nested transactions.
    DGNPLATFORM_EXPORT void ReverseAll (bool prompt);

    //! Reverse all element changes back to a previously saved TxnPos.
    //! @param[in] pos a TxnPos obtained from a previous call to GetCurrTxnPos.
    //! @return SUCCESS if the transactions were reversed, ERROR if TxnPos is invalid.
    //! @remarks This method cancels all pending nested transactions.
    //! @see  GetCurrTxnPos CancelToPos
    DGNPLATFORM_EXPORT StatusInt ReverseToPos (TxnId pos);

    //! Get the Id of the most recently commited transaction.
    //! @return the current TxnPos. This value can be saved and later used to reverse changes that happen after this time.
    //! @see   ReverseToPos CancelToPos
    DGNPLATFORM_EXPORT TxnId GetCurrTxnId();

    //! Reverse and then cancel all element changes back to a previously saved TxnPos. This method is
    //! the same as calling ReverseToPos followed (on successful return) by ClearReversedTxns, and is provided as a convenience.
    //! @param[in] pos a TxnPos obtained from a previous call to GetCurrTxnPos.
    //! @param[in] callRestartFunc whether to restart the current tool afterwards, only the current tool should pass false.
    //! @return SUCCESS if the transactions were reversed and cleared, ERROR if TxnPos is invalid.
    //! @remarks This method cancels all pending nested transactions.
    DGNPLATFORM_EXPORT StatusInt CancelToPos (TxnId pos, bool callRestartFunc=true);

    //! Clear vestiges of any reversed transactions from memory. Reversed transactions can be reinstated. Therefore, they still
    //! occupy memory in the cache and in the transaction buffer. This method clears them and makes them non-reinstateable.
    //! @remarks If any transactions are reversed, the Transaction Manager will automatically call this method before any new element
    //!          changes can be journaled. That is, after a reverse, ClearReversedTxns is implied if anything other than Reinstate happens.
    DGNPLATFORM_EXPORT void ClearReversedTxns ();

    //! Reinstate the most recent previously applied and then reversed transaction. Since at any time multiple transactions can be reversed, it
    //! may take multiple calls to this method to reinstate all reversed operations.
    //! @return SUCCESS if a reversed transaction was reinstated, ERROR if no transactions were reversed.
    DGNPLATFORM_EXPORT StatusInt ReinstateTxn ();

    //! Query if undo/redo is in progress
    DGNPLATFORM_EXPORT bool IsUndoInProgress ();

    //! Set value for the "TxnSource" to be saved in undo
    DGNPLATFORM_EXPORT void SetTxnSource (UInt64 souce);

    //! Set description of current txn. Used to show what will be undone/redone.
    DGNPLATFORM_EXPORT void SetTxnDescription (Utf8CP descr);

    //! Get the DgnProject of this ITxnManager
    DGNPLATFORM_EXPORT DgnProjectR GetDgnProject();
    //@}
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   07/13
//=======================================================================================
struct TxnAdmin : DgnHost::IHostObject
    {
//__PUBLISH_SECTION_END__
    typedef bvector<TxnMonitorP> TxnMonitors;
protected:
    TxnMonitors m_monitors;

    template <typename CALLER> void CallMonitors (CALLER const& caller)
        {
        try {
            for (auto curr = m_monitors.begin(); curr!=m_monitors.end(); )
                {
                if (*curr == NULL)
                    curr = m_monitors.erase(curr);
                else
                    {caller (**curr); ++curr;}
                }
            }
        catch (...) {}
        }

public:
    DEFINE_BENTLEY_NEW_DELETE_OPERATORS
    
    virtual void _OnHostTermination (bool isProcessShutdown) override {delete this;}
    virtual bool _OnPromptReverseAll() {return true;}
    virtual void _RestartTool()  {}
    virtual void _OnNothingToUndo() {}
    virtual void _OnPrepareForUndoRedo(){}
    virtual void _OnNothingToRedo(){}

    DGNPLATFORM_EXPORT virtual void _OnTxnBoundary (TxnSummaryCR);
    DGNPLATFORM_EXPORT virtual void _OnTxnReverse (TxnSummaryCR, bool isUndo);
    DGNPLATFORM_EXPORT virtual void _OnTxnReversed (TxnSummaryCR, bool isUndo);
    DGNPLATFORM_EXPORT virtual void _OnUndoRedoFinished (DgnProjectR, bool isUndo);
//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__
    //! @name Transaction Monitors
    //@{
    //! Add a TxnMonitor. The monitor will be notified of all transaction events until it is dropped.
    //! @param monitor a monitor to add
    DGNPLATFORM_EXPORT void AddTxnMonitor (TxnMonitor& monitor);

    //! Drop a TxnMonitor.
    //! @param monitor a monitor to drop
    DGNPLATFORM_EXPORT void DropTxnMonitor (TxnMonitor& monitor);
    //@}

    };

END_BENTLEY_DGNPLATFORM_NAMESPACE

/** @endcond */
