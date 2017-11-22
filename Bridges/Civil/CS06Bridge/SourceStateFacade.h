/*--------------------------------------------------------------------------------------+
|
|     $Source: CS06Bridge/SourceStateFacade.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#ifndef __SOURCESTATEFACADE_H__
#define __SOURCESTATEFACADE_H__

#include <DgnDb06BimTeleporter/ISourceState.h>

BEGIN_CS06BRIDGE_NAMESPACE

struct SourceStateFacade : public Teleporter::ISourceState
    {
    private:
        Dgn::iModelBridgeSyncInfoFile::SourceState m_sourceState;

    public:
        SourceStateFacade(Dgn::iModelBridgeSyncInfoFile::SourceState sourceState);
        virtual ~SourceStateFacade() = default;

        virtual void PopulateWith(double lastModifiedTime, Utf8StringCR hash) override;
        virtual double GetLastModifiedTime() const override;
        virtual Utf8StringCR GetHash() const override;
    };

END_CS06BRIDGE_NAMESPACE

#endif
