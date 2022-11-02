/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <DgnPlatform/Annotations/AnnotationFrameStyle.h>
#include "Annotations.fb.h"

BEGIN_BENTLEY_DGN_NAMESPACE

//=======================================================================================
// @bsiclass
//=======================================================================================
struct AnnotationFrameStylePersistence : public NonCopyableClass
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
    AnnotationFrameStylePersistence(){}

public:
    static BentleyStatus EncodeAsFlatBuf(FB::AnnotationFrameStyleSetters&, AnnotationFrameStylePropertyBagCR);
    static BentleyStatus EncodeAsFlatBuf(FB::AnnotationFrameStyleSetters&, AnnotationFrameStylePropertyBagCR, FlatBufEncodeOptions);
    static BentleyStatus EncodeAsFlatBuf(bvector<Byte>&, AnnotationFrameStyleCR, FlatBufEncodeOptions);
    
    static BentleyStatus DecodeFromFlatBuf(AnnotationFrameStylePropertyBagR, FB::AnnotationFrameStyleSetterVector const&);
    static BentleyStatus DecodeFromFlatBuf(AnnotationFrameStyleR, ByteCP, size_t numBytes);

}; // AnnotationFrameStylePersistence

END_BENTLEY_DGN_NAMESPACE
