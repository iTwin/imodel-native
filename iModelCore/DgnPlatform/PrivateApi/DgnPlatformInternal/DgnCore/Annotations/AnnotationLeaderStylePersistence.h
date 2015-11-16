//-------------------------------------------------------------------------------------- 
//     $Source: PrivateApi/DgnPlatformInternal/DgnCore/Annotations/AnnotationLeaderStylePersistence.h $
//  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//-------------------------------------------------------------------------------------- 
/*__BENTLEY_INTERNAL_ONLY__*/
#pragma once

#include <DgnPlatform/Annotations/AnnotationLeaderStyle.h>
#include "Annotations.fb.h"

BEGIN_BENTLEY_DGN_NAMESPACE

//! @addtogroup Annotations
//! @beginGroup

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     05/2014
//=======================================================================================
struct AnnotationLeaderStylePersistence : public NonCopyableClass
{
private:
    AnnotationLeaderStylePersistence(){}

public:
    static BentleyStatus EncodeAsFlatBuf(FB::AnnotationLeaderStyleSetters&, AnnotationLeaderStylePropertyBagCR);
    static BentleyStatus EncodeAsFlatBuf(bvector<Byte>&, AnnotationLeaderStyleCR);
    
    static BentleyStatus DecodeFromFlatBuf(AnnotationLeaderStylePropertyBagR, FB::AnnotationLeaderStyleSetterVector const&);
    static BentleyStatus DecodeFromFlatBuf(AnnotationLeaderStyleR, ByteCP, size_t numBytes);

}; // AnnotationLeaderStylePersistence

//! @endGroup

END_BENTLEY_DGN_NAMESPACE
