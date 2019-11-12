/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include "CS06BridgeInternal.h"
#include "ChangeDetectorFacade.h"
#include "SourceIdentityFacade.h"
#include "SourceStateFacade.h"
#include "RecordFacade.h"
#include "SourceItemProxy.h"

BEGIN_CS06BRIDGE_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                Jonathan.DeCarlo                    11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
ChangeDetectorFacade::ResultsFacade::ResultsFacade(iModelBridgeSyncInfoFile::ChangeDetector::Results const& results) 
    : m_results(results), Teleporter::IChangeDetector::IResults()
    {
    m_sourceIdentity = new SourceIdentityFacade(m_results.GetSourceIdentity());
    m_currentState = new SourceStateFacade(m_results.GetCurrentState());
    m_record = new RecordFacade(m_results.GetSyncInfoRecord());
    m_changeType = static_cast<Teleporter::IChangeDetector::ChangeType>(static_cast<int>(m_results.GetChangeType()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                Jonathan.DeCarlo                    11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
ChangeDetectorFacade::ResultsFacade::~ResultsFacade()
    {
    delete m_sourceIdentity;
    m_sourceIdentity = nullptr;
    delete m_currentState;
    m_currentState = nullptr;
    delete m_record;
    m_record = nullptr;
    m_changeType = Teleporter::IChangeDetector::ChangeType::New;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                Jonathan.DeCarlo                    11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Teleporter::ISourceIdentity const* ChangeDetectorFacade::ResultsFacade::GetSourceIdentity() const
    {
    return m_sourceIdentity;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                Jonathan.DeCarlo                    11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Teleporter::IChangeDetector::ChangeType ChangeDetectorFacade::ResultsFacade::GetChangeType() const
    {
    return m_changeType;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                Jonathan.DeCarlo                    11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Teleporter::IRecord const* ChangeDetectorFacade::ResultsFacade::GetSyncInfoRecord() const
    {
    return m_record;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                Jonathan.DeCarlo                    11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Teleporter::ISourceState const* ChangeDetectorFacade::ResultsFacade::GetCurrentState() const
    {
    return m_currentState;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                Jonathan.DeCarlo                    11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
ChangeDetectorFacade::ChangeDetectorFacade(Dgn::iModelBridgeSyncInfoFile::ChangeDetector* changeDetector, 
    Dgn::iModelBridgeSyncInfoFile::ROWID fileScopeId) : m_changeDetectorPtr(changeDetector), 
    m_fileScopeId(fileScopeId), Teleporter::IChangeDetector()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                Jonathan.DeCarlo                    11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Teleporter::IChangeDetector::IResults* ChangeDetectorFacade::DetectChange(Utf8CP kind, Teleporter::ISourceItem* item)
    {
    SourceItemProxy sourceItemProxy(item);
    return new ResultsFacade(m_changeDetectorPtr->_DetectChange(m_fileScopeId, kind, sourceItemProxy));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                Jonathan.DeCarlo                    11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void ChangeDetectorFacade::FreeResults(Teleporter::IChangeDetector::IResults* results) const
    {
    delete results;
    }

END_CS06BRIDGE_NAMESPACE
