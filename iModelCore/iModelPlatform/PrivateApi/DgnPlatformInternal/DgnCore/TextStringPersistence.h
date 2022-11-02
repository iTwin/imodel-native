/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <DgnPlatform/TextString.h>
#include "TextString.fb.h"

BEGIN_BENTLEY_DGN_NAMESPACE

//! @addtogroup TextModule
//! @beginGroup

//=======================================================================================
// @bsiclass
//=======================================================================================
struct TextStringPersistence : public NonCopyableClass
{
    //=======================================================================================
    // @bsiclass
    //=======================================================================================
    enum class FlatBufEncodeOptions
    {
        IncludeGlyphLayoutData = 1 << 0,
        Default = 0
    };

private:
    TextStringPersistence(){}

public:
    static BentleyStatus EncodeAsFlatBuf(flatbuffers::Offset<FB::TextString>&, flatbuffers::FlatBufferBuilder&, TextStringCR, DgnDbR, FlatBufEncodeOptions);
    static BentleyStatus EncodeAsFlatBuf(bvector<Byte>&, TextStringCR, DgnDbR, FlatBufEncodeOptions);
    static BentleyStatus DecodeFromFlatBuf(TextStringR, FB::TextString const&);
    static BentleyStatus DecodeFromFlatBuf(TextStringR, Byte const*, size_t numBytes);
};

//! @endGroup

END_BENTLEY_DGN_NAMESPACE
