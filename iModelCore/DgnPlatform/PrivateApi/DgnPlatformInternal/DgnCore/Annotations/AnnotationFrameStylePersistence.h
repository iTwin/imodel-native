/*--------------------------------------------------------------------------------------+
|
|  $Source: PrivateApi/DgnPlatformInternal/DgnCore/Annotations/AnnotationFrameStylePersistence.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
/*__BENTLEY_INTERNAL_ONLY__*/
#pragma once

#include <DgnPlatform/Annotations/AnnotationFrameStyle.h>
#include "Annotations.fb.h"

BEGIN_BENTLEY_DGN_NAMESPACE

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     05/2014
//=======================================================================================
struct AnnotationFrameStylePersistence : public NonCopyableClass
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
    AnnotationFrameStylePersistence(){}

public:
    static BentleyStatus EncodeAsFlatBuf(FB::AnnotationFrameStyleSetters&, AnnotationFrameStylePropertyBagCR);
    static BentleyStatus EncodeAsFlatBuf(FB::AnnotationFrameStyleSetters&, AnnotationFrameStylePropertyBagCR, FlatBufEncodeOptions);
    static BentleyStatus EncodeAsFlatBuf(bvector<Byte>&, AnnotationFrameStyleCR, FlatBufEncodeOptions);
    
    static BentleyStatus DecodeFromFlatBuf(AnnotationFrameStylePropertyBagR, FB::AnnotationFrameStyleSetterVector const&);
    static BentleyStatus DecodeFromFlatBuf(AnnotationFrameStyleR, ByteCP, size_t numBytes);

}; // AnnotationFrameStylePersistence

END_BENTLEY_DGN_NAMESPACE
