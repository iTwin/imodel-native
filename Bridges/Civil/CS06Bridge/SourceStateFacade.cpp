/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include "CS06BridgeInternal.h"
#include "SourceStateFacade.h"

BEGIN_CS06BRIDGE_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                Jonathan.DeCarlo                    11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
SourceStateFacade::SourceStateFacade(Dgn::iModelBridgeSyncInfoFile::SourceState sourceState) 
    : m_sourceState(sourceState), Teleporter::ISourceState()
    {

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                Jonathan.DeCarlo                    11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void SourceStateFacade::PopulateWith(double lastModifiedTime, Utf8StringCR hash)
    {
    m_sourceState = Dgn::iModelBridgeSyncInfoFile::SourceState(lastModifiedTime, hash);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                Jonathan.DeCarlo                    11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
double SourceStateFacade::GetLastModifiedTime() const
    {
    return m_sourceState.GetLastModifiedTime();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                Jonathan.DeCarlo                    11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR SourceStateFacade::GetHash() const
    {
    return m_sourceState.GetHash();
    }

END_CS06BRIDGE_NAMESPACE
