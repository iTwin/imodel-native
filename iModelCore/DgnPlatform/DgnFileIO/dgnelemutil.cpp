/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnFileIO/dgnelemutil.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   08/01
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt    BentleyApi::elemUtil_replaceLinkage
(
DgnElementP  pElm,          /* <=> Input element */
UShort          *pOldLinkage,   /*  => pointer to start of old linkage */
UShort          *pNewLinkage,   /*  => pointer to start of new linkage */
bool            allowSizeChange /*  => whether replace linkage can be different size */
)
    {
    int oldLinkWords = LinkageUtil::GetWords ((LinkageHeader *) pOldLinkage);
    int newLinkWords = LinkageUtil::GetWords ((LinkageHeader *) pNewLinkage);

    if (newLinkWords != oldLinkWords)
        {
        UShort      *pOldLinkEnd = NULL, *pNewLinkEnd = NULL, *pOldElemEnd = NULL;

        if (!allowSizeChange)
            return ERROR;

        pOldLinkEnd = pOldLinkage + oldLinkWords;
        pNewLinkEnd = pOldLinkage + newLinkWords;
        pOldElemEnd = ((UShort *) pElm) + pElm->GetSizeWords();

        memmove (pNewLinkEnd, pOldLinkEnd, (pOldElemEnd - pOldLinkEnd) * 2);
        pElm->SetSizeWords(pElm->GetSizeWords() + (newLinkWords - oldLinkWords));
        }

    memcpy (pOldLinkage, pNewLinkage, newLinkWords*2);
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* This function can be used to process all of the linkages on an element. A callback function does the processing. The callback can request to
* either replace or delete each linkage.
* @bsimethod                                                    Josh.Schifter   08/00
+---------------+---------------+---------------+---------------+---------------+------*/
static int processLinkages (LinkageProcessFuncP linkFuncP, void *pLinkFuncArg, DgnElementP pElm, bool allowMdlCallback, bool readOnly)
    {
    LinkageHeader   tmpHdr;
    UShort         *pw, *pwEnd, *pLinkBuf = NULL;
    UShort          linkWords, linkID;
    int             retCode, status = SUCCESS;

    pLinkBuf = (UShort *) _alloca (pElm->Size());

    DgnElementP replaceLinkBufP = !readOnly ? (DgnElementP) malloc (MAX_V8_ELEMENT_SIZE) : NULL;

    if (NULL != replaceLinkBufP)
        (*(Int16*)replaceLinkBufP) = 0;

    /* Get word pointers to start of attribute data and end of element */
    pw    = (UShort*) pElm;
    pwEnd = pw + pElm->GetSizeWords();
    pw   += pElm->GetAttributeOffset();

    /* Scan through attribute data */
    while (pw < pwEnd)
        {
        memcpy (&tmpHdr, pw, sizeof(tmpHdr));

        /*---------------------------------------------------------------
        Get the link size and primary ID. If the "user" bit is not set
        assume its a DMRS (4 word) linkage.
        ---------------------------------------------------------------*/
        linkID    = 0;
        linkWords = LinkageUtil::GetWords (&tmpHdr);

        /* This linkage is crap... */
        if (0 == linkWords || linkWords > pElm->GetSizeWords())
            {
            if (NULL != replaceLinkBufP)
                free (replaceLinkBufP);
            return PROCESS_ATTRIB_ERROR_BADELEM;
            }

        if (tmpHdr.user)
            {
            linkID = tmpHdr.primaryID;
            }

        memcpy (pLinkBuf, pw, (linkWords) * 2);

        /*-----------------------------------------------------------
        Working on a new copy of the linkage... swap first word if
        its a DMRS linkage or first two words if its a user linkage.
        -----------------------------------------------------------*/
        retCode = (*linkFuncP) ((LinkageHeader *) replaceLinkBufP, pLinkFuncArg, (LinkageHeader *) pLinkBuf, pElm);

        if (retCode & PROCESS_ATTRIB_STATUS_DELETE)
            {
            ptrdiff_t        maxLinkWords = (pwEnd - pw);
            unsigned short  *pLinkEnd;

            // Make sure the linkage does not run beyond the end of the element
            if (linkWords > maxLinkWords)
                linkWords = static_cast<UShort>(maxLinkWords);

            pLinkEnd = pw + linkWords;

            memmove (pw, pLinkEnd, (pwEnd - pLinkEnd) * 2);
            pElm->SetSizeWords(pElm->GetSizeWords() - linkWords);
            pwEnd = (UShort*) pElm + pElm->GetSizeWords();

            linkWords = 0;
            }
        else if (retCode & PROCESS_ATTRIB_STATUS_REPLACE)
            {
            if (SUCCESS != elemUtil_replaceLinkage (pElm, pw, (UShort *) replaceLinkBufP, true))
                status |= PROCESS_ATTRIB_ERROR_MAXSIZE;
            else
                linkWords = LinkageUtil::GetWords ((LinkageHeader *) replaceLinkBufP);
            }

        if (retCode & PROCESS_ATTRIB_STATUS_ABORT)
            {
            status |= PROCESS_ATTRIB_ERROR_ABORT;
            break;
            }

        pw += linkWords;
        }

    if (NULL != replaceLinkBufP)
        free (replaceLinkBufP);

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* This function can be used to process all of the linkages on an element. A callback function does the processing. The callback can request to
* either replace or delete each linkage.
* @bsimethod                                                    Josh.Schifter   08/00
+---------------+---------------+---------------+---------------+---------------+------*/
int BentleyApi::mdlElement_processLinkages (LinkageProcessFuncP linkFuncP, void *pLinkFuncArg, DgnElementP pElm)
    {
    if (NULL == linkFuncP)
        return PROCESS_ATTRIB_ERROR_BADARG;

    if (pElm->GetSizeWords() < pElm->GetAttributeOffset())
        return PROCESS_ATTRIB_ERROR_BADELEM;

    /* If there are no linkages we're done */
    if (pElm->GetSizeWords() == pElm->GetAttributeOffset())
        return SUCCESS;

    return processLinkages (linkFuncP, pLinkFuncArg, pElm, false, false);
    }

/*---------------------------------------------------------------------------------**//**
* This function can be used to process all of the linkages on an element. A callback function does the processing. The callback can request to
* either replace or delete each linkage.
* @bsimethod                                                    Josh.Schifter   08/00
+---------------+---------------+---------------+---------------+---------------+------*/
int BentleyApi::mdlElement_processLinkagesDirect (LinkageProcessFuncP linkFuncP, void *pLinkFuncArg, DgnElementP pElm)
    {
    if (NULL == linkFuncP)
        return PROCESS_ATTRIB_ERROR_BADARG;

    if (pElm->GetSizeWords() < pElm->GetAttributeOffset())
        return PROCESS_ATTRIB_ERROR_BADELEM;

    /* If there are no linkages we're done */
    if (pElm->GetSizeWords() == pElm->GetAttributeOffset())
        return SUCCESS;

    return processLinkages (linkFuncP, pLinkFuncArg, pElm, false, false);
    }

/*---------------------------------------------------------------------------------**//**
* This function can be used to process all of the linkages on an element in a read-only fashion.
* It is a bit more efficient than calling mdlElement_processLinkagesDirect - since it allocates &
* frees an entire element for every linkage processed.
* A callback function does the processing.
* @bsimethod                                                    Josh.Schifter   08/00
+---------------+---------------+---------------+---------------+---------------+------*/
int   BentleyApi::mdlElement_processLinkagesReadOnly
(
LinkageProcessFuncP linkFuncP,          /* => Function to process linkages     */
void                *pLinkFuncArg,      /* => Argument to linkFunc             */
DgnElementP  pElm               /* => Input element                    */
)
    {
    if (NULL == linkFuncP) 
        return PROCESS_ATTRIB_ERROR_BADARG;

    if (pElm->GetSizeWords() < pElm->GetAttributeOffset())
        return PROCESS_ATTRIB_ERROR_BADELEM;

    /* If there are no linkages we're done */
    if (pElm->GetSizeWords() == pElm->GetAttributeOffset())
        return SUCCESS;

    return processLinkages (linkFuncP, pLinkFuncArg, pElm, false, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    JVB                             8/91
+---------------+---------------+---------------+---------------+---------------+------*/
void*    BentleyApi::elemUtil_extractLinkage
(                                       /* <= Returns address of link or NULL  */
void*           pLinkBuf,       /* <= Link (2 words swapped) (NULL)    */
T_LinkFunc      linkFunc,       /* => Func to process links  (NULL)    */
DgnElementCP     elm,            /* => Input element                    */
int             reqID           /* => Requested link ID (0 = DMRS)     */
)
    {
    if (elm->GetSizeWords() <= elm->GetAttributeOffset())
        return NULL;

    /* Get word pointers to start of attribute data and end of element */
    UShort const* pw    = (UShort *) elm;
    UShort const* pwEnd = pw + elm->GetSizeWords();
    void*         lastLink = NULL;

    pw   += elm->GetAttributeOffset();

    /* Scan through attribute data - return when "reqID" is found */
    while (pw < pwEnd)
        {
        LinkageHeader const*   tmpHdr = (LinkageHeader const*) pw;

        /*---------------------------------------------------------------
        Get the link size and primary ID. If the "user" bit is not set
        assume its a DMRS (4 word) linkage.
        ---------------------------------------------------------------*/
        UShort linkWords = LinkageUtil::GetWords (tmpHdr);

        if (0 == linkWords)         // Avoid infinite loop.
            return NULL;

        UShort linkID = (tmpHdr->user) ? tmpHdr->primaryID : 0;

        if (linkID == reqID)
            {
            if (pLinkBuf)
                memcpy (pLinkBuf, pw, (linkWords) * 2);

            /*-----------------------------------------------------------
            If a "linkFunc" was passed in, call it and keep going unless
            "linkFunc" returns a non-zero value (processing multiple
            linkages of the same type). If "linkFunc" is null, return
            with the address of the current linkage.
            -----------------------------------------------------------*/
            if (NULL == linkFunc)
                return (void *) pw;

            if (SUCCESS != linkFunc (tmpHdr))
                return (void *) pw;

            lastLink = (void *) pw;
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
* @bsimethod                                                    Keith.Bentley   12/02
+---------------+---------------+---------------+---------------+---------------+------*/
void     BentleyApi::mdlElement_visitLinkages
(
VisitLinkageFunc    visitFunc,
void*               arg,
DgnElementCP         elm
)
    {
    /* If there are no linkages we're done */
    if (NULL == elm || (elm->GetSizeWords() <= elm->GetAttributeOffset()))
        return;

    /* Get word pointers to start of attribute data and end of element */
    LinkageHeader const* thisLinkage = (LinkageHeader const*) ((UShort*) elm + elm->GetAttributeOffset());
    LinkageHeader const* end         = (LinkageHeader const*) ((UShort*) elm + elm->GetSizeWords());

    while (thisLinkage < end)
        {
        int linkWords = LinkageUtil::GetWords (thisLinkage);

        if ((0 == linkWords) || (SUCCESS != visitFunc (thisLinkage, arg)))
            return;

        LinkageHeader const* lastLinkage = thisLinkage;
        
        thisLinkage = (LinkageHeader const*) ((UShort*) thisLinkage + linkWords);
        
        if (lastLinkage > thisLinkage)
            {
            //  I've seen thisLinkage wrap when the element was near the upper limit of the address range.
            //  It takes a bogus linkWords, but it does happen.
            BeAssert (thisLinkage > lastLinkage);
            return;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JVB             08/91
+---------------+---------------+---------------+---------------+---------------+------*/
int      BentleyApi::elemUtil_deleteLinkage (DgnElementP elm, int reqID)
    {
    LinkageHeader  *pLink;
    short          *linkStart, *linkEnd, *elmEnd;
    ptrdiff_t       linkWords, maxLinkWords;

    if (!(pLink = (LinkageHeader *) elemUtil_extractLinkage (NULL, NULL, elm, reqID)))
        return (ERROR);

    linkStart       = (short*) pLink;
    elmEnd          = (short*) elm + elm->GetSizeWords();
    linkWords       = LinkageUtil::GetWords (pLink);
    maxLinkWords    = (elmEnd - linkStart);

    // Make sure the linkage does not run beyond the end of the element
    if (linkWords > maxLinkWords)
        linkWords = maxLinkWords;

    linkEnd = linkStart + linkWords;

    memmove (linkStart, linkEnd, (elmEnd - linkEnd) * 2);
    elm->SetSizeWords(elm->GetSizeWords() - static_cast<UInt32>(linkWords));

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JVB             08/91
+---------------+---------------+---------------+---------------+---------------+------*/
int      BentleyApi::elemUtil_appendLinkage (DgnElementP elm, LinkageHeaderCP linkage)
    {
    short      *pwEnd;
    UInt32      attrOffset;

    /* Get attribute index ... make sure its valid. */
    attrOffset = elm->GetAttributeOffset();
    if (attrOffset > elm->GetSizeWords())
        return ERROR;

    /* Get word pointer to end of element */
    pwEnd = (short*)elm + elm->GetSizeWords();

    memcpy (pwEnd, linkage, LinkageUtil::GetWords (linkage) * 2);
    elm->SetSizeWords(elm->GetSizeWords() + LinkageUtil::GetWords (linkage));

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                  Mike.Stratoti   02/2002
+---------------+---------------+---------------+---------------+---------------+------*/
size_t DgnElementHeader::CopyTo (DgnElementHeaderR rhs) const
    {
    size_t size = Size();
    memcpy (&rhs, this, size);
    return size;
    }
