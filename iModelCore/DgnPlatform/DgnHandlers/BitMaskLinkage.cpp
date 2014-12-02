/*----------------------------------------------------------------------+
|
|     $Source: DgnHandlers/BitMaskLinkage.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+----------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <RmgrTools/RscMgr/bnryport.h>

// Output of mdlCnv_compileDataDefFromFileWithAllocType (...(RscFileHandle)NULL, DataDefID_BitMaskLinkage, ...)
static byte s_compiledBitMaskLinkageConvRules[] =
    {
    0x70, 0x6F, 0x76, 0x63, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00,
    0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x15, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00,
    0x04, 0x00, 0x00, 0x00, 0x0D, 0xF0, 0xAD, 0xBA, 0x0D, 0xF0, 0xAD, 0xBA, 0x02, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00,
    0x04, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00,
    0x04, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,
    0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x16, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00,
    0x04, 0x00, 0x00, 0x00, 0x0D, 0xF0, 0xAD, 0xBA, 0x0D, 0xF0, 0xAD, 0xBA
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    DeepakMalkan    11/00
+---------------+---------------+---------------+---------------+---------------+------*/
static int bitMaskLinkage_extractByIndex (UShort* pLinkageKeyOut, BitMaskH ppBitMaskOut, UInt32* pNumShortsOut, DgnElementCP pElementIn, int linkageIndexIn)
    {
    int                     status = DGNHANDLERS_STATUS_LinkageNotFound;
    MSBitMaskLinkage        *pBitMaskLinkage = NULL;
    int                     maxBitMaskSize = 0;

    /* No linkage can be larger than the total size of all the linkages on the element + padding */
    maxBitMaskSize = (sizeof (MSBitMaskLinkage) + (pElementIn->GetSizeWords() - pElementIn->GetAttributeOffset()) * 2 + 7) & ~7;
    pBitMaskLinkage = (MSBitMaskLinkage*)_alloca (maxBitMaskSize);
    memset (pBitMaskLinkage, 0, maxBitMaskSize);

    if (NULL != linkage_extractLinkageByIndex (pBitMaskLinkage, pElementIn, LINKAGEID_BitMask, s_compiledBitMaskLinkageConvRules, linkageIndexIn))
        {
        if (pLinkageKeyOut)
            *pLinkageKeyOut = pBitMaskLinkage->data.linkageKey;

        /* If required, allocate ppLinkageBitMaskOut */
        if (ppBitMaskOut)
            {
            *ppBitMaskOut = BitMask::Create (pBitMaskLinkage->data.defaultBitValue);

            (*ppBitMaskOut)->SetFromBitArray (pBitMaskLinkage->data.numValidBits, pBitMaskLinkage->data.bitMaskArray);
            }

        if (pNumShortsOut)
            *pNumShortsOut = pBitMaskLinkage->data.numShorts;

        status = SUCCESS;
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    DeepakMalkan    11/00
+---------------+---------------+---------------+---------------+---------------+------*/
static int bitMaskLinkage_extractByKeyAndIndex (BitMaskH ppBitMaskOut, UInt32* pNumShortsOut, DgnElementCP pElementIn, UShort linkageKeyIn, int linkageIndexIn)
    {
    int         status = DGNHANDLERS_STATUS_LinkageNotFound;
    int         iLinkage;
    UShort      linkageKey;
    int         linkageIndex = 0;

    /* set output bitmask to NULL in case of error */
    if (NULL != ppBitMaskOut)
        *ppBitMaskOut = NULL;

    if (linkageIndexIn >= 0)
        {
        for (iLinkage = 0; SUCCESS == bitMaskLinkage_extractByIndex (&linkageKey, NULL, NULL, pElementIn, iLinkage); iLinkage++)
            {
            if (linkageKey == linkageKeyIn)
                {
                if (linkageIndex == linkageIndexIn)
                    {
                    status = bitMaskLinkage_extractByIndex (NULL, ppBitMaskOut, pNumShortsOut, pElementIn, iLinkage);
                    break;
                    }

                linkageIndex++;
                }
            }
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    DeepakMalkan    11/00
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt BitMaskLinkage::ExtractBitMask (BitMaskH ppBitMaskOut, DgnElementCP pElementIn, UShort linkageKeyIn, int linkageIndexIn)
    {
    return bitMaskLinkage_extractByKeyAndIndex (ppBitMaskOut, NULL, pElementIn, linkageKeyIn, linkageIndexIn);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    DeepakMalkan    11/00
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt BitMaskLinkage::SetBitMaskUsingDescr (MSElementDescrH ppElemDscrIn, UShort linkageKeyIn, BitMaskCP pBitMaskIn)
    {
    int             status = ERROR;
    DgnElement  *pElement = NULL, *pElementCopy = NULL;
    int             bitMaskSize = 0;
    UInt32          numElementShorts = 0;
    UInt32          numValidBits = pBitMaskIn->GetCapacity ();
    UInt32          numShorts = (numValidBits + 15) / 16;

    if (NULL != ppElemDscrIn && NULL != *ppElemDscrIn && NULL != pBitMaskIn && numValidBits > 0)
        {
        pElement = &(*ppElemDscrIn)->ElementR();

        if (SUCCESS == bitMaskLinkage_extractByKeyAndIndex (NULL, &numElementShorts, pElement, linkageKeyIn, 0) && numElementShorts == numShorts)
            {
            pElementCopy = pElement;
            }
        else    /* The bit-mask size has changed, which means that the element size will change - we will need a new element */
            {
            size_t elemSize = (*ppElemDscrIn)->Element().Size ();
            bitMaskSize = LinkageUtil::CalculateSize ((sizeof (MSBitMaskLinkage) + (numShorts * sizeof(short)) + 7) & ~7);
            pElementCopy = (DgnElement*)_alloca (elemSize + bitMaskSize); /* Always add bitMaskSize to ensure that pElementCopy is of sufficient size */
            memcpy (pElementCopy, &(*ppElemDscrIn)->Element(), elemSize);
            }

        if (SUCCESS == (status = BitMaskLinkage::SetBitMask (pElementCopy, linkageKeyIn, pBitMaskIn)))
            {
            if (pElementCopy != pElement)
                (*ppElemDscrIn)->ReplaceElement (*pElementCopy);
            }
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    DeepakMalkan    11/00
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt BitMaskLinkage::SetBitMask (DgnElementP pElementIn, UShort linkageKey, BitMaskCP pBitMaskIn)
    {
    int                     status = ERROR;
    int                     iLinkage;
    UShort                  *pFileLinkageData = NULL;
    MSBitMaskLinkage        *pBitMaskLinkage = NULL;
    int                     maxBitMaskSize = 0;
    UInt32                  numValidBits = pBitMaskIn->GetCapacity ();
    UInt32                  numShorts = (numValidBits + 15) / 16;

    if (NULL != pElementIn && NULL != pBitMaskIn && numValidBits > 0)
        {
        /* No linkage can be larger than the total size of all the linkages on the element + padding */
        maxBitMaskSize = (sizeof (MSBitMaskLinkage) + (pElementIn->GetSizeWords() - pElementIn->GetAttributeOffset()) * 2 + 7) & ~7;

        pBitMaskLinkage = (MSBitMaskLinkage*)_alloca (maxBitMaskSize);
        memset(pBitMaskLinkage, 0, maxBitMaskSize);

        for (iLinkage = 0; NULL != (pFileLinkageData = (UShort*)linkage_extractLinkageByIndex (pBitMaskLinkage, pElementIn,
                                           LINKAGEID_BitMask,
                                           s_compiledBitMaskLinkageConvRules, iLinkage)); iLinkage++)
            {
            if (pBitMaskLinkage->data.linkageKey == linkageKey)
                {
                /* If the size of the bitmask-linkage does not change, then we can write in place
                 * else we need to delete the linkage & append a new one
                 */
                if (pBitMaskLinkage->data.numShorts == numShorts)
                    {
                    memcpy (pBitMaskLinkage->data.bitMaskArray, pBitMaskIn->GetBitArray (), numShorts * sizeof(short));
                    pBitMaskLinkage->data.numValidBits = numValidBits;

                    status = mdlCnv_bufferToFileFormatWithRules ((byte*)(pFileLinkageData + 2), NULL, s_compiledBitMaskLinkageConvRules,
                                                        (byte*)&pBitMaskLinkage->data, maxBitMaskSize);
                    }
                else if (1 == linkage_deleteLinkageByIndex (pElementIn, LINKAGEID_BitMask, iLinkage))
                    {
                    status = BitMaskLinkage::AppendBitMask (pElementIn, linkageKey, pBitMaskIn);
                    }
                else
                    {
                    status = ERROR;
                    }
                break;
                }

            /* initialize buffer before next call to extract in "for" loop */
            memset (pBitMaskLinkage, 0, maxBitMaskSize);
            }

        /* If the linkage is not found, then just add a new one */
        if (NULL == pFileLinkageData)
            {
            status = BitMaskLinkage::AppendBitMask (pElementIn, linkageKey, pBitMaskIn);
            }
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    DeepakMalkan    11/00
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt BitMaskLinkage::AppendBitMask (DgnElementP pElementIn, UShort linkageKeyIn, BitMaskCP pBitMaskIn)
    {
    int                     status = ERROR;
    MSBitMaskLinkage        *pBitMaskLinkage = NULL;
    int                     bitMaskSize = 0;
    UInt32                  numValidBits = pBitMaskIn->GetCapacity ();
    UInt32                  numShorts = (numValidBits + 15) / 16;

    if (NULL != pElementIn && NULL != pBitMaskIn && numValidBits > 0)
        {
        /* initialize "pBitMaskLinkage" */
        bitMaskSize = sizeof(MSBitMaskLinkage) + (numShorts * sizeof(short));
        pBitMaskLinkage = (MSBitMaskLinkage*)_alloca (bitMaskSize);
        memset(pBitMaskLinkage, 0, bitMaskSize);

        /* Set up pBitMaskLinkage->data */
        pBitMaskLinkage->data.linkageKey    = linkageKeyIn;
        pBitMaskLinkage->data.numValidBits  = numValidBits;
        pBitMaskLinkage->data.numShorts     = numShorts;
        memcpy(pBitMaskLinkage->data.bitMaskArray, pBitMaskIn->GetBitArray (), numShorts * sizeof(short));

        /* Set up pBitMaskLinkage->header */
        pBitMaskLinkage->header.primaryID = LINKAGEID_BitMask;
        pBitMaskLinkage->header.user      = true;

        status = linkage_appendToElement (pElementIn, &pBitMaskLinkage->header, &pBitMaskLinkage->data, s_compiledBitMaskLinkageConvRules);
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    DeepakMalkan    11/00
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt BitMaskLinkage::EnsureCapacityUsingDescr (MSElementDescrH ppElemDscrIn, UShort linkageKeyIn, int numBitsIn)
    {
    DgnElement  *pElement = NULL, *pElementCopy = NULL;
    int             bitMaskSize = 0;
    BitMaskP        pBitMask = NULL;
    UInt32          numElementShorts = 0;
    UInt32          numInputShorts = (numBitsIn + 15) / 16;
    bool            elementSizeChanged = false;

    if (NULL != ppElemDscrIn && NULL != *ppElemDscrIn && numBitsIn > 0)
        {
        pElement = &(*ppElemDscrIn)->ElementR();

        if (SUCCESS == bitMaskLinkage_extractByKeyAndIndex (NULL, &numElementShorts, pElement, linkageKeyIn, 0) &&
            numElementShorts < numInputShorts)          /* Only do something if bit-mask is to be expanded */
            {
            bitMaskLinkage_extractByKeyAndIndex (&pBitMask, NULL, pElement, linkageKeyIn, 0);
            pBitMask->EnsureCapacity ( numBitsIn);

            size_t elemSize = (*ppElemDscrIn)->Element().Size ();
            bitMaskSize = LinkageUtil::CalculateSize ((sizeof (MSBitMaskLinkage) + (numInputShorts * sizeof(short)) + 7) & ~7);
            pElementCopy = (DgnElement*)_alloca (elemSize + bitMaskSize); /* Always add bitMaskSize to ensure that pElementCopy is of sufficient size */
            memcpy (pElementCopy, &(*ppElemDscrIn)->Element(), elemSize);

            if (SUCCESS == BitMaskLinkage::SetBitMask (pElementCopy, linkageKeyIn, pBitMask))
                (*ppElemDscrIn)->ReplaceElement (*pElementCopy);

            pBitMask->Free ();

            elementSizeChanged = true;
            }
        }

    return elementSizeChanged;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    DeepakMalkan    11/00
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt BitMaskLinkage::DeleteBitMask (DgnElementP pElementIn, UShort linkageKeyIn, int linkageIndexIn)
    {
    int     status = ERROR;
    int     iLinkage;
    UShort  linkageKey;
    int     linkageIndex = 0;

    for (iLinkage = 0; SUCCESS == bitMaskLinkage_extractByIndex (&linkageKey, NULL, NULL, pElementIn, iLinkage); iLinkage++)
        {
        if (linkageKey == linkageKeyIn)
            {
            if (linkageIndex == linkageIndexIn &&
                1 == linkage_deleteLinkageByIndex (pElementIn, LINKAGEID_BitMask, iLinkage))
                {
                status = SUCCESS;
                break;
                }

            linkageIndex++;
            }
        }

    return status;
    }
