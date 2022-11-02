//---------------------------------------------------------------------------------------------
//  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//  See LICENSE.md in the repository root for full copyright notice.
//---------------------------------------------------------------------------------------------
#pragma once

#include <DgnPlatform/Annotations/TextAnnotation.h>

BEGIN_BENTLEY_DGN_NAMESPACE

//=======================================================================================
// @bsiclass
//=======================================================================================
struct TextAnnotationPersistence : public NonCopyableClass
{
private:
    TextAnnotationPersistence(){}

public:
    static BentleyStatus EncodeAsFlatBuf(bvector<Byte>&, TextAnnotationCR);
    
    static BentleyStatus DecodeFromFlatBuf(TextAnnotationR, ByteCP, size_t numBytes);

    static DGNPLATFORM_EXPORT BentleyStatus DecodeFromFlatBufWithRemap(TextAnnotationR annotation, ByteCP, size_t numBytes, DgnImportContext&);

}; // TextAnnotationPersistence

END_BENTLEY_DGN_NAMESPACE
