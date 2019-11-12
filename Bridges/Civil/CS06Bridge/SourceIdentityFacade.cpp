/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include "CS06BridgeInternal.h"
#include "SourceIdentityFacade.h"

BEGIN_CS06BRIDGE_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                Jonathan.DeCarlo                    11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
SourceIdentityFacade::SourceIdentityFacade(Dgn::iModelBridgeSyncInfoFile::SourceIdentity sourceIdentity)
    : m_sourceIdentity(sourceIdentity), Teleporter::ISourceIdentity()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                Jonathan.DeCarlo                    11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void SourceIdentityFacade::PopulateWith(Teleporter::ROWID scope, const Utf8String& kind, const Utf8String& id)
    {
    m_sourceIdentity = Dgn::iModelBridgeSyncInfoFile::SourceIdentity(scope, kind, id);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                Jonathan.DeCarlo                    11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Teleporter::ROWID SourceIdentityFacade::GetScopeROWID() const
    {
    return m_sourceIdentity.GetScopeROWID();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                Jonathan.DeCarlo                    11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR SourceIdentityFacade::GetKind() const
    {
    return m_sourceIdentity.GetKind();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                Jonathan.DeCarlo                    11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR SourceIdentityFacade::GetId() const
    {
    return m_sourceIdentity.GetId();
    }

END_CS06BRIDGE_NAMESPACE
