/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnHandlers/DgnStoreHandlers.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
/** @cond BENTLEY_SDK_Internal */

#include <DgnPlatform/DgnCore/Handler.h>

#if defined (NEEDS_WORK_DGNITEM)
BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

/// @addtogroup Handler
/// @beginGroup

/*=================================================================================**//**
* The default type handler for the DGNSTORE_HDR type that corresponds to the
* DgnStoreHdr structure. This non-graphic element type was created to store binary
* non-element data in the design file prior to the advent of XAttributes.
* The dgn store has two representations, it can be a complex element of type
* DGNSTORE_HDR with zero or more components of type DGNSTORE_COMP, it can also be a
* series of sibling DGNSTORE_COMP elements that are children of a cell. With the later
* varient the first DGNSTORE_COMP in the series is special and uses the DgnStoreHdr
* structure instead of the DgnStoreComp structure. The methods on DgnStoreHdrHandler
* will transparently extract information from both types, but the create method will
* always produce the preferred DGNSTORE_HDR varient which was introduced in
* MicroStation V8.
* @note Don't create new kinds of dgn stores use XAttributes instead!
* @bsiclass                                                     Brien.Bastings  11/06
+===============+===============+===============+===============+===============+======*/
struct DgnStoreHdrHandler : Handler
{
    DEFINE_T_SUPER(Handler)
    ELEMENTHANDLER_DECLARE_MEMBERS (DgnStoreHdrHandler, DGNPLATFORM_EXPORT)
//__PUBLISH_SECTION_END__
enum
    {
    MAX_DGNSTORE_SIZE = (MAX_V8_ELEMENT_SIZE - 200),
    };

protected:

// Handler
DGNPLATFORM_EXPORT virtual void _GetTypeName (WStringR string, UInt32 desiredLength) override;

//__PUBLISH_SECTION_START__
//__PUBLISH_CLASS_VIRTUAL__
public:

DGNPLATFORM_EXPORT static bool           IsDgnStoreElement (ElementHandleCR in, UInt32 dgnStoreId = 0, UInt32 applicationId = 0);
DGNPLATFORM_EXPORT static BentleyStatus  GetDgnStoreIds (UInt32* dgnStoreId, UInt32* applicationId, ElementHandleCR in);
DGNPLATFORM_EXPORT static BentleyStatus  Extract (void** dataPP, UInt32* dataSizeP, UInt32 dgnStoreId, UInt32 applicationId, ElementHandleCR eh);
DGNPLATFORM_EXPORT static BentleyStatus  ExtractFromCell (void** dataPP, UInt32* dataSizeP, UInt32 dgnStoreId, UInt32 applicationId, ElementHandleCR eh);
DGNPLATFORM_EXPORT static void           FreeExtractedData (void* dataP);

DGNPLATFORM_EXPORT static MSElementDescrPtr Create (void* dataP, UInt32 dataSize, UInt32 dgnStoreId, UInt32 applicationId, DgnModelR);
DGNPLATFORM_EXPORT static BentleyStatus  Create (EditElementHandleR eeh, void* dataP, UInt32 dataSize, UInt32 dgnStoreId, UInt32 applicationId, DgnModelR);
DGNPLATFORM_EXPORT static BentleyStatus  AppendToCell (EditElementHandleR cellEeh, void* dataP, UInt32 dataSize, UInt32 dgnStoreId, UInt32 applicationId);
DGNPLATFORM_EXPORT static BentleyStatus  RemoveFromCell (EditElementHandleR cellEeh, UInt32 dgnStoreId, UInt32 applicationId);

}; // DgnStoreHdrHandler

END_BENTLEY_DGNPLATFORM_NAMESPACE
#endif

/** @endcond */
