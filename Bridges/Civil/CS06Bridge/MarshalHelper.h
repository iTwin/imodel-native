/*--------------------------------------------------------------------------------------+
|
|     $Source: CS06Bridge/MarshalHelper.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

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
