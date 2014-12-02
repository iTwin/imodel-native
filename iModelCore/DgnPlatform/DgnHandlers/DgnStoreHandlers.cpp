/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnHandlers/DgnStoreHandlers.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>
#include    <algorithm>

#define DATA_BLOCK_SIZE         5120

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/00
+---------------+---------------+---------------+---------------+---------------+------*/
bool DgnStoreHdrHandler::IsDgnStoreElement (ElementHandleCR eh, UInt32 dgnStoreId, UInt32 applicationId)
    {
    if (!eh.IsValid ())
        return false;

    DgnElementCP elm = eh.GetElementCP ();

    if (DGNSTORE_HDR != elm->GetLegacyType() && (DGNSTORE_COMP != elm->GetLegacyType() || 0 != elm->ToDgnStoreComp().sequenceNo))
        return false;

    return (0 == dgnStoreId || dgnStoreId == elm->ToDgnStoreHdr().dgnStoreId) && (0 == applicationId || applicationId == elm->ToDgnStoreHdr().applicationId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/00
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DgnStoreHdrHandler::GetDgnStoreIds (UInt32* dgnStoreId, UInt32* applicationId, ElementHandleCR eh)
    {
    if (!DgnStoreHdrHandler::IsDgnStoreElement (eh, 0, 0))
        return ERROR;

    DgnElementCP elm = eh.GetElementCP ();

    if (dgnStoreId)
        *dgnStoreId = elm->ToDgnStoreHdr().dgnStoreId;

    if (applicationId)
        *applicationId = elm->ToDgnStoreHdr().applicationId;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     12/97
+---------------+---------------+---------------+---------------+---------------+------*/
static UInt32  computeCheckSum (void* dataPtr, int dataSize)
    {
    byte const* dataP = (byte*) dataPtr;
    UInt32      sum = 0;

    for (int i=0; i<dataSize; i++, dataP++)
        sum ^= (*dataP << ((i % 4) << 3));

    return sum;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/00
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DgnStoreHdrHandler::Extract (void** dataPP, UInt32* dataSizeP, UInt32 dgnStoreId, UInt32 applicationId, ElementHandleCR eh)
    {
    if (!eh.IsValid () || !DgnStoreHdrHandler::IsDgnStoreElement (eh, dgnStoreId, applicationId))
        return ERROR;

    void*       dataP = *dataPP;
    UInt32      cmpnNum = 0, nBytes = 0, nElements = 0, nTotalBytes = 0, checkSum = 0;
    DgnElementCP hdrElm = eh.GetElementCP ();

    nBytes      = hdrElm->ToDgnStoreHdr().dataSize;
    nElements   = hdrElm->ToDgnStoreHdr().componentCount + 1;
    nTotalBytes = hdrElm->ToDgnStoreHdr().totalSize;
    checkSum    = hdrElm->ToDgnStoreHdr().checkSum;

    // Make sure nTotalBytes is large enough to copy nBytes...
    if (nTotalBytes < nBytes || nTotalBytes > MAX_V8_ELEMENT_SIZE * nElements) // The nTotalBytes is at the same offset as displayable element symbology....So sanity test.
        {
        // If this is the only dgn store...might was well see if checksum turns out ok.
        if (1 == nElements)
            nTotalBytes = nBytes;
        else
            return ERROR;
        }

    if (NULL == (dataP = malloc (nTotalBytes)))
        return ERROR;

    memcpy (dataP, &hdrElm->ToDgnStoreHdr().elementData[0], nBytes);
    *dataSizeP = nBytes;

    MSElementDescrVecCP components=NULL;
    MSElementDescrVec::const_iterator componentIter(0);
    SubElementRefVecP   subElements = NULL;
    SubElementRefVec::iterator subElIter(0);
    EditElementHandle  childEh;

    if (DGNSTORE_HDR == eh.GetLegacyType())
        {
        if (eh.PeekElementDescrCP())
            {
            components = &eh.PeekElementDescrCP()->Components();
            if (!components->empty())
                {
                componentIter = components->begin();
                childEh.SetElementDescr (componentIter->get(), false);
                }
            }
        else
            {
            ElementRefP child = NULL;

            subElements = eh.GetElementRef()->GetSubElements();

            if (NULL != subElements)
                {
                subElIter = subElements->begin();
                child = *subElIter;
                }

            childEh.SetElementRef (child);
            }
        }
    else
        {
        if (eh.PeekElementDescrCP ())
            {
            auto parent = eh.PeekElementDescrCP()->GetParent();
            if (NULL == parent)
                {
                BeAssert(false);
                return ERROR;
                }
            components = &parent->Components();
            componentIter = components->Find(*eh.PeekElementDescrCP());
            if (componentIter == components->end() || ++componentIter == components->end())
                {
                //BeAssert(false);
                return ERROR;
                }

            childEh.SetElementDescr (componentIter->get(), false);
            }
        else
            {
            ElementRefP child = NULL;
            ElementRefP parent = eh.GetElementRef()->GetParentElementRef();

            if (parent)
                {
                subElements = parent->GetSubElements();
                subElIter = std::find (subElements->begin(), subElements->end(), eh.GetElementRef());

                if (subElIter != subElements->end())     // move to next sibling
                    ++subElIter;

                child = subElIter == subElements->end() ? NULL : *subElIter;
                }

            childEh.SetElementRef (child);
            }
        }

    for (; childEh.IsValid () && DGNSTORE_COMP == childEh.GetLegacyType() && ++cmpnNum < nElements; )
        {
        DgnElementCP childElm = childEh.GetElementCP ();

        nBytes = childElm->ToDgnStoreComp().dataSize;

        if (childElm->ToDgnStoreComp().sequenceNo == cmpnNum && *dataSizeP + nBytes <= nTotalBytes)
            {
            memcpy ((char*)dataP + *dataSizeP, &childElm->ToDgnStoreComp().elementData[0], nBytes);
            *dataSizeP += nBytes;
            }
        else
            {
            break;
            }

        if (childEh.PeekElementDescrCP ())
            {
            MSElementDescrP next = NULL;
            if (components && (++componentIter != components->end()))
                next = const_cast<MSElementDescrP> (componentIter->get());

            childEh.SetElementDescr (next, false);
            }
        else
            {
            ElementRefP next = NULL;

            if (subElements && (++subElIter != subElements->end()))
                next = *subElIter;

            childEh.SetElementRef (next);
            }
        }

    if (*dataSizeP == nTotalBytes && checkSum == computeCheckSum (dataP, *dataSizeP))
        {
        *dataPP = dataP;

        return SUCCESS;
        }

    free (dataP);

    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/00
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DgnStoreHdrHandler::ExtractFromCell
(
void**          dataPP,
UInt32*         dataSizeP,
UInt32          dgnStoreId,                     /* => ex. DGN_STORE_ID */
UInt32          applicationId,                  /* => unique application ID */
ElementHandleCR    eh
)
    {
    if (!eh.IsValid () || CELL_HEADER_ELM != eh.GetLegacyType())
        return ERROR;

    ChildElemIter childEh (eh, ExposeChildrenReason::Count);

    for (; childEh.IsValid (); childEh = childEh.ToNext ())
        {
        if (!DgnStoreHdrHandler::IsDgnStoreElement (childEh, dgnStoreId, applicationId))
            continue;

        return DgnStoreHdrHandler::Extract (dataPP, dataSizeP, dgnStoreId, applicationId, childEh);
        }

    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/09
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnStoreHdrHandler::FreeExtractedData (void* dataP)
    {
    FREE_AND_CLEAR (dataP);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/00
+---------------+---------------+---------------+---------------+---------------+------*/
static MSElementDescrPtr createDgnStoreHdrElement (UInt16 hdrType, char** dataPP, UInt32* dataSizeP, UInt32 maxDgnStoreSize, UInt32 checkSum, UInt32 dgnStoreId, UInt32 applicationId, DgnModelR model)
    {
    int         elementSize = 0;
    UInt32      dataSize = 0;
    UInt32      nElements = ((*dataSizeP + maxDgnStoreSize - 1) / maxDgnStoreSize) - 1;
    bool        lastElement = *dataSizeP <= maxDgnStoreSize;
    DgnV8ElementBlank   element;

    dataSize    = lastElement ? *dataSizeP : maxDgnStoreSize;
    elementSize = offsetof (DgnStoreHdr, elementData) + dataSize;

    memset (&element, 0, sizeof (element));

    element.SetLegacyType(hdrType);
    element.SetComplexHeader(DGNSTORE_HDR == hdrType);
    element.SetSizeWordsNoAttributes((elementSize+1) / 2);

    element.ToDgnStoreHdrR().componentCount  = nElements;
    element.ToDgnStoreHdrR().dgnStoreId      = dgnStoreId;
    element.ToDgnStoreHdrR().applicationId   = applicationId;
    element.ToDgnStoreHdrR().checkSum        = checkSum;
    element.ToDgnStoreHdrR().totalSize       = *dataSizeP;
    element.ToDgnStoreHdrR().dataSize        = dataSize;

    memcpy (&element.ToDgnStoreHdrR().elementData[0], *dataPP, dataSize);

    (*dataPP)  += dataSize;
    *dataSizeP -= dataSize;

    return new MSElementDescr(element, model);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/00
+---------------+---------------+---------------+---------------+---------------+------*/
static MSElementDescrPtr createDgnStoreCompElement (UInt32 cmpnNo, char** dataPP, UInt32* dataSizeP, UInt32 maxDgnStoreSize, DgnModelR model)
    {
    int         elementSize = 0;
    UInt32      dataSize = 0;
    bool        lastElement = *dataSizeP <= maxDgnStoreSize;
    DgnV8ElementBlank element;

    dataSize    = lastElement ? *dataSizeP : maxDgnStoreSize;
    elementSize = offsetof (DgnStoreComp, elementData) + dataSize;

    memset (&element, 0, sizeof (element));

    element.SetLegacyType(DGNSTORE_COMP);
    element.SetSizeWordsNoAttributes((elementSize+1) / 2);

    element.ToDgnStoreCompR().sequenceNo = cmpnNo;    // Used to recognize first 38 in a cell
    element.ToDgnStoreCompR().dataSize   = dataSize;

    memcpy (&element.ToDgnStoreCompR().elementData[0], *dataPP, dataSize);

    (*dataPP)  += dataSize;
    *dataSizeP -= dataSize;

    return new MSElementDescr(element, model);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/00
+---------------+---------------+---------------+---------------+---------------+------*/
MSElementDescrPtr DgnStoreHdrHandler::Create(void* dataP, UInt32 dataSize, UInt32 dgnStoreId, UInt32 applicationId, DgnModelR model)
    {
    UInt32          cmpnNo = 0;
    UInt32          checkSum = computeCheckSum (dataP, dataSize);
    MSElementDescrPtr edP = createDgnStoreHdrElement(DGNSTORE_HDR, (char**) &dataP, &dataSize, MAX_DGNSTORE_SIZE, checkSum, dgnStoreId, applicationId, model);

    while (dataSize > 0)
        edP->AddComponent(*createDgnStoreCompElement (++cmpnNo, (char **) &dataP, &dataSize, MAX_DGNSTORE_SIZE, model));

    return edP;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DgnStoreHdrHandler::Create(EditElementHandleR eeh, void* dataP, UInt32 dataSize, UInt32 dgnStoreId, UInt32 applicationId, DgnModelR model)
    {
    MSElementDescrPtr dscr = Create(dataP, dataSize, dgnStoreId, applicationId, model);
    eeh.SetElementDescr(dscr.get(), false);
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/00
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DgnStoreHdrHandler::AppendToCell (EditElementHandleR cellEeh, void* dataP, UInt32 dataSize, UInt32 dgnStoreId, UInt32 applicationId)
    {
    MSElementDescrPtr dscr = Create(dataP, dataSize, dgnStoreId, applicationId, *cellEeh.GetDgnModelP ());
    cellEeh.GetElementDescrP()->AddComponent(*dscr);
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DgnStoreHdrHandler::RemoveFromCell (EditElementHandleR cellEeh, UInt32 dgnStoreId, UInt32 applicationId)
    {
    if (!cellEeh.IsValid ())
        return ERROR;

    MSElementDescrP cellEdP = cellEeh.GetElementDescrP (); // Make sure we have edP...don't extract, could be nested cell...
    if (!cellEdP)
        return ERROR;

    MSElementDescrVec breps;
    for (auto& child : cellEdP->Components())
        {
        if (DgnStoreHdrHandler::IsDgnStoreElement (ElementHandle(child.get(), false), dgnStoreId, applicationId))
            breps.push_back(child);
        }

    for (auto child : breps)
        cellEdP->RemoveComponent(*child);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Evan.Williams                  09/09
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnStoreHdrHandler::_GetTypeName (WStringR descr, UInt32 desiredLength)
    {
    descr.assign (DgnHandlersMessage::GetStringW(DgnHandlersMessage::IDS_TYPENAMES_DGNSTORE_HDR));
    }
