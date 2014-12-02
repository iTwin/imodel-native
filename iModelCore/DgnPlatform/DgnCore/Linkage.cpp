/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/Linkage.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <RmgrTools/RscMgr/bnryport.h>
#include <DgnPlatform/DgnCore/Linkage1.h>

// MSStringLinkageData is {Int16,pad,Int32,char[...]} since it is written to file in "unpacked" format.
// Our native->file format "conversion" logic for MSStringLinkageData is: write Int16, skip 2 bytes, write Int32, ....
// That is, the file data contains padding.
// 3rd party (mistakenly) reads MSStringLinkageData as {Int32,Int32,char[...]}. 
// Therefore, it will get garbage or cccc in the high word of the first field unless we zero out the
// buffer before we apply the native->file format conversion logic to it.
// While we could (and did) fix the MSStringLinkageData struct definition to include a pad field, there may
// be other structs with the same problem. It's safer to do the memset.
#define CLEAR_FILE_BUFFER_BEFORE_CONVERT 

static UInt8 s_multiStateMaskLinkageConvRulesCompiled[] = 
    {
    0x70,0x6f,0x76,0x63,0x84,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x4,0x0,0x0,0x0,0x4,0x0,0x0,0x0,
    0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x5,0x0,0x0,0x0,0x15,0x0,0x0,0x0,0x4,0x0,0x0,0x0,
    0x4,0x0,0x0,0x0,0xd,0xd,0xd,0xd,0xd,0xd,0xd,0xd,0x2,0x0,0x0,0x0,0x4,0x0,0x0,0x0,
    0x4,0x0,0x0,0x0,0x4,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x5,0x0,0x0,0x0,0x4,0x0,0x0,0x0,
    0x4,0x0,0x0,0x0,0x2,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x2,0x0,0x0,0x0,0x2,0x0,0x0,0x0,
    0x2,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x16,0x0,0x0,0x0,0x4,0x0,0x0,0x0,
    0x4,0x0,0x0,0x0,0xd,0xd,0xd,0xd,0xd,0xd,0xd,0xd
    };

static void *s_multiStateMaskLinkageConvRules = s_multiStateMaskLinkageConvRulesCompiled;
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  10/00
+---------------+---------------+---------------+---------------+---------------+------*/
int LinkageUtil::SetWords (LinkageHeader *linkHdrP, int wordLength)
    {
    if (linkHdrP->user && (wordLength > 0))
        {
        int         exponent=0, mantissa=wordLength;

        /* if the mantissa is exactly 256, we still want to store that as a V7 compatible linkage */
        if (mantissa > 256)
            {
            while (mantissa > 255)
                {
                exponent++;
                mantissa = (mantissa + 1)/2;
                }
            }

        if (0 == exponent) /* wdMantissa is still wtf when wdExponent is zero */
            {
            linkHdrP->wdMantissa = wordLength-1;
            linkHdrP->remote     = false;
            }
        else
            {
            int multiplier = 1 << exponent;
            mantissa = (wordLength + multiplier - 1) / multiplier;

            BeAssert (mantissa < 256);

            linkHdrP->wdMantissa = mantissa;
            linkHdrP->wdExponent = exponent;
            linkHdrP->remote     = true;
            }

        return SUCCESS;
        }

    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  10/00
+---------------+---------------+---------------+---------------+---------------+------*/
UInt16 LinkageUtil::GetWords (LinkageHeader const* linkHdrP)
    {
    if (linkHdrP->user)
        {
        UInt16  numWordsInLinkage;
        if (linkHdrP->remote)
            // for Run-Time Check Failure #1 - A cast to a smaller data type has caused a lost of data, mask the source of the cast with bitmask
            numWordsInLinkage = (linkHdrP->wdMantissa * (1 << linkHdrP->wdExponent)) & 0xFFFF;
        else
            numWordsInLinkage = linkHdrP->wdMantissa+1;
        
        return numWordsInLinkage;
        }

    /* DMRS linkage is fixed size. */
    return 4;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Deepak.Malkan   12/02
+---------------+---------------+---------------+---------------+---------------+------*/
int LinkageUtil::CalculateSize (int bufferSizeIn)
    {
    LinkageHeader   linkageHeader;
    int             linkageWords;

    linkageWords = (bufferSizeIn + 1) / 2;

    linkageHeader.user = 1;
    LinkageUtil::SetWords (&linkageHeader, linkageWords);
    linkageWords = LinkageUtil::GetWords (&linkageHeader);

    return linkageWords * 2;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      01/2006
+---------------+---------------+---------------+---------------+---------------+------*/
void* BentleyApi::linkage_padToLinkageSize (void const* data, UInt32 nActualBytesData)
    {
    UInt32 nLinkageBytesTotal = LinkageUtil::CalculateSize (nActualBytesData + sizeof(LinkageHeader));
    UInt32 nLinkageBytesData  = nLinkageBytesTotal - sizeof(LinkageHeader);
    byte* cc = (byte*) malloc (nLinkageBytesData);
    BeAssert (nLinkageBytesData >= nActualBytesData);
    memcpy (cc, data, nActualBytesData);
    memset (cc+nActualBytesData, 0, nLinkageBytesData-nActualBytesData);
    return cc;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Bill.Dehorty    02/92
+---------------+---------------+---------------+---------------+---------------+------*/
static int     convertLinkageDataFromFile
(
void            *internalP,     /* <=  converted linkage data */
void            *convRulesP,    /*  => Conversion rules. */
const void      *dataP,         /*  => linkage data to be converted */
int             maxLinkageBytes /*  => Size of buffer that dataP points to */
)
    {
    bool        eightByteAligned;
    int         status;
    int         actualSize;

    if ((status = mdlCnv_bufferFromFileFormatWithRules ((byte*) internalP, &eightByteAligned, &actualSize, convRulesP, (byte*) dataP, maxLinkageBytes)) == SUCCESS)
        {
        /* If double alignment is needed, pad 4 bytes after linkhdr. */
        if (eightByteAligned)
            {
#if !defined (DOUBLE_ALIGN4_1ST_ALIGN8)
            memmove (((byte *)internalP)+4, internalP, actualSize);
#endif
            }
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Bill.Dehorty    05/94
+---------------+---------------+---------------+---------------+---------------+------*/
static int     copyLinkageToWorkArea
(
UShort          *linkBufP,      /* <= converted linkage */
void            *convRulesP,    /*  => Conversion rules.     */
const UShort    *rawLinkP,      /*  => data to convert */
int             linkSize,       /*  => size of linkage */
UShort          *linkID         /*  => linkage id (DMRS or user) */
)
    {
    int         status;
    int         convDesired = (convRulesP != NULL);

    if (convDesired)
        {
        /* Copy raw linkage header. */
        memcpy (linkBufP, rawLinkP, sizeof(LinkageHeader));
        /* Convert data part of linkage. */
        if ((status = convertLinkageDataFromFile (linkBufP+2, convRulesP,
                                                  rawLinkP+2, linkSize - sizeof (LinkageHeader)))
            != SUCCESS)
            return status;
        }
    else
        /* Copy full raw linkage */
        memcpy (linkBufP, rawLinkP, linkSize);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BSI             08/91
+---------------+---------------+---------------+---------------+---------------+------*/
Public void     *BentleyApi::linkage_extractFromElement
(                               /* <= Returns address of raw link or NULL */
void*           inLinkBuf,      /* <= converted linkage                */
DgnElementCP     elm,            /*  => Input element                   */
int             reqID,          /*  => Requested link ID (0 = DMRS)    */
void*           convRulesP,     /*  => Conversion rules.               */
LinkageFunc     linkFunc,       /*  => Func to process links  (NULL)   */
CallbackArgP    paramsP         /*  => User params to linkFunc()       */
)
    {
    LinkageHeader tmpHdr;
    UShort      *pw, *pwEnd;
    UShort      linkWords, linkID;
    void        *lastLink = NULL;
    UInt32      attrOffset;

    /* Get attribute index ... make sure its valid. */
    attrOffset = elm->GetAttributeOffset();
    if (attrOffset > elm->GetSizeWords())
        return NULL;

    /* Get word pointers to start of attribute data and end of element */
    pw    = (UShort *) elm;
    pwEnd = pw + elm->GetSizeWords();
    pw   += attrOffset;

    /* Scan through attribute data - return when "reqID" is found */
    while (pw < pwEnd)
        {
        memcpy (&tmpHdr, pw, sizeof(LinkageHeader));
        /*---------------------------------------------------------------
        Get the link size and primary ID. If the "user" bit is not set
        assume its a DMRS (4 word) linkage.
        ---------------------------------------------------------------*/
        linkID    = 0;
        linkWords = LinkageUtil::GetWords (&tmpHdr);

        if (0 == linkWords)         // Avoid infinite loop.
            return NULL;

        if (tmpHdr.user)
            {
            linkID = tmpHdr.primaryID;
            }

        if (linkID == reqID)
            {
            UShort      *pLinkBuf = (UShort*)inLinkBuf;

            if (pLinkBuf == NULL)
                pLinkBuf = (UShort*) alloca (sizeof(LinkageHeader) + linkWords*2 + 256);


            if (SUCCESS != copyLinkageToWorkArea (pLinkBuf, convRulesP, pw, linkWords*2, &linkID))
                return NULL;

            /*-----------------------------------------------------------
            If a "linkFunc" was passed in, call it and keep going unless
            "linkFunc" returns a non-zero value (processing multiple
            linkages of the same type). If "linkFunc" is null, return
            with the address of the current linkage.
            -----------------------------------------------------------*/
            if (linkFunc)
                {
                if ((*linkFunc)((LinkageHeader *) pLinkBuf, paramsP) != SUCCESS)
                    return pw;
                else
                    lastLink = (void*)pw;
                }
            else
                return pw;
            }

        pw += linkWords;
        }

    /*-------------------------------------------------------------------
    If a "linkFunc" was passed in, "lastLink" will be the address of
    the last link processed. Otherwise it will be NULL - no link found.
    -------------------------------------------------------------------*/
    return lastLink;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BSI             08/91
+---------------+---------------+---------------+---------------+---------------+------*/
Public int      BentleyApi::linkage_deleteFromElement                
(                               /* <= Returns num linkages deleted. */
DgnElementP      elm,            /*  => Input element                   */
int             reqID,          /*  => Requested link ID (0 = DMRS)    */
void*           convRulesP,     /* <=> data conversion rules           */
LinkageFunc     linkFunc,       /*  => Func to filter links, NULL = ALL */
CallbackArgP    paramsP         /*  => User params to linkFunc()       */
)
    {
    LinkageHeader tmpHdr;
    UShort      *linkStart, *linkEnd;
    UShort      *pw, *pwEnd;
    UShort      linkID;
    UInt32      attrOffset;
    int         numDeleted = 0, linkWords;

    /* Get attribute index ... make sure it's valid. */
    attrOffset = elm->GetAttributeOffset();
    if (attrOffset > elm->GetSizeWords())
        return (0);

    /* Get word pointers to start of attribute data and end of element */
    pw    = (UShort *) elm;
    pwEnd = pw + elm->GetSizeWords();
    pw   += attrOffset;

    /* Scan through attribute data */
    while (pw < pwEnd)
        {
        memcpy (&tmpHdr, pw, sizeof(tmpHdr));

        /*---------------------------------------------------------------
        Get the link size and primary ID. If the "user" bit is not set
        assume its a DMRS (4 word) linkage.
        ---------------------------------------------------------------*/
        linkID    = 0;
        if (0 == (linkWords = LinkageUtil::GetWords (&tmpHdr)))
            break;          // Avoid infinite loop.

        if (tmpHdr.user)
            {
            linkID = tmpHdr.primaryID;
            }

        if (linkID == reqID)
            {
            UShort      *pLinkBuf;

            pLinkBuf = (UShort*) alloca (sizeof(LinkageHeader) + linkWords*2 + 256);

            if (copyLinkageToWorkArea (pLinkBuf, convRulesP, pw, linkWords*2, &linkID)
                != SUCCESS)
                return numDeleted;

            /*-----------------------------------------------------------
            If a "linkFunc" was passed in, call it.
            If the linkFunc returns "true", delete this linkage and
            continue processing others.

            If a "linkFunc" was not passed in, delete this linkage and
            continue processing others.
            -----------------------------------------------------------*/
            if ((linkFunc == NULL) || (*linkFunc)((LinkageHeader *) pLinkBuf, paramsP))
                {
                linkStart = pw;
                linkEnd   = linkStart + linkWords;
                if (linkEnd > pwEnd)
                    {
                    linkEnd = pwEnd;
                    linkWords = static_cast<int>(linkEnd - linkStart);
                    }
                memmove (linkStart, linkEnd, (pwEnd - linkEnd) * 2);
                elm->SetSizeWords(elm->GetSizeWords() - linkWords);
                pwEnd -= linkWords;
                numDeleted++;
                continue;
                }
            }
        pw += linkWords;
        }

    return numDeleted;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BSI             08/91
+---------------+---------------+---------------+---------------+---------------+------*/
Public StatusInt      BentleyApi::linkage_appendToElement
(
DgnElement       *elmP,          /* <= Element to have data appended. */
LinkageHeader   *linkHdrP,      /* <=> Linkage hdr to append (words set). */
void const      *dataP,         /*  => Data to append. */
void            *convRulesP    /*   => data conversion rules           */
)
    {
    int         paddingSize, dataSize;
    byte        *fileDataP;
    UShort      *pwEnd;
    UInt32      attrOffset;
    int         byteLength, wordLength, status;

    if ((dataP == NULL) || (linkHdrP == NULL) || (elmP == NULL))
        return DGNHANDLERS_STATUS_BadArg;

    /* If conversion rules were supplied, convert data and calculate length. */
    if (convRulesP != NULL)
        {
        DgnV8ElementBlank fileFormatData;
        fileDataP = (byte *)&fileFormatData;

#if defined (CLEAR_FILE_BUFFER_BEFORE_CONVERT)
        // This is a workaround for an apparent bug in the buffer to file format and can be removed
        // when this is fixed  - RBB 04/01.
        memset (fileDataP, 0, 48);
#endif
        if ((status = mdlCnv_bufferToFileFormatWithRules (fileDataP, &byteLength, convRulesP, (byte*) dataP, sizeof(fileFormatData))) != SUCCESS)
            return status;

        /* Pad buffer so round-off to 4 word multiple won't truncate good data */
        memset (fileDataP + byteLength, '\0', 7);
        byteLength += 7;

        /*-------------------------------------------------------------------
        Generate linkage size (in words) padded to 8-byte boundary.
        -------------------------------------------------------------------*/
        byteLength += sizeof (LinkageHeader);
        wordLength = ((byteLength - (byteLength % 8)) / 2);

        /* Fill wtf if not DMRS linkage. */
        if (linkHdrP->user)
            LinkageUtil::SetWords (linkHdrP, wordLength);
        }
    else
        {
        /*---------------------------------------------------------------
           No published structure was supplied. Caller converted data and
            calculated wtf for us.
        ---------------------------------------------------------------*/
        wordLength = LinkageUtil::GetWords (linkHdrP);
        fileDataP  = (byte*) dataP;
        }

    /* Get attribute index ... make sure its valid. */
    attrOffset = elmP->GetAttributeOffset();
    if (attrOffset > elmP->GetSizeWords())
        return (ERROR);

    /* Get word pointer to end of element */
    pwEnd = (UShort*)elmP + elmP->GetSizeWords();

    /* Copy the linkage header. */
    memcpy (pwEnd, linkHdrP, sizeof(LinkageHeader));

    /* Now copy the file-format data. */
    memcpy (pwEnd+2, fileDataP, dataSize = (wordLength - 2) * 2);

    // It is possible that due to the way we calculate linkage sizes the
    // linkage size has been increased beyond the requested size.  In this
    // case, we need to increase the word length appropriately and clear
    // the extra space in the element.  This fix added 11/2001 (08.00.10.06)
    // and invalid large linkages may have been created prior to this.

    if ((paddingSize = LinkageUtil::GetWords (linkHdrP) - wordLength) > 0)
        {
        memset (pwEnd + 2 + (dataSize / 2), 0, 2*paddingSize);
        wordLength += paddingSize;
        }

    // Note - Use LinkageUtil::GetWords to increment rather than wordLength as this may be larger.
    elmP->SetSizeWords(elmP->GetSizeWords() + wordLength);

    return (SUCCESS);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/2009
+---------------+---------------+---------------+---------------+---------------+------*/
static void copyLinkage (void* slot, LinkageHeader const& newHdr, void const* newData, size_t newSize)
    {
    memcpy (       slot,               &newHdr, sizeof(newHdr));
    memcpy ((byte*)slot+sizeof(newHdr), newData, newSize - sizeof(newHdr));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/2009
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus insertNewLinkage (MSElementDescrR dscr, size_t linkageOffset, size_t oldBytesToRemove, size_t newLinkageSize, 
                LinkageHeader const& newLinkageHeader, void const* newLinkageData)
    {
    BeAssert ((newLinkageSize   & 3) == 0);       // linkage sizes are always a multiple of 4
    BeAssert ((oldBytesToRemove & 3) == 0);

    size_t oldElmSize  = dscr.Element().Size();
    size_t newElmSize  = oldElmSize + (newLinkageSize - oldBytesToRemove);
    dscr.ReserveMemory(newElmSize);

    byte* thisElm = (byte*) &dscr.ElementR();

    byte* startSave = thisElm + linkageOffset + oldBytesToRemove;
    byte* endSave = thisElm + oldElmSize;

    // note: this is an overlaping memcpy - that's expected and supported.
    if (startSave < endSave)
        memmove (thisElm + linkageOffset + newLinkageSize, startSave, endSave-startSave);

    copyLinkage(thisElm + linkageOffset, newLinkageHeader, newLinkageData, newLinkageSize);

    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/2009
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus MSElementDescr::InsertLinkage (size_t linkageOffset, LinkageHeader const& newLinkageHeader, void const* newLinkageData)
    {
    return insertNewLinkage (*this, linkageOffset, 0, 2*LinkageUtil::GetWords(&newLinkageHeader), newLinkageHeader, newLinkageData);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/2009
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus MSElementDescr::AppendLinkage (LinkageHeader const& newLinkageHeader, void const* newLinkageData)
    {
    return insertNewLinkage (*this, Element().Size(), 0, 2*LinkageUtil::GetWords (&newLinkageHeader), newLinkageHeader, newLinkageData);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/2009
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus MSElementDescr::ReplaceLinkage (LinkageHeader** oldLinkageHeaderH, LinkageHeader const& newLinkageHeader, void const* newLinkageData)
    {
    if (NULL == oldLinkageHeaderH || NULL == *oldLinkageHeaderH)
        {BeAssert (false); return BSIERROR;}

    LinkageHeaderP   oldLinkageHeader = *oldLinkageHeaderH;

    size_t linkageOffset = (byte*)oldLinkageHeader - (byte*)&Element();
    if (linkageOffset/2 < Element().GetAttributeOffset() || linkageOffset/2 >= Element().GetSizeWords())
        {
        BeAssert (false && "oldLinkageHeader is not in the supplied element");
        return BSIERROR;
        }

    UInt32 oldWords = LinkageUtil::GetWords(oldLinkageHeader);
    UInt32 newWords = LinkageUtil::GetWords(&newLinkageHeader);

    if (newWords == oldWords)
        {
        copyLinkage ((void*)oldLinkageHeader, newLinkageHeader, newLinkageData, 2*newWords);
        return BSISUCCESS;
        }
        
    if (insertNewLinkage (*this, linkageOffset, 2*oldWords, 2*newWords, newLinkageHeader, newLinkageData) != BSISUCCESS)
        return BSIERROR;

    *oldLinkageHeaderH = (LinkageHeader*)((byte*)&Element() + linkageOffset);
    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Bhupinder.Singh 08/94
+---------------+---------------+---------------+---------------+---------------+------*/
void* BentleyApi::linkage_extractLinkageByIndex
(
void            *inLinkBuf,     /* <= converted linkage                */
DgnElementCP     elmP,          /*  => Input element                   */
int             reqID,          /*  => Requested link ID (0 = DMRS)    */
void            *convRulesP,    /* <=> Conversion rules.               */
int             nth             /*  => index of linkage reqd           */
)
    {
    const UShort *pwEnd;
    int         attCount = 0;
    UInt32      attrOffset;
    union
        {
        const UShort            *pw;
        const LinkageHeader     *pLH;
        } u;

    /* Get attribute index ... make sure its valid. */
    if (elmP->GetSizeWords() <= (attrOffset = elmP->GetAttributeOffset()) )
        return NULL;

    /* Get word pointers to start of attribute data and end of element */
    u.pw    = (UShort *) elmP;
    pwEnd = u.pw + elmP->GetSizeWords();
    u.pw   += attrOffset;

    attCount = 0;
    /* Scan through attribute data - return when "reqID" is found and index is nth */
    while (u.pw < pwEnd)
        {
        UShort  linkWords;
        UShort  linkID;
        /*---------------------------------------------------------------
        Get the link size and primary ID. If the "user" bit is not set
        assume its a DMRS (4 word) linkage.
        ---------------------------------------------------------------*/
        linkWords = LinkageUtil::GetWords (u.pLH);
        if (u.pLH->user)
            linkID = u.pLH->primaryID;
        else
            linkID = 0;

        if (0 == linkWords)         // Avoid infinite loop.
            return NULL;

        if (linkID == reqID)
            {
            if (attCount++ == nth)
                {
                if (NULL != inLinkBuf)
                    {
                    UShort      *pLinkBuf;

                    pLinkBuf =  (UShort*)inLinkBuf;

                    return (SUCCESS == copyLinkageToWorkArea (pLinkBuf, convRulesP, u.pw, linkWords*2, &linkID) ? (void *) u.pw : NULL );
                    }
                else
                    return (void *) u.pw;
                }
            }

        u.pw += linkWords;
        }

    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Bill.Dehorty    06/95
+---------------+---------------+---------------+---------------+---------------+------*/
int   BentleyApi::linkage_deleteLinkageByIndex /* <= num linkages deleted */
(
DgnElement       *elmP, /*  => Input element                         */
int             reqID, /*  => Requested link ID (0 = DMRS)          */
int             nth    /*  => index of linkage to delete (-1 = all) */
)
    {
    LinkageHeader tmpHdr;
    UShort      *pw, *pwEnd;
    UShort      linkWords, linkID;
    UInt32      attrOffset;
    int         attCount, numDeleted = 0;

    /* Get attribute index ... make sure its valid. */
    attrOffset = elmP->GetAttributeOffset();
    if (attrOffset > elmP->GetSizeWords())
        return 0;

    /* Get word pointers to start of attribute data and end of element */
    pw    = (UShort *) elmP;
    pwEnd = pw + elmP->GetSizeWords();
    pw   += attrOffset;

    attCount = 0;
    /* Scan through attribute data, delete appropriate linkage(s) */
    while (pw < pwEnd)
        {
        memcpy (&tmpHdr, pw, sizeof(LinkageHeader));
        /*---------------------------------------------------------------
        Get the link size and primary ID. If the "user" bit is not set
        assume its a DMRS (4 word) linkage.
        ---------------------------------------------------------------*/
        linkID    = 0;
        linkWords = LinkageUtil::GetWords (&tmpHdr);

        if (tmpHdr.user)
            {
            linkID = tmpHdr.primaryID;
            }

        if (linkID == reqID)
            {
            UShort  *linkStart, *linkEnd;

            if ((attCount++ == nth) || (nth == -1))
                {
                linkStart = pw;
                linkEnd   = linkStart + linkWords;
                memmove (linkStart, linkEnd, (pwEnd - linkEnd) * 2);
                elmP->SetSizeWords(elmP->GetSizeWords() - linkWords);
                pwEnd -= linkWords;
                numDeleted++;
                if (nth > -1)
                    return 1;
                else
                    numDeleted++;

                continue;
                }
            }

        pw += linkWords;
        }

    return numDeleted;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Bill.Dehorty    06/95
+---------------+---------------+---------------+---------------+---------------+------*/
int   BentleyApi::mdlLinkage_numLinkages /* <= Num linkages matching reqId */
(
DgnElement       *elmP,          /*  => Input element                   */
int             reqID           /*  => Requested link ID (0 = DMRS)    */
)
    {
    LinkageHeader tmpHdr;
    UShort      *pw, *pwEnd;
    UShort      linkWords, linkID;
    UInt32      attrOffset;
    int         attCount;

    if (elmP == NULL)
        return 0;

    /* Get attribute index ... make sure its valid. */
    attrOffset = elmP->GetAttributeOffset();
    if (attrOffset > elmP->GetSizeWords())
        return 0;

    /* Get word pointers to start of attribute data and end of element */
    pw    = (UShort *) elmP;
    pwEnd = pw + elmP->GetSizeWords();
    pw   += attrOffset;

    attCount = 0;
    /* Scan through attribute data - count up linkages matching "reqID" */
    while (pw < pwEnd)
        {
        memcpy (&tmpHdr, pw, sizeof(LinkageHeader));
        /*---------------------------------------------------------------
        Get the link size and primary ID. If the "user" bit is not set
        assume its a DMRS (4 word) linkage.
        ---------------------------------------------------------------*/
        linkID    = 0;
        linkWords = LinkageUtil::GetWords (&tmpHdr);

        /* This linkage is crap... */
        if (0 == linkWords || linkWords > elmP->GetSizeWords())
            return 0;

        if (tmpHdr.user)
            {
            linkID     = tmpHdr.primaryID;
            }

        if (linkID == reqID)
            attCount++;

        pw += linkWords;
        }

    return attCount;
    }

/*---------------------------------------------------------------------------------**//**
* Copy linkage of specified ID between elements.
* @bsimethod                                                    Ray.Bentley     01/02
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt BentleyApi::mdlLinkage_copy (DgnElementP pDestElement, DgnElementCP pSourceElement, int linkageId)
    {
    short*  pLinkage;
    if (NULL != (pLinkage = (short*) linkage_extractFromElement (NULL, pSourceElement, linkageId, NULL, NULL, NULL)))
        {
        LinkageHeader   tmpHdr;

        memcpy (&tmpHdr, pLinkage, sizeof(LinkageHeader));
        return linkage_appendToElement (pDestElement, &tmpHdr, pLinkage + 2, NULL);
        }

    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    DeepakMalkan    10/03
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt LinkageUtil::AppendElementIDArrayLinkage
(
DgnElementP  elementIn,                  /* => input element to append linkage to */
UShort      linkageKeyIn,               /* => input linkage key */
UInt32      elementIDCountIn,           /* => input number of ElementIDs in array */
ElementId const*  elementIDArrayIn            /* => input linkage array */
)
    {
    if (NULL == elementIn || NULL == elementIDArrayIn || elementIDCountIn <= 0)
        return ERROR;

    MSElementIDArrayLinkage*    elementIDArrayLinkage = NULL;
    long                        linkageSize = (sizeof (*elementIDArrayLinkage) - sizeof (elementIDArrayLinkage->data.linkageArray)) + (elementIDCountIn * sizeof (*elementIDArrayIn));

    elementIDArrayLinkage = (MSElementIDArrayLinkage *) _alloca (linkageSize);
    if (NULL == elementIDArrayLinkage)
        return ERROR;

    memset (elementIDArrayLinkage, 0, linkageSize);

    /* Set up elementIDArrayLinkage->data */
    elementIDArrayLinkage->data.linkageKey = linkageKeyIn;
    elementIDArrayLinkage->data.linkageArrayLength = elementIDCountIn;
    memcpy (elementIDArrayLinkage->data.linkageArray, elementIDArrayIn, elementIDCountIn * sizeof (*elementIDArrayIn));

    /* Set up elementIDArrayLinkage->header */
    elementIDArrayLinkage->header.primaryID = LINKAGEID_ElementIDArray;
    elementIDArrayLinkage->header.user      = true;

    long    linkageBytes = (linkageSize + 7) & ~7;
    LinkageUtil::SetWords (&elementIDArrayLinkage->header, linkageBytes / sizeof (short));

    return elemUtil_appendLinkage (elementIn, &elementIDArrayLinkage->header);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    DeepakMalkan    10/03
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt LinkageUtil::SetElementIDArrayLinkage
(
DgnElementP          elementIn,              /* => input element on which to append linkage - should be of sufficient size */
UShort              linkageKeyIn,           /* => input linkage key */
UInt32              elementIDCountIn,       /* => input number of ElementIDs in array */
ElementId const*    elementIDArrayIn        /* => input linkage array */
)
    {
    if (NULL == elementIn || NULL == elementIDArrayIn || elementIDCountIn <= 0)
        return ERROR;

    MSElementIDArrayLinkage*    elementIDArrayLinkage;
    for (int iLinkage = 0; NULL != (elementIDArrayLinkage = (MSElementIDArrayLinkage *) linkage_extractLinkageByIndex (NULL, elementIn, LINKAGEID_ElementIDArray, NULL, iLinkage)); iLinkage++)
        {
        if (elementIDArrayLinkage->data.linkageKey == linkageKeyIn)
            {
            if (elementIDArrayLinkage->data.linkageArrayLength == elementIDCountIn)
                {
                memcpy (elementIDArrayLinkage->data.linkageArray, elementIDArrayIn, elementIDCountIn * sizeof (*elementIDArrayIn));
                return SUCCESS;
                }

            if (1 == linkage_deleteLinkageByIndex (elementIn, LINKAGEID_ElementIDArray, iLinkage))
                return LinkageUtil::AppendElementIDArrayLinkage (elementIn, linkageKeyIn, elementIDCountIn, elementIDArrayIn);

            return ERROR;
            }
        }

    /* If the linkage is not found, then just add a new one */
    return LinkageUtil::AppendElementIDArrayLinkage (elementIn, linkageKeyIn, elementIDCountIn, elementIDArrayIn);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    DeepakMalkan    10/03
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt MSElementIDArrayLinkage::ExtractByIndex
(
UShort*             linkageKeyOut,
bvector<UInt64>*    elementIDArrayOut,
DgnElementCP         elementIn,
int                 linkageIndexIn
)
    {
    MSElementIDArrayLinkage*    elementIDArrayLinkage;
    if (NULL == (elementIDArrayLinkage = (MSElementIDArrayLinkage *) linkage_extractLinkageByIndex (NULL, elementIn, LINKAGEID_ElementIDArray, NULL, linkageIndexIn)))
        return DGNHANDLERS_STATUS_LinkageNotFound;

    if (NULL != linkageKeyOut)
        *linkageKeyOut = elementIDArrayLinkage->data.linkageKey;

    if (NULL != elementIDArrayOut)
        elementIDArrayOut->assign (elementIDArrayLinkage->data.linkageArray, elementIDArrayLinkage->data.linkageArray + elementIDArrayLinkage->data.linkageArrayLength);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    DeepakMalkan    10/03
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt MSElementIDArrayLinkage::ExtractNamedByIndex
(
bvector<UInt64>*        elementIDArrayOut,
DgnElementCP             elementIn,
UShort                  linkageKeyIn,
int                     linkageIndexIn
)
    {
    if (linkageIndexIn < 0)
        return ERROR;

    UShort  linkageKey;
    int     linkageIndex = 0;
    for (int iLinkage = 0; SUCCESS == MSElementIDArrayLinkage::ExtractByIndex (&linkageKey, NULL, elementIn, iLinkage); iLinkage++)
        {
        if (linkageKey != linkageKeyIn)
            continue;

        if (linkageIndex == linkageIndexIn)
            return MSElementIDArrayLinkage::ExtractByIndex (&linkageKey, elementIDArrayOut, elementIn, iLinkage);

        linkageIndex++;
        }

    return DGNHANDLERS_STATUS_LinkageNotFound;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Deepak.Malkan                   06/2006
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt   multiStateMaskLinkage_extractByIndex
(
UShort*             linkageKeyOut,
MultiStateMask**    multiStateMaskOut,
UInt32*             numShortsOut,
DgnElementCP         elementIn,
int                 linkageIndexIn
)
    {
    /* No linkage can be larger than the total size of all the linkages on the element + padding */
    int linkageSize = (sizeof (MSMultiStateMaskLinkage) + (elementIn->GetSizeWords() - elementIn->GetAttributeOffset()) * 2 + 7) & ~7;

    MSMultiStateMaskLinkage*    linkage = (MSMultiStateMaskLinkage*) _alloca (linkageSize);
    memset (linkage, 0, linkageSize);

    if (NULL == linkage_extractLinkageByIndex (linkage, elementIn, LINKAGEID_MultiStateMask, s_multiStateMaskLinkageConvRules, linkageIndexIn))
        return DGNHANDLERS_STATUS_LinkageNotFound;

    if (NULL != linkageKeyOut)
        *linkageKeyOut = linkage->data.linkageKey;

    /* If required, allocate multiStateMaskOut */
    if (NULL != multiStateMaskOut)
        *multiStateMaskOut = new MultiStateMask (linkage->data.numBitsPerState, linkage->data.numStates, linkage->data.stateArray, linkage->data.defaultValue);

    if (NULL != numShortsOut)
        *numShortsOut = linkage->data.numShorts;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Deepak.Malkan                   06/2006
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt   multiStateMaskLinkage_extractByKeyAndIndex
(
MultiStateMask**    multiStateMaskOut,
UInt32*             numShortsOut,
DgnElementCP         elementIn,
UShort              linkageKeyIn,
int                 linkageIndexIn
)
    {
    /* set output bitmask to NULL in case of error */
    if (NULL != multiStateMaskOut)
        *multiStateMaskOut = NULL;

    if (linkageIndexIn < 0)
        return DGNHANDLERS_STATUS_LinkageNotFound;

    UShort      linkageKey;
    int         linkageIndex = 0;
    for (int index = 0; SUCCESS == multiStateMaskLinkage_extractByIndex (&linkageKey, NULL, NULL, elementIn, index); index++)
        {
        if (linkageKey == linkageKeyIn)
            {
            if (linkageIndex == linkageIndexIn)
                return multiStateMaskLinkage_extractByIndex (NULL, multiStateMaskOut, numShortsOut, elementIn, index);

            linkageIndex++;
            }
        }

    return DGNHANDLERS_STATUS_LinkageNotFound;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Deepak.Malkan                   06/2006
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   BentleyApi::mdlLinkage_extractMultiStateMask
(
MultiStateMask**    multiStateMaskOut,
DgnElementCP         elementIn,
UShort              linkageKeyIn,
int                 linkageIndexIn
)
    {
    return multiStateMaskLinkage_extractByKeyAndIndex (multiStateMaskOut, NULL, elementIn, linkageKeyIn, linkageIndexIn);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Deepak.Malkan                   06/2006
+---------------+---------------+---------------+---------------+---------------+------*/
int      BentleyApi::mdlLinkage_appendMultiStateMask
(
DgnElementP      elementIn,
UShort          linkageKeyIn,
MultiStateMask* multiStateMaskIn
)
    {
    if (NULL == elementIn || NULL == multiStateMaskIn)
        return ERROR;

    UInt32  numStates = multiStateMaskIn->GetNumStates ();
    if (numStates <= 0)
        return ERROR;

    UInt32  numShorts = multiStateMaskIn->CalculateArraySize ();

    /* initialize "linkage" */
    int                         linkageSize = sizeof (MSMultiStateMaskLinkage) + (numShorts * sizeof (UInt16));
    MSMultiStateMaskLinkage *   linkage = (MSMultiStateMaskLinkage*) _alloca (linkageSize);
    memset (linkage, 0, linkageSize);

    /* Set up linkage->data */
    linkage->data.linkageKey = linkageKeyIn;
    linkage->data.numBitsPerState = multiStateMaskIn->GetNumBitsPerState ();
    linkage->data.defaultValue = multiStateMaskIn->GetDefaultStateValue ();
    linkage->data.numStates = multiStateMaskIn->GetNumStates ();
    linkage->data.numShorts = numShorts;
    multiStateMaskIn->CopyTo (linkage->data.stateArray);

    /* Set up linkage->header */
    linkage->header.primaryID = LINKAGEID_MultiStateMask;
    linkage->header.user      = true;

    return linkage_appendToElement (elementIn, &linkage->header, &linkage->data, s_multiStateMaskLinkageConvRules);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Deepak.Malkan                   06/2006
+---------------+---------------+---------------+---------------+---------------+------*/
Public StatusInt    BentleyApi::mdlLinkage_setMultiStateMask
(
DgnElementP          elementIn,
UShort              linkageKey,
MultiStateMask*     multiStateMaskIn
)
    {
    if (NULL == elementIn || NULL == multiStateMaskIn)
        return ERROR;

    UInt32  numStates = multiStateMaskIn->GetNumStates ();
    if (numStates <= 0)
        return ERROR;

    UInt32  numShorts = multiStateMaskIn->CalculateArraySize ();

    /* No linkage can be larger than the total size of all the linkages on the element + padding */
    int                         linkageSize = (sizeof (MSMultiStateMaskLinkage) + (elementIn->GetSizeWords() - elementIn->GetAttributeOffset()) * 2 + 7) & ~7;
    MSMultiStateMaskLinkage *   linkage = (MSMultiStateMaskLinkage*) _alloca(linkageSize);
    memset (linkage, 0, linkageSize);

    UShort *    fileLinkageData = NULL;
    for (int index = 0; NULL != (fileLinkageData = (UShort *) linkage_extractLinkageByIndex (linkage, elementIn, LINKAGEID_MultiStateMask, s_multiStateMaskLinkageConvRules, index)); index++)
        {
        if (linkage->data.linkageKey == linkageKey)
            {
            /* If the size of the linkage does not change, then we can write in place
             * else we need to delete the linkage & append a new one
             */
            if (linkage->data.numShorts == numShorts)
                {
                multiStateMaskIn->CopyTo (linkage->data.stateArray);
                linkage->data.numStates = numStates;

                return mdlCnv_bufferToFileFormatWithRules ((byte *) (fileLinkageData + 2), NULL, s_multiStateMaskLinkageConvRules, (byte *) &linkage->data, MAX_V8_ELEMENT_SIZE);
                }
            else if (1 == linkage_deleteLinkageByIndex (elementIn, LINKAGEID_MultiStateMask, index))
                {
                return mdlLinkage_appendMultiStateMask (elementIn, linkageKey, multiStateMaskIn);
                }
            else
                {
                return ERROR;
                }
            }

        /* initialize buffer before next call to extract in "for" loop */
        memset (linkage, 0, linkageSize);
        }

    /* If the linkage is not found, then just add a new one */
    return mdlLinkage_appendMultiStateMask (elementIn, linkageKey, multiStateMaskIn);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Deepak.Malkan                   06/2006
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt    BentleyApi::mdlLinkage_deleteMultiStateMask
(
DgnElementP      elementIn,
UShort          linkageKeyIn,
int             linkageIndexIn
)
    {
    UShort  linkageKey;
    int     linkageIndex = 0;
    for (int index = 0; SUCCESS == multiStateMaskLinkage_extractByIndex (&linkageKey, NULL, NULL, elementIn, index); index++)
        {
        if (linkageKey == linkageKeyIn)
            {
            if (linkageIndex == linkageIndexIn && 1 == linkage_deleteLinkageByIndex (elementIn, LINKAGEID_MultiStateMask, index))
                return SUCCESS;

            linkageIndex++;
            }
        }

    return ERROR;
    }

#if defined (NEEDS_WORK_DGNITEM)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    RayBentley                      09/01
+---------------+---------------+---------------+---------------+---------------+------*/
bool        BentleyApi::mdlLinkage_getInfiniteLine
(
bool*    pInfiniteStart,
bool*    pInfiniteEnd,
DgnElementCP pElement
)
    {
    if (LINE_ELM != pElement->GetLegacyType())
        return  false;

    InfiniteLineLinkage const* iline = (InfiniteLineLinkage const*) elemUtil_extractLinkage (NULL, NULL, pElement, LINKAGEID_InfiniteLine);
    if (NULL == iline)
        return  false;

    if (NULL != pInfiniteStart)
        *pInfiniteStart = iline->infiniteStart;

    if (NULL != pInfiniteEnd)
        *pInfiniteEnd   = iline->infiniteStart;

    return  iline->infiniteStart || iline->infiniteEnd;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    BrienBastings                   01/01
+---------------+---------------+---------------+---------------+---------------+------*/
int BentleyApi::mdlLinkage_setElementIds (MSElementDescrP pDscr, ElementId const* uniqueIdsP, UInt16 numIds, UShort applicationID, UShort applicationValue, int copyOption)
    {
    int   bufferSize = offsetof (DependencyLinkage, root) + numIds * sizeof (ElementId);

    DependencyManagerLinkage::DeleteLinkageFromMSElement (&pDscr->ElementR(), applicationID, applicationValue);

    DependencyLinkage   *depLinkageP = (DependencyLinkage *) _alloca (bufferSize);
    DependencyManagerLinkage::InitLinkage (*depLinkageP, applicationID, DEPENDENCY_DATA_TYPE_ELEM_ID, copyOption);

    depLinkageP->appValue   = applicationValue;
    depLinkageP->nRoots     = numIds;

    for (int i=0; i<numIds; i++)
        depLinkageP->root.elemid[i] = uniqueIdsP[i].GetValue();

    EditElementHandle eh(pDscr, false);
    return DependencyManagerLinkage::AppendLinkage (eh, *depLinkageP, 0);
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          mdlLinkage_getClipMaskElementIds                        |
|                                                                       |
| author        BrienBastings                             01/01         |
|                                                                       |
+----------------------------------------------------------------------*/
int      BentleyApi::mdlLinkage_getClipMaskElementIds
(
ElementId       *uniqueIdsP,            /* <= uniqueId */
int             maxClipElms,            /*  => number of ids to extract */
DgnElementCP     elmP                   /*  => element to test */
)
    {
    return mdlLinkage_getElementIds (uniqueIdsP, maxClipElms, elmP, DEPENDENCYAPPID_MicroStation, DEPENDENCYAPPVALUE_ClipMaskElement);
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          mdlLinkage_getElementIds                                |
|                                                                       |
| author        BrienBastings                             01/01         |
|                                                                       |
+----------------------------------------------------------------------*/
int      BentleyApi::mdlLinkage_getElementIds
(
ElementId       *uniqueIdsP,            /* <= uniqueId */
int             maxUniqueIds,           /* => maximum number of unique-ids that updateIdsP can hold */
DgnElementCP     elmP,                   /* => element to test */
UShort          applicationID,          /* => dependency application id (DEPENDENCYAPPVALUE_...) */
UShort          applicationValue        /* => dependency application value (DEPENDENCYAPPVALUE...) */
)
    {
    bvector<UInt64> idsFound;
    DependencyManagerLinkage::GetRootElementIdsInMSElement (idsFound, elmP, applicationID, applicationValue);

    if (uniqueIdsP != nullptr)
        {
        int copyCount = std::min(maxUniqueIds, (int)idsFound.size());
        for (int i=0; i<copyCount; ++i)
            uniqueIdsP[i] = ElementId(idsFound[i]);
        }
    return static_cast<int>(idsFound.size());
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    
+---------------+---------------+---------------+---------------+---------------+------*/
static int      linkage_deleteDoubleArrayLinkage
(
DgnElementP     pElementIn,         /* => input element */
DgnModelR       dgnModel,
UShort          linkageKeyIn,       /* => input linkage key */
size_t          linkageIndexIn      /* => input index */
)
    {
    if (NULL == pElementIn)
        return DGNHANDLERS_STATUS_BadArg;

    EditElementHandle eeh (*pElementIn, dgnModel);

    // if we find one that has the linkageKey we are looking for, either replace the data or delete it.
    size_t     numMatchingDoubleArrayLinkagesFound = 0;
    for (ElementLinkageIterator linkageIt = eeh.BeginElementLinkages(); linkageIt.IsValid(); linkageIt.ToNext())
        {
        LinkageHeaderCP linkage = linkageIt.GetLinkage();
        if (LINKAGEID_DoubleArray != linkage->primaryID)
            continue;

        UInt32  elementCountFound;
        UInt16  linkageKeyFound;
        ElementLinkageUtil::GetDoubleArrayDataCP (linkageIt, linkageKeyFound, elementCountFound);

        if (linkageKeyFound == linkageKeyIn)
            {
            if (linkageIndexIn == numMatchingDoubleArrayLinkagesFound)
                {
                StatusInt status = eeh.RemoveElementLinkage (linkageIt);
                if (SUCCESS == status)
                    eeh.GetElementCP ()->CopyTo (*pElementIn);
                return status;
                }
            numMatchingDoubleArrayLinkagesFound++;
            }
        }

    return DGNHANDLERS_STATUS_LinkageNotFound;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/2009
+---------------+---------------+---------------+---------------+---------------+------*/
int      BentleyApi::linkage_appendDoubleArrayLinkage
(
DgnElementP      pElementIn,             /* => input element to append string linkage to */
UShort          linkageKey,             /* => input linkage key */
UInt32          elementCount,           /* => input number of doubles in array */
const double*   pLinkageDoubleArrayIn   /* => input linkage array */
)
    {
    if (NULL == pElementIn || NULL == pLinkageDoubleArrayIn)
        return ERROR;

    if (0 == elementCount)
        return SUCCESS;

    DataExternalizer writer;

    writer.put ((UInt16) linkageKey);
    writer.put ((UInt16) 0); // padding
    writer.put ((UInt32) elementCount);
    writer.put (pLinkageDoubleArrayIn, elementCount);

    return ElementLinkageUtil::AddLinkage (*pElementIn, LINKAGEID_DoubleArray, writer);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/2009
+---------------+---------------+---------------+---------------+---------------+------*/
int      BentleyApi::linkage_appendByteArrayLinkage
(
DgnElement       *pElementIn,            /* => input element to append string linkage to */
UShort          linkageKey,             /* => input linkage key */
UInt32          elementCount,           /* => input number of doubles in array */
const byte     *pLinkageByteArrayIn  /* => input linkage array */
)
    {
    if (NULL == pElementIn || NULL == pLinkageByteArrayIn)
        {
        BeAssert (false);
        return ERROR;
        }    
    if (elementCount == 0)
        return SUCCESS;

    MSByteArrayLinkage* blink;
    size_t linkageAlloc = offsetof (MSByteArrayLinkage, data.linkageArray) + elementCount;

    // round up to multiple of 8 bytes (4 words). Memset entire array to 0.
    linkageAlloc = (linkageAlloc + 7) & ~7;
    blink = (MSByteArrayLinkage*) _alloca (linkageAlloc);
    memset (blink, 0, linkageAlloc);

    blink->header.user              = 1;
    blink->header.primaryID         = LINKAGEID_ByteArray;
    blink->data.linkageKey          = linkageKey;
    blink->data.linkageArrayLength  = elementCount;
    memcpy (blink->data.linkageArray, pLinkageByteArrayIn, elementCount*sizeof(*pLinkageByteArrayIn));

    LinkageUtil::SetWords (&blink->header, (int)(linkageAlloc/sizeof (short)));
    return elemUtil_appendLinkage (pElementIn, &blink->header);
    }

/*----------------------------------------------------------------------+
|                                                                       |
| adapted from: mdlLinkage_setSymbologyLinkage                          |
|                                                                       |
| author        BarryBentley                        03/01               |
|                                                                       |
+----------------------------------------------------------------------*/
StatusInt      BentleyApi::linkage_setSymbologyLinkage
(
EditElementHandleR  eh,
UInt16          linkageKeyIn,
bool            overrideStyle,
bool            overrideWeight,
bool            overrideColor,
bool            overrideLevel,
Int32           style,
UInt32          weight,
UInt32          color,
UInt32          level
)
    {
    for (ElementLinkageIterator ilinkage = eh.BeginElementLinkages(); ilinkage != eh.EndElementLinkages(); ++ilinkage)
        {
        MSSymbologyLinkage* pSymbologyLinkage = (MSSymbologyLinkage*)ilinkage.GetLinkage();
        if (pSymbologyLinkage->header.primaryID == LINKAGEID_Symbology && pSymbologyLinkage->data.linkageKey == linkageKeyIn)
            {
            if (pSymbologyLinkage->data.linkageKey == linkageKeyIn)
                {
                pSymbologyLinkage->data.overrideStyle   = overrideStyle;
                pSymbologyLinkage->data.overrideWeight  = overrideWeight;
                pSymbologyLinkage->data.overrideColor   = overrideColor;
                pSymbologyLinkage->data.overrideLevel   = overrideLevel;
                pSymbologyLinkage->data.style           = style;
                pSymbologyLinkage->data.weight          = weight;
                pSymbologyLinkage->data.color           = color;
                pSymbologyLinkage->data.level           = level;
                return SUCCESS;
                }
            }
        }

    /* if we got here, we didn't find existing matching linkage */
    MSSymbologyLinkage sl;
    memset (&sl, 0, sizeof(MSSymbologyLinkage));

    /* Set up sl.data */
    sl.header.primaryID     = LINKAGEID_Symbology;
    sl.header.user          = true;
    sl.data.linkageKey      = linkageKeyIn;
    sl.data.overrideStyle   = overrideStyle;
    sl.data.overrideWeight  = overrideWeight;
    sl.data.overrideColor   = overrideColor;
    sl.data.overrideLevel   = overrideLevel;
    sl.data.style           = style;
    sl.data.weight          = weight;
    sl.data.color           = color;
    sl.data.level           = level;

    int const linkBytes = (sizeof (sl) + 7) & ~7;
    LinkageUtil::SetWords (&sl.header, linkBytes / sizeof (short));

    return eh.AppendElementLinkage (NULL, sl.header, &sl.data);
    }

/*----------------------------------------------------------------------+
|                                                                       |
| adapted from: mdlLinkage_getSymbologyLinkage                          |
|                                                                       |
| author        BarryBentley                        03/01               |
|                                                                       |
+----------------------------------------------------------------------*/
StatusInt BentleyApi::linkage_getSymbologyLinkage
(
bool            *pOverrideStyle,
bool            *pOverrideWeight,
bool            *pOverrideColor,
bool            *pOverrideLevel,
Int32           *pStyle,
UInt32          *pWeight,
UInt32          *pColor,
UInt32          *pLevel,
int             linkageKeyIn,
ElementHandleCR    eh
)
    {
    for (ConstElementLinkageIterator ilinkage = eh.BeginElementLinkages(); ilinkage != eh.EndElementLinkages(); ++ilinkage)
        {
        MSSymbologyLinkage* pSymbologyLinkage = (MSSymbologyLinkage*)ilinkage.GetLinkage();
        if (pSymbologyLinkage->header.primaryID == LINKAGEID_Symbology && pSymbologyLinkage->data.linkageKey == linkageKeyIn)
            {
            if (pOverrideStyle) *pOverrideStyle = pSymbologyLinkage->data.overrideStyle;
            if (pOverrideWeight)*pOverrideWeight= pSymbologyLinkage->data.overrideWeight;
            if (pOverrideColor) *pOverrideColor = pSymbologyLinkage->data.overrideColor;
            if (pOverrideLevel) *pOverrideLevel = pSymbologyLinkage->data.overrideLevel;
            if (pStyle)         *pStyle         = pSymbologyLinkage->data.style;
            if (pWeight)        *pWeight        = pSymbologyLinkage->data.weight;
            if (pColor)         *pColor         = pSymbologyLinkage->data.color;
            if (pLevel)         *pLevel         = pSymbologyLinkage->data.level;
            return SUCCESS;
            }
        }
    return DGNHANDLERS_STATUS_LinkageNotFound;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| adapted from: mdlLinkage_deleteSymbologyLinkage                       |
|                                                                       |
| author        BarryBentley                              03/01         |
|                                                                       |
+----------------------------------------------------------------------*/
StatusInt BentleyApi::linkage_deleteSymbologyLinkage
(
EditElementHandleR eh,
int             linkageKeyIn
)
    {
    for (ElementLinkageIterator ilinkage = eh.BeginElementLinkages(); ilinkage != eh.EndElementLinkages(); ++ilinkage)
        {
        MSSymbologyLinkage* pSymbologyLinkage = (MSSymbologyLinkage*)ilinkage.GetLinkage();
        if (pSymbologyLinkage->header.primaryID == LINKAGEID_Symbology && pSymbologyLinkage->data.linkageKey == linkageKeyIn)
            {
            return eh.RemoveElementLinkage (ilinkage);
            }
        }

    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  08/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            ElementLinkageUtil::InitLinkageHeader (LinkageHeader& linkHdr, UInt16 primaryID, size_t rawDataBytes)
    {
    memset (&linkHdr, 0, sizeof (linkHdr));

    linkHdr.user = 1;
    linkHdr.primaryID = primaryID;

    LinkageUtil::SetWords (&linkHdr, (int) ((((sizeof (LinkageHeader) + rawDataBytes) + 7) & ~7) / sizeof (short)));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  08/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   ElementLinkageUtil::AddLinkage (DgnElementR elm, UInt16 primaryID, DataExternalizer& writer)
    {
    LinkageHeader   linkHdr;

    ElementLinkageUtil::InitLinkageHeader (linkHdr, primaryID, writer.getBytesWritten ());

    return (SUCCESS == linkage_appendToElement (&elm, &linkHdr, (void *) writer.getBuf (), NULL) ? SUCCESS : ERROR);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  08/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   ElementLinkageUtil::AddLinkage (EditElementHandleR eeh, UInt16 primaryID, DataExternalizer& writer)
    {
    DgnV8ElementBlank elm(*eeh.GetElementCP());

    if (SUCCESS != AddLinkage (elm, primaryID, writer))
        return ERROR;

    eeh.ReplaceElement (&elm);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  08/09
+---------------+---------------+---------------+---------------+---------------+------*/
double const*   ElementLinkageUtil::GetDoubleArrayDataCP (ConstElementLinkageIterator li, UInt16& linkageKey, UInt32& numEntries)
    {
    if (!li->user)
        return NULL;

    if (LINKAGEID_DoubleArray != li->primaryID)
        return NULL;

    UInt16* data = (UInt16 *) li.GetData ();
    
    linkageKey = *data;

    data++; data++; // skip padding...

    numEntries = *((UInt32 *) data);

    if (0 == numEntries)
        return NULL;

    data++; data++; // move to start of doubles...

    return (double const*) data;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  08/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   ElementLinkageUtil::AppendDoubleArrayData (EditElementHandleR eeh, UInt16 linkageKey, UInt32 numEntries, double const* doubleData)
    {
    DgnV8ElementBlank elm(*eeh.GetElementCP());

    if (SUCCESS != linkage_appendDoubleArrayLinkage (&elm, linkageKey, numEntries, doubleData))
        return ERROR;

    return (BentleyStatus) eeh.ReplaceElement (&elm);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  08/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   ElementLinkageUtil::DeleteDoubleArrayData (EditElementHandleR eeh, UInt16 linkageKey, size_t index)
    {
    DgnV8ElementBlank elm(*eeh.GetElementCP());

    if (SUCCESS != linkage_deleteDoubleArrayLinkage (&elm, *eeh.GetDgnModelP(), linkageKey, index))
        return ERROR;

    return (BentleyStatus) eeh.ReplaceElement (&elm);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   08/09
+---------------+---------------+---------------+---------------+---------------+------*/
byte const*     ElementLinkageUtil::GetByteArrayDataCP (ConstElementLinkageIterator li, UInt16& linkageKey, UInt32& numEntries)
    {
    if (!li->user)
        return NULL;

    if (LINKAGEID_ByteArray != li->primaryID)
        return NULL;

    MSByteArrayLinkageData* baLinkage = (MSByteArrayLinkageData *) li.GetData ();
    
    linkageKey = baLinkage->linkageKey;
    numEntries = baLinkage->linkageArrayLength;

    if (0 == numEntries)
        return NULL;

    return (byte const *) baLinkage->linkageArray;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   08/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   ElementLinkageUtil::AppendByteArrayData (EditElementHandleR eeh, UInt16 linkageKey, UInt32 numEntries, byte const* byteData)
    {
    DgnV8ElementBlank   elm (*eeh.GetElementCP());

    if (SUCCESS != linkage_appendByteArrayLinkage (&elm, linkageKey, numEntries, byteData))
        return ERROR;

    return (BentleyStatus) eeh.ReplaceElement (&elm);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   10/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ElementLinkageUtil::GetClippingDepths (ElementHandleCR eh, double& front, double& back, ClipMask& clipMask)
    {
    for (ConstElementLinkageIterator li = eh.BeginElementLinkages(); li != eh.EndElementLinkages(); ++li)
        {
        UInt16          linkageKey;
        UInt32          numEntries;
        double const*   doubleData = ElementLinkageUtil::GetDoubleArrayDataCP (li, linkageKey, numEntries);

        if (NULL == doubleData || DOUBLEARRAY_LINKAGE_KEY_ClippingDepth != linkageKey)
            continue;

        if (3 != numEntries)
            break;

        front = *doubleData;
        back  = *(doubleData+1);

        clipMask = static_cast <ClipMask>((UInt16) *(doubleData+2));
        return BSISUCCESS;
        }

    return BSIERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   10/09
+---------------+---------------+---------------+---------------+---------------+------*/
void ElementLinkageUtil::SetClippingDepths (EditElementHandleR eeh, double front, double back, ClipMask clipMask)
    {
    double  data[3];
    data[0] = front;
    data[1] = back;
    data[2] = (double) clipMask;

    // remove or modify existing linkage.
    for (ElementLinkageIterator li = eeh.BeginElementLinkages(); li != eeh.EndElementLinkages(); ++li)
        {
        UInt16      linkageKey;
        UInt32      numEntries;
        double*     doubleData = (double*) ElementLinkageUtil::GetDoubleArrayDataCP (li, linkageKey, numEntries);

        if (NULL == doubleData || DOUBLEARRAY_LINKAGE_KEY_ClippingDepth != linkageKey)
            continue;

        if (numEntries != _countof (data)) // bad linkage...delete and add a new one...
            {
            eeh.RemoveElementLinkage (li);
            break;
            }

        memcpy (doubleData, data, _countof (data) * sizeof (double));
        return;
        }

    ElementLinkageUtil::AppendDoubleArrayData (eeh, DOUBLEARRAY_LINKAGE_KEY_ClippingDepth, _countof (data), data);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
struct XDataHeader
    {
    Int16       m_groupCode;
    UInt16      m_bytesToFollow;
    UInt32      m_filler;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/11
+---------------+---------------+---------------+---------------+---------------+------*/
void            XDataHelper::Iterator::ToNext ()
    {
    UInt16*     data = (UInt16 *) m_li.GetData ();
    UInt32      numBytes = *((UInt32 *) data);

    if (0 == numBytes)
        return;

    data++; data++; // move to start of byte data...

    XDataHeader*  header = (XDataHeader*) (((UInt8*) data) + m_dataOffset);
    int           totalGroupCodeSize = (sizeof (*header) + header->m_bytesToFollow);

    if ((m_dataOffset + totalGroupCodeSize) >= numBytes)
        {
        m_data = NULL;
        m_dataOffset = 0;

        return;
        }

    m_data = (UInt8*) header;
    m_dataOffset += totalGroupCodeSize;

    // If were not looking to iterate a specific application group we can quit now...
    if (!m_appId.IsValid())
        return;

    if (m_inGroup)
        {
        // Stop iterator early if we hit another app group...
        if (DWGXDATA_Application_Name == GetGroupCode (m_data))
            {
            m_data = NULL;
            m_dataOffset = 0;
            }

        return;
        }

    UInt64      searchId;

    DataConvert::ReverseUInt64 (searchId, m_appId.GetValue());

    // Advance iterator to specified app group...save/restore m_appId so we can use ToNext to advance...
    AutoRestore <ElementId> saveAppId (&m_appId, ElementId());

    do
        {
        if (m_inGroup = (DWGXDATA_Application_Name == GetGroupCode (m_data) && searchId == *((UInt64*) GetDataCP (m_data))))
            break;

        ToNext ();

        } while (m_data);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/11
+---------------+---------------+---------------+---------------+---------------+------*/
XDataHelper::DWGXDataType XDataHelper::Iterator::GetGroupCode (UInt8 const* xDataHdr)
    {
    return (DWGXDataType) (xDataHdr ? ((XDataHeader const*) xDataHdr)->m_groupCode : 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/11
+---------------+---------------+---------------+---------------+---------------+------*/
size_t          XDataHelper::Iterator::GetDataSize (UInt8 const* xDataHdr)
    {
    return (xDataHdr ? ((XDataHeader const*) xDataHdr)->m_bytesToFollow : 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/11
+---------------+---------------+---------------+---------------+---------------+------*/
UInt8 const*    XDataHelper::Iterator::GetDataCP (UInt8 const* xDataHdr)
    {
    return (xDataHdr ? (xDataHdr + sizeof (XDataHeader)) : 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/11
+---------------+---------------+---------------+---------------+---------------+------*/
void            XDataHelper::Add (DataExternalizer& writer, DWGXDataType groupCode, UInt8 const* bytes, size_t nBytes)
    {
    if (0 == groupCode || 0 == nBytes || NULL == bytes)
        return;

    writer.put ((Int16) groupCode);
    writer.put ((UInt16) nBytes);
    writer.put ((UInt32) 0); // padding
    writer.put (bytes, nBytes);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/11
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   XDataHelper::Add (DataExternalizer& writer, DWGXDataType groupCode, ElementId value)
    {
    if (DWGXDATA_Application_Name != groupCode && DWGXDATA_DatabaseHandle != groupCode)
        return ERROR;

    UInt64      storageId;

    DataConvert::ReverseUInt64 (storageId, value.GetValue()); // Stored reversed...
    XDataHelper::Add (writer, groupCode, (UInt8 const*) &storageId, sizeof (storageId));

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/11
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   XDataHelper::Add (DataExternalizer& writer, DWGXDataType groupCode, CharCP value)
    {
    if (DWGXDATA_String != groupCode && DWGXDATA_ControlString != groupCode && DWGXDATA_LayerName != groupCode)
        return ERROR;

    XDataHelper::Add (writer, groupCode, (UInt8 const*) value, strlen (value));

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/11
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   XDataHelper::Add (DataExternalizer& writer, DWGXDataType groupCode, DPoint3dCR value)
    {
    if (DWGXDATA_Point != groupCode && DWGXDATA_Space_Point != groupCode && DWGXDATA_Disp_Point != groupCode && DWGXDATA_Dir_Point != groupCode)
        return ERROR;

    XDataHelper::Add (writer, groupCode, (UInt8 const*) &value, sizeof (value));

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/11
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   XDataHelper::Add (DataExternalizer& writer, DWGXDataType groupCode, double value)
    {
    if (DWGXDATA_Real != groupCode && DWGXDATA_Dist != groupCode && DWGXDATA_Scale != groupCode)
        return ERROR;

    XDataHelper::Add (writer, groupCode, (UInt8 const*) &value, sizeof (value));

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/11
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   XDataHelper::Add (DataExternalizer& writer, DWGXDataType groupCode, Int16 value)
    {
    if (DWGXDATA_Integer != groupCode)
        return ERROR;

    XDataHelper::Add (writer, groupCode, (UInt8 const*) &value, sizeof (value));

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/11
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   XDataHelper::Add (DataExternalizer& writer, DWGXDataType groupCode, Int32 value)
    {
    if (DWGXDATA_Long_Integer != groupCode)
        return ERROR;

    XDataHelper::Add (writer, groupCode, (UInt8 const*) &value, sizeof (value));

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/11
+---------------+---------------+---------------+---------------+---------------+------*/
bool            XDataHelper::CollectOtherData (DataExternalizer& writer, ConstElementLinkageIteratorR li, UInt64 excludeId)
    {
    bool        appIdFound = false;
    bool        inAppGroup = false;

    for (UInt8 const* xDataHdr : XDataHelper::Collection (li))
        {
        if (DWGXDATA_Application_Name == Iterator::GetGroupCode (xDataHdr))
            {
            if (inAppGroup = (excludeId == *((UInt64*) Iterator::GetDataCP (xDataHdr))))
                appIdFound = true;
            }

        if (inAppGroup)
            continue;

        Add (writer, Iterator::GetGroupCode (xDataHdr), Iterator::GetDataCP (xDataHdr), Iterator::GetDataSize (xDataHdr));
        }

    return appIdFound;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/11
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   XDataHelper::Append (EditElementHandleR eeh, DataExternalizer& writer)
    {
    // Data to append MUST begin with application id...
    if (0 == writer.getBytesWritten () || DWGXDATA_Application_Name != Iterator::GetGroupCode (writer.getBuf ()))
        return ERROR;

    ElementLinkageIterator li = eeh.BeginElementLinkages (LINKAGEID_XData);

    if (li.IsValid ())
        {
        // Append to existing linkage...existing application id data replaces with that of input...
        DataExternalizer tmpWriter;

        CollectOtherData (tmpWriter, li, *((UInt64*) Iterator::GetDataCP (writer.getBuf ())));
        tmpWriter.put (writer.getBuf (), writer.getBytesWritten ());

        if (SUCCESS != eeh.RemoveElementLinkage (li))
            return ERROR;

        DataExternalizer tmpWriter2;

        tmpWriter2.put ((UInt32) tmpWriter.getBytesWritten ()); // Total size of all group codes...
        tmpWriter2.put (tmpWriter.getBuf (), tmpWriter.getBytesWritten ());

        return ElementLinkageUtil::AddLinkage (eeh, LINKAGEID_XData, tmpWriter2);
        }

    // No existing XData...add new linkage...
    DataExternalizer tmpWriter;

    tmpWriter.put ((UInt32) writer.getBytesWritten ()); // Total size of all group codes...
    tmpWriter.put (writer.getBuf (), writer.getBytesWritten ());

    return ElementLinkageUtil::AddLinkage (eeh, LINKAGEID_XData, tmpWriter);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/11
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   XDataHelper::Delete (EditElementHandleR eeh, ElementId appId)
    {
    ElementLinkageIterator li = eeh.BeginElementLinkages (LINKAGEID_XData);

    if (!li.IsValid ())
        return ERROR;

    DataExternalizer writer;

    if (appId.IsValid())
        {
        UInt64  searchId;

        DataConvert::ReverseUInt64 (searchId, appId.GetValue());

        if (!CollectOtherData (writer, li, searchId))
            return ERROR;
        }

    if (SUCCESS != eeh.RemoveElementLinkage (li))
        return ERROR;

    if (0 == writer.getBytesWritten ())
        return SUCCESS;

    return Append (eeh, writer);
    }
