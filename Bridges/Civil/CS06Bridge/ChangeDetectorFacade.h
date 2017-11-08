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

struct ChangeDetectorFacade : RefCountedBase, Teleporter::IChangeDetector
    {
    public:
        ChangeDetectorFacade();
        virtual ~ChangeDetectorFacade() = default;
    };

END_CS06BRIDGE_NAMESPACE

#endif
