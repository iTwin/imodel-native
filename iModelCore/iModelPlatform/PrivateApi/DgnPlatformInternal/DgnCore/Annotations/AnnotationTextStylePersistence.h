//---------------------------------------------------------------------------------------------
//  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//  See LICENSE.md in the repository root for full copyright notice.
//---------------------------------------------------------------------------------------------
#pragma once

#include <DgnPlatform/Annotations/AnnotationTextStyle.h>
#include "Annotations.fb.h"

BEGIN_BENTLEY_DGN_NAMESPACE

//=======================================================================================
// @bsiclass
//=======================================================================================
struct AnnotationTextStylePersistence : public NonCopyableClass
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
    AnnotationTextStylePersistence(){}

public:
    static BentleyStatus EncodeAsFlatBuf(FB::AnnotationTextStyleSetters&, AnnotationTextStylePropertyBagCR);
    static BentleyStatus EncodeAsFlatBuf(FB::AnnotationTextStyleSetters&, AnnotationTextStylePropertyBagCR, FlatBufEncodeOptions);
    static BentleyStatus EncodeAsFlatBuf(bvector<Byte>&, AnnotationTextStyleCR, FlatBufEncodeOptions);
    
    static BentleyStatus DecodeFromFlatBuf(AnnotationTextStylePropertyBagR, FB::AnnotationTextStyleSetterVector const&);
    static BentleyStatus DecodeFromFlatBuf(AnnotationTextStyleR, ByteCP, size_t numBytes);

}; // AnnotationTextStylePersistence

END_BENTLEY_DGN_NAMESPACE
