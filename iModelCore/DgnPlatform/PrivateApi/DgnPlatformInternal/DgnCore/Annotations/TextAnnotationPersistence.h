//-------------------------------------------------------------------------------------- 
//     $Source: PrivateApi/DgnPlatformInternal/DgnCore/Annotations/TextAnnotationPersistence.h $
//  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//-------------------------------------------------------------------------------------- 
/*__BENTLEY_INTERNAL_ONLY__*/
#pragma once

#include <DgnPlatform/DgnCore/Annotations/TextAnnotation.h>
#include "Annotations.fb.h"

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//! @addtogroup Annotations
//! @beginGroup

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     07/2014
//=======================================================================================
struct TextAnnotationPersistence : public NonCopyableClass
{
private:
    TextAnnotationPersistence(){}

public:
    static BentleyStatus EncodeAsFlatBuf(bvector<Byte>&, TextAnnotationCR);
    
    static BentleyStatus DecodeFromFlatBuf(TextAnnotationR, ByteCP, size_t numBytes);

}; // TextAnnotationPersistence

//! @endGroup

END_BENTLEY_DGNPLATFORM_NAMESPACE
