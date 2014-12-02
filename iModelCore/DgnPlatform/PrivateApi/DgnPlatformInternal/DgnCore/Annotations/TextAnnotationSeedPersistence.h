//-------------------------------------------------------------------------------------- 
//     $Source: PrivateApi/DgnPlatformInternal/DgnCore/Annotations/TextAnnotationSeedPersistence.h $
//  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//-------------------------------------------------------------------------------------- 
/*__BENTLEY_INTERNAL_ONLY__*/
#pragma once

#include <DgnPlatform/DgnCore/Annotations/TextAnnotationSeed.h>
#include "Annotations.fb.h"

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//! @addtogroup Annotations
//! @beginGroup

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
        ExcludeNonPropertyData = 1 << 0,
        Default = 0
    
    }; // FlatBufEncodeOptions

private:
    TextAnnotationSeedPersistence(){}

public:
    static BentleyStatus EncodeAsFlatBuf(FB::TextAnnotationSeedSetters&, TextAnnotationSeedPropertyBagCR);
    static BentleyStatus EncodeAsFlatBuf(bvector<Byte>&, TextAnnotationSeedCR, FlatBufEncodeOptions);
    
    static BentleyStatus DecodeFromFlatBuf(TextAnnotationSeedPropertyBagR, FB::TextAnnotationSeedSetterVector const&);
    static BentleyStatus DecodeFromFlatBuf(TextAnnotationSeedR, ByteCP, size_t numBytes);

}; // TextAnnotationSeedPersistence

//! @endGroup

END_BENTLEY_DGNPLATFORM_NAMESPACE
