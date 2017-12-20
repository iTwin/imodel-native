/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/TestHelpers/TestSelectionProvider.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
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
    bmap<ECDb const*, INavNodeKeysContainerCPtr> m_selections;
    bmap<ECDb const*, INavNodeKeysContainerCPtr> m_subSelections;
protected:
    INavNodeKeysContainerCPtr _GetSelection(ECDbCR ecdb) const override
        {
        auto iter = m_selections.find(&ecdb);
        return (m_selections.end() != iter) ? iter->second : nullptr;
        }
    INavNodeKeysContainerCPtr _GetSubSelection(ECDbCR ecdb) const override
        {
        auto iter = m_subSelections.find(&ecdb);
        return (m_subSelections.end() != iter) ? iter->second : nullptr;
        }
public:
    void SetSelection(ECDbCR ecdb, INavNodeKeysContainerCR selection) {m_selections[&ecdb] = &selection;}
    void SetSubSelection(ECDbCR ecdb, INavNodeKeysContainerCR selection) {m_subSelections[&ecdb] = &selection;}
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                11/2017
+===============+===============+===============+===============+===============+======*/
struct TestSelectionManager : ISelectionManager
{
private:
    IConnectionCacheCR m_connections;
    bmap<ECDb const*, INavNodeKeysContainerCPtr> m_selections;
    bmap<ECDb const*, INavNodeKeysContainerCPtr> m_subSelections;
    bset<ISelectionChangesListener*> m_listeners;
protected:
    INavNodeKeysContainerCPtr _GetSelection(ECDbCR ecdb) const override
        {
        auto iter = m_selections.find(&ecdb);
        return (m_selections.end() != iter) ? iter->second : nullptr;
        }
    INavNodeKeysContainerCPtr _GetSubSelection(ECDbCR ecdb) const override
        {
        auto iter = m_subSelections.find(&ecdb);
        return (m_subSelections.end() != iter) ? iter->second : nullptr;
        }
    void _AddListener(ISelectionChangesListener& listener) override {m_listeners.insert(&listener);}
    void _RemoveListener(ISelectionChangesListener& listener) override {m_listeners.erase(&listener);}
    void _AddToSelection(ECDbCR db, Utf8CP, bool isSub, INavNodeKeysContainerCR keys, RapidJsonValueCR) override
        {
        INavNodeKeysContainerCPtr curr = isSub ? m_subSelections[&db] : m_selections[&db];
        NavNodeKeySet list;
        if (curr.IsValid())
            {
            for (NavNodeKeyCPtr key : *curr)
                list.insert(key);
            }
        for (NavNodeKeyCPtr key : keys)
            list.insert(key);
        if (isSub)
            SetSubSelection(db, *NavNodeKeySetContainer::Create(list));
        else
            SetSelection(db, *NavNodeKeySetContainer::Create(list));
        }
    void _RemoveFromSelection(ECDbCR db, Utf8CP, bool isSub, INavNodeKeysContainerCR keys, RapidJsonValueCR) override
        {
        INavNodeKeysContainerCPtr curr = isSub ? m_subSelections[&db] : m_selections[&db];
        NavNodeKeyList list;
        for (NavNodeKeyCPtr key : *curr)
            {
            if (keys.end() == keys.find(key))
                list.push_back(key);
            }
        if (isSub)
            SetSubSelection(db, *NavNodeKeyListContainer::Create(list));
        else
            SetSelection(db, *NavNodeKeyListContainer::Create(list));
        }
    void _ChangeSelection(ECDbCR db, Utf8CP, bool isSub, INavNodeKeysContainerCR keys, RapidJsonValueCR) override
        {
        if (isSub)
            SetSubSelection(db, keys);
        else
            SetSelection(db, keys);
        }
    void _ClearSelection(ECDbCR db, Utf8CP, bool isSub, RapidJsonValueCR) override
        {
        if (isSub)
            SetSubSelection(db, *NavNodeKeyListContainer::Create());
        else
            SetSelection(db, *NavNodeKeyListContainer::Create());
        }
public:
    TestSelectionManager(IConnectionCacheCR connections) : m_connections(connections) {}
    void SetSelection(ECDbCR ecdb, INavNodeKeysContainerCR selection)
        {
        m_selections[&ecdb] = &selection;
        m_subSelections.erase(&ecdb);
        IConnectionCPtr connection = m_connections.GetConnection(ecdb);
        for (ISelectionChangesListener* listener : m_listeners)
            listener->NotifySelectionChanged(SelectionChangedEvent(*connection, "TestSource", SelectionChangeType::Replace, false, selection));
        }
    void SetSubSelection(ECDbCR ecdb, INavNodeKeysContainerCR selection)
        {
        m_subSelections[&ecdb] = &selection;
        IConnectionCPtr connection = m_connections.GetConnection(ecdb);
        for (ISelectionChangesListener* listener : m_listeners)
            listener->NotifySelectionChanged(SelectionChangedEvent(*connection, "TestSource", SelectionChangeType::Replace, true, selection));
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
    void _OnSelectionChanged(SelectionChangedEventCR evt) override
        {
        if (m_callback)
            m_callback(evt);
        }
public:
    void SetCallback(std::function<void(SelectionChangedEventCR)> callback) {m_callback = callback;}
};

END_ECPRESENTATIONTESTS_NAMESPACE