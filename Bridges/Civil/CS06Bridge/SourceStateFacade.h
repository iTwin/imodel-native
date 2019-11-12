/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

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
