/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include "CS06BridgeInternal.h"
#include "SourceItemProxy.h"

BEGIN_CS06BRIDGE_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                Jonathan.DeCarlo                    11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
SourceItemProxy::SourceItemProxy(Teleporter::ISourceItem* sourceItem) 
    : m_sourceItem(sourceItem), Dgn::iModelBridgeSyncInfoFile::ISourceItem(), RefCountedBase()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                Jonathan.DeCarlo                    11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String SourceItemProxy::_GetId()
    {
    return m_sourceItem->GetId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                Jonathan.DeCarlo                    11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
double SourceItemProxy::_GetLastModifiedTime()
    {
    return m_sourceItem->GetLastModifiedTime();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                Jonathan.DeCarlo                    11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String SourceItemProxy::_GetHash()
    {
    return m_sourceItem->GetHash();
    }

END_CS06BRIDGE_NAMESPACE
