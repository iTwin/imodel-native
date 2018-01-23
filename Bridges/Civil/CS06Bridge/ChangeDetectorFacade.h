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

struct ChangeDetectorFacade : public Teleporter::IChangeDetector
    {
    struct ResultsFacade : public Teleporter::IChangeDetector::IResults
        {
        private:
            Dgn::iModelBridgeSyncInfoFile::ChangeDetector::Results m_results;
            Teleporter::ISourceIdentity* m_sourceIdentity;
            Teleporter::ISourceState* m_currentState;
            Teleporter::IRecord* m_record;
            Teleporter::IChangeDetector::ChangeType m_changeType;

        public:
            ResultsFacade(Dgn::iModelBridgeSyncInfoFile::ChangeDetector::Results const& results);
            virtual ~ResultsFacade();

            virtual Teleporter::ISourceIdentity const* GetSourceIdentity() const override;
            virtual Teleporter::IChangeDetector::ChangeType GetChangeType() const override;
            virtual Teleporter::IRecord const* GetSyncInfoRecord() const override;
            virtual Teleporter::ISourceState const* GetCurrentState() const override;
        };

    private:
        Dgn::iModelBridgeSyncInfoFile::ChangeDetectorPtr m_changeDetectorPtr;
        Dgn::iModelBridgeSyncInfoFile::ROWID m_fileScopeId;

    public:
        ChangeDetectorFacade(Dgn::iModelBridgeSyncInfoFile::ChangeDetector* changeDetector, Dgn::iModelBridgeSyncInfoFile::ROWID fileScopeId);
        virtual ~ChangeDetectorFacade() = default;

        virtual Teleporter::IChangeDetector::IResults* DetectChange(Utf8CP kind, Teleporter::ISourceItem* item) override;
        virtual void FreeResults(Teleporter::IChangeDetector::IResults* results) const override;
    };

END_CS06BRIDGE_NAMESPACE

#endif
