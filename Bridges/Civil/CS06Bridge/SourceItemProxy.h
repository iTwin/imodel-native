/*--------------------------------------------------------------------------------------+
|
|     $Source: CS06Bridge/SourceItemProxy.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#ifndef __SOURCEITEMPROXY_H__
#define __SOURCEITEMPROXY_H__

#include <DgnDb06BimTeleporter/ISourceItem.h>

BEGIN_CS06BRIDGE_NAMESPACE

struct SourceItemProxy : public Dgn::iModelBridgeSyncInfoFile::ISourceItem, RefCountedBase
    {
    private:
        Teleporter::ISourceItem* m_sourceItem;

    public:
        SourceItemProxy(Teleporter::ISourceItem* sourceItem);
        virtual ~SourceItemProxy() = default;

        virtual Utf8String _GetId() override;
        virtual double _GetLastModifiedTime() override;
        virtual Utf8String _GetHash() override;
    };

END_CS06BRIDGE_NAMESPACE

#endif
