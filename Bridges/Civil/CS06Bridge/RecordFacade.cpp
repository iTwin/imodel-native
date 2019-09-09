/*--------------------------------------------------------------------------------------+
|
|     $Source: CS06Bridge/RecordFacade.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "CS06BridgeInternal.h"
#include "RecordFacade.h"
#include "SourceIdentityFacade.h"
#include "SourceStateFacade.h"

BEGIN_CS06BRIDGE_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                Jonathan.DeCarlo                    11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
RecordFacade::RecordFacade(Dgn::iModelBridgeSyncInfoFile::Record record) 
    : m_record(record), Teleporter::IRecord()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                Jonathan.DeCarlo                    11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void RecordFacade::PopulateWith(Teleporter::ROWID rid, Dgn::DgnElementId eid,
    Teleporter::ISourceIdentity const* sourceId, Teleporter::ISourceState const* sourceState)
    {
    // Normally we would accept references for parameters, but they can't be because they are abstract classes.
    BeAssert(sourceId != nullptr);
    BeAssert(sourceState != nullptr);

    m_record = Dgn::iModelBridgeSyncInfoFile::Record(
        rid, 
        eid, 
        iModelBridgeSyncInfoFile::SourceIdentity(sourceId->GetScopeROWID(), sourceId->GetKind(), sourceId->GetId()),
        iModelBridgeSyncInfoFile::SourceState(sourceState->GetLastModifiedTime(), sourceState->GetHash()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                Jonathan.DeCarlo                    11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bool RecordFacade::IsValid() const
    {
    return m_record.IsValid();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                Jonathan.DeCarlo                    11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Teleporter::ROWID RecordFacade::GetROWID() const
    {
    return m_record.GetROWID();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                Jonathan.DeCarlo                    11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::DgnElementId RecordFacade::GetDgnElementId() const
    {
    return m_record.GetDgnElementId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                Jonathan.DeCarlo                    11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Teleporter::ISourceIdentity const* RecordFacade::AllocateSourceIdentity() const
    {
    return new SourceIdentityFacade(m_record.GetSourceIdentity());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                Jonathan.DeCarlo                    11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void RecordFacade::FreeSourceIdentity(Teleporter::ISourceIdentity const* sourceIdentity) const
    {
    delete sourceIdentity;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                Jonathan.DeCarlo                    11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Teleporter::ISourceState const* RecordFacade::AllocateSourceState() const
    {
    return new SourceStateFacade(m_record.GetSourceState());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                Jonathan.DeCarlo                    11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void RecordFacade::FreeSourceState(Teleporter::ISourceState const* sourceState) const
    {
    delete sourceState;
    }

END_CS06BRIDGE_NAMESPACE
