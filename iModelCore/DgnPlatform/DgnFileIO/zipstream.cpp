/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnFileIO/zipstream.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>
#include    <DgnPlatform/DgnCore/ZipStream.h>

USING_NAMESPACE_BENTLEY_SQLITE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   11/10
+---------------+---------------+---------------+---------------+---------------+------*/
DgnZLib::Zipper::Zipper(UInt32 initialSize)
    {
    m_size = initialSize;
    m_buffer = (byte*) malloc (initialSize);
    m_stream.next_out = m_buffer;
    m_stream.avail_out = m_size;
    m_stream.zalloc = (alloc_func) NULL;
    m_stream.zfree  = (free_func) NULL;
    m_stream.opaque = (voidpf) NULL;
    deflateInit2_(&m_stream, 6, Z_DEFLATED, MAX_WBITS, 8, Z_DEFAULT_STRATEGY, ZLIB_VERSION, sizeof(z_stream));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   11/10
+---------------+---------------+---------------+---------------+---------------+------*/
DgnZLib::Zipper::~Zipper()
    {
    deflateEnd (&m_stream);
    free (m_buffer);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   11/10
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnZLib::Zipper::Init (UInt32 maxBufferSize)
    {
    if (maxBufferSize > m_size)
        {
        free(m_buffer);
        m_size = maxBufferSize;
        m_buffer = (byte*) malloc (maxBufferSize);
        }
 
    m_stream.next_out  = m_buffer;
    m_stream.avail_out = m_size;
    deflateReset(&m_stream);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   11/10
+---------------+---------------+---------------+---------------+---------------+------*/
BeSQLite::ZipErrors DgnZLib::Zipper::Write (byte const* data, UInt32 size)
    {
    m_stream.next_in  = (byte*) data;
    m_stream.avail_in = size;

    // compress input buffer into internal buffer
    StatusInt status = deflate (&m_stream, Z_NO_FLUSH);
    if (Z_OK != status)
        return  ZIP_ERROR_COMPRESSION_ERROR;

    return  m_stream.avail_out > 0 ? ZIP_SUCCESS : ZIP_ERROR_BUFFER_FULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   11/10
+---------------+---------------+---------------+---------------+---------------+------*/
ZipErrors DgnZLib::Zipper::Finish ()
    {
    m_stream.avail_in = 0;
    StatusInt status = deflate (&m_stream, Z_FINISH);
    return  (0 < status) ? ZIP_SUCCESS : ZIP_ERROR_BUFFER_FULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   11/10
+---------------+---------------+---------------+---------------+---------------+------*/
DgnZLib::UnZipper::UnZipper()
    {
    m_stream.zalloc = (alloc_func) NULL;
    m_stream.zfree  = (free_func) NULL;
    m_stream.opaque = (voidpf) NULL;
    inflateInit (&m_stream);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   11/10
+---------------+---------------+---------------+---------------+---------------+------*/
DgnZLib::UnZipper::~UnZipper()
    {
    inflateEnd (&m_stream);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   11/10
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnZLib::UnZipper::Init (byte const* zipped, UInt32 zipSize)
    {
    m_stream.avail_in = zipSize;
    m_stream.next_in  = (byte*) zipped;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   11/10
+---------------+---------------+---------------+---------------+---------------+------*/
ZipErrors DgnZLib::UnZipper::Read (byte const* buffer, UInt32 bufSize, UInt32* bytesActuallyRead)
    {
    // point output pointer to their buffer
    m_stream.next_out  = (byte*) buffer;
    m_stream.avail_out = bufSize;
    *bytesActuallyRead = 0;

    // while there's still room in their output buffer, keep inflating
    while (0 != m_stream.avail_out)
        {
        // for compatibility with DgnV8, we don't check for CRC errors
        int err = inflateChecked (&m_stream, Z_NO_FLUSH, 0);
        // inflate into their buffer
        if (Z_OK != err)
            {
            // partial read
            *bytesActuallyRead = bufSize - m_stream.avail_out;

            /* if we read any, return SUCCESS, otherwise END_OF_DATA */
            if (Z_STREAM_END == err)
                {
                BeAssert(0 < *bytesActuallyRead);
                return (0 < *bytesActuallyRead) ? ZIP_SUCCESS : ZIP_ERROR_END_OF_DATA;
                }

            /* some other type of error */
            BeAssert(0);
            return ZIP_ERROR_COMPRESSION_ERROR;
            }
        }

    // The caller's read buffer is now full.
    //  Report how much was processed, and return SUCCESS to indicate that there's
    //  more waiting to be read.
    *bytesActuallyRead = bufSize;

    return  ZIP_SUCCESS;
    }
