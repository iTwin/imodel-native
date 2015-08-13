/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/Tools/BitMask.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <DgnPlatform/DgnPlatform.h>

#include <stdio.h>

BEGIN_BENTLEY_NAMESPACE

enum class BitMaskOperation
    {
    Or          = 1,
    And         = 2,
    Xor         = 3,
    OrNot       = 4,
    AndNot      = 5,
    };

/*=================================================================================**//**
 A BitMask holds a variable-sized array of bits.

 BitMasks are used when a variable-sized array of on/off values is needed. For example,
 each ViewController contains a BitMask (for each displayed model) that contains a bit for each level
 that indicates whether the level is to be displayed or not.

* @bsiclass
+===============+===============+===============+===============+===============+======*/
typedef BitMaskP* BitMaskH;

/*=================================================================================**//**
 A Bitmask holds a variable-sized array of bits.

 BitMask stores the capacity of the bitmask (see #GetCapacity and #EnsureCapacity), and a default
 value (see #GetDefaultBitValue and #SetDefaultBitValue) that is used if the value of
 a bit higher than the capacity is needed.

 BitMasks are created by the static #Create and #Clone methods, and must be freed using the #Free
 method. The static #FreeAndClear method can be used to check for a NULL BitMask before freeing.

 @bsiclass                                                     Barry.Bentley   11/09
+===============+===============+===============+===============+===============+======*/
struct      BitMask : NonCopyableClass
{
private:
#if defined (DEBUG_BITMASK)
    static size_t       s_totalBitMaskMemoryMalloced;
#endif

    uint16_t*             m_bitArray;
    uint32_t            m_numValidBits;
    bool                m_defaultBitValue;    /* default value for bits beyond end of array */

    BitMask (bool defaultBitValue);
    ~BitMask();

    void                                AllocBitArray       (uint32_t numBits);
    void                                ReallocBitArray     (uint32_t newNumBits);
    void                                FreeBitArray ();

public:
    DGNPLATFORM_EXPORT const uint16_t*        GetBitArray() const;
    DGNPLATFORM_EXPORT StatusInt            SetByBitPosition (uint32_t arrayIndex, uint16_t bitPosition, bool value);
    DGNPLATFORM_EXPORT void                 SetAll (bool value);

public:

//! Creates a new BitMask instance.
//! @param[in]     defaultBitValue  The value used for bits higher than the BitMask capacity (see #GetCapacity)
//! @return        The newly created BitMask.
//! @note          The initial capacity is zero.<br>The caller is responsible for freeing the BitMask using the #Free method.
//! @see           EnsureCapacity.
DGNPLATFORM_EXPORT static BitMaskP     Create (bool defaultBitValue);

//! Creates a new BitMask instance from an existing BitMask.
//! @param[in]     source           The BitMask that is copied.
//! @return        The copied BitMask.
//! @note          The caller is responsible for freeing the BitMask using the #Free method.
//! @see           SetFromBitMask.
DGNPLATFORM_EXPORT static BitMaskP     Clone (BitMaskCR source);

//! Convenience method to Free a BitMask.
//! @param[in]     bitMask          A pointer to a BitMask that is is to be freed. The pointer can point to a NULL value.
//!                                 The value of the pointer is set to zero after the BitMask is freed.
DGNPLATFORM_EXPORT static void         FreeAndClear (BitMaskH bitMask);

//! Frees this BitMask
DGNPLATFORM_EXPORT void                Free ();

//! Gets the capacity of this BitMask.
//! @return        The capacity of this BitMask.
//! @see           EnsureCapacity, SetCapacity
DGNPLATFORM_EXPORT uint32_t            GetCapacity () const;

//! Sets this BitMask's capacity to at least the value specified.
//! @param[in]      numBits         The number of bits required.
//! @note           If the current capacity is greater than or equal to numBits, the call does nothing.
//!                 <br>If the current capacity is less than numBits, the newly added bits are set to the default value.
//! @see            SetCapacity, GetCapacity.
DGNPLATFORM_EXPORT StatusInt           EnsureCapacity (uint32_t numBits);

//! Sets this BitMasks' capacity to exactly the value specified.
//! @param[in]      numBits         The number of bits required.
//! @note           If the current capacity is less than numBits, the newly added bits are set to the default value.
//! @see            EnsureCapacity, GetCapacity.
DGNPLATFORM_EXPORT StatusInt           SetCapacity (uint32_t numBits);

//! Gets this BitMask's default bit value.
//! @return          The value of the default bit.
//! @see            SetDefaultBitValue.
DGNPLATFORM_EXPORT bool                GetDefaultBitValue () const;

//! Sets this BitMask's default bit value.
//! @param[in]      value           The default bit value.
//! @see            GetDefaultBitValue.
DGNPLATFORM_EXPORT void                SetDefaultBitValue (bool value);

//! Tests a bit of this BitMask.
//! @param[in]      bitIndex        The bit to test.
//! @return         The value of the bit.
DGNPLATFORM_EXPORT bool                Test (uint32_t bitIndex) const;

//! Tests the specified bits of this BitMask.
//! @param[in]      testMask        The bits to test.
//! @return         True if any of the true bits in testMask are also true in this BitMask.
DGNPLATFORM_EXPORT bool                Test (BitMaskCR testMask) const;


//! Sets a bit of this BitMask to the specified value.
//! @param[in]      bitIndex        The bit to set.
//! @param[in]      value           The value to set.
DGNPLATFORM_EXPORT StatusInt            SetBit (uint32_t bitIndex, bool value);

//! Sets a bit of this BitMask to true.
//! @param[in]      bitIndex        The bit to set to true.
DGNPLATFORM_EXPORT StatusInt           Set (uint32_t bitIndex);

//! Sets all bits of this BitMask to true.
DGNPLATFORM_EXPORT void                SetAll ();

//! Sets a bit of this BitMask to false.
//! @param[in]      bitIndex        The bit to set to false.
DGNPLATFORM_EXPORT StatusInt           Clear (uint32_t bitIndex);

//! Sets all bits of this BitMask to false.
DGNPLATFORM_EXPORT void                ClearAll ();

//! Inverts the specified bit of this BitMask.
//! @param[in]      bitIndex        The bit to set to invert.
DGNPLATFORM_EXPORT StatusInt           Invert (uint32_t bitIndex);

//! Inverts all bits of this BitMask.
DGNPLATFORM_EXPORT void                InvertAll ();

//! Sets this BitMask to the logical Or of this Bitmask and the argument.
//! @param[in]      rhs             The right hand operand of the Or operation.
//! @note           If necessary, the capacity of this BitMask is increased to that of rhs.
DGNPLATFORM_EXPORT void                Or (BitMaskCR rhs);

//! Sets this BitMask to the logical And of this Bitmask and the argument.
//! @param[in]      rhs             The right hand operand of the And operation.
//! @note           If necessary, the capacity of this BitMask is increased to that of rhs.
DGNPLATFORM_EXPORT void                And (BitMaskCR rhs);

//! Sets this BitMask to the logical Xor of this Bitmask and the argument.
//! @param[in]      rhs             The right hand operand of the XOr operation.
//! @note           If necessary, the capacity of this BitMask is increased to that of rhs.
DGNPLATFORM_EXPORT void                XOr (BitMaskCR rhs);

//! Sets this BitMask to the logical And this Bitmask and the insverse of the argument.
//! @param[in]      rhs             The right hand operand of the AndNot operation.
//! @note           If necessary, the capacity of this BitMask is increased to that of rhs.
DGNPLATFORM_EXPORT void                AndNot (BitMaskCR rhs);

//! Sets this BitMask to the logical Or of this Bitmask and the inverse the argument.
//! @param[in]      rhs             The right hand operand of the OrNot operation.
//! @note           If necessary, the capacity of this BitMask is increased to that of rhs.
DGNPLATFORM_EXPORT void                OrNot (BitMaskCR rhs);

//! Sets the bits specified to the value specified.
//! @param[in]      arraySize       The number of entries in bitIndexArray.
//! @param[in]      bitIndexArray   An array of bit indexes.
//! @param[in]      value           The value to set.
//! @note           If necessary, the capacity of this BitMask is increased.
DGNPLATFORM_EXPORT StatusInt           SetBits (uint32_t arraySize, const uint32_t *bitIndexArray, bool value);

//! Copies the contents of the given BitMask to this BitMask.
//! @param[in]      source          The BitMask to copy the contents of.
DGNPLATFORM_EXPORT StatusInt           SetFromBitMask (BitMaskCR source);

//! Sets this BitMask from source data.
//! @param[in]      numValidBits    The number of valid bits in the input bitArray
//! @param[in]      bitArray        An array of at least (numValidBits+15)%16 UInt16's that contain the values.
//! @note           If necessary, the capacity of this BitMask is increased.
DGNPLATFORM_EXPORT StatusInt           SetFromBitArray (uint32_t numValidBits, const uint16_t* bitArray);

//! Sets this BitMask from a string containing the bits that are true.
//! @param[in]      sourceString    The string containing integers that are the bits to set to true. It is of the form n,m,...p-q,r-s
//! @param[in]      indexOrigin     Either 0 or 1. The value is subtracted from each integer specified in sourceString.
//! @param[in]      maxIndex        The maximum value that is allowed in sourceString.
DGNPLATFORM_EXPORT void SetFromString (Utf8StringCR sourceString, uint32_t indexOrigin, uint32_t maxIndex);

//! The count of bits that are true in this BitMask.
//! @return         The number of true bits.
DGNPLATFORM_EXPORT uint32_t            GetNumBitsSet () const;

//! Tests this BitMask for equality with another BitMask.
//! @param[in]      other           The other BitMask
//! @return         true if the two BitMasks are equal.
//! @note           The Capacity of the two BitMasks must be equal, but it is not necessary for the DefaultBitValues to be the same.
DGNPLATFORM_EXPORT bool                IsEqual (BitMaskCP other) const;

//! Returns true if any bit in this BitMask is true.
//! @return         true if any bit is true.
//! @note           This tests bits up to the Capacity of the BitMask. If Capacity is 0, it returns the DefaultBitValue.
DGNPLATFORM_EXPORT bool                AnyBitSet () const;

//! Sets this BitMask to the logical operation specified between this BitMask and the argument.
//! @param[in]      rhs             The right hand operand of the And operation.
//! @param[in]      operation       The operation to perform.
//! @note           If necessary, the capacity of this BitMask is increased.
DGNPLATFORM_EXPORT void                LogicalOperation (BitMaskCR rhs, BitMaskOperation operation);

//! Dumps the contents of this BitMask as a file to specified file.
//! @param[in]      file            The file to which the output is dumped.
//! @param[in]      asString        The output is a string if true, binary otherwise.
//! @param[in]      indexOrigin     If asString true, this indicates the index value of the lowest bit. Must be 0 or 1.
DGNPLATFORM_EXPORT void                Dump (FILE *file, bool asString, uint32_t indexOrigin) const;


//! Create a string representing the contents of this BitMask.
//! @param[out]     outString       The string that is filled in.
//! @param[in]      indexOrigin     If asString true, this indicates the index value of the lowest bit. Must be 0 or 1.
DGNPLATFORM_EXPORT void ToString (Utf8StringR outString, uint32_t indexOrigin) const;
};

/*=================================================================================**//**
 BitMaskHolder acts as a smart pointer. Create a BitMaskHolder as a stack variable and it
 automatically cleans up the BitMask memory when it goes out of scope. 
+===============+===============+===============+===============+===============+======*/
struct BitMaskHolder 
{
private:
    BitMask*    m_mask;

public:
//! Construct a new Instance of BitMaskHolder.
//! @param[in]     defaultBitValue  The value used for bits higher than the BitMask capacity.
explicit BitMaskHolder (bool defaultBitValue=false) {m_mask = BitMask::Create (defaultBitValue);}

//!  Destuctor
~BitMaskHolder() {m_mask->Free();}

//! Construct a new Instance of BitMaskHolder from an existing BitMaskHolder.
//! The BitMask held by source is copied.
//! @param[in]  source   Existing BitMaskHolder.
BitMaskHolder (BitMaskHolder const& source) {m_mask = BitMask::Clone(*source.m_mask);}

//! Construct a new Instance of BitMaskHolder from an existing BitMask.
//! The source BitMask is copied.
//! @param[in]  source  Existing BitMask.
explicit BitMaskHolder (BitMask const& source) {m_mask = BitMask::Clone(source);}

//! Assign a new BitMask to this BitMaskHolder
BitMaskHolder& operator=(BitMaskHolder const& source) {if (this == &source) return *this; m_mask->Free(); m_mask = BitMask::Clone(*source.m_mask); return *this;}

//! Assign a new BitMask to this BitMaskHolder
BitMaskHolder& operator=(BitMaskCR source) {m_mask->Free(); m_mask = BitMask::Clone(source); return *this;}

//! Assign a new BitMask to this BitMaskHolder. Takes over responsibility for freeing the source.
void SetBitMask (BitMaskP source) 
    {
    if (source == m_mask)
       return;

    if (NULL != m_mask) 
        m_mask->Free(); 

    m_mask = source; 
    }

//! Access the BitMask held by this BitMaskHolder.
BitMaskP  operator->() {return m_mask;}

//  Access the BitMask's methods
BitMaskCP operator->() const {return m_mask;}

//! Access the BitMask held by this BitMaskHolder.
BitMaskR  operator*() {return *m_mask;}

//!  Access the BitMask held by this BitMaskHolder.
BitMaskP *operator& () { return &m_mask; }

//!  Access the BitMask held by this BitMaskHolder.
operator BitMaskP () { return m_mask; }

//   Access the BitMask's methods
BitMaskCR  operator*() const {return *m_mask;}

//!  Access the BitMask held by this BitMaskHolder.
BitMaskCP get() const {return m_mask;}
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   09/12
//=======================================================================================
template <bool defaultVal> struct BitMaskRef : NonCopyableClass
{
private:
    BitMaskP    m_mask;

public:
    void Clear() {if (m_mask) m_mask->Free(); m_mask=NULL;}
    explicit BitMaskRef () {m_mask = NULL;}
    ~BitMaskRef () {Clear();}

    BitMaskP GetBitMask() {if (NULL == m_mask) {m_mask=BitMask::Create(defaultVal);} return m_mask;}
    bool IsValid() const {return NULL != m_mask;}
};

//__PUBLISH_SECTION_END__

/*----------------------------------------------------------------------------------*//**
* Copy the contents of the bitmask to a Byte array so it can be written to a file.
* The bytes can be converted back to a bitmask via bitMask_fromFileByteArray.
* pNumBytes should contain the number of bytes in pByteArray when this function is called.
* When the function returns it will contain the actual number of bytes required to represent
* the bitmask.
*
* @param    pBitMaskIn          IN bit mask object
* @param    pNumBytes           OUT number of filled bytes in pByteArray
* @param    pByteArray          OUT ptr to buffer to hold bytes.  It must be freed by the caller.
*
* @return   SUCCESS or ERROR
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       bitMask_toFileByteArray
(
BitMaskCP       pBitMaskIn,
uint16_t*         pNumBytes,
Byte**          pByteArray
);

/*----------------------------------------------------------------------------------*//**
* Free the Byte array from bitMask_toFileByteArray.
*
* @param    pByteArray          IN ptr to buffer from bitMask_toFileByteArray.
* @return   void
*
+---------------+---------------+---------------+---------------+---------------+------*/
void            bitMask_freeFileByteArray
(
Byte**          pByteArray
);

/*----------------------------------------------------------------------------------*//**
* Get the contents of the bitmask from a Byte array that was read from a file
*
* @param    ppBitMaskOut        OUT bit mask object
* @param    numBytes            IN number of bytes in pByteArray
* @param    pByteArray          IN buffer containing bytes acquired via bitMask_toFileByteArray.
* @return   SUCCESS or ERROR if array size doesn't match bitmask size or allocation fails.
*
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       bitMask_fromFileByteArray
(
BitMaskH        ppBitMaskOut,
const uint16_t  numBytes,
const Byte*     pByteArray
);

//__PUBLISH_SECTION_START__

END_BENTLEY_NAMESPACE
