/*--------------------------------------------------------------------------------------+
|
|     $Source: Tools/ToolSubs/bitmask/multistatemask.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <string.h>
#include <DgnPlatform/Tools/MultiStateMask.h>
#include <RmgrTools/Tools/memutil.h>
#include <RmgrTools/Tools/heapsig.h>
#include <RmgrTools/Tools/toolsubsStdio.h>


/*======================================================================+
|                                                                       |
|   Public Code Section                                                 |
|                                                                       |
+======================================================================*/
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Deepak.Malkan                   06/2006
+---------------+---------------+---------------+---------------+---------------+------*/
MultiStateMask::MultiStateMask (UInt16 numBitsPerStateIn, UInt16 defaultStateValueIn)
    {
    this->Initialize (numBitsPerStateIn, 0, NULL, defaultStateValueIn);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Deepak.Malkan                   06/2006
+---------------+---------------+---------------+---------------+---------------+------*/
MultiStateMask::MultiStateMask (UInt16 numBitsPerStateIn, UInt32 numStatesIn, UInt16 const * stateArrayIn, UInt16 defaultStateValueIn)
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
void    MultiStateMask::Initialize (UInt16 numBitsPerStateIn, UInt32 numStatesIn, UInt16 const * stateArrayIn, UInt16 defaultStateValueIn)
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
UInt16      MultiStateMask::GetDefaultWord () const
    {
    if (0 == m_defaultStateValue)
        return 0;

    if (m_defaultStateValue == this->GetMaxStateValue ())
        return 0xffff;

    UInt16  defaultWord = 0;

    for (int index = 0; index < m_numStatesPerWord; index++)
        {
        UInt16  indexMask = m_defaultStateValue;
        indexMask <<= index * m_numBitsPerState;
        defaultWord |= indexMask;
        }

    return defaultWord;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Deepak.Malkan                   06/2006
+---------------+---------------+---------------+---------------+---------------+------*/
void    MultiStateMask::EnsureCapacity (UInt32 numStatesIn)
    {
    if (numStatesIn <= 0)
        return;

    size_t requiredArraySize = CalculateArraySizeForStates (numStatesIn, m_numStatesPerWord);
    size_t currentArraySize  = m_stateArray.size ();
    if (requiredArraySize > currentArraySize)
        {
        UInt16 defaultWord = GetDefaultWord ();
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
void    MultiStateMask::EnsureExactCapacity (UInt32 numStatesIn)
    {
    EnsureCapacity (numStatesIn);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Deepak.Malkan                   06/2006
+---------------+---------------+---------------+---------------+---------------+------*/
UInt16    MultiStateMask::GetState (UInt32 stateIndexIn) const
    {
    if (stateIndexIn >= m_numStates)
        return m_defaultStateValue;

    UInt32  arrayIndex = this->GetArrayIndex (stateIndexIn);

    UInt16  wordIndex = this->GetWordIndex (stateIndexIn);
    UInt16  bitsOffset = wordIndex * m_numBitsPerState;

    UInt16  maxValue = this->GetMaxStateValue ();
    UInt16  getMask = maxValue << bitsOffset;

    UInt16  offsetState = m_stateArray[arrayIndex] & getMask;
    UInt16  state = offsetState >> bitsOffset;
    BeAssert (state <= m_maxStateValue);

    return state;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Deepak.Malkan                   06/2006
+---------------+---------------+---------------+---------------+---------------+------*/
void        MultiStateMask::SetState (UInt32 stateIndexIn, UInt16 stateValueIn)
    {
    this->EnsureCapacity (stateIndexIn + 1);

    if (stateValueIn > m_maxStateValue)
        {
        BeAssert(0);
        stateValueIn = m_maxStateValue;
        }

    UInt32  arrayIndex = this->GetArrayIndex (stateIndexIn);

    UInt16  wordIndex = this->GetWordIndex (stateIndexIn);
    UInt16  bitsOffset = wordIndex * m_numBitsPerState;

    UInt16  clearMask = this->GetMaxStateValue();
    clearMask <<= bitsOffset;
    clearMask = ~clearMask;
    m_stateArray[arrayIndex] &= clearMask;

    UInt16  setMask = stateValueIn << bitsOffset;
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
bool        MultiStateMask::IsEquivalent (MultiStateMaskCP other, UInt16 dontCareValue) const
    {
    if (NULL == other)
        return false;

    if (this == other)
        return true;

    if (m_numBitsPerState != other->m_numBitsPerState)
        return false;

    // if there are more states in one mask than the other, but all of the states in the larger
    // mask beyond those in the smaller mask are the "don't care" state, they're the same.
    UInt32  maxFullWordsThis    = m_numStates / m_numStatesPerWord;
    UInt32  maxFullWordsOther   = other->m_numStates / other->m_numStatesPerWord;

    UInt32  fullWords = (maxFullWordsThis < maxFullWordsOther) ? maxFullWordsThis : maxFullWordsOther;
    for (UInt32 wordIndex = 0; wordIndex < fullWords; wordIndex++)
        {
        if (m_stateArray[wordIndex] != other->m_stateArray[wordIndex])
            return false;
        }

    bool    thisHasLessStates   = m_numStates < other->m_numStates;
    UInt32  leastNumStates      = thisHasLessStates ? m_numStates : other->m_numStates;
    UInt32  maxNumStates        = thisHasLessStates ? other->m_numStates : m_numStates;

    // check the bits above those we were able to check by comparing entire words.
    for (UInt32 stateIndex = fullWords * m_numStatesPerWord; stateIndex < leastNumStates; stateIndex++)
        {
        if (GetState (stateIndex) != other->GetState (stateIndex) )
            return false;
        }

    // all of the bits in the larger array, beyond those in the smaller array, must be the dontCareValue.
    MultiStateMaskCP    checkForDontCare = thisHasLessStates ? other : this;
    for (UInt32 stateIndex = leastNumStates; stateIndex < maxNumStates; stateIndex++)
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
    Int64   hashCode = 0;
    for (UInt32 index = 0; index < m_numStates; index++)
        {
        UInt16  state = this->GetState (index);
        hashCode += index * state;
        }
    fwprintf (fileIn, L"hashCode %lld\n", hashCode);
    
    if (m_numStates > 48)
        {
        toolSubsystem_fwprintf (fileIn, L"\n    ");
        }

    for (UInt32 index = 0; index < m_numStates; index++)
        {
        UInt16  state = this->GetState (index);
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
void MultiStateMask::CopyTo (UInt16 *pArray)
    {
    std::copy (m_stateArray.begin (), m_stateArray.end (), pArray);
    }

