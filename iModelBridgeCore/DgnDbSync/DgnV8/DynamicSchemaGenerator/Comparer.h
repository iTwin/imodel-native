/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <DgnDbSync/DgnDbSync.h> // NB: Must include this first!

BEGIN_DGNDBSYNC_DGNV8_NAMESPACE
struct InternalComparer
    {
    static bool IsChanged(ECN::ECSchemaCP existing, ECN::ECSchemaCP incoming);
    };

END_DGNDBSYNC_DGNV8_NAMESPACE
