/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include <ECPresentation/Update.h>
#include "ExtendedData.h"

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void SelectionUpdateRecordsHandler::_Start() {m_toRemove.clear(); m_toRefresh.clear();}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void SelectionUpdateRecordsHandler::_Accept(UpdateRecord const& record)
    {
    if (ChangeType::Insert == record.GetChangeType())
        return;

    NavNodeCR node = *record.GetNode();
    IConnectionCPtr connection = m_connections.GetConnection(NavNodeExtendedData(node).GetConnectionId());
    if (connection.IsNull())
        {
        BeAssert(false);
        return;
        }

    KeySetCPtr selection = m_selectionManager.GetSelection(connection->GetECDb());
    if (selection->Contains(*node.GetKey()))
        {
        if (ChangeType::Delete == record.GetChangeType())
            {
            auto iter = m_toRemove.find(connection.get());
            if (m_toRemove.end() == iter)
                iter = m_toRemove.Insert(connection.get(), NavNodeKeyList()).first;
            iter->second.push_back(node.GetKey());
            }
        else if (ChangeType::Update == record.GetChangeType())
            {
            m_toRefresh.insert(connection.get());
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void SelectionUpdateRecordsHandler::_Accept(FullUpdateRecord const& record) {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void SelectionUpdateRecordsHandler::_Finish()
    {
    for (auto entry : m_toRemove)
        {
        IConnectionCP connection = entry.first;
        KeySetCPtr keys = KeySet::Create(entry.second);
        m_selectionManager.RemoveFromSelection(connection->GetECDb(), "RefreshSelectionTask", false, *keys);
        m_toRefresh.erase(connection);
        }
    for (IConnectionCP connection : m_toRefresh)
        m_selectionManager.RefreshSelection(connection->GetECDb(), "RefreshSelectionTask", false);
    }