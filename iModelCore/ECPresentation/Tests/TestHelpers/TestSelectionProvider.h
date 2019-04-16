/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include "ECPresentationTest.h"
#include <ECPresentation/SelectionManager.h>
#include <ECPresentation/Connection.h>

BEGIN_ECPRESENTATIONTESTS_NAMESPACE

USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                06/2017
+===============+===============+===============+===============+===============+======*/
struct TestSelectionProvider : ISelectionProvider
{
private:
    bmap<ECDb const*, KeySetCPtr> m_selections;
    bmap<ECDb const*, KeySetCPtr> m_subSelections;
protected:
    KeySetCPtr _GetSelection(ECDbCR ecdb) const override
        {
        auto iter = m_selections.find(&ecdb);
        return (m_selections.end() != iter) ? iter->second : nullptr;
        }
    KeySetCPtr _GetSubSelection(ECDbCR ecdb) const override
        {
        auto iter = m_subSelections.find(&ecdb);
        return (m_subSelections.end() != iter) ? iter->second : nullptr;
        }
public:
    void SetSelection(ECDbCR ecdb, KeySetCR selection) {m_selections[&ecdb] = &selection;}
    void SetSubSelection(ECDbCR ecdb, KeySetCR selection) {m_subSelections[&ecdb] = &selection;}
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                11/2017
+===============+===============+===============+===============+===============+======*/
struct TestSelectionManager : ISelectionManager
{
private:
    IConnectionCacheCR m_connections;
    bmap<ECDb const*, KeySetPtr> m_selections;
    bmap<ECDb const*, KeySetPtr> m_subSelections;
    bset<ISelectionChangesListener*> m_listeners;
protected:
    KeySetCPtr _GetSelection(ECDbCR ecdb) const override
        {
        auto iter = m_selections.find(&ecdb);
        return (m_selections.end() != iter) ? iter->second : KeySet::Create();
        }
    KeySetCPtr _GetSubSelection(ECDbCR ecdb) const override
        {
        auto iter = m_subSelections.find(&ecdb);
        return (m_subSelections.end() != iter) ? iter->second : KeySet::Create();
        }
    void _AddListener(ISelectionChangesListener& listener) override {m_listeners.insert(&listener);}
    void _RemoveListener(ISelectionChangesListener& listener) override {m_listeners.erase(&listener);}
    folly::Future<folly::Unit> _AddToSelection(ECDbCR db, Utf8CP, bool isSub, KeySetCR keys, RapidJsonValueCR, uint64_t) override
        {
        KeySetPtr curr = isSub ? m_subSelections[&db] : m_selections[&db];
        if (curr.IsNull())
            curr = KeySet::Create();

        curr->MergeWith(keys);
        if (isSub)
            SetSubSelection(db, *curr);
        else
            SetSelection(db, *curr);
        return folly::makeFuture();
        }
    folly::Future<folly::Unit> _RemoveFromSelection(ECDbCR db, Utf8CP, bool isSub, KeySetCR keys, RapidJsonValueCR, uint64_t) override
        {
        KeySetPtr curr = isSub ? m_subSelections[&db] : m_selections[&db];
        if (curr.IsNull())
            curr = KeySet::Create();

        curr->Remove(keys);
        if (isSub)
            SetSubSelection(db, *curr);
        else
            SetSelection(db, *curr);
        return folly::makeFuture();
        }
    folly::Future<folly::Unit> _ChangeSelection(ECDbCR db, Utf8CP, bool isSub, KeySetCR keys, RapidJsonValueCR, uint64_t) override
        {
        if (isSub)
            SetSubSelection(db, keys);
        else
            SetSelection(db, keys);
        return folly::makeFuture();
        }
    folly::Future<folly::Unit> _ClearSelection(ECDbCR db, Utf8CP, bool isSub, RapidJsonValueCR, uint64_t) override
        {
        if (isSub)
            SetSubSelection(db, *KeySet::Create());
        else
            SetSelection(db, *KeySet::Create());
        return folly::makeFuture();
        }
    folly::Future<folly::Unit> _RefreshSelection(ECDbCR db, Utf8CP name, bool isSub, RapidJsonValueCR extendedData, uint64_t) override
        {
        KeySetPtr curr = isSub ? m_subSelections[&db] : m_selections[&db];
        if (curr.IsNull())
            curr = KeySet::Create();
        return ChangeSelection(db, name, isSub, *curr, extendedData);
        }
public:
    TestSelectionManager(IConnectionCacheCR connections) : m_connections(connections) {}
    void SetSelection(ECDbCR ecdb, KeySetCR selection)
        {
        m_selections[&ecdb] = const_cast<KeySetP>(&selection);
        m_subSelections.erase(&ecdb);
        IConnectionCPtr connection = m_connections.GetConnection(ecdb);
        for (ISelectionChangesListener* listener : m_listeners)
            listener->NotifySelectionChanged(*SelectionChangedEvent::Create(*connection, "TestSource", SelectionChangeType::Replace, false, selection));
        }
    void SetSubSelection(ECDbCR ecdb, KeySetCR selection)
        {
        m_subSelections[&ecdb] = const_cast<KeySetP>(&selection);
        IConnectionCPtr connection = m_connections.GetConnection(ecdb);
        for (ISelectionChangesListener* listener : m_listeners)
            listener->NotifySelectionChanged(*SelectionChangedEvent::Create(*connection, "TestSource", SelectionChangeType::Replace, true, selection));
        }
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                11/2017
+===============+===============+===============+===============+===============+======*/
struct TestSelectionChangesListener : ISelectionChangesListener
{
private:
    std::function<void(SelectionChangedEventCR)> m_callback;
protected:
    folly::Future<folly::Unit> _OnSelectionChanged(SelectionChangedEventCR evt) override
        {
        if (m_callback)
            m_callback(evt);
        return folly::makeFuture();
        }
public:
    TestSelectionChangesListener(std::function<void(SelectionChangedEventCR)> callback = nullptr) : m_callback(callback) {}
    void SetCallback(std::function<void(SelectionChangedEventCR)> callback) {m_callback = callback;}
};

END_ECPRESENTATIONTESTS_NAMESPACE