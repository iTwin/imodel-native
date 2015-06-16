/*--------------------------------------------------------------------------------------+
|
|     $Source: Tools/ToolSubs/bitmask/BitMask.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>
#include    <string.h>
#include    <stdlib.h>
#include    <DgnPlatform/Tools/stringop.h>
#include    <RmgrTools/Tools/memutil.h>
#include    <DgnPlatform/Tools/BitMask.h>
#include    <RmgrTools/Tools/toolsubsStdio.h>
#include    <Bentley/BeFile.h>

/*----------------------------------------------------------------------+
|   Local defines                                                       |
+----------------------------------------------------------------------*/
#define SHORTS_FROM_BITS(arg)                   ((arg + 15) / 16)
#define SHORT_INDEX_FROM_BIT(arg)               (arg / 16)
#define SHORT_MASK_FROM_BIT(arg)                (1 << ((arg) % 16))

BEGIN_BENTLEY_NAMESPACE

/*======================================================================+
|   Private code section                                                |
+======================================================================*/
#if defined (DEBUG_BITMASK)
size_t          BitMask::s_totalBitMaskMemoryMalloced;
#endif

/*---------------------------------------------------------------------------------**//**
* finds the highest number in an array of bits to set.
* @bsimethod                                                    Deepak.Malkan   09/00
+---------------+---------------+---------------+---------------+---------------+------*/
static bool     FindHighestBitInArray
(
uint32_t*         highestFound,           // <= highest found.
uint32_t        arraySize,              // => IN number of bits to set
const uint32_t*   pBitArrayIn             // => IN bit index array
)
    {
    bool    anyValid = false;
    uint32_t maxBitNum = 0;

    for (uint32_t iArray = 0; iArray < arraySize; iArray++, pBitArrayIn++)
        {
        if ( (*pBitArrayIn != 0xffffffff) && (*pBitArrayIn >= maxBitNum) )
            {
            maxBitNum   = *pBitArrayIn;
            anyValid    = true;
            }
        }
    *highestFound = maxBitNum;
    return anyValid;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   11/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            BitMask::AllocBitArray (uint32_t numBits)
    {
    BeAssert (NULL == m_bitArray);

    uint32_t arraySize  = SHORTS_FROM_BITS (numBits);
    uint32_t arrayBytes = arraySize * sizeof (uint16_t);

    if (arrayBytes > 0)
        m_bitArray      = static_cast <uint16_t*> (bentleyAllocator_malloc(arrayBytes));

    m_numValidBits  = numBits;
#if defined (DEBUG_BITMASK)
    s_totalBitMaskMemoryMalloced += arrayBytes;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   11/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            BitMask::ReallocBitArray (uint32_t newNumBits)
    {
    if (NULL == m_bitArray)
        {
        // allocate.
        AllocBitArray (newNumBits);
        }
    else if (0 == newNumBits)
        {
        FreeBitArray ();
        }
    else
        {
        // change size.
        uint32_t oldArraySize = SHORTS_FROM_BITS (m_numValidBits) * sizeof (uint16_t);
        uint32_t newArraySize = SHORTS_FROM_BITS (newNumBits) * sizeof (uint16_t);

        if (oldArraySize != newArraySize)
            {
            m_bitArray      = static_cast <uint16_t*> (memutil_realloc (m_bitArray, newArraySize));
#if defined (DEBUG_BITMASK)
            s_totalBitMaskMemoryMalloced += (newArraySize - oldArraySize);
#endif
            }

        m_numValidBits      = newNumBits;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   11/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            BitMask::FreeBitArray ()
    {
    m_numValidBits = 0;

    if (NULL == m_bitArray)
        return;

#if defined (DEBUG_BITMASK)
    s_totalBitMaskMemoryMalloced -= (SHORTS_FROM_BITS (m_numValidBits) * sizeof (uint16_t));
#endif
    memutil_free (m_bitArray);
    m_bitArray  = NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   11/09
+---------------+---------------+---------------+---------------+---------------+------*/
BitMask::BitMask (bool defaultBitValue) : m_defaultBitValue (defaultBitValue)
    {
    m_bitArray     = NULL;
    m_numValidBits = 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   11/09
+---------------+---------------+---------------+---------------+---------------+------*/
BitMask::~BitMask ()
    {
    FreeBitArray();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   11/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            BitMask::Free ()
    {
    delete this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   11/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            BitMask::FreeAndClear (BitMaskH bitMaskH)
    {
    if (NULL == bitMaskH)
        return;

    BitMaskP    bitMask;
    if (NULL == (bitMask = *bitMaskH))
        return;
    bitMask->Free();

    *bitMaskH = NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   11/09
+---------------+---------------+---------------+---------------+---------------+------*/
BitMaskP        BitMask::Create (bool defaultBitValue)
    {
    return new BitMask (defaultBitValue);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   11/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool            BitMask::GetDefaultBitValue () const
    {
    return m_defaultBitValue;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   11/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            BitMask::SetDefaultBitValue (bool defaultBitValue)
    {
    if (defaultBitValue == m_defaultBitValue)
        return;

    // make sure all bits beyond the valid bits are set to the default value.
    uint16_t arrayIndex = static_cast <uint16_t> (SHORT_INDEX_FROM_BIT (m_numValidBits));
    unsigned int numBitsToSet = (16 - (m_numValidBits % 16)) % 16;
    uint16_t *pShort = &m_bitArray[arrayIndex];
    unsigned int iBit;
    uint16_t bitMask;
    for (iBit=0, bitMask = 1 << 15; iBit < numBitsToSet; iBit++, bitMask = bitMask >> 1)
        {
        if (defaultBitValue)
            *pShort |= bitMask;
        else
            *pShort &= ~bitMask;
        }

    m_defaultBitValue = defaultBitValue;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   11/09
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t        BitMask::GetCapacity () const
    {
    return m_numValidBits;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   11/09
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       BitMask::EnsureCapacity (uint32_t numBits)
    {
    // this method only increases the capacity - never decreases it.
    if (0xffffffff == numBits)
        return BSIERROR;

    if (m_numValidBits >= numBits)
        return SUCCESS;

    uint32_t    numShorts     = SHORTS_FROM_BITS (numBits);
    uint32_t    currentShorts = SHORTS_FROM_BITS (m_numValidBits);

    if (numShorts > currentShorts)
        {
        ReallocBitArray (numBits);
        memset ((m_bitArray + currentShorts), m_defaultBitValue ? 0xffff : 0, sizeof(uint16_t) * (numShorts - currentShorts));
        }
    else
        {
        // numBits increased, but doesn't require new array members.
        m_numValidBits = numBits;
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   11/09
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       BitMask::SetCapacity (uint32_t numBits)
    {
    if (numBits > m_numValidBits)
        return EnsureCapacity (numBits);

    // this method also makes the bit mask smaller.
    uint32_t    numShorts     = SHORTS_FROM_BITS (numBits);
    uint32_t    currentShorts = SHORTS_FROM_BITS (m_numValidBits);

    if (numShorts < currentShorts)
        {
        ReallocBitArray (numBits);
        }
    else if (numBits < m_numValidBits)
        {
        // numBits decreased, but same number of array members.
        m_numValidBits = numBits;
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   11/09
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       BitMask::SetBits (uint32_t arraySize, const uint32_t* bitIndexArray, bool value)
    {
    if ( (NULL == bitIndexArray) || (0 == arraySize) )
        return BSIERROR;

    // make sure we have enough bits.
    uint32_t highestBitIndex;
    if (!FindHighestBitInArray (&highestBitIndex, arraySize, bitIndexArray))
        return SUCCESS;

    // ensure that the bit array is large enough.
    EnsureCapacity (highestBitIndex+1);

    uint32_t        iArray;
    const uint32_t*   pBitIndex;
    for (iArray = 0, pBitIndex = bitIndexArray; iArray < arraySize; iArray++, pBitIndex++)
        {
        if (*pBitIndex != 0xffffffff)
            {
            int     arrayIndex  = SHORT_INDEX_FROM_BIT (*pBitIndex);
            int     mask        = SHORT_MASK_FROM_BIT (*pBitIndex);

            if (value)
                m_bitArray[arrayIndex] |= mask;
            else
                m_bitArray[arrayIndex] &= ~mask;
            }
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   11/09
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt  BitMask::SetBit (uint32_t bitIndex, bool value)
    {
    return SetBits (1, &bitIndex, value);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   11/09
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       BitMask::Set (uint32_t bitIndex)
    {
    return SetBits (1, &bitIndex, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   11/09
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       BitMask::Clear (uint32_t bitIndex)
    {
    return SetBits (1, &bitIndex, false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   11/09
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       BitMask::SetFromBitArray (uint32_t numValidBits, const uint16_t* pBitArrayIn)
    {
    if (numValidBits <= 0 || NULL == pBitArrayIn)
        {
        FreeBitArray ();
        return SUCCESS;
        }

    StatusInt   status;
    if (SUCCESS != (status = SetCapacity (numValidBits)))
        return  status;

    memcpy (m_bitArray, pBitArrayIn, sizeof(uint16_t) * SHORTS_FROM_BITS (numValidBits));

    // make sure all bits beyond the valid bits are set to the default value */
    unsigned int arrayIndex = SHORT_INDEX_FROM_BIT (numValidBits);
    uint16_t *pShort = &m_bitArray[arrayIndex];
    unsigned int numBitsToSet = (16 - (numValidBits % 16)) % 16;
    uint16_t bitMask;
    unsigned int iBit;
    for (iBit=0, bitMask = 1 << 15; iBit < numBitsToSet; iBit++, bitMask = bitMask >> 1)
        {
        if (m_defaultBitValue)
            *pShort |= bitMask;
        else
            *pShort &= ~bitMask;
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   11/09
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       BitMask::SetFromBitMask (BitMaskCR source)
    {
    m_defaultBitValue = source.m_defaultBitValue;
    return SetFromBitArray (source.m_numValidBits, source.m_bitArray);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   11/09
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       BitMask::SetByBitPosition (uint32_t bitArrayOffset, uint16_t bitPosition, bool value)
    {
    uint32_t currentArraySize = SHORTS_FROM_BITS (m_numValidBits);
    if (bitArrayOffset >= currentArraySize)
        return BSIERROR;

    if (value)
        m_bitArray[bitArrayOffset] |= bitPosition;
    else
        m_bitArray[bitArrayOffset] &= ~bitPosition;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   11/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            BitMask::SetAll (bool value)
    {
    uint16_t setValue = value ? 0xffff : 0;
    if (NULL != m_bitArray)
        memset (m_bitArray, setValue, sizeof(uint16_t) * SHORTS_FROM_BITS (m_numValidBits));

    // when we set all, we set the default value, too */
    m_defaultBitValue = value;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   11/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            BitMask::SetAll ()
    {
    SetAll (true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   11/09
+---------------+---------------+---------------+---------------+---------------+------*/
void BitMask::ClearAll ()
    {
    SetAll (false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   11/09
+---------------+---------------+---------------+---------------+---------------+------*/
void BitMask::SetFromString (Utf8StringCR sourceString, uint32_t indexOrigin, uint32_t maxIndexIn)
    {
    size_t  stringLen = sourceString.length();
    if (0 == stringLen)
        return;

    Utf8P  sourceStringCopy = static_cast<Utf8P>(_alloca ((stringLen+1) * sizeof(char)));
    strcpy (sourceStringCopy, sourceString.c_str());

    // step past white space */
    Utf8P pString = sourceStringCopy, pStringContext = NULL;
    while ((' ' == *pString) || ('\t' == *pString))
        pString++;

    Utf8CP    pLastChar = sourceStringCopy + stringLen;

    /* get the first substring into temp (substrings are n or n-m) */
    while ( (0 != *pString) && (*pString >= '0') && (*pString <= '9') && (NULL != (pString = BeStringUtilities::Strtok (pString, " ,", &pStringContext))) )
        {
        uint32_t startRange, endRange;

        /* extract "startRange" (and possibly "endRange") from the substring */
        int scanCount = BE_STRING_UTILITIES_UTF8_SSCANF (pString, "%u-%u", (uint32_t*) &startRange, (uint32_t*) &endRange);

        /* if "endRange" not specified, set it to be the same as "startRange" */
        if (scanCount < 2)
            endRange = startRange;

        /* make sure "startRange" is greater than or equal to "endRange" */
        if (endRange < startRange)
            {
            uint32_t    tempRange;
            tempRange  = endRange;
            endRange   = startRange;
            startRange = tempRange;
            }

        if (startRange < indexOrigin)
            startRange = indexOrigin;

        if (endRange < indexOrigin)
            endRange = indexOrigin;

        /* subtract "indexOriginIn" from m and n */
        startRange -= indexOrigin;
        endRange   -= indexOrigin;

        /* set all bits in the range */
        for (uint32_t iRange = startRange; iRange <= endRange; iRange++)
            {
            if (iRange < maxIndexIn)
                SetBits (1, &iRange, 1);
            }

        // point to the next token in pString */
        pString += strlen (pString);
        if (pString < pLastChar)
            pString++;

        Utf8String trimmedPString = pString;
        trimmedPString.Trim ();
        
        strcpy (pString, trimmedPString.c_str ());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   11/09
+---------------+---------------+---------------+---------------+---------------+------*/
void BitMask::ToString (Utf8StringR stringOut, uint32_t indexOrigin) const
    {
    stringOut = "";

    bool        bFirstPass = true, bCurrentRun = false;
    char        currentString[100];
    uint32_t    lastBit = 0, firstBitRange = 0, lastBitRange = 0, nextBit = 0;
    uint32_t    numBits = GetCapacity();

    for (uint32_t iBit = 0; iBit < numBits; iBit++)
        {
        if (!Test (iBit))
            continue;

        if (bFirstPass)
            {
            bFirstPass = false;

            firstBitRange = iBit;
            lastBit = iBit;
            nextBit = iBit;

            continue;
            }

        firstBitRange = lastBit;
        lastBitRange  = iBit;

        if (!bCurrentRun)
            nextBit = firstBitRange;

        if (nextBit == lastBitRange - 1)
            {
            bCurrentRun = true;
            nextBit = lastBitRange;
            }
        else
            {
            if (bCurrentRun)
                {
                BeStringUtilities::Snprintf (currentString, _countof(currentString), "%d-%d,", firstBitRange + indexOrigin, nextBit + indexOrigin);
                stringOut.append (currentString);
                bCurrentRun = false;
                }
            else
                {
                BeStringUtilities::Snprintf (currentString, _countof(currentString), "%d,", firstBitRange + indexOrigin);
                stringOut.append (currentString);
                }

            lastBit = iBit;
            firstBitRange = lastBit;
            }
        }

    if (bFirstPass)
        return;

    // cleanup leftover
    if (bCurrentRun)
        {
        BeStringUtilities::Snprintf (currentString, _countof(currentString), "%d-%d", firstBitRange + indexOrigin, lastBitRange + indexOrigin);
        stringOut.append (currentString);
        }
    else
        {
        BeStringUtilities::Snprintf (currentString, _countof(currentString), "%d", firstBitRange + indexOrigin);
        stringOut.append (currentString);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   11/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            BitMask::Dump
(
FILE*           pFileIn,        // => IN file to dump into */
bool            asString,       // => IN if true, then dump as a string, else dump as bits
uint32_t        indexOrigin
) const
    {
    if (!asString)
        {
        for (uint32_t iBit = 0; iBit < m_numValidBits; iBit++)
            {
            uint16_t *pShort = &m_bitArray[SHORT_INDEX_FROM_BIT(iBit)];

            if (*pShort & SHORT_MASK_FROM_BIT (iBit))
                toolSubsystem_fwprintf(pFileIn, L"1");
            else
                toolSubsystem_fwprintf(pFileIn, L"0");

            /* put a space every 10 bits */
            if (0 == (iBit + 1) % 16)
                toolSubsystem_fwprintf(pFileIn, L" ");
            /* put a line feed every 12 shorts */
            if (0 == (iBit + 1) % 120)
                toolSubsystem_fwprintf (pFileIn, L"\n");
            }
        }
    else
        {
        Utf8String     dumpString;
        ToString (dumpString, indexOrigin);

        if (dumpString.length() > 0)
            toolSubsystem_fprintf (pFileIn, "%ls", dumpString.c_str());
        else
            toolSubsystem_fprintf (pFileIn, "NULL");
        }

    toolSubsystem_fwprintf(pFileIn, L" numValidBits %3ld, defaultBitValue %d", m_numValidBits, m_defaultBitValue);
    toolSubsystem_fwprintf(pFileIn, L"\n");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   11/09
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       BitMask::Invert (uint32_t bitIndex)
    {
    if (bitIndex > m_numValidBits)
        return BSIERROR;

    uint32_t arrayIndex      = SHORT_INDEX_FROM_BIT (bitIndex);
    uint16_t bitPositionMask = SHORT_MASK_FROM_BIT (bitIndex);
    uint16_t *pShort         = &m_bitArray[arrayIndex];

    *pShort = *pShort ^ bitPositionMask;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   11/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            BitMask::InvertAll ()
    {
    uint32_t    numShorts = SHORTS_FROM_BITS (m_numValidBits);
    uint16_t*     pShort;
    uint32_t    iShort;

    for (iShort = 0, pShort = m_bitArray; iShort < numShorts; iShort++, pShort++)
        *pShort = ~(*pShort);

    // change its default value, too.
    m_defaultBitValue = !m_defaultBitValue;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   11/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool            BitMask::Test (uint32_t bitIndex) const
    {
    if (bitIndex < m_numValidBits)
        {
        int     arrayIndex = SHORT_INDEX_FROM_BIT (bitIndex);
        int     bitFlag = SHORT_MASK_FROM_BIT (bitIndex);

        return (0 != (m_bitArray[arrayIndex] & bitFlag)) ? true : false;
        }
    else
        {
        return m_defaultBitValue;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   11/09
+---------------+---------------+---------------+---------------+---------------+------*/
const uint16_t*   BitMask::GetBitArray () const
    {
    return m_bitArray;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   11/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool            BitMask::Test (BitMaskCR testMask) const
    {
    const uint16_t*   bitMaskP  = m_bitArray;
    const uint16_t*   testMaskP = testMask.m_bitArray;
    const uint16_t*   endBitMaskP;
    int     lastIndex = SHORTS_FROM_BITS (m_numValidBits);
    int     testIndex = SHORTS_FROM_BITS (testMask.m_numValidBits);

    if (testIndex < lastIndex)
        lastIndex = testIndex;

    for (endBitMaskP = bitMaskP + lastIndex; bitMaskP < endBitMaskP; bitMaskP++, testMaskP++)
        {
        if (0 != (*bitMaskP & *testMaskP))
            return true;
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   11/09
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t        BitMask::GetNumBitsSet () const
    {
    uint32_t    numBitsSet = 0;
    for (uint32_t iBitIndex = 0; iBitIndex < m_numValidBits; iBitIndex++)
        {
        if (Test (iBitIndex))
            numBitsSet++;
        }
    return numBitsSet;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   11/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool            BitMask::IsEqual (BitMaskCP other) const
    {
    if (this == other)
        return true;

    if (NULL == other)
        return false;

    if (m_numValidBits != other->m_numValidBits)
        return false;

    uint16_t *pShortArray1 = m_bitArray;
    uint16_t *pShortArray2 = other->m_bitArray;
    for (uint32_t iShort = 0, numShorts = SHORTS_FROM_BITS (m_numValidBits); iShort < numShorts; iShort++, pShortArray1++, pShortArray2++)
        {
        if (*pShortArray1 != *pShortArray2)
            return false;
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   11/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool            BitMask::AnyBitSet () const
    {
    if ( (NULL == m_bitArray) || (0 == m_numValidBits) )
        return m_defaultBitValue;


    // test the part of the array that is fully populated.
    uint32_t        lastFullIndex = m_numValidBits/16;
    const uint16_t*   bitMaskP      = m_bitArray;
    const uint16_t*   endBitMaskP;
    for (bitMaskP = m_bitArray, endBitMaskP = bitMaskP + lastFullIndex; bitMaskP < endBitMaskP; bitMaskP++)
        {
        if (0 != *bitMaskP)
            return true;
        }

    // if we got here, need to test the individual bits of the last bitMask.
    for (uint32_t iBit = lastFullIndex * sizeof(uint16_t); iBit < m_numValidBits; iBit++)
        {
        if (Test (iBit))
            return true;
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   11/09
+---------------+---------------+---------------+---------------+---------------+------*/
BitMaskP        BitMask::Clone (BitMaskCR source)
    {
    BitMaskP    pCopy = BitMask::Create (source.GetDefaultBitValue());

    pCopy->SetFromBitArray (source.GetCapacity(), source.GetBitArray());

    return pCopy;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   11/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            BitMask::LogicalOperation (BitMaskCR source, BitMaskOperation operation)
    {
    uint32_t        inputValidBits  = source.m_numValidBits;
    uint32_t        outputValidBits = m_numValidBits;

    /* if the defaultBitValue of the output bitMask is true, then widen unless BitMaskOperation::Or/BitMaskOperation::OrNot, which can never clear a bit */
    /* if the defaultBitValue of the output bitMask is false, then widen unless BitMaskOperation::And/BitMaskOperation::AndNot, which can never set a bit */
    if ( (m_defaultBitValue  && (operation != BitMaskOperation::Or ) && (operation != BitMaskOperation::OrNot ) ) ||
         (!m_defaultBitValue && (operation != BitMaskOperation::And) && (operation != BitMaskOperation::AndNot) ) )
        {
        if ( outputValidBits < inputValidBits )
            EnsureCapacity (inputValidBits);
        }

    // find the end of the loop */
    uint32_t        outputValidShorts;
    uint32_t        outputCount;
    outputCount = outputValidShorts = SHORTS_FROM_BITS (m_numValidBits);

    /* now we have to worry about the case where the input bitmask is narrower than the output bit mask */
    uint32_t        inputValidShorts;
    if (outputValidShorts > (inputValidShorts = SHORTS_FROM_BITS (inputValidBits)))
        {
        bool    outputCouldChangeFromDefaultBits = ( (operation == BitMaskOperation::Xor) ||
                                                    ((operation == BitMaskOperation::And || operation == BitMaskOperation::AndNot) && source.m_defaultBitValue == 0) ||
                                                    ((operation == BitMaskOperation::Or  || operation == BitMaskOperation::OrNot ) && source.m_defaultBitValue == 1) );
        if (!outputCouldChangeFromDefaultBits)
            outputCount = inputValidShorts;
        }

    uint16_t       *pOutEnd;
    const uint16_t *pInEnd;
    const uint16_t *pIn;
    uint16_t       *pOut;
    uint16_t        defaultInMask = (0 == source.m_defaultBitValue) ? 0 : 0xffff;
    for (pIn = source.m_bitArray, pOut = m_bitArray, pOutEnd = pOut + outputCount, pInEnd = pIn + inputValidShorts;
                pOut < pOutEnd; pOut++, pIn++)
        {
        uint16_t input = (pIn < pInEnd) ? *pIn : defaultInMask;

        if (operation == BitMaskOperation::Or)
            *pOut = *pOut | input;

        else if (operation == BitMaskOperation::Xor)
            *pOut = *pOut ^ input;

        else if (operation == BitMaskOperation::And)
            *pOut = *pOut & input;

        else if (operation == BitMaskOperation::OrNot)
            *pOut = *pOut | ~input;

        else if (operation == BitMaskOperation::AndNot)
            *pOut = *pOut & ~input;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   11/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            BitMask::Or (BitMaskCR source)
    {
    LogicalOperation (source, BitMaskOperation::Or);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   11/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            BitMask::And (BitMaskCR source)
    {
    LogicalOperation (source, BitMaskOperation::And);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   11/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            BitMask::XOr (BitMaskCR source)
    {
    LogicalOperation (source, BitMaskOperation::Xor);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   11/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            BitMask::AndNot (BitMaskCR source)
    {
    LogicalOperation (source, BitMaskOperation::AndNot);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   11/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            BitMask::OrNot (BitMaskCR source)
    {
    LogicalOperation (source, BitMaskOperation::OrNot);
    }

// NOTE: DO NOT change the BoolInt's in the two functions below to bool - the byte arrays created this way are stored persistently in V8 files!
/*----------------------------------------------------------------------------------*//**
* Copy the contents of the bitmask to a Byte array so it can be written to a file
* @bsimethod                    ChuckKirschman                  07/01
+---------------+---------------+---------------+---------------+---------------+------*/
Public StatusInt  bitMask_toFileByteArray
(
BitMaskCP       pBitMaskIn,           /* => IN bit mask array */
uint16_t*         pNumBytes,             /* <= OUT number of filled bytes in pByteArray */
Byte**          pByteArray            /* <= Pointer to place for buffer.  Up to caller to free. */
)
    {

    if (NULL != pByteArray)
        *pByteArray = NULL;

    if (NULL == pBitMaskIn || !pBitMaskIn->AnyBitSet ())
        {
        if (NULL != pNumBytes)
            *pNumBytes = 0;

        return (SUCCESS);
        }

    uint32_t numValidBits  = pBitMaskIn->GetCapacity();
    uint16_t bytesRequired = SHORTS_FROM_BITS (numValidBits) * sizeof(uint16_t) + sizeof(uint32_t) + sizeof (uint32_t) & 0xffff;

    if (NULL != pNumBytes)
        *pNumBytes = bytesRequired;

    if (NULL == pByteArray)
        return SUCCESS;

    Byte *pByte, *pBuffer;
    if (NULL == (pBuffer = (Byte *)bentleyAllocator_calloc (bytesRequired * (sizeof *pByte))))
        return BSIERROR;

    pByte = pBuffer;
    memcpy (pByte, &numValidBits, sizeof (uint32_t));
    pByte += sizeof (uint32_t);

    uint32_t defaultBitValue = pBitMaskIn->GetDefaultBitValue();
    memcpy (pByte, &defaultBitValue, sizeof (uint32_t));
    pByte += sizeof (uint32_t);

    memcpy (pByte, pBitMaskIn->GetBitArray(), SHORTS_FROM_BITS (numValidBits) * sizeof(uint16_t));
    *pByteArray = pBuffer;

    return SUCCESS;
    }

/*----------------------------------------------------------------------------------*//**
* Get the contents of the bitmask from a Byte array that was read from a file
* @bsimethod                    ChuckKirschman                  07/01
+---------------+---------------+---------------+---------------+---------------+------*/
Public StatusInt  bitMask_fromFileByteArray
(
BitMaskH        ppBitMaskOut,           /* => IN bit mask array */
const uint16_t  numBytes,               /* => IN number of bytes in pByteArray */
const Byte*     pByteArray              /* => IN buffer to hold bytes.  It should be numBytes long */
)
    {
    const Byte *pByte = pByteArray;

    if (NULL == ppBitMaskOut)
        return BSIERROR;

    *ppBitMaskOut = NULL;

    if (0 == numBytes)
        return SUCCESS;

    uint32_t numValidBits;
    memcpy (&numValidBits, pByte, sizeof (numValidBits));
    pByte += sizeof (numValidBits);

    uint32_t defaultBitValue;
    memcpy (&defaultBitValue, pByte, sizeof (uint32_t));
    pByte += sizeof (uint32_t);

    /* Some error checking - the required size must equal the number of bytes remaining */
    uint32_t numBytesRemaining = numBytes - sizeof (uint32_t) - sizeof (uint32_t);
    uint32_t bitMaskSizeBytes  = SHORTS_FROM_BITS (numValidBits) * sizeof(uint16_t);

    if (numBytesRemaining != bitMaskSizeBytes)
        return BSIERROR;

    *ppBitMaskOut = BitMask::Create (TO_BOOL (defaultBitValue));
    (*ppBitMaskOut)->SetFromBitArray (bitMaskSizeBytes * 8, (const uint16_t *)pByte);

    return SUCCESS;
    }

/*----------------------------------------------------------------------------------*//**
* Copy the contents of the bitmask to a Byte array so it can be written to a file
* @bsimethod                    ChuckKirschman                  07/01
+---------------+---------------+---------------+---------------+---------------+------*/
Public void     bitMask_freeFileByteArray
(
Byte**  pByteArray            /* => buffer to free */
)
    {
    if (NULL != pByteArray && NULL != *pByteArray)
        {
        memutil_free (*pByteArray);
        *pByteArray = NULL;
        }
    }


END_BENTLEY_NAMESPACE
