/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <DgnPlatform/Annotations/TextAnnotationSeed.h>
#include "Annotations.fb.h"

BEGIN_BENTLEY_DGN_NAMESPACE

//=======================================================================================
// @bsiclass
//=======================================================================================
struct TextAnnotationSeedPersistence : public NonCopyableClass
{
    //=======================================================================================
    // @bsiclass
    //=======================================================================================
    enum struct FlatBufEncodeOptions
    {
        SettersAreOverrides = 1 << 0,
        Default = 0
    
    }; // FlatBufEncodeOptions

private:
    TextAnnotationSeedPersistence(){}

public:
    static BentleyStatus EncodeAsFlatBuf(FB::TextAnnotationSeedSetters&, TextAnnotationSeedPropertyBagCR);
    static BentleyStatus EncodeAsFlatBuf(FB::TextAnnotationSeedSetters&, TextAnnotationSeedPropertyBagCR, FlatBufEncodeOptions);
    static BentleyStatus EncodeAsFlatBuf(bvector<Byte>&, TextAnnotationSeedCR, FlatBufEncodeOptions);
    
    static BentleyStatus DecodeFromFlatBuf(TextAnnotationSeedPropertyBagR, FB::TextAnnotationSeedSetterVector const&);
    static BentleyStatus DecodeFromFlatBuf(TextAnnotationSeedR, ByteCP, size_t numBytes);
};

END_BENTLEY_DGN_NAMESPACE
