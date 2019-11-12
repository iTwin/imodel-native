//---------------------------------------------------------------------------------------------
//  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//  See COPYRIGHT.md in the repository root for full copyright notice.
//---------------------------------------------------------------------------------------------
/*__BENTLEY_INTERNAL_ONLY__*/
#pragma once

#include <DgnPlatform/Annotations/TextAnnotation.h>

BEGIN_BENTLEY_DGN_NAMESPACE

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

    static DGNPLATFORM_EXPORT BentleyStatus DecodeFromFlatBufWithRemap(TextAnnotationR annotation, ByteCP, size_t numBytes, DgnImportContext&);

}; // TextAnnotationPersistence

END_BENTLEY_DGN_NAMESPACE
