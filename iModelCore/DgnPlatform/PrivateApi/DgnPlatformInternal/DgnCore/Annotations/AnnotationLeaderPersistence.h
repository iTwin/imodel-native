//---------------------------------------------------------------------------------------------
//  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//  See COPYRIGHT.md in the repository root for full copyright notice.
//---------------------------------------------------------------------------------------------
/*__BENTLEY_INTERNAL_ONLY__*/
#pragma once

#include <DgnPlatform/Annotations/AnnotationLeader.h>

BEGIN_BENTLEY_DGN_NAMESPACE

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     05/2014
//=======================================================================================
struct AnnotationLeaderPersistence : public NonCopyableClass
{
private:
    AnnotationLeaderPersistence(){}

public:
    static BentleyStatus EncodeAsFlatBuf(flatbuffers::Offset<FB::AnnotationLeader>&, flatbuffers::FlatBufferBuilder&, AnnotationLeaderCR);
    static BentleyStatus EncodeAsFlatBuf(bvector<Byte>&, AnnotationLeaderCR);

    static BentleyStatus DecodeFromFlatBuf(AnnotationLeaderR, FB::AnnotationLeader const&);
    static BentleyStatus DecodeFromFlatBuf(AnnotationLeaderR, ByteCP, size_t numBytes);

}; // AnnotationLeaderPersistence

END_BENTLEY_DGN_NAMESPACE
