//---------------------------------------------------------------------------------------------
//  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//  See LICENSE.md in the repository root for full copyright notice.
//---------------------------------------------------------------------------------------------
#pragma once

#include <DgnPlatform/Annotations/AnnotationLeaderStyle.h>
#include "Annotations.fb.h"

BEGIN_BENTLEY_DGN_NAMESPACE

//=======================================================================================
// @bsiclass
//=======================================================================================
struct AnnotationLeaderStylePersistence : public NonCopyableClass
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
    AnnotationLeaderStylePersistence(){}

public:
    static BentleyStatus EncodeAsFlatBuf(FB::AnnotationLeaderStyleSetters&, AnnotationLeaderStylePropertyBagCR);
    static BentleyStatus EncodeAsFlatBuf(FB::AnnotationLeaderStyleSetters&, AnnotationLeaderStylePropertyBagCR, FlatBufEncodeOptions);
    static BentleyStatus EncodeAsFlatBuf(bvector<Byte>&, AnnotationLeaderStyleCR, FlatBufEncodeOptions);
    
    static BentleyStatus DecodeFromFlatBuf(AnnotationLeaderStylePropertyBagR, FB::AnnotationLeaderStyleSetterVector const&);
    static BentleyStatus DecodeFromFlatBuf(AnnotationLeaderStyleR, ByteCP, size_t numBytes);

}; // AnnotationLeaderStylePersistence

END_BENTLEY_DGN_NAMESPACE
