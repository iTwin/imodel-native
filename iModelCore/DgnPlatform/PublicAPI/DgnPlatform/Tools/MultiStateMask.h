/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/Tools/MultiStateMask.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

#include <math.h>
#include <vector>

BEGIN_BENTLEY_NAMESPACE
/*=================================================================================**//**
* @bsiclass                                                     Deepak.Malkan   06/2006
+===============+===============+===============+===============+===============+======*/
struct  MultiStateMask
{
typedef bvector<uint16_t> StateArray;

private:
    uint16_t        m_numBitsPerState;      /* Determines the max. number of possible states */
    uint16_t        m_defaultStateValue;    /* default value for states beyond end of array */
    uint16_t        m_numStatesPerWord;     /* Number of states that are stored in one word (short) */
    uint16_t        m_maxStateValue;        /* Maximum value of an individual state */
    uint32_t        m_numStates;            /* Number of states in array */
    StateArray      m_stateArray;           /* Array of words (shorts) - each word stores [16/m_numBitsPerState] number of states */

private:
    inline static uint16_t  GetWordSizeInBits () { return sizeof (uint16_t) * 8; }
    inline static uint32_t  CalculateArraySizeForStates (uint32_t numStatesIn, uint16_t numStatesPerWordIn) { return (numStatesIn + numStatesPerWordIn - 1) / numStatesPerWordIn; }
    inline static uint16_t  CalculateMaxStateValue (uint16_t numBitsPerStateIn) { return ((0x1 << numBitsPerStateIn) - 1); }
    inline static uint16_t  CalculateNumStatesPerWord (uint16_t numBitsPerStateIn) { return GetWordSizeInBits () / numBitsPerStateIn; }

private:
    inline uint16_t         GetWordIndex (uint32_t stateIndexIn) const { return stateIndexIn % m_numStatesPerWord; }
    inline uint32_t         GetArrayIndex (uint32_t stateIndexIn) const { return stateIndexIn / m_numStatesPerWord; }

    void                    Initialize (uint16_t numBitsPerStateIn, uint32_t numStatesIn, uint16_t const * stateArrayIn, uint16_t defaultStateValueIn);
    void                    FreeStateArray ();

    uint16_t                GetDefaultWord () const;

public:
    DGNPLATFORM_EXPORT MultiStateMask (uint16_t numBitsPerStateIn, uint16_t defaultStateValueIn);
    DGNPLATFORM_EXPORT MultiStateMask (uint16_t numBitsPerStateIn, uint32_t numStatesIn, uint16_t const * stateArrayIn, uint16_t defaultStateValueIn);
    DGNPLATFORM_EXPORT MultiStateMask (MultiStateMask const* copyMaskIn);

    inline uint16_t         GetNumBitsPerState () const { return m_numBitsPerState; }
    inline uint16_t         GetDefaultStateValue () const { return m_defaultStateValue; }
    inline uint16_t         GetMaxStateValue () const { return m_maxStateValue; }
    inline uint32_t         GetNumStates () const { return m_numStates; }
    inline uint32_t         CalculateArraySize () const { return CalculateArraySizeForStates (m_numStates, m_numStatesPerWord); }

    DGNPLATFORM_EXPORT void                    EnsureCapacity (uint32_t numStatesIn);
    DGNPLATFORM_EXPORT void                    EnsureExactCapacity (uint32_t numStatesIn);

    DGNPLATFORM_EXPORT uint16_t                GetState (uint32_t stateIndexIn) const;
    DGNPLATFORM_EXPORT void                    SetState (uint32_t stateIndexIn, uint16_t stateValueIn);

    DGNPLATFORM_EXPORT bool                    IsEqual (MultiStateMask * maskIn);
    DGNPLATFORM_EXPORT bool                    IsEquivalent (MultiStateMask const* maskIn, uint16_t dontCareState) const;

    DGNPLATFORM_EXPORT void                    Dump (FILE * fileIn);
    DGNPLATFORM_EXPORT void                    CopyTo (uint16_t *pArrayOut);

};
END_BENTLEY_NAMESPACE
