//-------------------------------------------------------------------------------------- 
//     $Source: PrivateApi/DgnPlatformInternal/DgnCore/Annotations/AnnotationLeaderPersistence.h $
//  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//-------------------------------------------------------------------------------------- 
/*__BENTLEY_INTERNAL_ONLY__*/
#pragma once

#include <DgnPlatform/Annotations/AnnotationLeader.h>

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//! @addtogroup Annotations
//! @beginGroup

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

//! @endGroup

END_BENTLEY_DGNPLATFORM_NAMESPACE
