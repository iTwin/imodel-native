/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnFileIO/strlinkage.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>
#include    <DgnPlatform/Tools/varichar.h>
#include <RmgrTools/RscMgr/bnryport.h>
#include <DgnPlatform/DgnCore/Linkage1.h>

// output of mdlCnv_compileDataDefFromFileWithAllocType (...(RscFileHandle)NULL, DataDefID_StringLinkage, ...):
static byte s_compiledStringLinkageConvRules[] =
    {
    0x70,0x6f,0x76,0x63,0x84,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x4,0x0,0x0,0x0,0x4,0x0,0x0
   ,0x0,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x5,0x0,0x0,0x0,0x15,0x0,0x0,0x0,0x4,0x0,0x0
   ,0x0,0x4,0x0,0x0,0x0,0xd,0xf0,0xad,0xba,0xd,0xf0,0xad,0xba,0x2,0x0,0x0,0x0,0x4,0x0
   ,0x0,0x0,0x4,0x0,0x0,0x0,0x2,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x5,0x0,0x0,0x0,0x4,0x0
   ,0x0,0x0,0x4,0x0,0x0,0x0,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x0,0x0,0x0,0x1,0x0
   ,0x0,0x0,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x16,0x0,0x0,0x0,0x4,0x0
   ,0x0,0x0,0x4,0x0,0x0,0x0,0xd,0xf0,0xad,0xba,0xd,0xf0,0xad,0xba
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/94
+---------------+---------------+---------------+---------------+---------------+------*/
void     mdlLinkage_initializeStringConversionRules ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Matt.Watkins    08/02
+---------------+---------------+---------------+---------------+---------------+------*/
static void* linkage_getStringConversionRules ()
    {
    /* make sure the linkage conversion rules are compiled in the context of ustation.dll. */
    return  &s_compiledStringLinkageConvRules;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      12/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus LinkageUtil::CreateStringLinkage (bvector<UInt8>& linkage, UInt16 linkageKey, WCharCP pLinkageStringIn)
    {
    if (NULL == pLinkageStringIn)
        return ERROR;

    // Based on analyzing the old code, these do NOT write NULL terminators.
    bvector<VariChar> variBuffer;
    VariCharConverter::UnicodeToVariChar (variBuffer, pLinkageStringIn, LangCodePage::Unicode, false);
    
    /* Initialize an MSStringLinkageData from the input string */
    size_t stringBytes = (variBuffer.size () * sizeof (VariChar));
    size_t msStringLinkageDataBufSize = sizeof (MSStringLinkageData) + stringBytes;

    MSStringLinkageData* inputStrData = static_cast<MSStringLinkageData *>(_alloca (msStringLinkageDataBufSize));

    inputStrData->linkageKey           = linkageKey;
    inputStrData->mustBeZero           = 0;     // Must be zero!
    inputStrData->linkageStringLength  = (UInt32) stringBytes;
    
    memcpy (inputStrData->linkageString, &variBuffer[0], stringBytes);

    //  Allocate output MSStringLinkage (including LinkageHeader)
    size_t msStringLinkageBufSize = sizeof (LinkageHeader) + msStringLinkageDataBufSize + 15;    // in case conversion alignment rules cause data to expand
    MSStringLinkage* outputStrLink = static_cast<MSStringLinkage *>(_alloca (msStringLinkageBufSize));

    //  Convert inputStrData to file format
    int strFileDataLength; // <= actual size of file-format data
    mdlCnv_bufferToFileFormatWithRules ((byte*)&outputStrLink->data, &strFileDataLength, linkage_getStringConversionRules(), (byte*)inputStrData, (int)msStringLinkageDataBufSize);

    //  Set up linkage header
    memset (&outputStrLink->header, 0, sizeof(outputStrLink)->header);
    outputStrLink->header.primaryID          = LINKAGEID_String;
    outputStrLink->header.user               = true;
    int linkageLength = (sizeof (LinkageHeader) + strFileDataLength + 7) & ~7;
    LinkageUtil::SetWords (&outputStrLink->header, linkageLength/2);

    linkage.assign ((byte*)outputStrLink, (byte*)outputStrLink + msStringLinkageBufSize);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/2010
+---------------+---------------+---------------+---------------+---------------+------*/
WString LinkageUtil::GetStringFromStringLinkage (MSStringLinkageData const& data)
    {
    // NB: We know that string linkages always encode their wide chars in the Unicode locale.

    WString uniString;
    VariCharConverter::VariCharToUnicode (uniString, data.linkageString, data.linkageStringLength, LangCodePage::Unicode);

    return uniString;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/01
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt     appendStringLinkage
(
DgnElement*                  elementIn,              /* => input element to append string linkage to */
MSStringLinkage*            strLinkP,               /* => input linkage buffer */
UShort                      linkageKey,             /* => input linkage key */
bvector<VariChar> const&    variBuffer              /* => input linkage string */
)
    {
    size_t stringBytes = (variBuffer.size () * sizeof (VariChar));
    
    memcpy (strLinkP->data.linkageString, &variBuffer[0], stringBytes);

    /* Set up stringLinkage.header */
    strLinkP->header.primaryID          = LINKAGEID_String;
    strLinkP->header.user               = true;

    /* Set up stringLinkage.data */
    strLinkP->data.linkageKey           = linkageKey;
    strLinkP->data.mustBeZero           = 0;     // Must be zero!
    strLinkP->data.linkageStringLength  = static_cast<UInt32>(stringBytes);

    return linkage_appendToElement (elementIn, &strLinkP->header, &strLinkP->data, linkage_getStringConversionRules ());
    }

#ifdef UNUSED_FUNCTION
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/01
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus     appendStringLinkage
(
DgnElement*          elementIn,              /* => input element to append string linkage to */
MSStringLinkage*    strLinkP,               /* => input linkage buffer */
UShort              linkageKey,             /* => input linkage key */
WCharCP             linkageStringIn         /* => input linkage string */
)
    {
    // Based on analyzing the old code, these do NOT write NULL terminators.
    bvector<VariChar> variBuffer;
    VariCharConverter::UnicodeToVariChar (variBuffer, linkageStringIn, LangCodePage::Unicode, false);

    return appendStringLinkage (elementIn, strLinkP, linkageKey, variBuffer);
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Deepak.Malkan   07/00
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt      LinkageUtil::AppendStringLinkage
(
DgnElementP      elementIn,              /* => input element to append string linkage to */
UShort          linkageKey,             /* => input linkage key */
WCharCP         linkageStringIn         /* => input linkage string */
)
    {
    if (NULL == elementIn || NULL == linkageStringIn)
        return ERROR;

    /* Figure out how large a buffer we are going to need */
    // Based on analyzing the old code, these do NOT write NULL terminators.
    bvector<VariChar> variBuffer;
    VariCharConverter::UnicodeToVariChar (variBuffer, linkageStringIn, LangCodePage::Unicode, false);

    size_t  stringBytes = variBuffer.size ();
    size_t  bufSize     = (sizeof (MSStringLinkage) + stringBytes + 7) & ~7;

    MSStringLinkage*    strLinkP = NULL;
    if (stringBytes > 0 &&
        NULL != (strLinkP = static_cast<MSStringLinkage *>(_alloca (bufSize))))
        {
        memset (strLinkP, 0, bufSize);

        return appendStringLinkage (elementIn, strLinkP, linkageKey, variBuffer);
        }

    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Deepak.Malkan   07/00
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   LinkageUtil::SetStringLinkage
(
DgnElementP      elementIn,              /* => input element to append string linkage to */
UShort          linkageKey,             /* => input linkage key */
WCharCP         linkageStringIn         /* => input linkage string */
)
    {
    if (NULL == elementIn || NULL == linkageStringIn)
        return ERROR;

    /* Figure out how large a buffer we are going to need */
    // Based on analyzing the old code, these do NOT write NULL terminators.
    bvector<VariChar> variBuffer;
    VariCharConverter::UnicodeToVariChar (variBuffer, linkageStringIn, LangCodePage::Unicode, false);

    size_t  stringBytes = variBuffer.size ();
    size_t  bufSize     = (sizeof (MSStringLinkage) + stringBytes + 7) & ~7;

    if (elementIn->GetSizeWords() > elementIn->GetAttributeOffset())
        {
        size_t attrBytes = (sizeof (MSStringLinkage) + (elementIn->GetSizeWords() - elementIn->GetAttributeOffset()) * 2 + 7) & ~7;

        /* Use larger of two, guarentees that buffer is large enough to extract largest string linkage */
        bufSize = attrBytes > bufSize ? attrBytes : bufSize;
        }

    MSStringLinkage*    strLinkP = NULL;
    if (stringBytes > 0 &&
        NULL != (strLinkP = static_cast<MSStringLinkage *>(_alloca (bufSize))))
        {
        int         iLinkage = 0;
        UShort      *pLinkage = NULL;

        memset (strLinkP, 0, bufSize);

        for (iLinkage = 0; NULL != (pLinkage = (UShort *) linkage_extractLinkageByIndex (strLinkP, elementIn, LINKAGEID_String, &s_compiledStringLinkageConvRules, iLinkage)); iLinkage++)
            {
            if (strLinkP->data.linkageKey == linkageKey)
                {
                /* check if we can replace or need to delete/add */
                if (strLinkP->data.linkageStringLength == (UInt32) stringBytes)
                    {
                    memcpy (strLinkP->data.linkageString, &variBuffer[0], stringBytes);

                    mdlLinkage_initializeStringConversionRules ();

                    int linkageSizeInBytes = static_cast<int>(2* (LinkageUtil::GetWords((LinkageHeader*)pLinkage) - 1));

                    return mdlCnv_bufferToFileFormatWithRules ((byte *) (pLinkage + 2), NULL, &s_compiledStringLinkageConvRules,
                                                      (byte *) &strLinkP->data, linkageSizeInBytes);
                    }
                else
                    {
                    if (1 != linkage_deleteLinkageByIndex (elementIn, LINKAGEID_String, iLinkage))
                        return ERROR;
                    }

                break;
                }

            /* initialize "stringLinkage" before next call to extract */
            memset (strLinkP, 0, bufSize);
            }

        /* If the linkage is not found or was different size add new one */
        return appendStringLinkage (elementIn, strLinkP, linkageKey, variBuffer);
        }

    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/00
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt     addStringLinkageToDescr
(
MSElementDescrP    elemDscrIn,             /* <=> input-output element descriptor to set linkage-string to */
UShort              linkageKey,             /*  => input linkage key */
WCharCP             linkageStringIn,        /*  => input linkage string */
bool                overrideExisting        /*  => replace existing */
)
    {
    StatusInt         status = ERROR;
    if (NULL != elemDscrIn &&
        NULL != linkageStringIn && wcslen (linkageStringIn) > 0)
        {
        size_t      stringBytes = 0, bufSize = 0;

        // Based on analyzing the old code, these do NOT write NULL terminators.
        bvector<VariChar> variBuffer;
        VariCharConverter::UnicodeToVariChar (variBuffer, linkageStringIn, LangCodePage::Unicode, false);

        stringBytes = variBuffer.size ();
        bufSize     = LinkageUtil::CalculateSize ((sizeof (MSStringLinkage) + stringBytes + 7) & ~7);
        size_t elemSize = elemDscrIn->Element().Size ();

        elemDscrIn->ReserveMemory(elemSize + bufSize);
        /* Allocate element buffer large enough to hold new linkage */
        if (overrideExisting)
            status = LinkageUtil::SetStringLinkage(&elemDscrIn->ElementR(), linkageKey, linkageStringIn);
        else
            status = LinkageUtil::AppendStringLinkage(&elemDscrIn->ElementR(), linkageKey, linkageStringIn);
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/00
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt      LinkageUtil::SetStringLinkageUsingDescr
(
MSElementDescrP     elemDscrIn,             /* <=> input-output element descriptor to set linkage-string to */
UShort              linkageKey,             /*  => input linkage key */
WCharCP             linkageStringIn         /*  => input linkage string */
)
    {
    return addStringLinkageToDescr (elemDscrIn, linkageKey, linkageStringIn, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/00
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt      LinkageUtil::AppendStringLinkageUsingDescr
(
MSElementDescrP     elemDscrIn,             /* <=> input-output element descriptor to set linkage-string to */
UShort              linkageKey,             /*  => input linkage key */
WCharCP             linkageStringIn         /*  => input linkage string */
)
    {
    return addStringLinkageToDescr (elemDscrIn, linkageKey, linkageStringIn, false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Deepak.Malkan   07/00
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       LinkageUtil::ExtractStringLinkageByIndex (UShort* linkageKey, WCharP linkageString, int bufferSize, int linkageIndex, DgnElementCP elm)
    {
    int                 status = DGNHANDLERS_STATUS_LinkageNotFound, bufSize = 0;
    MSStringLinkage*    strLinkP = NULL;

    BeAssert (NULL != elm);
    if (NULL == elm)
        return DGNHANDLERS_STATUS_LinkageNotFound;

    /*  No linkages on this element */
    if (elm->GetSizeWords() <= elm->GetAttributeOffset())
        return DGNHANDLERS_STATUS_LinkageNotFound;

    /* buffer guarenteed to be big enough to extract largest string linkage */
    bufSize = (sizeof (MSStringLinkage) + (elm->GetSizeWords() - elm->GetAttributeOffset()) * 2 + 7) & ~7;

    if (NULL != (strLinkP = static_cast<MSStringLinkage *>(_alloca (bufSize))))
        {
        memset (strLinkP, 0, bufSize);

        if (NULL != linkage_extractLinkageByIndex (strLinkP, elm, LINKAGEID_String, linkage_getStringConversionRules (), linkageIndex))
            {
            status = SUCCESS;

            if (linkageKey)
                *linkageKey = strLinkP->data.linkageKey;

            if (linkageString)
                {
                WString uniString;
                VariCharConverter::VariCharToUnicode (uniString, strLinkP->data.linkageString, strLinkP->data.linkageStringLength, LangCodePage::Unicode);

                // Copy what we can... old behavior.
                BeStringUtilities::Wcsncpy (linkageString, bufferSize, uniString.c_str ());

                if ((uniString.size () + 1) > (size_t)bufferSize)
                    status = DGNHANDLERS_STATUS_ElementTooLarge;
                }
            }
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/08
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       LinkageUtil::ExtractNamedStringLinkageByIndex (WCharP linkageStringOut, int bufferSize, UShort linkageKey, int linkageIndex, DgnElementCP elm)
    {
    WString         wString;
    StatusInt       status;

    if (SUCCESS == (status = ExtractNamedStringLinkageByIndex (wString, linkageKey, linkageIndex, elm)))
        {
        if (NULL != linkageStringOut)
            {
            // Copy what we can... old behavior.
            BeStringUtilities::Wcsncpy (linkageStringOut, bufferSize, wString.c_str ());

            if ((wString.size () + 1) > (size_t)bufferSize)
            return DGNHANDLERS_STATUS_ElementTooLarge;
            }
        }
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/08
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       LinkageUtil::ExtractNamedStringLinkageByIndex (WStringR linkageWStringOut, UShort linkageKey, int linkageIndex, DgnElementCP elm)
    {
    BeAssert (NULL != elm);
    if (linkageIndex < 0 || NULL == elm)
        return DGNHANDLERS_STATUS_LinkageNotFound;

    /* No element or no linkages on this element */
    UInt32      attrOffset;
    if (elm->GetSizeWords() <= (attrOffset = elm->GetAttributeOffset()))
        return DGNHANDLERS_STATUS_LinkageNotFound;

    /* buffer guarenteed to be big enough to extract largest string linkage */
    int                 strLinkBufSize = (sizeof (MSStringLinkage) + (elm->GetSizeWords() - elm->GetAttributeOffset()) * 2 + 7) & ~7;
    MSStringLinkage*    strLinkP = static_cast<MSStringLinkage *>(_alloca (strLinkBufSize));

    int                 iMatchingKeyIndex = 0;

    union
        {
        const UShort            *pw;
        const LinkageHeader     *pLH;
        const MSStringLinkage   *pSL;
        } u;

    /* Get word pointers to start of attribute data and end of element */
    u.pw    = (UShort *) elm;
    UShort const *pwEnd = u.pw + elm->GetSizeWords();
    u.pw   += attrOffset;

    /* Scan through attribute data - return when "reqID" is found and index is nth */
    while (u.pw < pwEnd)
        {
        UShort  linkWords;
        UShort  linkID = 0;

        /*---------------------------------------------------------------
        Get the link size and primary ID. If the "user" bit is not set
        assume its a DMRS (4 word) linkage.
        ---------------------------------------------------------------*/
        linkWords = LinkageUtil::GetWords (u.pLH);

        if (0 == linkWords)         // Avoid infinite loop.
            return DGNHANDLERS_STATUS_LinkageNotFound;

        if (u.pLH->user)
            linkID = u.pLH->primaryID;

        if (linkID == LINKAGEID_String && u.pSL->data.linkageKey == linkageKey)
            {
            UShort* target = reinterpret_cast <UShort*>(strLinkP);
            target [linkWords] = 0; //  Make sure it is NULL terminated.

            memcpy (strLinkP, u.pw, 2 * linkWords);

            if (iMatchingKeyIndex == linkageIndex)
                {
                VariCharConverter::VariCharToUnicode (linkageWStringOut, strLinkP->data.linkageString, strLinkP->data.linkageStringLength, LangCodePage::Unicode);

                return SUCCESS;
                }
            // found one, but not the one we're looking for.
            iMatchingKeyIndex++;
            }

        u.pw += linkWords;
        }

    return DGNHANDLERS_STATUS_LinkageNotFound;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    DeepakMalkan                    07/00
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus      LinkageUtil::DeleteStringLinkage
(
DgnElementP      elementIn,                  /* => input element */
UShort          linkageKeyIn,               /* => input linkage key */
int             linkageIndexIn              /* => input index */
)
    {
    int         linkageIndex = 0;
    UShort      linkageKey = 0;
    for (int iLinkage = 0; SUCCESS == LinkageUtil::ExtractStringLinkageByIndex (&linkageKey, NULL, 0, iLinkage, elementIn); iLinkage++)
        {
        if (linkageKey == linkageKeyIn)
            {
            if (linkageIndex == linkageIndexIn &&
                1 == linkage_deleteLinkageByIndex (elementIn, LINKAGEID_String, iLinkage))
                return SUCCESS;

            linkageIndex++;
            }
        }

    return ERROR;
    }

