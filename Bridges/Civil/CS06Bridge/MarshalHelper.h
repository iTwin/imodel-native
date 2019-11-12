/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#ifndef __MARSHALHELPER_H__
#define __MARSHALHELPER_H__

BEGIN_CS06BRIDGE_NAMESPACE

struct MarshalHelper
    {
    private:
        MarshalHelper() = default;

    public:
        static BentleyG06::BeFileName MarshalBimBeFileNameTo06BeFileName(BentleyApi::BeFileNameCR fileName);
    };

END_CS06BRIDGE_NAMESPACE

#endif
