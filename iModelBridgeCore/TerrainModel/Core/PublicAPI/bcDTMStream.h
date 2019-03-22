/*--------------------------------------------------------------------------------------+
|
|     $Source: Core/PublicAPI/bcDTMStream.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//__BENTLEY_INTERNAL_ONLY__
#pragma once

BEGIN_BENTLEY_TERRAINMODEL_NAMESPACE
struct IBcDtmStream
{
public:
virtual int Seek(long offset, int origin) = 0 ;
virtual int Ftell() = 0 ;
virtual size_t Read(void* dest, size_t elementSize, size_t count) = 0;
virtual size_t Write(void* source, size_t elementSize, size_t count) = 0;
};
END_BENTLEY_TERRAINMODEL_NAMESPACE
/*__PUBLISH_SECTION_END__*/
