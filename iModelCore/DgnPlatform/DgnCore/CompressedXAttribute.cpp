/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/CompressedXAttribute.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>
#include    <DgnPlatform/DgnCore/ZipStream.h>

enum  PersistentXAttr_compressionType
    {
    COMPRESSIONTYPE_Disallowed      = 1,
    COMPRESSIONTYPE_Uncompressed    = 2,
    COMPRESSIONTYPE_LZW             = 3
    };

struct CompressedXAttrHeader
    {
    UInt16      type;
    UInt16      padding;
    UInt32      uncompressedSize;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Chuck.Kirschman 11/04
+---------------+---------------+---------------+---------------+---------------+------*/
static PersistentXAttr_compressionType   compressedXAttribute_getCompression
(
size_t      bufferSize,
Int32       compressionThreshold
)
    {
    PersistentXAttr_compressionType   type;

    if (compressionThreshold < 0)
        type = COMPRESSIONTYPE_Disallowed;
    else if (bufferSize > (UInt32)compressionThreshold)
        type = COMPRESSIONTYPE_LZW;
    else
        type = COMPRESSIONTYPE_Uncompressed;

    return type;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Chuck.Kirschman 11/04
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt compressedXAttribute_createBuffer
(
byte**      outBuffer,         // <= Must be freed by caller
UInt32*     outTotalSize,      // <= returns number of bytes used in buffer
byte const* inBuffer,
UInt32      inBufferSize,
Int32       compressionThreshold
)
    {
    if (NULL == outBuffer || 0 == inBufferSize)
        return DGNHANDLERS_STATUS_BadArg;

    CompressedXAttrHeader header;
    memset (&header, 0, sizeof(header));
    header.uncompressedSize = static_cast<UInt32>(inBufferSize);
    header.type = (UInt16) compressedXAttribute_getCompression (header.uncompressedSize, compressionThreshold);

    if (header.uncompressedSize == 0)
        return DGNHANDLERS_STATUS_BadArg;

    // Additional size according to zlib
    UInt32      bufferSize =  (UInt32)((inBufferSize * 1.2 + 12) + sizeof(CompressedXAttrHeader));
    byte        *buffer = (byte *)memutil_calloc (bufferSize, sizeof(buffer)[0], 'StrX');

    if (NULL == buffer)
        return DGNMODEL_STATUS_OutOfMemory;

    int     headerSize = sizeof(header);
    int     dataSize = 0;

    memcpy (buffer, &header, headerSize);

    byte    *dataPtr = buffer+headerSize;
    UInt32  remainingBufferSpace = bufferSize-headerSize;

    switch (header.type)
        {
        case COMPRESSIONTYPE_Disallowed:
        case COMPRESSIONTYPE_Uncompressed:
            if (remainingBufferSpace < header.uncompressedSize)
                return DGNHANDLERS_STATUS_ElementTooLarge;

            memcpy (dataPtr, inBuffer, header.uncompressedSize);
            dataSize = header.uncompressedSize;
            break;

        case COMPRESSIONTYPE_LZW:
            DgnZLib::Zipper zip(remainingBufferSpace);
            if (ZIP_SUCCESS != zip.Write(inBuffer, inBufferSize))
                return ERROR;
            
            if (ZIP_SUCCESS != zip.Finish())
                return ERROR;

            if (zip.GetCompressedSize() > remainingBufferSpace)
                return ERROR;
            
            dataSize = zip.GetCompressedSize();
            memcpy(dataPtr, zip.GetResult(), dataSize);
            
            break;
        }
    *outTotalSize = (headerSize + dataSize);
    *outBuffer = buffer;
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Chuck.Kirschman 11/04
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       CompressedXAttribute::Add
(
ElementRefP           elemRef,                // => Element ref to attach XAttribute to
XAttributeHandlerId  handlerId,
UInt32               xAttrId,                // => If INVALID_XATTR_ID, a new id is assigned & returned in outXAttrId
byte const*          buffer,
size_t               bufferSize,
Int32                compressionThreshold,   // => If -1, compression is disabled; otherwise attempt to compress only if larger than this
UInt32*              outXAttrId              // => If non-NULL, actual ID assigned to XAttribute
)
    {
    StatusInt   status;
    byte        *tmpBuffer;
    UInt32      totalSize=0;

    if (NULL == elemRef || NULL == buffer)
        return DGNHANDLERS_STATUS_BadArg;

    if (SUCCESS != (status = compressedXAttribute_createBuffer (&tmpBuffer, &totalSize, buffer, (UInt32) bufferSize, compressionThreshold)))
        return status;

    UInt32  newId = 0;
    status = elemRef->GetDgnProject()->GetTxnManager().GetCurrentTxn().AddXAttribute (elemRef, handlerId, xAttrId, tmpBuffer, totalSize, &newId);
    if (SUCCESS == status && NULL != outXAttrId)
        *outXAttrId = newId;

    memutil_free (tmpBuffer);
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Chuck.Kirschman 11/04
+---------------+---------------+---------------+---------------+---------------+------*/
// This function should only be called when constructing a complex element descriptor
StatusInt   CompressedXAttribute::ScheduleAdd
(
EditElementHandleP     ehandle,
XAttributeHandlerId handlerId,
UInt32              xAttrId,
byte const*         buffer,
size_t              bufferSize,
Int32               compressionThreshold    // => If -1, compression is disabled; otherwise attempt to compress only if larger than this
)
    {
    StatusInt   status;
    byte        *tmpBuffer;
    UInt32      totalSize=0;

    if (NULL == ehandle || NULL == buffer)
        return DGNHANDLERS_STATUS_BadArg;

    if (SUCCESS != (status = compressedXAttribute_createBuffer (&tmpBuffer, &totalSize, buffer, (UInt32) bufferSize, compressionThreshold)))
        return status;

    // Tell the element descr to add this Xattribute when rewrite is called
    status = ehandle->ScheduleWriteXAttribute (handlerId, xAttrId, static_cast<UInt32>(totalSize), tmpBuffer);

    memutil_free (tmpBuffer);
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Chuck.Kirschman 11/04
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   CompressedXAttribute::Replace
(
XAttributeHandle&   attrHandle,
byte const*         buffer,
size_t              bufferSize,
Int32               compressionThreshold    // => If -1, compression is disabled; otherwise attempt to compress only if larger than this
)
    {
    StatusInt   status;
    byte        *tmpBuffer;
    UInt32 totalSize=0;

    if (NULL == buffer)
        return DGNHANDLERS_STATUS_BadArg;

    if (SUCCESS != (status = compressedXAttribute_createBuffer (&tmpBuffer, &totalSize, buffer, (UInt32) bufferSize, compressionThreshold)))
        return status;

    status = attrHandle.GetElementRef()->GetDgnProject()->GetTxnManager().GetCurrentTxn().ReplaceXAttributeData (attrHandle, tmpBuffer, totalSize);

    memutil_free (tmpBuffer);
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Chuck.Kirschman 11/04
+---------------+---------------+---------------+---------------+---------------+------*/
// This function should only be called when constructing a complex element descriptor
StatusInt   CompressedXAttribute::ScheduleReplace
(
EditElementHandleP     ehandle,
XAttributeHandlerId handlerId,
UInt32              xAttrId,
byte const*         buffer,
size_t              bufferSize,
Int32               compressionThreshold    // => If -1, compression is disabled; otherwise attempt to compress only if larger than this
)
    {
    StatusInt   status;
    byte        *tmpBuffer;
    UInt32      totalSize=0;

    if (NULL == ehandle || NULL == buffer)
        return DGNHANDLERS_STATUS_BadArg;

    if (SUCCESS != (status = compressedXAttribute_createBuffer (&tmpBuffer, &totalSize, buffer, (UInt32) bufferSize, compressionThreshold)))
        return status;

    // Tell the element descr to add this Xattribute when rewrite is called
    status = ehandle->ScheduleWriteXAttribute (handlerId, xAttrId, static_cast<UInt32>(totalSize), tmpBuffer);

    memutil_free (tmpBuffer);
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    03/06
+---------------+---------------+---------------+---------------+---------------+------*/
// This function is useful from within an XAttributeHandler::OnPreprocessCopy callback
StatusInt   CompressedXAttribute::Replace
(
IReplaceXAttribute *xAttrReplacer,
XAttributeHandlerId handlerId,
UInt32              xAttrId,
byte const*         buffer,
size_t              bufferSize,
Int32               compressionThreshold    // => If -1, compression is disabled; otherwise attempt to compress only if larger than this
)
    {
    StatusInt   status;
    byte        *tmpBuffer;
    UInt32      totalSize=0;

    if (NULL == xAttrReplacer || NULL == buffer)
        return DGNHANDLERS_STATUS_BadArg;

    if (SUCCESS != (status = compressedXAttribute_createBuffer (&tmpBuffer, &totalSize, buffer, (UInt32) bufferSize, compressionThreshold)))
        return status;

    xAttrReplacer->Add (handlerId, xAttrId, static_cast<UInt32>(totalSize), tmpBuffer);
    status = SUCCESS;

    memutil_free (tmpBuffer);
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Chuck.Kirschman 11/04
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   CompressedXAttribute::GetUncompressedSize
(
XAttributeHandle const&   xattr,
size_t*                   size
)
    {
    return xattr.IsValid() ? CompressedXAttribute::GetUncompressedSizeFromData (xattr.PeekData(), size) : ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Chuck.Kirschman 11/04
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   CompressedXAttribute::GetUncompressedSizeFromData
(
void const*                 data,
size_t*                     size
)
    {
    CompressedXAttrHeader     header;
    memcpy (&header, data, sizeof(header));

    if (NULL != size)
        *size = header.uncompressedSize;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     10/05
*
*   static function needed when working directly with data (from shared file merge).
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   CompressedXAttribute::ExtractBufferFromData
(
void const*     pData,
size_t          dataSize,
byte*           buffer,
size_t          bufferLength
)
    {
    byte const* data = (byte const*) pData;
    CompressedXAttrHeader     header;

    memcpy (&header, data, sizeof(header));
    data += sizeof(header);

    if (header.uncompressedSize > bufferLength)
        return DGNHANDLERS_STATUS_ElementTooLarge;

    switch (header.type)
        {
        case COMPRESSIONTYPE_Disallowed:
        case COMPRESSIONTYPE_Uncompressed:
            memcpy (buffer, data, header.uncompressedSize);
            break;

        case COMPRESSIONTYPE_LZW:
            UInt32  compressedSize = static_cast<UInt32>(dataSize - sizeof(header));
            ULong   uncompressedSize = header.uncompressedSize;
            UInt32  bytesActuallyRead = 0;

            DgnZLib::UnZipper unzip;
            unzip.Init(data, compressedSize);
            if (ZIP_SUCCESS != unzip.Read(buffer, uncompressedSize, &bytesActuallyRead))
                return ERROR;

            BeAssert(header.uncompressedSize == bytesActuallyRead);
            break;
        }

    return SUCCESS;
   }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Chuck.Kirschman 11/04
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   CompressedXAttribute::ExtractBuffer
(
XAttributeHandle const&   xattr,
byte*                     buffer,
size_t                    bufferLength
)
    {
    return xattr.IsValid() ? CompressedXAttribute::ExtractBufferFromData (xattr.PeekData(), xattr.GetSize(), buffer, bufferLength) : ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Chuck.Kirschman 11/04
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   CompressedXAttribute::Delete
(
ElementRefP          elemRef,
XAttributeHandle    attrHandle
)
    {
    if (!attrHandle.IsValid())
        return DGNHANDLERS_STATUS_LinkageNotFound;

    return attrHandle.GetElementRef()->GetDgnProject()->GetTxnManager().GetCurrentTxn().DeleteXAttribute (attrHandle);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Chuck.Kirschman 11/04
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   StringXAttribute::Add
(
ElementRefP      elemRef,                // => Element ref to attach XAttribute to
WCharCP         string,
UInt32          attrId,                 // => If appID is INVALID_XATTR_ID, a new id is assigned & returned in outXAttrId
UInt16          appType,                // => This should be one of STRING_LINKAGE_KEY values in msdefs.h
Int32           compressionThreshold,   // => If -1, compression is disabled; otherwise attempt to compress only if greater than this many bytes
UInt32*         outXAttrId              // => If not NULL, actual XAttr attrId used.
)
    {
    Utf16Buffer utf16;
    BeStringUtilities::WCharToUtf16 (utf16, string, BeStringUtilities::AsManyAsPossible);
    XAttributeHandlerId handlerId (XATTRIBUTEID_String, appType);
    return CompressedXAttribute::Add (elemRef, handlerId, attrId, (byte const*)utf16.data(), utf16.size()*sizeof(utf16[0]), compressionThreshold, outXAttrId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Chuck.Kirschman 11/04
+---------------+---------------+---------------+---------------+---------------+------*/
// This function should only be called when constructing a complex element descriptor
StatusInt   StringXAttribute::ScheduleAdd
(
EditElementHandleP ehandle,
WCharCP         string,
UInt32          attrId,                 // => If appID is INVALID_XATTR_ID, a new id is assigned & returned in outXAttrId
UInt16          appType,                // => This should be one of STRING_LINKAGE_KEY values in msdefs.h
Int32           compressionThreshold    // => If -1, compression is disabled; otherwise attempt to compress only if greater than this many bytes
)
    {
    Utf16Buffer utf16;
    BeStringUtilities::WCharToUtf16 (utf16, string, BeStringUtilities::AsManyAsPossible);
    XAttributeHandlerId handlerId (XATTRIBUTEID_String, appType);
    return CompressedXAttribute::ScheduleAdd (ehandle, handlerId, attrId, (byte const*)utf16.data(), utf16.size()*sizeof(utf16[0]), compressionThreshold);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Chuck.Kirschman 11/04
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   StringXAttribute::Replace
(
ElementRefP      elemRef,                // =>
WCharCP         string,
UInt32          attrId,                 // =>
UInt16          appType,                // => This should be one of STRING_LINKAGE_KEY values in msdefs.h
Int32           compressionThreshold    // => If -1, compression is disabled; otherwise attempt to compress only if larger than this
)
    {
    Utf16Buffer utf16;
    BeStringUtilities::WCharToUtf16 (utf16, string, BeStringUtilities::AsManyAsPossible);
    XAttributeHandlerId handlerId (XATTRIBUTEID_String, appType);
    XAttributeHandle    attrHandle (elemRef, handlerId, attrId);
    return CompressedXAttribute::Replace (attrHandle, (byte const*)utf16.data(), utf16.size()*sizeof(utf16[0]), compressionThreshold);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Chuck.Kirschman 11/04
+---------------+---------------+---------------+---------------+---------------+------*/
// This function should only be called when constructing a complex element descriptor
StatusInt   StringXAttribute::ScheduleReplace
(
EditElementHandleP ehandle,
WCharCP         string,
UInt32          attrId,                 // => If appID is INVALID_XATTR_ID, a new id is assigned & returned in outXAttrId
UInt16          appType,                // => This should be one of STRING_LINKAGE_KEY values in msdefs.h
Int32           compressionThreshold    // => If -1, compression is disabled; otherwise attempt to compress only if greater than this many bytes
)
    {
    Utf16Buffer utf16;
    BeStringUtilities::WCharToUtf16 (utf16, string, BeStringUtilities::AsManyAsPossible);
    XAttributeHandlerId handlerId (XATTRIBUTEID_String, appType);
    return CompressedXAttribute::ScheduleReplace (ehandle, handlerId, attrId, (byte const*)utf16.data(), utf16.size()*sizeof(utf16[0]), compressionThreshold);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Chuck.Kirschman 11/04
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   StringXAttribute::Extract
(
WChar*        string,            // <=> buffer to copy string into; can be NULL if gettting length
size_t          length,             // => Length of string.
size_t*         actualLength,       // <= actual length; can be NULL
ElementRefP      elemRef,            // =>
UInt32          attrId,             // =>
UInt16          appType             // => This should be one of STRING_LINKAGE_KEY values in msdefs.h
)
    {
    XAttributeHandlerId handlerId (XATTRIBUTEID_String, appType);
    XAttributeHandle xattr (elemRef, handlerId, attrId);
    StatusInt           status = SUCCESS;

    if (!xattr.IsValid())
        return ERROR;

    if (NULL != actualLength)
        status = CompressedXAttribute::GetUncompressedSize (xattr, actualLength);

    // Bail out now if caller is just interested in the size.
    if (NULL == string || 0 == length)
        return status;

    return CompressedXAttribute::ExtractBuffer (xattr, (byte *)string, length * sizeof(string)[0]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley 03/2009
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   StringXAttribute::Extract
(
WChar*        string,            // <=> buffer to copy string into; can be NULL if gettting length
size_t          length,             // => Length of string.
size_t*         actualLength,       // <= actual length; can be NULL
ElementHandleCR    elemHandle,         // =>
UInt32          attrId,             // =>
UInt16          appType             // => This should be one of STRING_LINKAGE_KEY values in msdefs.h
)
    {
    XAttributeHandlerId handlerId (XATTRIBUTEID_String, appType);
    ElementHandle::XAttributeIter  xattr (elemHandle, handlerId, attrId);
    StatusInt           status = SUCCESS;

    if (!xattr.IsValid())
        return ERROR;

    if (NULL != actualLength)
        CompressedXAttribute::GetUncompressedSizeFromData (xattr.PeekData(), actualLength);

    // Bail out now if caller is just interested in the size.
    if (NULL == string || 0 == length)
        return status;

    return CompressedXAttribute::ExtractBufferFromData (xattr.PeekData(), xattr.GetSize(), (byte*)string, length * sizeof(string)[0]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Chuck.Kirschman 11/04
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   StringXAttribute::Delete
(
ElementRefP      elemRef,
UInt32          attrId,
UInt16          appType             // => This should be one of STRING_LINKAGE_KEY values in msdefs.h
)
    {
    XAttributeHandlerId handlerId (XATTRIBUTEID_String, appType);
    XAttributeHandle    attrHandle (elemRef, handlerId, attrId);

    if (!attrHandle.IsValid())
        return DGNHANDLERS_STATUS_LinkageNotFound;

    return attrHandle.GetElementRef()->GetDgnProject()->GetTxnManager().GetCurrentTxn().DeleteXAttribute (attrHandle);
    }

