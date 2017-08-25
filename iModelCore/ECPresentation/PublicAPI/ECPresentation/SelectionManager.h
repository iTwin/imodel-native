/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ECPresentation/SelectionManager.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <ECPresentation/ECPresentationTypes.h>
#include <ECPresentation/NavNode.h>

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
    virtual INavNodeKeysContainerCPtr _GetSelection(BeSQLite::EC::ECDbR) const = 0;

    //! @see GetSubSelection
    virtual INavNodeKeysContainerCPtr _GetSubSelection(BeSQLite::EC::ECDbR) const = 0;

public:
    //! Virtual destructor.
    virtual ~ISelectionProvider() {}

    //! Get the main selection.
    //! @param[in] connection The connection to get the selection for.
    INavNodeKeysContainerCPtr GetSelection(BeSQLite::EC::ECDbR connection) const {return _GetSelection(connection);}
    
    //! Get the sub-selection.
    //! @param[in] connection The connection to get the sub-selection for.
    INavNodeKeysContainerCPtr GetSubSelection(BeSQLite::EC::ECDbR connection) const {return _GetSubSelection(connection);}
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
struct SelectionChangedEvent : RapidJsonExtendedDataHolder<>
{
    ECPRESENTATION_EXPORT static const Utf8CP JSON_MEMBER_Source;
    ECPRESENTATION_EXPORT static const Utf8CP JSON_MEMBER_ConnectionId;
    ECPRESENTATION_EXPORT static const Utf8CP JSON_MEMBER_IsSubSelection;
    ECPRESENTATION_EXPORT static const Utf8CP JSON_MEMBER_ChangeType;
    ECPRESENTATION_EXPORT static const Utf8CP JSON_MEMBER_Keys;
    ECPRESENTATION_EXPORT static const Utf8CP JSON_MEMBER_ExtendedData;

private:
    BeSQLite::EC::ECDb* m_connection;
    Utf8String m_sourceName;
    SelectionChangeType m_changeType;
    bool m_isSubSelection;
    INavNodeKeysContainerCPtr m_keys;

public:
    //! Constructor. Creates the event based on the supplied parameters.
    SelectionChangedEvent(BeSQLite::EC::ECDbR connection, Utf8String sourceName, SelectionChangeType changeType, bool isSubSelection, INavNodeKeysContainerCR keys)
        : m_connection(&connection), m_sourceName(sourceName), m_changeType(changeType), m_isSubSelection(isSubSelection), m_keys(&keys)
        {}

    //! Copy-constructor. Copies the supplied event.
    SelectionChangedEvent(SelectionChangedEventCR other)
        : m_connection(other.m_connection), m_sourceName(other.m_sourceName), m_changeType(other.m_changeType), m_isSubSelection(other.m_isSubSelection), m_keys(other.m_keys)
        {}

    //! Constructor. Creates the event from the JSON object. Uses the supplied connection cache to find the connection.
    ECPRESENTATION_EXPORT SelectionChangedEvent(IConnectionCacheCR, JsonValueCR);

    //! Is this event valid.
    bool IsValid() const {return nullptr != m_connection;}

    //! Get the connection.
    BeSQLite::EC::ECDbR GetConnection() const {return *m_connection;}

    //! Get the selection source name.
    Utf8StringCR GetSourceName() const {return m_sourceName;}

    //! Get the selection change type.
    SelectionChangeType GetChangeType() const {return m_changeType;}

    //! Is this a sub-selection change event.
    bool IsSubSelection() const {return m_isSubSelection;}

    //! Get the selection affected by this selection change event.
    INavNodeKeysContainerCR GetSelectedKeys() const {return *m_keys;}

    //! Serialize this event to JSON.
    ECPRESENTATION_EXPORT rapidjson::Document AsJson(rapidjson::Document::AllocatorType* = nullptr) const;
};

//=======================================================================================
//! An interface for selection change listeners.
//! @see SelectionManager::AddListener
//! @ingroup GROUP_UnifiedSelection
// @bsiclass                                    Grigas.Petraitis                08/2016
//=======================================================================================
struct ISelectionChangesListener
{
friend struct SelectionManager;

protected:
    //! Virtual destructor.
    virtual ~ISelectionChangesListener() {}

    //! A callback that's called when the selection changes.
    //! @param[in] evt The selection change event.
    virtual void _OnSelectionChanged(SelectionChangedEventCR evt) = 0;
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
    void OnRegistered(SelectionManager&);
    void OnUnregistered(SelectionManager&);

protected:
    //! ISelectionChangesListener implementation. Handles the selection event.
    ECPRESENTATION_EXPORT void _OnSelectionChanged(SelectionChangedEventCR) override;

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

    //! Create content request options object from the provided SelectionChangedEvent.
    //! The format of the JSON object depends on the @ref IECPresentationManager implementation.
    virtual Json::Value _CreateContentOptionsForSelection(SelectionChangedEventCR) const {return Json::Value::GetNull();}

    //! Called to select ECInstances.
    //! @param[in] evt  The selection change event that resulted in this method being called.
    //! @param[in] keys The keys of ECInstances to select.
    virtual void _SelectInstances(SelectionChangedEventCR evt, bvector<BeSQLite::EC::ECInstanceKey> const& keys) {}
    
//__PUBLISH_SECTION_END__
protected:
    //! Handle the supplied selection change event.
    //! @param[in] evt The event to handle.
    ECPRESENTATION_EXPORT void HandleSelectionChangeEvent(SelectionChangedEventCR evt);

//__PUBLISH_SECTION_START__
protected:
    //! Constructor.
    SelectionSyncHandler() : m_manager(nullptr) {}

    //! Add to selection.
    //! @param[in] connection The connection to add the selection to.
    //! @param[in] isSubSelection A flag indicating whether to add to the sub-selection or the main selection.
    //! @param[in] keys The keys to add to selection.
    //! @see SelectionManager::AddToSelection
    ECPRESENTATION_EXPORT void AddToSelection(BeSQLite::EC::ECDbR connection, bool isSubSelection, INavNodeKeysContainerCR keys);
    
    //! Remove from selection.
    //! @param[in] connection The connection to remove the selection from.
    //! @param[in] isSubSelection A flag indicating whether to remove from the sub-selection or the main selection.
    //! @param[in] keys The keys to remove from selection.
    //! @see SelectionManager::RemoveFromSelection
    ECPRESENTATION_EXPORT void RemoveFromSelection(BeSQLite::EC::ECDbR connection, bool isSubSelection, INavNodeKeysContainerCR keys);
    
    //! Change selection.
    //! @param[in] connection The connection to change the selection in.
    //! @param[in] isSubSelection A flag indicating whether to change the sub-selection or the main selection.
    //! @param[in] keys The keys indicating the new selection.
    //! @see SelectionManager::ChangeSelection
    ECPRESENTATION_EXPORT void ChangeSelection(BeSQLite::EC::ECDbR connection, bool isSubSelection, INavNodeKeysContainerCR keys);
    
    //! Clear selection.
    //! @param[in] connection The connection to clear the selection in.
    //! @param[in] isSubSelection A flag indicating whether to clear the sub-selection or the main selection.
    //! @see SelectionManager::ClearSelection
    ECPRESENTATION_EXPORT void ClearSelection(BeSQLite::EC::ECDbR connection, bool isSubSelection);
};

//=======================================================================================
//! The selection manager which stores the overall selection.
//! @see PAGE_UnifiedSelection
//! @ingroup GROUP_UnifiedSelection
// @bsiclass                                    Grigas.Petraitis                08/2016
//=======================================================================================
struct SelectionManager : NonCopyableClass, ISelectionProvider
{
    ECPRESENTATION_EXPORT static const Utf8CP MESSAGE_SelectionChanged;

    struct ECDbSelection;
    struct SelectionStorage;

private:
    mutable bmap<BeSQLite::BeGuid, ECDbSelection*> m_selections;
    bvector<ISelectionChangesListener*> m_listeners;
    bvector<SelectionSyncHandlerPtr> m_syncHandlers;

private:
    SelectionStorage& GetStorage(BeSQLite::EC::ECDbCR, bool isSubSelection) const;
    NativeLogging::ILogger& GetLogger() const;
    void OnConnectionClosed(BeSQLite::EC::ECDbCR) const;
    void BroadcastSelectionChangedEvent(BeSQLite::EC::ECDbR, Utf8CP source, SelectionChangeType changeType, bool isSubSelection, INavNodeKeysContainerCR, RapidJsonValueCR) const;
    
protected:
    //! @see ISelectionProvider::GetSelection
    ECPRESENTATION_EXPORT INavNodeKeysContainerCPtr _GetSelection(BeSQLite::EC::ECDbR) const override;
    
    //! @see ISelectionProvider::GetSubSelection
    ECPRESENTATION_EXPORT INavNodeKeysContainerCPtr _GetSubSelection(BeSQLite::EC::ECDbR) const override;

public:
    ECPRESENTATION_EXPORT ~SelectionManager();

    //! Add to selection.
    //! @param[in] connection The connection to add the selection to.
    //! @param[in] source The name of the selection source that is modifying the selection.
    //! @param[in] isSubSelection A flag indicating whether to add to the sub-selection or the main selection.
    //! @param[in] key The key to add to selection.
    //! @param[in] extendedData The extended data that should be stored in the selection change event.
    ECPRESENTATION_EXPORT void AddToSelection(BeSQLite::EC::ECDbR connection, Utf8CP source, bool isSubSelection, NavNodeKeyCR key, RapidJsonValueCR extendedData = rapidjson::Value());
    
    //! Add to selection.
    //! @param[in] connection The connection to add the selection to.
    //! @param[in] source The name of the selection source that is modifying the selection.
    //! @param[in] isSubSelection A flag indicating whether to add to the sub-selection or the main selection.
    //! @param[in] keys The keys to add to selection.
    //! @param[in] extendedData The extended data that should be stored in the selection change event.
    ECPRESENTATION_EXPORT void AddToSelection(BeSQLite::EC::ECDbR connection, Utf8CP source, bool isSubSelection, INavNodeKeysContainerCR keys, RapidJsonValueCR extendedData = rapidjson::Value());
    
    //! Remove from selection.
    //! @param[in] connection The connection to remove the selection from.
    //! @param[in] source The name of the selection source that is modifying the selection.
    //! @param[in] isSubSelection A flag indicating whether to remove from the sub-selection or the main selection.
    //! @param[in] key The key to remove from selection.
    //! @param[in] extendedData The extended data that should be stored in the selection change event.
    ECPRESENTATION_EXPORT void RemoveFromSelection(BeSQLite::EC::ECDbR connection, Utf8CP source, bool isSubSelection, NavNodeKeyCR key, RapidJsonValueCR extendedData = rapidjson::Value());
    
    //! Remove from selection.
    //! @param[in] connection The connection to remove the selection from.
    //! @param[in] source The name of the selection source that is modifying the selection.
    //! @param[in] isSubSelection A flag indicating whether to remove from the sub-selection or the main selection.
    //! @param[in] keys The keys to remove from selection.
    //! @param[in] extendedData The extended data that should be stored in the selection change event.
    ECPRESENTATION_EXPORT void RemoveFromSelection(BeSQLite::EC::ECDbR connection, Utf8CP source, bool isSubSelection, INavNodeKeysContainerCR keys, RapidJsonValueCR extendedData = rapidjson::Value());
    
    //! Change selection.
    //! @param[in] connection The connection to change the selection in.
    //! @param[in] source The name of the selection source that is modifying the selection.
    //! @param[in] isSubSelection A flag indicating whether to change the sub-selection or the main selection.
    //! @param[in] key The key indicating the new selection.
    //! @param[in] extendedData The extended data that should be stored in the selection change event.
    ECPRESENTATION_EXPORT void ChangeSelection(BeSQLite::EC::ECDbR connection, Utf8CP source, bool isSubSelection, NavNodeKeyCR key, RapidJsonValueCR extendedData = rapidjson::Value());
    
    //! Change selection.
    //! @param[in] connection The connection to change the selection in.
    //! @param[in] source The name of the selection source that is modifying the selection.
    //! @param[in] isSubSelection A flag indicating whether to change the sub-selection or the main selection.
    //! @param[in] keys The keys indicating the new selection.
    //! @param[in] extendedData The extended data that should be stored in the selection change event.
    ECPRESENTATION_EXPORT void ChangeSelection(BeSQLite::EC::ECDbR connection, Utf8CP source, bool isSubSelection, INavNodeKeysContainerCR keys, RapidJsonValueCR extendedData = rapidjson::Value());
    
    //! Clear selection.
    //! @param[in] connection The connection to clear the selection in.
    //! @param[in] source The name of the selection source that is modifying the selection.
    //! @param[in] isSubSelection A flag indicating whether to clear the sub-selection or the main selection.
    //! @param[in] extendedData The extended data that should be stored in the selection change event.
    ECPRESENTATION_EXPORT void ClearSelection(BeSQLite::EC::ECDbR connection, Utf8CP source, bool isSubSelection, RapidJsonValueCR extendedData = rapidjson::Value());

    //! Register the selection changes listener.
    //! @note The listener must stay valid for the lifetime of the selection manager.
    ECPRESENTATION_EXPORT void AddListener(ISelectionChangesListener&);

    //! Unregister the selection changes listener.
    ECPRESENTATION_EXPORT void RemoveListener(ISelectionChangesListener&);
    
    //! Register the selection synchronization handler.
    ECPRESENTATION_EXPORT void AddSyncHandler(SelectionSyncHandlerR);
    
    //! Unregister the selection synchronization handler.
    ECPRESENTATION_EXPORT void RemoveSyncHandler(SelectionSyncHandlerR);
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
