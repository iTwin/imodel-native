/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/Linkage1.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

#include "DgnElements.h"

/*----------------------------------------------------------------------+
|                                                                       |
|   Status defined for linkage process functions                        |
|                                                                       |
+----------------------------------------------------------------------*/
#define PROCESS_ATTRIB_STATUS_NOCHANGE  0
#define PROCESS_ATTRIB_STATUS_REPLACE   (1<<0)
#define PROCESS_ATTRIB_STATUS_DELETE    (1<<1)
#define PROCESS_ATTRIB_STATUS_ABORT     (1<<2)

#define PROCESS_ATTRIB_ERROR_ABORT      (1<<0)
#define PROCESS_ATTRIB_ERROR_MAXSIZE    (1<<1)
#define PROCESS_ATTRIB_ERROR_BADELEM    (1<<2)
#define PROCESS_ATTRIB_ERROR_BADARG     (1<<2)

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

/*=================================================================================**//**
* @bsiclass                                                     Brien.Bastings  02/09
+===============+===============+===============+===============+===============+======*/
struct LinkageUtil
{
public:

//! @description   Sets the size of the linkage. The maximum size of a linkage is determined
//!                by the size of the element that the linkage is to be appended to, since
//!                the total element size cannot exceed MAX_ELEMENT_WORDS, or 65535.
//! @param[in]     linkHdrP pointer to the linkage header to set the size of.
//! @param[in]     wordLength size of the linkage in words. If this value exceeds MAX_ELEMENT_WORDS an error is returned.
//! @return      SUCCESS unless wordlength exceeds MAX_ELEMENT_WORDS , then ERROR.
DGNPLATFORM_EXPORT static int SetWords (LinkageHeader* linkHdrP, int wordLength);

//! @description    Gets the size of the linkage, in two-byte words.
//! @param[in]      linkHdrP pointer to the linkage header.
//! @return         size, in words, of the linkage.  MAX_ELEMENT_WORDS is returned if the linkage
//!                 is too big.  This result typically indicates an error in data stored in the element.
DGNPLATFORM_EXPORT static UInt16 GetWords (LinkageHeader const* linkHdrP);

//! @description   Calculate the actual size (bytes) of a linkage given the number of
//!                bytes of data (including linkage header) that needs to be written into
//!                the linkage
//! @param [in]    bufferSizeIn    bytes of data that need to be written
//! @return      The calculated size of linkage in bytes
DGNPLATFORM_EXPORT static int CalculateSize (int bufferSizeIn);

//! @Description This function appends the specified string to the given element
//! as a data linkage using the given key.
//! @param[in] elementIn          the element to which the linkage-string is appended
//! @param[in] linkageKey         the linkage key to use for the given string data
//! @param[in] linkageStringIn    the string to append to the data linkage on the element
//! @Return SUCCESS unless the data could not be added to the linkage, then ERROR.
DGNPLATFORM_EXPORT static StatusInt SetStringLinkage (DgnElementP elementIn, UShort linkageKey, WCharCP linkageStringIn);

//! @Description This function sets the given string data linkage on the
//! given element descriptor.
//! @param[in] ppElemDscrIn           the element descriptor to set the linkage-string on
//! @param[in] linkageKeyIn           the linkage key to use for the string data
//! @param[in] linkageStringIn        the linkage string to add
//! @Remarks If a string linkage is found on the element descriptor with the same key as
//!          specified, the new string will replace the original.
//! @Return SUCCESS if the operation was completed successfully, otherwise ERROR or DGNHANDLERS_STATUS_LinkageNotFound.
DGNPLATFORM_EXPORT static StatusInt SetStringLinkageUsingDescr (MSElementDescrP elemDscrIn, UShort linkageKey, WCharCP linkageStringIn);

//! Extract the string from the linkage with the specified index.
//! @param[out] linkageKey The linkage key for the string of the given index
//! @param[out] linkageString The linkage string
//! @param[in] bufferSize The size of the pLinkageStringOut buffer
//! @param[in] linkageIndex The index of the string linkage to extract
//! @param[in] element The element containing the linkage
//! @return SUCCESS if the string was extracted, otherwise ERROR or DGNHANDLERS_STATUS_LinkageNotFound.
DGNPLATFORM_EXPORT static StatusInt ExtractStringLinkageByIndex (UShort* linkageKey, WCharP linkageString, int bufferSize, int linkageIndex, DgnElementCP element);

//! Extract the string from the linkage with the specified linkage key and index.
//! @param[out] linkageString The linkage string
//! @param[in] bufferSize The size of linkageString
//! @param[in] linkageKey The linkage key to extract
//! @param[in] linkageIndex The index to extract
//! @param[in] element The element containing the linkage
//! @return SUCCESS if the string was extracted, otherwise ERROR
DGNPLATFORM_EXPORT static StatusInt ExtractNamedStringLinkageByIndex (WCharP linkageString, int bufferSize, UShort linkageKey, int linkageIndex, DgnElementCP element);

//! Extract the string from the linkage with the specified linkage key and index.
//! @param[out] linkageString The linkage string
//! @param[in] linkageKey The linkage key to extract
//! @param[in] linkageIndex The index to extract
//! @param[in] element The element containing the linkage
//! @return SUCCESS if the string was extracted, otherwise ERROR
DGNPLATFORM_EXPORT static StatusInt ExtractNamedStringLinkageByIndex (WStringR linkageString, UShort linkageKey, int linkageIndex, DgnElementCP element);

//! @Description This function is used to delete the specified string linkage data
//!                 from the specified element.
//! @param[in] elementIn          the element to remove the string linkage data from
//! @param[in] linkageKeyIn       the linkage key for the string data
//! @param[in] linkageIndexIn     the index of the string data
//! @Return SUCCESS if the linkage could be found and removed, otherwise ERROR
DGNPLATFORM_EXPORT static BentleyStatus DeleteStringLinkage (DgnElementP elementIn, UShort linkageKeyIn, int linkageIndexIn);

//! @Description This function appends the specified string to the given element
//!                 as a data linkage using the given key.
//! @param[in] elementIn          the element to which the linkage-string is appended
//! @param[in] linkageKey         the linkage key to use for the given string data
//! @param[in] linkageStringIn    the string to append to the data linkage on the element
//! @Return SUCCESS unless the data could not be added to the linkage, then ERROR or DGNHANDLERS_STATUS_BadArg.
DGNPLATFORM_EXPORT static StatusInt AppendStringLinkage (DgnElementP elementIn, UShort linkageKey, WCharCP linkageStringIn);

//! @Description This function appends the given string data linkage to the
//!                 given element descriptor.
//! @param[in] ppElemDscrIn        the element descriptor to append the linkage-string to
//! @param[in] linkageKey          the linkage key to use for the string data
//! @param[in] linkageStringIn     the linkage string to append
//! @Return SUCCESS if the operation was completed successfully, otherwise ERROR.
DGNPLATFORM_EXPORT static StatusInt AppendStringLinkageUsingDescr (MSElementDescrP elemDscrIn, UShort linkageKey, WCharCP linkageStringIn);

DGNPLATFORM_EXPORT static BentleyStatus CreateStringLinkage (bvector<UInt8>& linkage, UInt16 linkageKey, WCharCP string);

DGNPLATFORM_EXPORT static WString  GetStringFromStringLinkage (MSStringLinkageData const&);

//! @description Sets the values in an array of elementID already set as a linkage on the specified element.
//!             Use a dependency linkage to make persistent references to other elements, not this function!
//! @param[in] elementIn          the element on which to set the array of elementids
//! @param[in] linkageKeyIn       the linkage key to use for the array
//! @param[in] elementIDCountIn   the number of elementID values in the array
//! @param[in] elementIDArrayIn   the array of elementIDvalues to set
//! @return SUCCESS If the operation is completed without error
//! @Remarks Use a dependency linkage to make persistent references to other elements!
DGNPLATFORM_EXPORT static StatusInt SetElementIDArrayLinkage (DgnElementP elementIn, UShort linkageKeyIn, UInt32 elementIDCountIn, ElementId const*  elementIDArrayIn);
    
//! @description Appends the specified array of elementID values as a linkage on the given element.
//!             Element needs to be of sufficient size.
//! Use a dependency linkage to make persistent references to other elements, not this function!
//! @param[in] elementIn          the element to append the linkage array to
//! @param[in] linkageKeyIn       the linkage key to use to append the linkage
//! @param[in] elementIDCountIn   the number of elementID values in the array
//! @param[in] elementIDArrayIn   the array of  elementID values to append as a linkage
//! @return SUCCESS If the operation was completed successfully.
//! @Remarks Use a dependency linkage to make persistent references to other elements!
DGNPLATFORM_EXPORT static StatusInt AppendElementIDArrayLinkage (DgnElementP elementIn, UShort linkageKeyIn, UInt32 elementIDCountIn, ElementId const* elementIDArrayIn);

}; // LinkageUtil

/*----------------------------------------------------------------------+
|                                                                       |
|   ElementIdArrayLinkage                                               |
|                                                                       |
+----------------------------------------------------------------------*/
typedef struct msElementIDArrayLinkageData
    {
    UInt16                  linkageKey;                     /* id identifying type of element-id linkage */
    UInt16                  padding;
    UInt32                  linkageArrayLength;             /* size of element-id array linkage data */
    UInt64                  linkageArray[1];
    } MSElementIDArrayLinkageData;

typedef struct msElementIDArrayLinkage
    {
    LinkageHeader               header;
    Int32                       padding;                    //  LinkageHeader is 4-bytes but MSElementIDArrayLinkageData is 8-byte
                                                            //  aligned on Windows and 4-byte aligned on other platforms.
    MSElementIDArrayLinkageData data;

    /*---------------------------------------------------------------------------------**//**
    * Extracts an array of elementID values from the linkage on the specified element.
    * Use a dependency linkage to make persistent references to other elements!
    * @param linkageKeyOut              OUT the linkage key on the array
    * @param elementIDArrayOut          OUT the array of linkage values on the element
    * @param elementIn                  IN the element from which to get the array
    * @param linkageIndexIn             IN the index of the array in the user data linkage
    * @return SUCCESS if the array was found and retrieved, otherwise ERROR
    * @Remarks Use a dependency linkage to make persistent references to other elements!
    * @group "mdlLinkage"
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    DGNPLATFORM_EXPORT static StatusInt ExtractByIndex
    (
    UShort *                linkageKeyOut,
    bvector<UInt64>* elementIDArrayOut,
    DgnElementCP             elementIn,
    int                     linkageIndexIn
    );

    /*---------------------------------------------------------------------------------**//**
    * Extracts an array of elementID values from the data linkage of the specified element.
    * Use a dependency linkage to make persistent references to other elements!
    * @param elementIDArrayOut      OUT the array of linkage values on the element
    * @param elementIn              IN the element from which to get the array
    * @param linkageKeyIn           IN the linkage key to use to get the array
    * @param linkageIndexIn         IN the index of the linkage containing the array
    * @return SUCCESS if the array is found on the element and returned successfully
    * @Remarks Use a dependency linkage to make persistent references to other elements!
    * @group "mdlLinkage"
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    DGNPLATFORM_EXPORT static StatusInt ExtractNamedByIndex
    (
    bvector<UInt64>* elementIDArrayOut,
    DgnElementCP             elementIn,
    UShort                  linkageKeyIn,
    int                     linkageIndexIn
    );


    } MSElementIDArrayLinkage;

//__PUBLISH_SECTION_END__

#define DataDefID_MultiStateMaskLinkage       1011
/*----------------------------------------------------------------------+
|                                                                       |
|   MultiStateMaskLinkage                                               |
|                                                                       |
+----------------------------------------------------------------------*/
typedef struct msMultiStateMaskLinkage
    {
    LinkageHeader               header;
    MSMultiStateMaskLinkageData data;
    } MSMultiStateMaskLinkage;

/*----------------------------------------------------------------------+
|                                                                       |
|   MSModelIdLinkage                                                    |
|                                                                       |
+----------------------------------------------------------------------*/
typedef struct msModelIdLinkageData
    {
    UInt16                  linkageKey;                     /* id identifying type of ModelId linkage */
    UInt16                  padding;
    UInt32                  modelID;
    UInt32                  padding4;                       /* brings struct size up to 16, since total linkage size must be a multiple of 8 */
    } MSModelIdLinkageData;

typedef struct msModelIdLinkage
    {
    LinkageHeader           hdr;
    MSModelIdLinkageData    data;
    } MSModelIdLinkage;

/*----------------------------------------------------------------------+
|                                                                       |
|   RefRenderingPlotLinkage                                             |
|                                                                       |
+----------------------------------------------------------------------*/
typedef struct msRefRenderingPlotLinkage
    {
    LinkageHeader               header;
    UInt32                      plotType:8;         // one of REF_PLOTTYPE_Xxxxx's
    UInt32                      reserved:24;        // unused bits
    ElementId                   elementId;          // element ID of display style, or rendering setting style
    } MSRefRenderingPlotLinkage;


//__PUBLISH_SECTION_START__
END_BENTLEY_DGNPLATFORM_NAMESPACE

BEGIN_BENTLEY_API_NAMESPACE


/*---------------------------------------------------------------------------------**//**
* @description Appends the specified multi state mask as a linkage on the given element.
* @param elementIn          IN the element to append multi-state mask linkage to
* @param linkageKeyIn       IN the linkage key to use when appending the linkage
* @param multiStateMaskIn   IN the multi state mask to add as a linkage
* @return SUCCESS if the linkage could be attached
* @group "mdlLinkage"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT int      mdlLinkage_appendMultiStateMask
(
DgnElementP          elementIn,
UShort              linkageKeyIn,
MultiStateMaskP     multiStateMaskIn
);

/*---------------------------------------------------------------------------------**//**
* Sets the contents of a multi state mask already appended as a linkage on an element
* @param elementIn          IN the element on which the linkage is attached
* @param linkageKey         IN the linkage key of the linkage
* @param multiStateMaskIn   IN the multi state mask contents to set
* @return SUCCESS if the linkage was found and could be set
* @group "mdlLinkage"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT int      mdlLinkage_setMultiStateMask
(
DgnElementP          elementIn,
UShort              linkageKey,
MultiStateMaskP     multiStateMaskIn
);

/*---------------------------------------------------------------------------------**//**
* Extracts the multi state mask linkage from the given element
* @param multiStateMaskOut      OUT the bit mask extracted from the linkage, must be freed by caller
* @param elementIn              IN the element that has the linkage attached
* @param linkageKeyIn           IN the linkage key for the linkage
* @param linkageIndexIn         IN the index of the linkage on the element
* @return SUCCESS if the linkage is found and extracted
* @group "mdlLinkage"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT int      mdlLinkage_extractMultiStateMask
(
MultiStateMaskP *   multiStateMaskOut,
DgnElementCP         elementIn,
UShort              linkageKeyIn,
int                 linkageIndexIn
);

/*---------------------------------------------------------------------------------**//**
* Deletes the multi state mask linkage from the specified element.
* @param elementIn      IN the element to delete the multi state mask linkage from
* @param linkageKeyIn   IN the linkage key for the linkage
* @param linkageIndexIn IN the index of the linkage on the element
* @return SUCCESS if the linkage was found and deleted successfully
* @group "mdlLinkage"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT int      mdlLinkage_deleteMultiStateMask
(
DgnElementP      elementIn,
UShort          linkageKeyIn,
int             linkageIndexIn
);

//__PUBLISH_SECTION_START__

END_BENTLEY_API_NAMESPACE
