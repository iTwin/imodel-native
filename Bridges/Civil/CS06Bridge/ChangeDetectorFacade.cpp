/*--------------------------------------------------------------------------------------+
|
|     $Source: CS06Bridge/ChangeDetectorFacade.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "CS06BridgeInternal.h"
#include "ChangeDetectorFacade.h"

BEGIN_CS06BRIDGE_NAMESPACE

ChangeDetectorFacade::ChangeDetectorFacade(Dgn::iModelBridgeSyncInfoFile::ChangeDetector* changeDetector, 
    Dgn::iModelBridgeSyncInfoFile::ROWID fileScopeId) : m_changeDetectorPtr(changeDetector), 
    m_fileScopeId(fileScopeId), Teleporter::IChangeDetector()
    {
    }

END_CS06BRIDGE_NAMESPACE
