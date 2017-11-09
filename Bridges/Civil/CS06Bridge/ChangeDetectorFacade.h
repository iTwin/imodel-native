/*--------------------------------------------------------------------------------------+
|
|     $Source: CS06Bridge/ChangeDetectorFacade.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#ifndef __CHANGEDETECTORFACADE_H__
#define __CHANGEDETECTORFACADE_H__

#include <DgnDb06BimTeleporter/IChangeDetector.h>

BEGIN_CS06BRIDGE_NAMESPACE

struct ChangeDetectorFacade : public Teleporter::IChangeDetector, RefCountedBase
    {
    private:
        Dgn::iModelBridgeSyncInfoFile::ChangeDetectorPtr m_changeDetectorPtr;
        Dgn::iModelBridgeSyncInfoFile::ROWID m_fileScopeId;

    public:
        ChangeDetectorFacade(Dgn::iModelBridgeSyncInfoFile::ChangeDetector* changeDetector, Dgn::iModelBridgeSyncInfoFile::ROWID fileScopeId);
        virtual ~ChangeDetectorFacade() = default;
    };

END_CS06BRIDGE_NAMESPACE

#endif
