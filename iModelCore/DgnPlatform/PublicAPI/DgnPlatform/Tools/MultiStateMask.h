/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/Tools/MultiStateMask.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

#include <math.h>
#include <vector>

BEGIN_BENTLEY_API_NAMESPACE
/*=================================================================================**//**
* @bsiclass                                                     Deepak.Malkan   06/2006
+===============+===============+===============+===============+===============+======*/
struct  MultiStateMask
{
typedef bvector<UInt16> StateArray;

private:
    UInt16          m_numBitsPerState;      /* Determines the max. number of possible states */
    UInt16          m_defaultStateValue;    /* default value for states beyond end of array */
    UInt16          m_numStatesPerWord;     /* Number of states that are stored in one word (short) */
    UInt16          m_maxStateValue;        /* Maximum value of an individual state */
    UInt32          m_numStates;            /* Number of states in array */
    StateArray      m_stateArray;           /* Array of words (shorts) - each word stores [16/m_numBitsPerState] number of states */

private:
    inline static UInt16    GetWordSizeInBits () { return sizeof (UInt16) * 8; }
    inline static UInt32    CalculateArraySizeForStates (UInt32 numStatesIn, UInt16 numStatesPerWordIn) { return (numStatesIn + numStatesPerWordIn - 1) / numStatesPerWordIn; }
    inline static UInt16    CalculateMaxStateValue (UInt16 numBitsPerStateIn) { return ((0x1 << numBitsPerStateIn) - 1); }
    inline static UInt16    CalculateNumStatesPerWord (UInt16 numBitsPerStateIn) { return GetWordSizeInBits () / numBitsPerStateIn; }

private:
    inline UInt16           GetWordIndex (UInt32 stateIndexIn) const { return stateIndexIn % m_numStatesPerWord; }
    inline UInt32           GetArrayIndex (UInt32 stateIndexIn) const { return stateIndexIn / m_numStatesPerWord; }

    void                    Initialize (UInt16 numBitsPerStateIn, UInt32 numStatesIn, UInt16 const * stateArrayIn, UInt16 defaultStateValueIn);
    void                    FreeStateArray ();

    UInt16                  GetDefaultWord () const;

public:
    DGNPLATFORM_EXPORT MultiStateMask (UInt16 numBitsPerStateIn, UInt16 defaultStateValueIn);
    DGNPLATFORM_EXPORT MultiStateMask (UInt16 numBitsPerStateIn, UInt32 numStatesIn, UInt16 const * stateArrayIn, UInt16 defaultStateValueIn);
    DGNPLATFORM_EXPORT MultiStateMask (MultiStateMaskCP copyMaskIn);

    inline UInt16           GetNumBitsPerState () const { return m_numBitsPerState; }
    inline UInt16           GetDefaultStateValue () const { return m_defaultStateValue; }
    inline UInt16           GetMaxStateValue () const { return m_maxStateValue; }
    inline UInt32           GetNumStates () const { return m_numStates; }
    inline UInt32           CalculateArraySize () const { return CalculateArraySizeForStates (m_numStates, m_numStatesPerWord); }

    DGNPLATFORM_EXPORT void                    EnsureCapacity (UInt32 numStatesIn);
    DGNPLATFORM_EXPORT void                    EnsureExactCapacity (UInt32 numStatesIn);

    DGNPLATFORM_EXPORT UInt16                  GetState (UInt32 stateIndexIn) const;
    DGNPLATFORM_EXPORT void                    SetState (UInt32 stateIndexIn, UInt16 stateValueIn);

    DGNPLATFORM_EXPORT bool                    IsEqual (MultiStateMask * maskIn);
    DGNPLATFORM_EXPORT bool                    IsEquivalent (MultiStateMaskCP maskIn, UInt16 dontCareState) const;

    DGNPLATFORM_EXPORT void                    Dump (FILE * fileIn);
    DGNPLATFORM_EXPORT void                    CopyTo (UInt16 *pArrayOut);

};
END_BENTLEY_API_NAMESPACE
