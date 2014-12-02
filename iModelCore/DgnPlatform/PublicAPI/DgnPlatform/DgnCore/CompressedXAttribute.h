/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/CompressedXAttribute.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

#include "XAttributeHandler.h"

enum
    {
    COMPRESSION_THRESHOLD_Disabled          = -1,       // Don't allow compression
    COMPRESSION_THRESHOLD_MinimumAccess     = 0,        // Don't access the data frequently; minimum memory usage
    COMPRESSION_THRESHOLD_FrequentAccess    = 1024,     // Access the data frequently; reasonable tradeoff of memory and speed
    };


BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//=======================================================================================
//!
//! @bsiclass
struct CompressedXAttribute
{
DGNPLATFORM_EXPORT static StatusInt   Add
(
ElementRefP           elemRef,                  // => Element ref to attach XAttribute to
XAttributeHandlerId  handlerId,
UInt32               xAttrId,                   // => If INVALID_XATTR_ID, a new id is assigned & returned in outXAttrId
byte const*          buffer,
size_t               bufferSize,
Int32                compressionThreshold,      // => If -1, compression is disabled; otherwise attempt to compress only if larger than this
UInt32*              outXAttrId=NULL            // => If non-NULL, actual ID assigned to XAttribute
);

DGNPLATFORM_EXPORT static StatusInt   ScheduleAdd
(
EditElementHandleP     ehandle,
XAttributeHandlerId handlerId,
UInt32              xAttrId,
byte const*         buffer,
size_t              bufferSize,
Int32               compressionThreshold    // => If -1, compression is disabled; otherwise attempt to compress only if larger than this
);

DGNPLATFORM_EXPORT static StatusInt   Replace
(
XAttributeHandle&   attrHandle,
byte const*         buffer,
size_t              bufferSize,
Int32               compressionThreshold    // => If -1, compression is disabled; otherwise attempt to compress only if larger than this
);

               static StatusInt   Replace
(
XAttributeHandle const& attrHandle,
byte const*         buffer,
size_t              bufferSize,
Int32               compressionThreshold    // => If -1, compression is disabled; otherwise attempt to compress only if larger than this
)
    {
    return Replace (const_cast<XAttributeHandle&>(attrHandle), buffer, bufferSize, compressionThreshold);
    }

DGNPLATFORM_EXPORT static StatusInt   Replace
(
IReplaceXAttribute* xAttrReplacer,
XAttributeHandlerId handlerId,
UInt32              xAttrId,
byte const*         buffer,
size_t              bufferSize,
Int32               compressionThreshold    // => If -1, compression is disabled; otherwise attempt to compress only if larger than this
);

DGNPLATFORM_EXPORT static StatusInt   ScheduleReplace
(
EditElementHandleP     ehandle,
XAttributeHandlerId handlerId,
UInt32              xAttrId,
byte const*         buffer,
size_t              bufferSize,
Int32               compressionThreshold    // => If -1, compression is disabled; otherwise attempt to compress only if larger than this
);

DGNPLATFORM_EXPORT static StatusInt   GetUncompressedSize
(
XAttributeHandle const&   xattr,
size_t*                   size
);

DGNPLATFORM_EXPORT static StatusInt   ExtractBuffer
(
XAttributeHandle const&   xattr,
byte*                     buffer,
size_t                    bufferLength
);

DGNPLATFORM_EXPORT static StatusInt   Delete
(
ElementRefP               elemRef,
XAttributeHandle         handler
);

DGNPLATFORM_EXPORT static StatusInt    ExtractBufferFromData
(
void const*             data,
size_t                  dataSize,
byte*                   buffer,
size_t                  bufferLength
);

DGNPLATFORM_EXPORT static StatusInt    GetUncompressedSizeFromData
(
void const*             data,
size_t*                 size
);

};

//=======================================================================================
//! @bsiclass
struct          StringXAttribute
{
DGNPLATFORM_EXPORT static StatusInt   Add
(
ElementRefP      elemRef,               // => Element ref to attach XAttribute to
WChar const*  string,
UInt32          attrId,                 // => If appID is INVALID_XATTR_ID, a new id is assigned & returned in outXAttrId
UInt16          appType,                // => This should be one of STRING_LINKAGE_KEY values in msdefs.h
Int32           compressionThreshold,   // => If -1, compression is disabled; otherwise attempt to compress only if greater than this many bytes
UInt32*         outXAttrId              // => If not NULL, actual XAttr attrId used.
);

DGNPLATFORM_EXPORT static StatusInt   ScheduleAdd
(
EditElementHandleP ehandle,
WChar const*  string,
UInt32          attrId,                 // => If appID is INVALID_XATTR_ID, a new id is assigned & returned in outXAttrId
UInt16          appType,                // => This should be one of STRING_LINKAGE_KEY values in msdefs.h
Int32           compressionThreshold    // => If -1, compression is disabled; otherwise attempt to compress only if greater than this many bytes
);

DGNPLATFORM_EXPORT static StatusInt   Replace
(
ElementRefP      elemRef,               // =>
WChar const*  string,
UInt32          attrId,                 // =>
UInt16          appType,                // => This should be one of STRING_LINKAGE_KEY values in msdefs.h
Int32           compressionThreshold    // => If -1, compression is disabled; otherwise attempt to compress only if larger than this
);

DGNPLATFORM_EXPORT static StatusInt   ScheduleReplace
(
EditElementHandleP ehandle,
WChar const*  string,
UInt32          attrId,                 // => If appID is INVALID_XATTR_ID, a new id is assigned & returned in outXAttrId
UInt16          appType,                // => This should be one of STRING_LINKAGE_KEY values in msdefs.h
Int32           compressionThreshold    // => If -1, compression is disabled; otherwise attempt to compress only if greater than this many bytes
);

DGNPLATFORM_EXPORT static StatusInt   Extract
(
WChar*        pString,            // <=> buffer to copy string into; can be NULL if gettting length
size_t          length,             // => Length of pString.
size_t*         actualLength,       // <= actual length; can be NULL
ElementRefP      elemRef,            // =>
UInt32          attrId,             // =>
UInt16          appType             // => This should be one of STRING_LINKAGE_KEY values in msdefs.h
);

static StatusInt   Extract
(
WChar*        pString,            // <=> buffer to copy string into; can be NULL if gettting length
size_t          length,             // => Length of pString.
size_t*         actualLength,       // <= actual length; can be NULL
ElementHandleCR    elemHandle,         // => source
UInt32          attrId,             // =>
UInt16          appType             // => This should be one of STRING_LINKAGE_KEY values in msdefs.h
);

DGNPLATFORM_EXPORT static StatusInt   Delete
(
ElementRefP      elemRef,
UInt32          attrId,
UInt16          appType             // => This should be one of STRING_LINKAGE_KEY values in msdefs.h
);

};

END_BENTLEY_DGNPLATFORM_NAMESPACE


