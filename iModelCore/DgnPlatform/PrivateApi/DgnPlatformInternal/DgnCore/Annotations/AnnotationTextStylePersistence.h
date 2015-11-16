//-------------------------------------------------------------------------------------- 
//     $Source: PrivateApi/DgnPlatformInternal/DgnCore/Annotations/AnnotationTextStylePersistence.h $
//  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//-------------------------------------------------------------------------------------- 
/*__BENTLEY_INTERNAL_ONLY__*/
#pragma once

#include <DgnPlatform/Annotations/AnnotationTextStyle.h>
#include "Annotations.fb.h"

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//! @addtogroup Annotations
//! @beginGroup

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     05/2014
//=======================================================================================
struct AnnotationTextStylePersistence : public NonCopyableClass
{
private:
    AnnotationTextStylePersistence(){}

public:
    static BentleyStatus EncodeAsFlatBuf(FB::AnnotationTextStyleSetters&, AnnotationTextStylePropertyBagCR);
    static BentleyStatus EncodeAsFlatBuf(bvector<Byte>&, AnnotationTextStyleCR);
    
    static BentleyStatus DecodeFromFlatBuf(AnnotationTextStylePropertyBagR, FB::AnnotationTextStyleSetterVector const&);
    static BentleyStatus DecodeFromFlatBuf(AnnotationTextStyleR, ByteCP, size_t numBytes);

}; // AnnotationTextStylePersistence

//! @endGroup

END_BENTLEY_DGNPLATFORM_NAMESPACE
