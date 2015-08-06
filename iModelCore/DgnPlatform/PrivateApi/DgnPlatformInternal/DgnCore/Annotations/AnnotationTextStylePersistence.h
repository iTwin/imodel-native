//-------------------------------------------------------------------------------------- 
//     $Source: PrivateApi/DgnPlatformInternal/DgnCore/Annotations/AnnotationTextStylePersistence.h $
//  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//-------------------------------------------------------------------------------------- 
/*__BENTLEY_INTERNAL_ONLY__*/
#pragma once

#include <DgnPlatform/DgnCore/Annotations/AnnotationTextStyle.h>
#include "Annotations.fb.h"

BEGIN_BENTLEY_DGN_NAMESPACE

//! @addtogroup Annotations
//! @beginGroup

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     05/2014
//=======================================================================================
struct AnnotationTextStylePersistence : public NonCopyableClass
{
    //=======================================================================================
    // @bsiclass                                                    Jeff.Marker     06/2014
    //=======================================================================================
    enum class FlatBufEncodeOptions
    {
        ExcludeNonPropertyData = 1 << 0,
        Default = 0
    
    }; // FlatBufEncodeOptions

private:
    AnnotationTextStylePersistence(){}

public:
    static BentleyStatus EncodeAsFlatBuf(FB::AnnotationTextStyleSetters&, AnnotationTextStylePropertyBagCR);
    static BentleyStatus EncodeAsFlatBuf(bvector<Byte>&, AnnotationTextStyleCR, FlatBufEncodeOptions);
    
    static BentleyStatus DecodeFromFlatBuf(AnnotationTextStylePropertyBagR, FB::AnnotationTextStyleSetterVector const&);
    static BentleyStatus DecodeFromFlatBuf(AnnotationTextStyleR, ByteCP, size_t numBytes);

}; // AnnotationTextStylePersistence

//! @endGroup

END_BENTLEY_DGN_NAMESPACE
