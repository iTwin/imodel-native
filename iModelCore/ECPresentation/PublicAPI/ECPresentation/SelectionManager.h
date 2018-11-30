/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ECPresentation/SelectionManager.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <ECPresentation/ECPresentationTypes.h>
#include <ECPresentation/KeySet.h>
#include <ECPresentation/Connection.h>

#define LOGGER_NAMESPACE_DGNCLIENTFX_SELECTION "DgnClientFx.Selection"

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

//=======================================================================================
//! Selection provider interface which provides main selection and sub-selection.
//! @note Sub-selection is always a subset of the main selection.
//! @ingroup GROUP_UnifiedSelection
// @bsiclass                                    Grigas.Petraitis                08/2016
//=======================================================================================
struct ISelectionProvider
{
protected:
    //! @see GetSelection
    virtual KeySetCPtr _GetSelection(ECDbCR) const = 0;

    //! @see GetSubSelection
    virtual KeySetCPtr _GetSubSelection(ECDbCR) const = 0;

public:
    //! Virtual destructor.
    virtual ~ISelectionProvider() {}

    //! Get the main selection.
    //! @param[in] db The db to get the selection for.
    KeySetCPtr GetSelection(ECDbCR db) const {return _GetSelection(db);}

    //! Get the sub-selection.
    //! @param[in] db The db to get the sub-selection for.
    KeySetCPtr GetSubSelection(ECDbCR db) const {return _GetSubSelection(db);}
};

//=======================================================================================
//! The type of selection change.
//! @ingroup GROUP_UnifiedSelection
// @bsiclass                                    Grigas.Petraitis                08/2016
//=======================================================================================
enum class SelectionChangeType
    {
    Add,     //!< Added to selection.
    Remove,  //!< Removed from selection.
    Replace, //!< Selection was replaced.
    Clear,   //!< Selection was cleared.
    };

//=======================================================================================
//! The event object that's sent when the selection changes.
//! @ingroup GROUP_UnifiedSelection
// @bsiclass                                    Grigas.Petraitis                08/2016
//=======================================================================================
struct SelectionChangedEvent : RefCountedBase, RapidJsonExtendedDataHolder<>
{
private:
    IConnection const* m_connection;
    Utf8String m_sourceName;
    SelectionChangeType m_changeType;
    bool m_isSubSelection;
    KeySetCPtr m_keys;
    uint64_t m_timestamp;

    SelectionChangedEvent(IConnectionCR connection, Utf8String sourceName, SelectionChangeType changeType, bool isSubSelection, KeySetCR keys, uint64_t timestamp)
        : m_connection(&connection), m_sourceName(sourceName), m_changeType(changeType), m_isSubSelection(isSubSelection), m_keys(&keys), m_timestamp(timestamp)
        {}

    SelectionChangedEvent(SelectionChangedEventCR other)
        : m_connection(other.m_connection), m_sourceName(other.m_sourceName), m_changeType(other.m_changeType), m_isSubSelection(other.m_isSubSelection),
        m_keys(other.m_keys), m_timestamp(other.m_timestamp)
        {}

public:
    //! Constructor. Creates the event based on the supplied parameters.
    ECPRESENTATION_EXPORT static SelectionChangedEventPtr Create(IConnectionCR connection, Utf8String sourceName, SelectionChangeType changeType, 
        bool isSubSelection, KeySetCR keys, uint64_t timestamp = BeTimeUtilities::GetCurrentTimeAsUnixMillis())
        {
        return new SelectionChangedEvent(connection, sourceName, changeType, isSubSelection, keys, timestamp);
        }

    //! Copy-constructor. Copies the supplied event.
    ECPRESENTATION_EXPORT static SelectionChangedEventPtr Create(SelectionChangedEventCR other) {return new SelectionChangedEvent(other);}

    //! Is this event valid.
    bool IsValid() const {return nullptr != m_connection;}
    
    //! Get the connection.
    IConnectionCR GetConnection() const {return *m_connection;}

    //! Get the ECDb.
    ECDbCR GetDb() const {return m_connection->GetECDb();}

    //! Get the selection source name.
    Utf8StringCR GetSourceName() const {return m_sourceName;}

    //! Get the selection change type.
    SelectionChangeType GetChangeType() const {return m_changeType;}

    //! Is this a sub-selection change event.
    bool IsSubSelection() const {return m_isSubSelection;}

    //! Get the selection affected by this selection change event.
    KeySetCR GetSelectedKeys() const {return *m_keys;}

    // Timestamp of when the selection event happened.
    uint64_t GetTimestamp() const {return m_timestamp;}

    //! Serialize this event to JSON.
    ECPRESENTATION_EXPORT rapidjson::Document AsJson(rapidjson::Document::AllocatorType* = nullptr) const;
    ECPRESENTATION_EXPORT static SelectionChangedEventPtr FromJson(IConnectionCacheCR, JsonValueCR);
};

//=======================================================================================
//! An interface for selection change listeners.
//! @see SelectionManager::AddListener
//! @ingroup GROUP_UnifiedSelection
// @bsiclass                                    Grigas.Petraitis                08/2016
//=======================================================================================
struct ISelectionChangesListener
{
protected:
    //! A callback that's called when the selection changes.
    //! @param[in] evt The selection change event.
    //! @note The callback may be called from any thread.
    virtual folly::Future<folly::Unit> _OnSelectionChanged(SelectionChangedEventCR evt) = 0;

public:
    //! Virtual destructor.
    virtual ~ISelectionChangesListener() {}
    folly::Future<folly::Unit> NotifySelectionChanged(SelectionChangedEventCR evt) { return _OnSelectionChanged(evt); }
};

//=======================================================================================
//! A base abstract class for selection synchronization handlers. Basically this is just a
//! helper for synchronizing selection in the @ref SelectionManager and other places like
//! DgnPlatform's SelectionSetManager or UI controls.
//!
//! For @ref SelectionManager -> outside synchronization it uses @ref IECPresentationManager
//! and its Content APIs to determine what should be selected.
//!
//! @see SelectionManager::AddSyncHandler
//!
//! @ingroup GROUP_UnifiedSelection
// @bsiclass                                    Grigas.Petraitis                08/2016
//=======================================================================================
struct SelectionSyncHandler : RefCountedBase, ISelectionChangesListener
{
friend struct SelectionManager;

    //===================================================================================
    //! Defines the synchronization direction.
    // @bsiclass                                Grigas.Petraitis                08/2016
    //===================================================================================
    enum class SyncDirection
        {
        SelectionManager, //!< The selection changes are sent to SelectionManager
        Outside,          //!< The selection changes from SelectionManager are sent to outside handlers
        Both              //!< Two-way synchronization
        };

private:
    SelectionManager* m_manager;

private:
    NativeLogging::ILogger& GetLogger() const;
    void OnRegistered(SelectionManager&);
    void OnUnregistered(SelectionManager&);
    folly::Future<folly::Unit> CallSelectInstances(SelectionChangedEventCR, bvector<ECClassInstanceKey>);

protected:
    //! ISelectionChangesListener implementation. Handles the selection event.
    ECPRESENTATION_EXPORT folly::Future<folly::Unit> _OnSelectionChanged(SelectionChangedEventCR) override;

protected:
    //! Get the selection event extended data. The content of the extended data
    //! depends on the @ref IECPresentationManager implementation.
    virtual rapidjson::Document _CreateSelectionEventExtendedData() const = 0;

    //! Get the name of this selection synchronization handler. This name is used
    //! to avoid handling selection change more than once.
    virtual Utf8CP _GetSelectionSourceName() const {return nullptr;}

    //! Called to determine whether this handler handles sub-selection.
    virtual bool _HandlesSubSelection() const {return true;}

    //! Called to determine synchronization direction of this handler.
    //! @note In case of SyncDirection::SelectionManager, none of the below virtual methods are called.
    virtual SyncDirection _GetSyncDirection() const = 0;

    //! Get the content display type used to query the content for.
    virtual Utf8CP _GetContentDisplayType() const {return nullptr;}

    //! Create content request options object from the provided SelectionChangedEvent.
    //! The format of the JSON object depends on the @ref IECPresentationManager implementation.
    virtual Json::Value _CreateContentOptionsForSelection(SelectionChangedEventCR) const {return Json::Value::GetNull();}

    //! Called to select ECInstances.
    //! @param[in] evt  The selection change event that resulted in this method being called.
    //! @param[in] keys The keys of ECInstances to select.
    virtual void _SelectInstances(SelectionChangedEventCR evt, bvector<ECClassInstanceKey> const& keys) {}

    //! Called to get an executor to call _SelectInstances callback.
    //! @return An executor to call the callback or nullptr to call using current executor on arbitrary thread.
    virtual folly::Executor* _GetSelectExecutor() const {return nullptr;}

//__PUBLISH_SECTION_END__
protected:
    //! Handle the supplied selection change event.
    //! @param[in] evt The event to handle.
    ECPRESENTATION_EXPORT folly::Future<folly::Unit> HandleSelectionChangeEvent(SelectionChangedEventCR evt);

//__PUBLISH_SECTION_START__
protected:
    //! Constructor.
    SelectionSyncHandler() : m_manager(nullptr) {}

    //! Get the selection manager used by this sync handler.
    //! @note Returns null if the handler is not registered.
    SelectionManagerP GetSelectionManager() const {return m_manager;}

    //! Add to selection.
    //! @param[in] db The ECDb to add the selection to.
    //! @param[in] isSubSelection A flag indicating whether to add to the sub-selection or the main selection.
    //! @param[in] keys The keys to add to selection.
    //! @param[in] timestamp Time of when the selection changed
    //! @see SelectionManager::AddToSelection
    ECPRESENTATION_EXPORT folly::Future<folly::Unit> AddToSelection(ECDbCR db, bool isSubSelection, KeySetCR keys, uint64_t timestamp = BeTimeUtilities::GetCurrentTimeAsUnixMillis());

    //! Remove from selection.
    //! @param[in] db The ECDb to remove the selection from.
    //! @param[in] isSubSelection A flag indicating whether to remove from the sub-selection or the main selection.
    //! @param[in] keys The keys to remove from selection.
    //! @param[in] timestamp Time of when the selection changed
    //! @see SelectionManager::RemoveFromSelection
    ECPRESENTATION_EXPORT folly::Future<folly::Unit> RemoveFromSelection(ECDbCR db, bool isSubSelection, KeySetCR keys, uint64_t timestamp = BeTimeUtilities::GetCurrentTimeAsUnixMillis());

    //! Change selection.
    //! @param[in] db The ECDb to change the selection in.
    //! @param[in] isSubSelection A flag indicating whether to change the sub-selection or the main selection.
    //! @param[in] keys The keys indicating the new selection.
    //! @param[in] timestamp Time of when the selection changed
    //! @see SelectionManager::ChangeSelection
    ECPRESENTATION_EXPORT folly::Future<folly::Unit> ChangeSelection(ECDbCR db, bool isSubSelection, KeySetCR keys, uint64_t timestamp = BeTimeUtilities::GetCurrentTimeAsUnixMillis());

    //! Clear selection.
    //! @param[in] db The ECDb to clear the selection in.
    //! @param[in] isSubSelection A flag indicating whether to clear the sub-selection or the main selection.
    //! @param[in] timestamp Time of when the selection changed
    //! @see SelectionManager::ClearSelection
    ECPRESENTATION_EXPORT folly::Future<folly::Unit> ClearSelection(ECDbCR db, bool isSubSelection, uint64_t timestamp = BeTimeUtilities::GetCurrentTimeAsUnixMillis());
};

//=======================================================================================
//! Selection manager interface which provides a way to listen for selection changes.
//! @note Sub-selection is always a subset of the main selection.
//! @ingroup GROUP_UnifiedSelection
// @bsiclass                                    Grigas.Petraitis                08/2016
//=======================================================================================
struct ISelectionManager : ISelectionProvider
{
protected:
    //! @see AddListener
    virtual void _AddListener(ISelectionChangesListener&) = 0;

    //! @see RemoveListener
    virtual void _RemoveListener(ISelectionChangesListener&) = 0;
    
    //! @see AddToSelection
    virtual folly::Future<folly::Unit> _AddToSelection(ECDbCR, Utf8CP source, bool isSubSelection, KeySetCR, RapidJsonValueCR extendedData, uint64_t timestamp) = 0;
    
    //! @see RemoveFromSelection
    virtual folly::Future<folly::Unit> _RemoveFromSelection(ECDbCR, Utf8CP source, bool isSubSelection, KeySetCR, RapidJsonValueCR extendedData, uint64_t timestamp) = 0;
    
    //! @see ChangeSelection
    virtual folly::Future<folly::Unit> _ChangeSelection(ECDbCR, Utf8CP source, bool isSubSelection, KeySetCR, RapidJsonValueCR extendedData, uint64_t timestamp) = 0;
    
    //! @see ClearSelection
    virtual folly::Future<folly::Unit> _ClearSelection(ECDbCR, Utf8CP source, bool isSubSelection, RapidJsonValueCR extendedData, uint64_t timestamp) = 0;

    //! @see RefreshSelection
    virtual folly::Future<folly::Unit> _RefreshSelection(ECDbCR, Utf8CP source, bool isSubSelection, RapidJsonValueCR extendedData, uint64_t timestamp) = 0;

public:
    //! Register the selection changes listener.
    //! @note The listener must stay valid for the lifetime of the selection provider.
    void AddListener(ISelectionChangesListener& listener) {_AddListener(listener);}

    //! Unregister the selection changes listener.
    void RemoveListener(ISelectionChangesListener& listener) {_RemoveListener(listener);}
    
    //! Add to selection.
    //! @param[in] db The ECDb to add the selection to.
    //! @param[in] source The name of the selection source that is modifying the selection.
    //! @param[in] isSubSelection A flag indicating whether to add to the sub-selection or the main selection.
    //! @param[in] key The key to add to selection.
    //! @param[in] extendedData The extended data that should be stored in the selection change event.
    //! @param[in] timestamp Time of when the selection changed
    ECPRESENTATION_EXPORT folly::Future<folly::Unit> AddToSelection(ECDbCR db, Utf8CP source, bool isSubSelection, NavNodeKeyCR key, RapidJsonValueCR extendedData = rapidjson::Value(), uint64_t timestamp = BeTimeUtilities::GetCurrentTimeAsUnixMillis());
    
    //! Add to selection.
    //! @param[in] db The ECDb to add the selection to.
    //! @param[in] source The name of the selection source that is modifying the selection.
    //! @param[in] isSubSelection A flag indicating whether to add to the sub-selection or the main selection.
    //! @param[in] key The key to add to selection.
    //! @param[in] extendedData The extended data that should be stored in the selection change event.
    //! @param[in] timestamp Time of when the selection changed
    ECPRESENTATION_EXPORT folly::Future<folly::Unit> AddToSelection(ECDbCR db, Utf8CP source, bool isSubSelection, ECClassInstanceKeyCR key, RapidJsonValueCR extendedData = rapidjson::Value(), uint64_t timestamp = BeTimeUtilities::GetCurrentTimeAsUnixMillis());
    
    //! Add to selection.
    //! @param[in] db The ECDb to add the selection to.
    //! @param[in] source The name of the selection source that is modifying the selection.
    //! @param[in] isSubSelection A flag indicating whether to add to the sub-selection or the main selection.
    //! @param[in] key The key to add to selection.
    //! @param[in] extendedData The extended data that should be stored in the selection change event.
    //! @param[in] timestamp Time of when the selection changed
    ECPRESENTATION_EXPORT folly::Future<folly::Unit> AddToSelection(ECDbCR db, Utf8CP source, bool isSubSelection, ECInstanceKeyCR key, RapidJsonValueCR extendedData = rapidjson::Value(), uint64_t timestamp = BeTimeUtilities::GetCurrentTimeAsUnixMillis());

    //! Add to selection.
    //! @param[in] db The ECDb to add the selection to.
    //! @param[in] source The name of the selection source that is modifying the selection.
    //! @param[in] isSubSelection A flag indicating whether to add to the sub-selection or the main selection.
    //! @param[in] keys The keys to add to selection.
    //! @param[in] extendedData The extended data that should be stored in the selection change event.
    //! @param[in] timestamp Time of when the selection changed
    folly::Future<folly::Unit> AddToSelection(ECDbCR db, Utf8CP source, bool isSubSelection, KeySetCR keys, RapidJsonValueCR extendedData = rapidjson::Value(), uint64_t timestamp = BeTimeUtilities::GetCurrentTimeAsUnixMillis())
        {
        return _AddToSelection(db, source, isSubSelection, keys, extendedData, timestamp);
        }

    //! Remove from selection.
    //! @param[in] db The ECDb to remove the selection from.
    //! @param[in] source The name of the selection source that is modifying the selection.
    //! @param[in] isSubSelection A flag indicating whether to remove from the sub-selection or the main selection.
    //! @param[in] key The key to remove from selection.
    //! @param[in] extendedData The extended data that should be stored in the selection change event.
    //! @param[in] timestamp Time of when the selection changed
    ECPRESENTATION_EXPORT folly::Future<folly::Unit> RemoveFromSelection(ECDbCR db, Utf8CP source, bool isSubSelection, NavNodeKeyCR key, RapidJsonValueCR extendedData = rapidjson::Value(), uint64_t timestamp = BeTimeUtilities::GetCurrentTimeAsUnixMillis());

    //! Remove from selection.
    //! @param[in] db The ECDb to remove the selection from.
    //! @param[in] source The name of the selection source that is modifying the selection.
    //! @param[in] isSubSelection A flag indicating whether to remove from the sub-selection or the main selection.
    //! @param[in] key The key to remove from selection.
    //! @param[in] extendedData The extended data that should be stored in the selection change event.
    //! @param[in] timestamp Time of when the selection changed
    ECPRESENTATION_EXPORT folly::Future<folly::Unit> RemoveFromSelection(ECDbCR db, Utf8CP source, bool isSubSelection, ECClassInstanceKeyCR key, RapidJsonValueCR extendedData = rapidjson::Value(), uint64_t timestamp = BeTimeUtilities::GetCurrentTimeAsUnixMillis());
    
    //! Remove from selection.
    //! @param[in] db The ECDb to remove the selection from.
    //! @param[in] source The name of the selection source that is modifying the selection.
    //! @param[in] isSubSelection A flag indicating whether to remove from the sub-selection or the main selection.
    //! @param[in] key The key to remove from selection.
    //! @param[in] extendedData The extended data that should be stored in the selection change event.
    //! @param[in] timestamp Time of when the selection changed
    ECPRESENTATION_EXPORT folly::Future<folly::Unit> RemoveFromSelection(ECDbCR db, Utf8CP source, bool isSubSelection, ECInstanceKeyCR key, RapidJsonValueCR extendedData = rapidjson::Value(), uint64_t timestamp = BeTimeUtilities::GetCurrentTimeAsUnixMillis());

    //! Remove from selection.
    //! @param[in] db The ECDb to remove the selection from.
    //! @param[in] source The name of the selection source that is modifying the selection.
    //! @param[in] isSubSelection A flag indicating whether to remove from the sub-selection or the main selection.
    //! @param[in] keys The keys to remove from selection.
    //! @param[in] extendedData The extended data that should be stored in the selection change event.
    //! @param[in] timestamp Time of when the selection changed
    folly::Future<folly::Unit> RemoveFromSelection(ECDbCR db, Utf8CP source, bool isSubSelection, KeySetCR keys, RapidJsonValueCR extendedData = rapidjson::Value(), uint64_t timestamp = BeTimeUtilities::GetCurrentTimeAsUnixMillis())
        {
        return _RemoveFromSelection(db, source, isSubSelection, keys, extendedData, timestamp);
        }

    //! Change selection.
    //! @param[in] db The ECDb to change the selection in.
    //! @param[in] source The name of the selection source that is modifying the selection.
    //! @param[in] isSubSelection A flag indicating whether to change the sub-selection or the main selection.
    //! @param[in] key The key indicating the new selection.
    //! @param[in] extendedData The extended data that should be stored in the selection change event.
    //! @param[in] timestamp Time of when the selection changed
    ECPRESENTATION_EXPORT folly::Future<folly::Unit> ChangeSelection(ECDbCR db, Utf8CP source, bool isSubSelection, NavNodeKeyCR key, RapidJsonValueCR extendedData = rapidjson::Value(), uint64_t timestamp = BeTimeUtilities::GetCurrentTimeAsUnixMillis());

    //! Change selection.
    //! @param[in] db The ECDb to change the selection in.
    //! @param[in] source The name of the selection source that is modifying the selection.
    //! @param[in] isSubSelection A flag indicating whether to change the sub-selection or the main selection.
    //! @param[in] key The key indicating the new selection.
    //! @param[in] extendedData The extended data that should be stored in the selection change event.
    //! @param[in] timestamp Time of when the selection changed
    ECPRESENTATION_EXPORT folly::Future<folly::Unit> ChangeSelection(ECDbCR db, Utf8CP source, bool isSubSelection, ECClassInstanceKeyCR key, RapidJsonValueCR extendedData = rapidjson::Value(), uint64_t timestamp = BeTimeUtilities::GetCurrentTimeAsUnixMillis());
    
    //! Change selection.
    //! @param[in] db The ECDb to change the selection in.
    //! @param[in] source The name of the selection source that is modifying the selection.
    //! @param[in] isSubSelection A flag indicating whether to change the sub-selection or the main selection.
    //! @param[in] key The key indicating the new selection.
    //! @param[in] extendedData The extended data that should be stored in the selection change event.
    //! @param[in] timestamp Time of when the selection changed
    ECPRESENTATION_EXPORT folly::Future<folly::Unit> ChangeSelection(ECDbCR db, Utf8CP source, bool isSubSelection, ECInstanceKeyCR key, RapidJsonValueCR extendedData = rapidjson::Value(), uint64_t timestamp = BeTimeUtilities::GetCurrentTimeAsUnixMillis());

    //! Change selection.
    //! @param[in] db The ECDb to change the selection in.
    //! @param[in] source The name of the selection source that is modifying the selection.
    //! @param[in] isSubSelection A flag indicating whether to change the sub-selection or the main selection.
    //! @param[in] keys The keys indicating the new selection.
    //! @param[in] extendedData The extended data that should be stored in the selection change event.
    //! @param[in] timestamp Time of when the selection changed
    folly::Future<folly::Unit> ChangeSelection(ECDbCR db, Utf8CP source, bool isSubSelection, KeySetCR keys, RapidJsonValueCR extendedData = rapidjson::Value(), uint64_t timestamp = BeTimeUtilities::GetCurrentTimeAsUnixMillis())
        {
        return _ChangeSelection(db, source, isSubSelection, keys, extendedData, timestamp);
        }

    //! Clear selection.
    //! @param[in] db The ECDb to clear the selection in.
    //! @param[in] source The name of the selection source that is modifying the selection.
    //! @param[in] isSubSelection A flag indicating whether to clear the sub-selection or the main selection.
    //! @param[in] extendedData The extended data that should be stored in the selection change event.
    //! @param[in] timestamp Time of when the selection changed
    folly::Future<folly::Unit> ClearSelection(ECDbCR db, Utf8CP source, bool isSubSelection, RapidJsonValueCR extendedData = rapidjson::Value(), uint64_t timestamp = BeTimeUtilities::GetCurrentTimeAsUnixMillis())
        {
        return _ClearSelection(db, source, isSubSelection, extendedData, timestamp);
        }

    //! Refresh selection.
    //! @param[in] db The ECDb to refresh the selection in.
    //! @param[in] source The name of the selection source that is modifying the selection.
    //! @param[in] isSubSelection A flag indicating whether to refresh the sub-selection or the main selection.
    //! @param[in] extendedData The extended data that should be stored in the selection change event.
    //! @param[in] timestamp Time of when the selection changed
    folly::Future<folly::Unit> RefreshSelection(ECDbCR db, Utf8CP source, bool isSubSelection, RapidJsonValueCR extendedData = rapidjson::Value(), uint64_t timestamp = BeTimeUtilities::GetCurrentTimeAsUnixMillis())
        {
        return _RefreshSelection(db, source, isSubSelection, extendedData, timestamp);
        }
};

//=======================================================================================
//! The selection manager which stores the overall selection.
//! @see PAGE_UnifiedSelection
//! @ingroup GROUP_UnifiedSelection
// @bsiclass                                    Grigas.Petraitis                08/2016
//=======================================================================================
struct SelectionManager : NonCopyableClass, ISelectionManager
{
    ECPRESENTATION_EXPORT static const Utf8CP MESSAGE_SelectionChanged;

    struct ECDbSelection;
    struct SelectionStorage;

private:
    IConnectionCacheCR m_connections;
    mutable bmap<Utf8String, ECDbSelection*> m_selections;
    bvector<ISelectionChangesListener*> m_listeners;
    bvector<SelectionSyncHandlerPtr> m_syncHandlers;
    mutable BeMutex m_mutex;

private:
    SelectionStorage& GetStorage(IConnectionCR, bool isSubSelection) const;
    NativeLogging::ILogger& GetLogger() const;
    void OnECDbClosed(ECDbCR) const;
    folly::Future<folly::Unit> BroadcastSelectionChangedEvent(IConnectionCR, Utf8CP source, SelectionChangeType changeType, bool isSubSelection, KeySetCR, RapidJsonValueCR, uint64_t) const;

protected:
    //! @see ISelectionProvider::GetSelection
    ECPRESENTATION_EXPORT KeySetCPtr _GetSelection(ECDbCR) const override;

    //! @see ISelectionProvider::GetSubSelection
    ECPRESENTATION_EXPORT KeySetCPtr _GetSubSelection(ECDbCR) const override;

    //! @see ISelectionManager::AddListener
    ECPRESENTATION_EXPORT void _AddListener(ISelectionChangesListener&) override;

    //! @see ISelectionManager::RemoveListener
    ECPRESENTATION_EXPORT void _RemoveListener(ISelectionChangesListener&) override;
    
    //! @see ISelectionManager::AddToSelection
    ECPRESENTATION_EXPORT folly::Future<folly::Unit> _AddToSelection(ECDbCR, Utf8CP, bool, KeySetCR, RapidJsonValueCR, uint64_t timestamp) override;
    
    //! @see ISelectionManager::RemoveFromSelection
    ECPRESENTATION_EXPORT folly::Future<folly::Unit> _RemoveFromSelection(ECDbCR, Utf8CP, bool, KeySetCR, RapidJsonValueCR, uint64_t timestamp) override;
    
    //! @see ISelectionManager::ChangeSelection
    ECPRESENTATION_EXPORT folly::Future<folly::Unit> _ChangeSelection(ECDbCR, Utf8CP, bool, KeySetCR, RapidJsonValueCR, uint64_t timestamp) override;
    
    //! @see ISelectionManager::ClearSelection
    ECPRESENTATION_EXPORT folly::Future<folly::Unit> _ClearSelection(ECDbCR, Utf8CP, bool, RapidJsonValueCR, uint64_t timestamp) override;

    //! @see ISelectionManager::RefreshSelection
    ECPRESENTATION_EXPORT folly::Future<folly::Unit> _RefreshSelection(ECDbCR, Utf8CP, bool, RapidJsonValueCR, uint64_t timestamp) override;

public:
    SelectionManager(IConnectionCacheCR connections) : m_connections(connections) {}
    ECPRESENTATION_EXPORT ~SelectionManager();

    //! Get the connection cache used by this selection manager.
    IConnectionCacheCR GetConnections() const {return m_connections;}

    //! Register the selection synchronization handler.
    ECPRESENTATION_EXPORT void AddSyncHandler(SelectionSyncHandlerR);

    //! Unregister the selection synchronization handler.
    ECPRESENTATION_EXPORT void RemoveSyncHandler(SelectionSyncHandlerR);
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
