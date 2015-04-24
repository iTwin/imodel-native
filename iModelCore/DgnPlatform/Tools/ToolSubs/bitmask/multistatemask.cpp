/*--------------------------------------------------------------------------------------+
|
|     $Source: Tools/ToolSubs/bitmask/multistatemask.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <string.h>
#include <DgnPlatform/Tools/MultiStateMask.h>
#include <RmgrTools/Tools/memutil.h>
#include <RmgrTools/Tools/heapsig.h>
#include <RmgrTools/Tools/toolsubsStdio.h>

BENTLEY_API_TYPEDEFS (MultiStateMask)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Deepak.Malkan                   06/2006
+---------------+---------------+---------------+---------------+---------------+------*/
MultiStateMask::MultiStateMask (uint16_t numBitsPerStateIn, uint16_t defaultStateValueIn)
    {
    this->Initialize (numBitsPerStateIn, 0, NULL, defaultStateValueIn);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Deepak.Malkan                   06/2006
+---------------+---------------+---------------+---------------+---------------+------*/
MultiStateMask::MultiStateMask (uint16_t numBitsPerStateIn, uint32_t numStatesIn, uint16_t const * stateArrayIn, uint16_t defaultStateValueIn)
    {
    this->Initialize (numBitsPerStateIn, numStatesIn, stateArrayIn, defaultStateValueIn);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Deepak.Malkan                   06/2006
+---------------+---------------+---------------+---------------+---------------+------*/
MultiStateMask::MultiStateMask (MultiStateMaskCP copyMaskIn)
    {
    this->Initialize (copyMaskIn->m_numBitsPerState, 0, NULL, copyMaskIn->m_defaultStateValue);

    m_numStates  = copyMaskIn->m_numStates;
    m_stateArray = copyMaskIn->m_stateArray;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Deepak.Malkan                   06/2006
+---------------+---------------+---------------+---------------+---------------+------*/
void    MultiStateMask::Initialize (uint16_t numBitsPerStateIn, uint32_t numStatesIn, uint16_t const * stateArrayIn, uint16_t defaultStateValueIn)
    {
    BeAssert (numBitsPerStateIn <= 8);

    memset (this, 0, sizeof (*this));

    m_numBitsPerState = numBitsPerStateIn;
    m_defaultStateValue = defaultStateValueIn;
    m_maxStateValue = CalculateMaxStateValue (numBitsPerStateIn);
    m_numStatesPerWord = CalculateNumStatesPerWord (numBitsPerStateIn);

    if (numStatesIn > 0)
        {
        m_numStates = numStatesIn;

        size_t requiredArraySize = CalculateArraySizeForStates (numStatesIn, m_numStatesPerWord);
        m_stateArray.reserve (requiredArraySize);
        std::copy (stateArrayIn, stateArrayIn + requiredArraySize, back_inserter (m_stateArray));
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Deepak.Malkan                   06/2006
+---------------+---------------+---------------+---------------+---------------+------*/
void    MultiStateMask::FreeStateArray ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Deepak.Malkan                   06/2006
+---------------+---------------+---------------+---------------+---------------+------*/
uint16_t    MultiStateMask::GetDefaultWord () const
    {
    if (0 == m_defaultStateValue)
        return 0;

    if (m_defaultStateValue == this->GetMaxStateValue ())
        return 0xffff;

    uint16_t defaultWord = 0;

    for (int index = 0; index < m_numStatesPerWord; index++)
        {
        uint16_t indexMask = m_defaultStateValue;
        indexMask <<= index * m_numBitsPerState;
        defaultWord |= indexMask;
        }

    return defaultWord;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Deepak.Malkan                   06/2006
+---------------+---------------+---------------+---------------+---------------+------*/
void    MultiStateMask::EnsureCapacity (uint32_t numStatesIn)
    {
    if (numStatesIn <= 0)
        return;

    size_t requiredArraySize = CalculateArraySizeForStates (numStatesIn, m_numStatesPerWord);
    size_t currentArraySize  = m_stateArray.size ();
    if (requiredArraySize > currentArraySize)
        {
        uint16_t defaultWord = GetDefaultWord ();
        // Resize and initialize new items with default words.
        m_stateArray.resize (requiredArraySize, defaultWord);
        m_numStates = numStatesIn;
        return;
        }
    
    // Already has the capacity, simply change the m_numStates.
    if (numStatesIn > m_numStates)
        m_numStates = numStatesIn;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Deepak.Malkan                   06/2006
+---------------+---------------+---------------+---------------+---------------+------*/
void    MultiStateMask::EnsureExactCapacity (uint32_t numStatesIn)
    {
    EnsureCapacity (numStatesIn);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Deepak.Malkan                   06/2006
+---------------+---------------+---------------+---------------+---------------+------*/
uint16_t  MultiStateMask::GetState (uint32_t stateIndexIn) const
    {
    if (stateIndexIn >= m_numStates)
        return m_defaultStateValue;

    uint32_t arrayIndex = this->GetArrayIndex (stateIndexIn);

    uint16_t wordIndex = this->GetWordIndex (stateIndexIn);
    uint16_t bitsOffset = wordIndex * m_numBitsPerState;

    uint16_t maxValue = this->GetMaxStateValue ();
    uint16_t getMask = maxValue << bitsOffset;

    uint16_t offsetState = m_stateArray[arrayIndex] & getMask;
    uint16_t state = offsetState >> bitsOffset;
    BeAssert (state <= m_maxStateValue);

    return state;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Deepak.Malkan                   06/2006
+---------------+---------------+---------------+---------------+---------------+------*/
void        MultiStateMask::SetState (uint32_t stateIndexIn, uint16_t stateValueIn)
    {
    this->EnsureCapacity (stateIndexIn + 1);

    if (stateValueIn > m_maxStateValue)
        {
        BeAssert(0);
        stateValueIn = m_maxStateValue;
        }

    uint32_t arrayIndex = this->GetArrayIndex (stateIndexIn);

    uint16_t wordIndex = this->GetWordIndex (stateIndexIn);
    uint16_t bitsOffset = wordIndex * m_numBitsPerState;

    uint16_t clearMask = this->GetMaxStateValue();
    clearMask <<= bitsOffset;
    clearMask = ~clearMask;
    m_stateArray[arrayIndex] &= clearMask;

    uint16_t setMask = stateValueIn << bitsOffset;
    m_stateArray[arrayIndex] |= setMask;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Deepak.Malkan                   11/2006
+---------------+---------------+---------------+---------------+---------------+------*/
bool      MultiStateMask::IsEqual (MultiStateMask * maskIn)
    {
    if (NULL == maskIn)
        return false;

    if (this == maskIn)
        return true;

    if (m_numBitsPerState != maskIn->m_numBitsPerState)
        return false;

    if (m_numStates != maskIn->m_numStates)
        return false;

    StateArray::iterator lit = m_stateArray.begin ();
    StateArray::iterator rit = maskIn->m_stateArray.begin ();

    for (;lit != m_stateArray.end (); ++lit, ++rit)
        if (*lit != *rit)
            return false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool        MultiStateMask::IsEquivalent (MultiStateMaskCP other, uint16_t dontCareValue) const
    {
    if (NULL == other)
        return false;

    if (this == other)
        return true;

    if (m_numBitsPerState != other->m_numBitsPerState)
        return false;

    // if there are more states in one mask than the other, but all of the states in the larger
    // mask beyond those in the smaller mask are the "don't care" state, they're the same.
    uint32_t maxFullWordsThis    = m_numStates / m_numStatesPerWord;
    uint32_t maxFullWordsOther   = other->m_numStates / other->m_numStatesPerWord;

    uint32_t fullWords = (maxFullWordsThis < maxFullWordsOther) ? maxFullWordsThis : maxFullWordsOther;
    for (uint32_t wordIndex = 0; wordIndex < fullWords; wordIndex++)
        {
        if (m_stateArray[wordIndex] != other->m_stateArray[wordIndex])
            return false;
        }

    bool    thisHasLessStates   = m_numStates < other->m_numStates;
    uint32_t leastNumStates      = thisHasLessStates ? m_numStates : other->m_numStates;
    uint32_t maxNumStates        = thisHasLessStates ? other->m_numStates : m_numStates;

    // check the bits above those we were able to check by comparing entire words.
    for (uint32_t stateIndex = fullWords * m_numStatesPerWord; stateIndex < leastNumStates; stateIndex++)
        {
        if (GetState (stateIndex) != other->GetState (stateIndex) )
            return false;
        }

    // all of the bits in the larger array, beyond those in the smaller array, must be the dontCareValue.
    MultiStateMaskCP    checkForDontCare = thisHasLessStates ? other : this;
    for (uint32_t stateIndex = leastNumStates; stateIndex < maxNumStates; stateIndex++)
        {
        if (dontCareValue != checkForDontCare->GetState (stateIndex))
            return false;
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Deepak.Malkan                   06/2006
+---------------+---------------+---------------+---------------+---------------+------*/
void    MultiStateMask::Dump (FILE * fileIn)
    {
    if (NULL == fileIn)
        return;

    toolSubsystem_fwprintf (fileIn, L" numBitsPerState %d, defaultValue %d, numStates %d ", m_numBitsPerState, m_defaultStateValue, m_numStates);
    // Calculate a quick hash-code
    int64_t hashCode = 0;
    for (uint32_t index = 0; index < m_numStates; index++)
        {
        uint16_t state = this->GetState (index);
        hashCode += index * state;
        }
    fwprintf (fileIn, L"hashCode %lld\n", hashCode);
    
    if (m_numStates > 48)
        {
        toolSubsystem_fwprintf (fileIn, L"\n    ");
        }

    for (uint32_t index = 0; index < m_numStates; index++)
        {
        uint16_t state = this->GetState (index);
        toolSubsystem_fwprintf (fileIn, L"%d", state);

        /* put a space every 16 states */
        if (0 == (index + 1) % 16)
            toolSubsystem_fwprintf (fileIn, L" ");
        /* put a line feed every 6 words */
        if (0 == (index + 1) % (6*16))
            toolSubsystem_fwprintf (fileIn, L"\n    ");
        }

    toolSubsystem_fwprintf (fileIn, L"\n");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Yogesh.Sajanikar 03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void MultiStateMask::CopyTo (uint16_t *pArray)
    {
    std::copy (m_stateArray.begin (), m_stateArray.end (), pArray);
    }

