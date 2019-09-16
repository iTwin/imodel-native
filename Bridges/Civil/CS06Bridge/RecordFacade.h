/*--------------------------------------------------------------------------------------+
|
|     $Source: CS06Bridge/RecordFacade.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#ifndef __RECORDFACADE_H__
#define __RECORDFACADE_H__

#include <DgnDb06BimTeleporter/IRecord.h>

BEGIN_CS06BRIDGE_NAMESPACE

struct RecordFacade : public Teleporter::IRecord
    {
    private:
        Dgn::iModelBridgeSyncInfoFile::Record m_record;

    public:
        RecordFacade(Dgn::iModelBridgeSyncInfoFile::Record record);
        virtual ~RecordFacade() = default;

        virtual void PopulateWith(Teleporter::ROWID rid, Dgn::DgnElementId eid, 
            Teleporter::ISourceIdentity const* sourceId, Teleporter::ISourceState const* sourceState) override;
        virtual bool IsValid() const override;
        virtual Teleporter::ROWID GetROWID() const override;
        virtual Dgn::DgnElementId GetDgnElementId() const override;
        virtual Teleporter::ISourceIdentity const* AllocateSourceIdentity() const override;
        virtual void FreeSourceIdentity(Teleporter::ISourceIdentity const* sourceIdentity) const override;
        virtual Teleporter::ISourceState const* AllocateSourceState() const override;
        virtual void FreeSourceState(Teleporter::ISourceState const* sourceState) const override;
    };

END_CS06BRIDGE_NAMESPACE

#endif
