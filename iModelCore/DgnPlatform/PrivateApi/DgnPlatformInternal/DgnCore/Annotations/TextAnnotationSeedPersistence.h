/*--------------------------------------------------------------------------------------+
|
|  $Source: PrivateApi/DgnPlatformInternal/DgnCore/Annotations/TextAnnotationSeedPersistence.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
/*__BENTLEY_INTERNAL_ONLY__*/
#pragma once

#include <DgnPlatform/Annotations/TextAnnotationSeed.h>
#include "Annotations.fb.h"

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//! @addtogroup Annotations
//! @beginGroup

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     07/2014
//=======================================================================================
struct TextAnnotationSeedPersistence : NonCopyableClass
{
    //=======================================================================================
    // @bsiclass                                                    Jeff.Marker     07/2014
    //=======================================================================================
    enum class FlatBufEncodeOptions
    {
        ExcludeNonPropertyData = 1 << 0,
        Default = 0
    };

private:
    TextAnnotationSeedPersistence(){}

public:
    static BentleyStatus EncodeAsFlatBuf(FB::TextAnnotationSeedSetters&, TextAnnotationSeedPropertyBagCR);
    static BentleyStatus EncodeAsFlatBuf(bvector<Byte>&, TextAnnotationSeedCR, FlatBufEncodeOptions);
    
    static BentleyStatus DecodeFromFlatBuf(TextAnnotationSeedPropertyBagR, FB::TextAnnotationSeedSetterVector const&);
    static BentleyStatus DecodeFromFlatBuf(TextAnnotationSeedR, ByteCP, size_t numBytes);
};

//! @endGroup

END_BENTLEY_DGNPLATFORM_NAMESPACE
