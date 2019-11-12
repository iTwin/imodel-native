/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
/*__BENTLEY_INTERNAL_ONLY__*/
#pragma once

#include <DgnPlatform/Annotations/TextAnnotationSeed.h>
#include "Annotations.fb.h"

BEGIN_BENTLEY_DGN_NAMESPACE

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     07/2014
//=======================================================================================
struct TextAnnotationSeedPersistence : public NonCopyableClass
{
    //=======================================================================================
    // @bsiclass                                                    Jeff.Marker     07/2014
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
