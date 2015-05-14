/*--------------------------------------------------------------------------------------+
|     $Source: PrivateApi/DgnPlatformInternal/DgnCore/TextStringPersistence.h $
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
+--------------------------------------------------------------------------------------*/
/*__BENTLEY_INTERNAL_ONLY__*/
#pragma once

#include <DgnPlatform/DgnCore/TextString.h>
#include "TextString.fb.h"

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//! @addtogroup TextModule
//! @beginGroup

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     01/2015
//=======================================================================================
struct TextStringPersistence : public NonCopyableClass
{
private:
    TextStringPersistence(){}

public:
    static BentleyStatus EncodeAsFlatBuf(flatbuffers::Offset<FB::TextString>&, flatbuffers::FlatBufferBuilder&, TextStringCR, DgnDbR);
    static BentleyStatus EncodeAsFlatBuf(bvector<Byte>&, TextStringCR, DgnDbR);
    static BentleyStatus DecodeFromFlatBuf(TextStringR, FB::TextString const&, DgnDbCR);
    static BentleyStatus DecodeFromFlatBuf(TextStringR, ByteCP, size_t numBytes, DgnDbCR);
};

//! @endGroup

END_BENTLEY_DGNPLATFORM_NAMESPACE
