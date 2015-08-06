//-------------------------------------------------------------------------------------- 
//     $Source: PrivateApi/DgnPlatformInternal/DgnCore/Annotations/AnnotationTextBlockPersistence.h $
//  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//-------------------------------------------------------------------------------------- 
/*__BENTLEY_INTERNAL_ONLY__*/
#pragma once

#include <DgnPlatform/DgnCore/Annotations/AnnotationTextBlock.h>
#include "Annotations.fb.h"

BEGIN_BENTLEY_DGN_NAMESPACE

//! @addtogroup Annotations
//! @beginGroup

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     05/2014
//=======================================================================================
struct AnnotationTextBlockPersistence : public NonCopyableClass
{
private:
    AnnotationTextBlockPersistence(){}

    static BentleyStatus EncodeAsFlatBuf(FB::AnnotationTextBlockRunOffsets&, flatbuffers::FlatBufferBuilder&, AnnotationRunBaseCR);
    static BentleyStatus EncodeAsFlatBuf(FB::AnnotationTextBlockParagraphOffsets&, flatbuffers::FlatBufferBuilder&, AnnotationParagraphCR);

public:
    static BentleyStatus EncodeAsFlatBuf(flatbuffers::Offset<FB::AnnotationTextBlock>&, flatbuffers::FlatBufferBuilder&, AnnotationTextBlockCR);
    static BentleyStatus EncodeAsFlatBuf(bvector<Byte>&, AnnotationTextBlockCR);

    static BentleyStatus DecodeFromFlatBuf(AnnotationTextBlockR, FB::AnnotationTextBlock const&);
    static BentleyStatus DecodeFromFlatBuf(AnnotationTextBlockR, ByteCP, size_t numBytes);

}; // AnnotationTextBlockPersistence

//! @endGroup

END_BENTLEY_DGN_NAMESPACE
