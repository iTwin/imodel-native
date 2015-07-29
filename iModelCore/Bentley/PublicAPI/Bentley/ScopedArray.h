/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Bentley/ScopedArray.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include "BeAssert.h"
#include "NonCopyableClass.h"
#if defined (TARGET_PROCESSOR_ARCHITECTURE_MEMORY_ALIGNMENT_REQUIRED)
#include <memory.h>
#include <string.h>
#endif

BEGIN_BENTLEY_NAMESPACE

BENTLEYDLL_EXPORT void UnalignedMemcpy(void*dest, void const*source, size_t num);

/*=================================================================================**//**
* Class to efficiently allocate a block of memory, freeing the memory when the function leaves the scope. ScopedArray provides a way to get the efficiency of stack allocation for the common case while preventing stack overflow in the unusual case.
*
* The template argument <b>THRESHOLD</b> specifies the number of bytes to allocate in the local frame. If a constructor requires more than <b>THRESHOLD</b> memory, ScopedArray allocates the memory from the heap.
*
* Note that allocating and freeing memory via ScopedArray does <i>not</i> invoke any constructors or destructors.
* That is because memory is not allocated as type \b T, even though the internal member is typed as \b T*.
*
* For a function called in a loop and with a shallow stack frame, it is appropriate to specify a large number for <b>THRESHOLD</b>. For a function called in a deep stack frame or using recursion, it is important to keep the value of <b>THRESHOLD</b> small.
*
* If you specify a custom <b>THRESHOLD</b>, it should be set to one less than a multiple of sizeof(\b T).
*
* \code
    // Use something like this if the stack depth is known to be large or is unpredictable.
    ScopedArray<DPoint3d> scoped (size);

    // Use something like this only when it is known to be safe.
    ScopedArray<DPoint3d, 30000> scoped1 (size);

    DPoint3d* data = scoped.GetData();
\endcode
*
* @bsiclass                                                                    03/2010
+===============+===============+===============+===============+===============+======*/
template<class T, size_t THRESHOLD = 511>
struct ScopedArray
    {
private:

    union
        {
        T m_unused; // This forces m_mem to be aligned according to T.

        struct
            {
            unsigned char   m_mem[THRESHOLD];   // The default value of 511 allows m_wasMalloced to fit within the 8/4-byte (x64/x86) alignment necessary for m_data.
                                                //  If you use a different value, it is probably worth making it one less than a multiple of sizeof(T).
            bool            m_wasMalloced;
            };
        };

    T* m_data;

#ifndef NDEBUG
    size_t const m_numItems;
#endif

    ScopedArray (ScopedArray const&);
    ScopedArray const& operator= (ScopedArray const&);
    void UnconditionalAllocate (size_t numItems)
        {
        size_t requiredSize = numItems * sizeof (T);

        if (requiredSize <= THRESHOLD)
            {
            m_data          = reinterpret_cast<T*>(m_mem);
            m_wasMalloced   = false;

            return;
            }

        m_data          = reinterpret_cast<T*>(new unsigned char[requiredSize]);
        m_wasMalloced   = true;
        }


public:
    
    //! Create a scoped array of a given size.
    //! @param[in] numItems       The number of items to allocate.
    ScopedArray (size_t numItems)
#ifndef NDEBUG
    : m_numItems (numItems)
#endif
        {
        UnconditionalAllocate (numItems);
        }
    
    // Allocate for numItems and copy from data
    ScopedArray (size_t numItems, T const *data)
#ifndef NDEBUG
    : m_numItems (numItems)
#endif    
        {
        UnconditionalAllocate (numItems);
        memcpy (GetData (), data, numItems * sizeof (T));
        }

    ~ScopedArray ()
        {
        if (!m_wasMalloced)
            return;

        delete[] reinterpret_cast<unsigned char*>(m_data);
        }

    //! Returns a pointer to this array's memory buffer.
    T* GetData ()
        {
        return m_data;
        }

    //! Get a pointer to the array stored in the ScopedArray.
    T const* GetDataCP () const
        {
        return m_data;
        }


    }; // ScopedArray

//=======================================================================================
//! AlignedArray ensures that a block of data is correctly aligned.
//! If the CPU architecture does not require aligned data, then this class nevers makes a copy of the data.
//! T is used to force AlignedArray to the proper alignment. It does not determine data size.  
// @bsiclass                                                                    03/2010
//=======================================================================================
template <class T, size_t THRESHOLD = 512, int ALIGNMENT=0x3>
struct AlignedArray : NonCopyableClass
    {
#if !defined (TARGET_PROCESSOR_ARCHITECTURE_MEMORY_ALIGNMENT_REQUIRED)
    AlignedArray () {;}
    //! This method just returns pData.
    //! @return pData
    //! @param pData the raw data array
    //! @param requiredSize size of the data block to be copied.
    T const* GetAlignedData (T const* pData, size_t requiredSize) {return pData;}
    //! Discard the copy of the data allocated by any previous call to GetAlignedData.
    void Clear() {;}
#else
    private:
    union
        {
        T               m_unused;           //!< This forces m_mem to be aligned according to T.
        unsigned char   m_mem[THRESHOLD];   //!< The inlined memory buffer used if a copy is required and if the object is small enough to fit.
        };

    T const* m_data;

    public:
    AlignedArray () :
        m_data (NULL)
        {
        }

    ~AlignedArray ()
        {
        Clear();
        }

    //! Discard the copy of the data allocated by any previous call to GetAlignedData.
    void Clear()
        {
        if ((unsigned char*)m_data != m_mem)
            delete[] (Byte*) m_data;
        m_data = NULL;
        }

    //! Make sure the specified data is aligned correctly. The data is copied to a correctly aligned buffer if necessary.
    //! The buffer is owned by this object and is discarded either by the Clear method or by the destructor.
    //! @return pData if it is aligned correctly or an aligned copy of its contents.
    //! @param pData the raw data array
    //! @param requiredSize size of the data block to be copied.
    T const* GetAlignedData (T const* pData, size_t requiredSize)
        {
        // Raw data is correctly aligned?
        if (0 == ((size_t)(intptr_t)pData & ALIGNMENT))
            return pData;

        // Not aligned. We must make a copy
        if (NULL != m_data)
            {
            BeAssert (false && "Call Clear before calling GetAlignedData a second time");
            return NULL;
            }
        if (requiredSize <= sizeof (m_mem))
            m_data = (T*) m_mem;
        else
            m_data = (T*) new Byte[requiredSize];

        Bentley::UnalignedMemcpy ((Byte*)m_data, pData, requiredSize);

        return m_data;
        }
#endif
    }; // AlignedArray

END_BENTLEY_NAMESPACE
