//-------------------------------------------------------------------------------------- 
//     $Source: PrivateApi/DgnPlatformInternal/DgnCore/Annotations/AnnotationTextStylePersistence.h $
//  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//-------------------------------------------------------------------------------------- 
/*__BENTLEY_INTERNAL_ONLY__*/
#pragma once

#include <DgnPlatform/Annotations/AnnotationTextStyle.h>
#include "Annotations.fb.h"

BEGIN_BENTLEY_DGN_NAMESPACE

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     05/2014
//=======================================================================================
struct AnnotationTextStylePersistence : public NonCopyableClass
{
    //=======================================================================================
    // @bsiclass                                                    Jeff.Marker     06/2014
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
