//---------------------------------------------------------------------------------------------
//  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//  See LICENSE.md in the repository root for full copyright notice.
//---------------------------------------------------------------------------------------------
#pragma once

#include <DgnPlatform/Annotations/AnnotationFrame.h>

BEGIN_BENTLEY_DGN_NAMESPACE

//=======================================================================================
// @bsiclass
//=======================================================================================
struct AnnotationFramePersistence : public NonCopyableClass
{
private:
    AnnotationFramePersistence(){}

public:
    static BentleyStatus EncodeAsFlatBuf(flatbuffers::Offset<FB::AnnotationFrame>&, flatbuffers::FlatBufferBuilder&, AnnotationFrameCR);
    static BentleyStatus EncodeAsFlatBuf(bvector<Byte>&, AnnotationFrameCR);

    static BentleyStatus DecodeFromFlatBuf(AnnotationFrameR, FB::AnnotationFrame const&);
    static BentleyStatus DecodeFromFlatBuf(AnnotationFrameR, ByteCP, size_t numBytes);

}; // AnnotationFramePersistence

END_BENTLEY_DGN_NAMESPACE
