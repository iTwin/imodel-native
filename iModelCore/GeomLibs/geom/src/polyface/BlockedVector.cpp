/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include <bsibasegeomPCH.h>

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
template <typename T>uint32_t BlockedVector<T>::NumPerStruct () const  {return m_numPerStruct;}

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
template <typename T>uint32_t BlockedVector<T>::StructsPerRow () const {return m_structsPerRow;}

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
template <typename T>void   BlockedVector<T>::SetStructsPerRow (uint32_t n)   {m_structsPerRow = n;}

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
template <typename T>uint32_t BlockedVector<T>::Tag () const          {return m_tag;}

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
template <typename T>uint32_t BlockedVector<T>::IndexFamily () const  {return m_indexFamily;}

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
template <typename T>uint32_t BlockedVector<T>::IndexedBy () const    {return m_indexedBy;}

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
//template <typename T>bool BlockedVector<T>::Active () const         {return this->m_active;}

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
//template <typename T>void BlockedVector<T>::SetActive (bool active) {this->m_active = active;}

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
template <typename T>
bool BlockedVector<T>::TryGetAt (size_t index, T const &defaultValue, T &value) const
    {
    size_t n = this->size ();
    if (index < n)
        {
        value = this->at(index);
        return true;
        }
    value = defaultValue;
    return false;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
template <typename T>
void BlockedVector<T>::SetTags
(
uint32_t numPerStruct,
uint32_t structsPerRow,
uint32_t tag,
uint32_t indexFamily,
uint32_t indexedBy,
bool   active
)
    {
    m_numPerStruct  = numPerStruct;
    m_structsPerRow = structsPerRow;
    m_tag           = tag;
    m_indexFamily   = indexFamily;
    m_indexedBy     = indexedBy;
    m_active        = active;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
template <typename T>
T *BlockedVector<T>::GetPtr
(
)
    {
    if (this->size () > 0)
        return &this->at(0);
    else
        return NULL;
    }
    

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
template <typename T>
T const *BlockedVector<T>::GetCP () const
    {
    if (this->size () > 0)
        return &this->at(0);
    else
        return NULL;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
template <typename T>
void BlockedVector<T>::CopyVectorFrom (bvector<T>&source)
    {
    this->clear ();
    for (size_t i = 0, n = source.size();
            i < n;
            i++)
        this->push_back (source[i]);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
template<typename T>
BlockedVector<T>::BlockedVector
(
uint32_t numPerStruct,
uint32_t structsPerRow,
uint32_t tag,
uint32_t indexFamily,
uint32_t indexedBy,
bool active
)
    {
    m_numPerStruct      = numPerStruct;
    m_structsPerRow  = structsPerRow;
    m_tag               = tag;
    m_indexFamily       = indexFamily;
    m_indexedBy         = indexedBy;
    m_active            = active;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
template <typename T>
BlockedVector<T>::BlockedVector ()
    {
    m_numPerStruct      = 1;
    m_structsPerRow     = 1;
    m_tag               = 0;
    m_indexFamily       = 0;
    m_indexedBy         = 0;
    m_active            = false;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
template<typename T>
uint32_t BlockedVector<T>::ClearAndAppendBlock (BlockedVector<T> &source, uint32_t i0, uint32_t n, uint32_t numWrap)
    {
    uint32_t numOut = 0;
    this->clear ();
    size_t numSource = source.size ();
    if (numSource > 0 && source.Active ())
        {
        for (; numOut < n && i0 + numOut < numSource; numOut++)
            {
            this->push_back (source[i0 + numOut]);
            }
        SetActive (true);

        if (numOut > 0)
            {
            auto numSoFar = numOut;
            for (uint32_t i = 0; i < numWrap; i++)
                {
                auto iA = i % numSoFar;     // prevent index overflow if numWrap > numSoFar
                T wrap = this->at(iA);
                this->push_back (wrap);
                numOut++;
                }
            }
        }
    return numOut;
    }

template<typename T>
void BlockedVector<T>::ClearAndAppend (bvector<T> const &source)
    {
    this->clear ();
    for (auto &value : source)
        this->push_back (value);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
template<typename T>
uint32_t BlockedVector<T>::ClearAndAppendBlock (T const *source, size_t numSource, uint32_t i0, uint32_t n, uint32_t numWrap)
    {
    uint32_t numOut = 0;
    this->clear ();
    if (source != NULL && numSource > 0)
        {
        for (; numOut < n && i0 + numOut < numSource; numOut++)
            {
            this->push_back (source[i0 + numOut]);
            }
        SetActive (true);

        if (numOut > 0)
            {
            auto numSoFar = numOut;
            for (uint32_t i = 0; i < numWrap; i++)
                {
                auto iA = i % numSoFar;     // prevent index overflow if numWrap > numSoFar
                T wrap = this->at(iA);
                this->push_back (wrap);
                numOut++;
                }
            }
        }
    return numOut;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
template<typename T>
size_t BlockedVector<T>::Append (T const *pData, size_t n)
    {
    if (n > 0 && pData != NULL)
        {
        size_t oldSize = this->size();
        this->resize(oldSize + n);
        std::copy(pData, pData + n, this->begin() + oldSize);
        SetActive (true);
        }
    return this->size ();
    }

template<typename T>
size_t BlockedVector<T>::Append (T const &data)
    {
    this->push_back (data);
    SetActive (true);
    return this->size ();
    }

template<typename T>
size_t BlockedVector<T>::AppendAndReturnIndex (T const &data)
    {
    size_t index = this->size ();
    this->push_back (data);
    SetActive (true);
    return index;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
template<typename T>
size_t BlockedVector<T>::Append (BlockedVector<T> const &source)
    {
    size_t n = source.size ();
    for (size_t i = 0; i < n; i++)
        this->push_back (source[i]);
    SetActive (true);
    return this->size ();
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
template<typename T>
void BlockedVector<T>::ReverseInRange (size_t iFirst, size_t iLast)
    {
    T temp;
    for (;iFirst < iLast; iFirst++, iLast--)
        {
        temp = this->at(iFirst);
        this->at(iFirst)= this->at(iLast);
        this->at(iLast) = temp;
        }
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
template<typename T>
void BlockedVector<T>::CopyData(size_t iFrom, size_t iTo)
    {
    size_t count = this->size ();
    if (iFrom != iTo && iFrom < count && iTo < count)
        this->at(iTo) = this->at (iFrom);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
template<typename T>
void BlockedVector<T>::Trim (size_t index0, size_t count)
    {
    size_t n = this->size ();
    if (index0 >= n)
        return;
    if (index0 + count > n)
        count = n - index0;
    if (index0 > 0)
        {
        for (size_t i = 0; i < count; i++)
            this->at(i) = this->at (index0 + i);
        }
    this->resize (count);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
template<typename T>
size_t BlockedVector<T>::NumCompleteRows ()
    {
    size_t numStruct = this->size ();
    size_t numPerRow = StructsPerRow ();
    return numPerRow > 1 ? numStruct / numPerRow : numStruct;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
template<typename T>
uint32_t BlockedVector<T>::ClearAndAppendByOneBasedIndices
(
bvector<int> &zeroBasedIndices,
bvector<BoolTypeForVector> *positive,
bvector<T> &source,
bvector<int> &oneBasedIndices,
uint32_t i0,
uint32_t numIndex,
uint32_t numWrap
)
    {
    uint32_t numOut;
    this->clear ();
    zeroBasedIndices.clear ();
    if (positive != NULL)
        positive->clear ();
    size_t maxIndex = oneBasedIndices.size ();
    size_t numSource = source.size ();
    for (numOut = 0; numOut < numIndex && i0 + numOut < maxIndex; numOut++)
        {
        int k1 = oneBasedIndices[i0 + numOut];
        if (k1 == 0)
            break;
        uint32_t k0 = abs (k1) - 1;
        if (k0 >= numSource)
            break;
        zeroBasedIndices.push_back (k0);
        if (positive != NULL)
            positive->push_back (k1 > 0);
        this->push_back (source[k0]);
        }
    if (numOut > 0)
        {
        auto numSoFar = numOut;
        for (uint32_t i = 0; i < numWrap; i++)
            {
            auto iA = i % numSoFar;     // prevent index overflow if numWrap > numSoFar
            T wrap = this->at(iA);
            this->push_back (wrap);
            numOut++;
            uint32_t index = zeroBasedIndices[iA];
            zeroBasedIndices.push_back (index);
            if (positive != NULL)
                {
                auto b = positive->at(iA);
                positive->push_back (b);
                }
            }
        }
    return numOut;
    }



/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
template<typename T>
uint32_t BlockedVector<T>::ClearAndAppendByOneBasedIndices
(
bvector<int> &zeroBasedIndices,
bvector<BoolTypeForVector> *positive,
T const *source,
size_t sourceCount,            
int const *oneBasedIndices,
size_t oneBasedIndexCount,
uint32_t i0,
uint32_t numIndex,
uint32_t numWrap
)
    {
    uint32_t numOut;
    this->clear ();
    zeroBasedIndices.clear ();
    if (positive != NULL)
        positive->clear ();
    if (oneBasedIndices == NULL)
        return 0;
    for (numOut = 0; numOut < numIndex && i0 + numOut < oneBasedIndexCount; numOut++)
        {
        int k1 = oneBasedIndices[i0 + numOut];
        if (k1 == 0)
            break;
        uint32_t k0 = abs (k1) - 1;
        if (k0 >= sourceCount)
            break;
        zeroBasedIndices.push_back (k0);
        if (positive != NULL)
            positive->push_back (k1 > 0);
        this->push_back (source[k0]);
        }
    if (numOut > 0)
        {
        auto numSoFar = numOut;
        for (uint32_t i = 0; i < numWrap; i++)
            {
            auto iA = i % numSoFar;     // prevent index overflow if numWrap > numSoFar
            T wrap = this->at(iA);
            zeroBasedIndices.push_back (zeroBasedIndices[iA]);
            this->push_back (wrap);
            numOut++;
            if (positive)
                positive->push_back(positive->at(iA));
            }
        }
    return numOut;
    }



/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
size_t BlockedVectorInt::CountZeros
(
)
    {
    size_t numZero = 0;
    size_t count = size ();
    for (size_t i = 0; i < count; i++)
        if (at(i) == 0)
            numZero++;
    return numZero;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool BlockedVectorInt::AddAndTerminate
(
int *pData,
size_t n
)
    {
    if (NULL != pData)
        {
        if (m_structsPerRow <= 1)
            {
            for (size_t i = 0; i < n; i++)
                push_back (pData[i]);
            push_back (0);
            }
        else if (n <= m_structsPerRow)
            {
            for (size_t i = 0; i < n; i++)
                push_back (pData[i]);
            for (size_t i = n; i < m_structsPerRow; i++)
                push_back (0);
            }
        return true;
        }
    return false;
    }

void BlockedVectorInt::AddSteppedBlock
(
int value,
int valueStep,
size_t numValue,
size_t numWrap,
size_t numTrailingZero,
bool clearFirst
)
    {
    if (Active())
        {
        if (clearFirst)
            clear();
        if (numValue > 0)
            {
            auto i0 = size();
            for (size_t i = 0; i < numValue; i++)
                push_back(value + (int)i * valueStep);
            for (size_t i = 0; i < numWrap; i++)
                {
                auto iA = i % numValue;     // prevent index overflow if numWrap > numValue
                int newvalue = at(i0 + iA);
                push_back(newvalue);
                }
            }
        if (numTrailingZero > 0)
            for (size_t i = 0; i < numTrailingZero; i++)
                push_back(0);
        }
    }
/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
void BlockedVectorInt::AddSequentialBlock
(
int firstValue,
size_t numValue,
size_t numWrap,
size_t numTrailingZero,
bool clearFirst
)
    {
    if (Active())
        {
        if (clearFirst)
            clear ();
        if (numValue > 0)
            {
            auto i0 = size();
            for (size_t i = 0; i < numValue; i++)
                push_back (firstValue + (int)i);
            for (size_t i = 0; i < numWrap; i++)
                {
                auto iA = i % numValue;     // prevent index overflow if numWrap > numValue
                int value = at(i0 + iA);
                push_back (value);
                }
            }
        if (numTrailingZero > 0)
            for (size_t i = 0; i < numTrailingZero; i++)
                push_back (0);
        }
    }

// If the current array has blocked structsPerRow, expand to variable length 0-terminated form.
// (ASSUMES all zeros in blocked form are placeholders.)
void BlockedVectorInt::AddTerminatedSequentialBlocks
(
size_t numRow,
size_t numPerRow,
bool clearFirst,
int firstValue,
int terminator
)
    {
    SetActive (true);
    if (clearFirst)
        clear ();
    int value = firstValue;
    for (uint32_t row = 0; row < numRow; row++)
        {
        for (uint32_t i = 0; i < numPerRow; i++)
            push_back (value++);
        push_back (terminator);
        }
    }
       
// If the current array has blocked structsPerRow, expand to variable length 0-terminated form.
// (ASSUMES all zeros in blocked form are placeholders.)
void BlockedVectorInt::ConvertBlockedToZeroTerminated ()
    {
    if (m_structsPerRow > 1)
        {
        int terminator = 0;
        size_t oldSize = size ();
        //size_t expandedRowSize = m_structsPerRow + 1;
        // Truncation loses partial rows, that's fine ...
        size_t oldNumRows = oldSize / m_structsPerRow;
        oldSize = oldNumRows * m_structsPerRow;
        // If all rows are dense, every row gets one added terminator ....
        size_t newBlockedSize = oldSize + oldNumRows;
        // Shift to end in place, working back-to-front ...
        size_t dest, source;
        size_t numCopiedFromCurrentRow;
        reserve (newBlockedSize);
        // force new entries ...
        for (dest = oldSize; dest < newBlockedSize; dest++)
            push_back (0);
        // indices are unsigned ... can't decrement beyond zero !!
        for (dest = newBlockedSize,
             source = oldSize,
            numCopiedFromCurrentRow = 0;
                source > 0;
            )
            {
            if (numCopiedFromCurrentRow == 0)
                at(--dest) = terminator;
            at(--dest) = at(--source);
            numCopiedFromCurrentRow++;
            if (numCopiedFromCurrentRow == m_structsPerRow)
                numCopiedFromCurrentRow = 0;
            }

        size_t newSize = 0;
        for (source = 0; source < newBlockedSize;)
            {
            int a = at(newSize++) = at(source++);
            // Skip additional terminators ...
            if (a == terminator)
                while (source < newBlockedSize && at(source) == terminator)
                    source++;
            }
        resize (newSize);
        m_structsPerRow = 0;    // No longer by row !!!
        }
    }


// Create indices for a rectangular grid.
void BlockedVectorInt::AddTerminatedGridBlocks
(
size_t numRow,
size_t numPerRow,
size_t rowStep,
size_t colStep,
bool triangulated,
bool clearFirst,
int firstValue,
int terminator
)
    {
    if (clearFirst)
        clear ();
    SetActive (true);
    //int value = firstValue;
    for (uint32_t row1 = 1; row1 < numRow; row1++)
        {
        uint32_t row0 = row1 - 1;
        for (uint32_t col1 = 1; col1 < numPerRow; col1++)
            {
            size_t col0 = col1 - 1;
			// The array contents are ints -- downncast the indices as created
            int  i00 = (int)(firstValue + col0 * rowStep + row0 * colStep);
            int  i01 = (int)(firstValue + col1 * rowStep + row0 * colStep);
            int  i10 = (int)(firstValue + col0 * rowStep + row1 * colStep);
            int  i11 = (int)(firstValue + col1 * rowStep + row1 * colStep);
            if (triangulated)
                {
                push_back (i00);
                push_back (i01);
                push_back (i10);
                push_back (terminator);
                push_back (i01);
                push_back (i11);
                push_back (i10);
                push_back (terminator);
                }
            else
                {
                push_back (i00);
                push_back (i01);
                push_back (i11);
                push_back (i10);
                push_back (terminator);
                }
            }
        }
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool BlockedVectorInt::MinMax(int &minValue, int &maxValue) const
    {
    minValue = INT_MAX;
    maxValue = INT_MIN;
    if (size() == 0)
        return false;
    minValue = maxValue = at(0);
    for (size_t i = 0; i < size (); i++)
        {
        int value = at(i);
        if (value < minValue)
            minValue = value;
        if (value > maxValue)
            maxValue = value;
        }
    return true;
    }



/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool BlockedVectorInt::AllNegativeInRange (size_t iFirst, size_t iLast)
    {
    for (size_t i = iFirst; i <= iLast; i++)
        if (at(i) >= 0)
            return false;
    return true;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
void BlockedVectorInt::NegateInRange (size_t iFirst, size_t iLast)
    {
    for (size_t i = iFirst; i <= iLast; i++)
        at(i) = - at (i);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
void BlockedVectorInt::AbsInRange (size_t iFirst, size_t iLast)
    {
    for (size_t i = iFirst; i <= iLast; i++)
        at(i) = abs( at (i));
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
void BlockedVectorInt::Abs ()
    {
    AbsInRange (0, size () - 1);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
void BlockedVectorInt::NegativeAbsInRange (size_t iFirst, size_t iLast)
    {
    for (size_t i = iFirst; i <= iLast; i++)
        at(i) = -abs( at (i));
    }



/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
void BlockedVectorInt::ShiftSignsFromCyclicPredecessorsInRange (size_t kFirst, size_t kLast)
    {
    int sign0 = at(kLast) >= 0 ? 1 : -1;
    int sign1;
    for (size_t k = kFirst; k <= kLast; k++)
        {
        sign1 = at(k) >= 0 ? 1 : -1;
        at(k) = sign0 * abs (at(k));
        sign0 = sign1;
        }
    }

/** From given start position, find final (inclusive) position and position for next start search.
  Initialize iFirst to zero before first call.
  Return false if no more faces. */
bool BlockedVectorInt::DelimitFace (int numPerFace, size_t iFirst, size_t &iLast, size_t &iNext)
    {
    size_t count = size ();
    if (iFirst >= count)
        {
        iNext = iFirst;
        iLast = iFirst - 1;
        return false;
        }
    if (numPerFace > 1)
        {
        iLast = iFirst;
        size_t limit = iNext;
        while (iLast < limit && at(iLast) != 0)
            iLast++;
        iLast--;
        iNext = iFirst + numPerFace;
        return iLast >= iFirst;
        }
    else
        {
        // zero terminated.go b
        // skip over leading zeros ..
        while (iFirst < count && at(iFirst) == 0)
            iFirst++;
        if (iFirst >= count)
            {
            iNext = iFirst;
            iLast = iFirst - 1;
            return false;
            }
        iLast = iFirst + 1;
        while (iLast < count && at(iLast) != 0)
            iLast++;
        iNext = iLast + 1;
        iLast--;
        return true;
        }
    }

/** Append from source array, shifting each nonzero index by signed shift */
void BlockedVectorInt::AppendShifted (BlockedVectorInt const &source, int shift)
    {
    size_t n = source.size ();
    for (size_t i = 0; i < n; i++)
        {
        int value = source[i];
        if (value > 0)
            value += shift;
        else if (value < 0)
            value -= shift;
        push_back (value);
        }
    }



template struct BlockedVector<DPoint3d>;
template struct BlockedVector<int>;
template struct BlockedVector<uint32_t>;
template struct BlockedVector<DVec3d>;
template struct BlockedVector<DPoint2d>;
template struct BlockedVector<FPoint3d>;
template struct BlockedVector<RgbFactor>;
template struct BlockedVector<FloatRgb>;
template struct BlockedVector<PolyfaceEdgeChain>;
#ifndef MinimalRefMethods
template struct BlockedVector<FacetFaceData>;
template struct BlockedVector<CurveTopologyId>;
#endif
END_BENTLEY_GEOMETRY_NAMESPACE
