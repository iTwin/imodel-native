/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/ZipStream.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

#include <stdio.h>      // from C library
#include <zlib/zlib.h>
#include <DgnPlatform/DgnPlatformErrors.r.h>

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

enum
{
    ZIP_OUTPUT_SIZE_STANDARD = 128*1024, // default to  128K
    ZIP_OUTPUT_SIZE_SMALL = 32*1024, // only if unable to allocate larger.
    ZIP_INPUT_SIZE  = 64*1024,       // default to  64K
    DEFAULT_ZIP_LEVEL = 4,
};


//=======================================================================================
// Did you used to call CompressData and UnCompressData directly? Use the Zipper and UnZipper classes (respectively) instead. This helps unify the code paths used for compression.
//! @bsiclass                                                     Keith.Bentley   10/08
//=======================================================================================
class DgnZLib
{
public:
    //=======================================================================================
    // @bsiclass                                                    Keith.Bentley   11/10
    //=======================================================================================
    struct Zipper
    {
    private:
        z_stream    m_stream;
        UInt32      m_size;
        byte*       m_buffer;

    public:
        DGNPLATFORM_EXPORT Zipper (UInt32 initialSize);
        DGNPLATFORM_EXPORT ~Zipper();
        DGNPLATFORM_EXPORT void Init(UInt32 maxBufferSize);
        DGNPLATFORM_EXPORT BeSQLite::ZipErrors Write(byte const*, UInt32 size);
        DGNPLATFORM_EXPORT BeSQLite::ZipErrors Finish();
        DGNPLATFORM_EXPORT UInt32 GetCompressedSize() { return m_size - m_stream.avail_out; }
        DGNPLATFORM_EXPORT byte const* GetResult() { return m_buffer; }
    };

    //=======================================================================================
    // @bsiclass                                                    Keith.Bentley   11/10
    //=======================================================================================
    struct UnZipper
    {
    private:
        z_stream    m_stream;

    public:
        DGNPLATFORM_EXPORT UnZipper();
        DGNPLATFORM_EXPORT ~UnZipper();
        DGNPLATFORM_EXPORT void Init(byte const* zipped, UInt32 zipSize);
        DGNPLATFORM_EXPORT BeSQLite::ZipErrors Read(byte const*, UInt32 size, UInt32* actuallyRead);
    };
};

END_BENTLEY_DGNPLATFORM_NAMESPACE
