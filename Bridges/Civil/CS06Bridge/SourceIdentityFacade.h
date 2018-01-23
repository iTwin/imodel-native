/*--------------------------------------------------------------------------------------+
|
|     $Source: CS06Bridge/SourceIdentityFacade.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#ifndef __SOURCEIDENTITYFACADE_H__
#define __SOURCEIDENTITYFACADE_H__

#include <DgnDb06BimTeleporter/ISourceIdentity.h>

BEGIN_CS06BRIDGE_NAMESPACE

struct SourceIdentityFacade : public Teleporter::ISourceIdentity
    {
    private:
        Dgn::iModelBridgeSyncInfoFile::SourceIdentity m_sourceIdentity;

    public:
        SourceIdentityFacade(Dgn::iModelBridgeSyncInfoFile::SourceIdentity sourceIdentity);
        virtual ~SourceIdentityFacade() = default;

        virtual void PopulateWith(Teleporter::ROWID scope, const Utf8String& kind, const Utf8String& id) override;
        virtual Teleporter::ROWID GetScopeROWID() const override;
        virtual Utf8StringCR GetKind() const override;
        virtual Utf8StringCR GetId() const override;
    };

END_CS06BRIDGE_NAMESPACE

#endif
