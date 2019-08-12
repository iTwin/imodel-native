//-------------------------------------------------------------------------------------- 
//  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//-------------------------------------------------------------------------------------- 
/*__BENTLEY_INTERNAL_ONLY__*/
#pragma once

#include <DgnPlatform/Annotations/AnnotationFrame.h>

BEGIN_BENTLEY_DGN_NAMESPACE

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     05/2014
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
