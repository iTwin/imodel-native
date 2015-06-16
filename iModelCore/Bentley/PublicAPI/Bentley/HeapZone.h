/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Bentley/HeapZone.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
/** @cond BENTLEY_SDK_Internal */

#define BOOST_NO_MT
#include <Bentley/bpool.h>
#include <Bentley/BentleyAllocator.h>

namespace BENTLEY_NAMESPACE_NAME {struct HeapZone; typedef struct HeapZone* HeapZoneP;}

BEGIN_BENTLEY_NAMESPACE

/*=================================================================================**//**
* @bsiclass                                                     Keith.Bentley   10/04
+===============+===============+===============+===============+===============+======*/
struct Heapzone_allocator
{
  typedef std::size_t size_type;
  typedef std::ptrdiff_t difference_type;

  static char* malloc(const size_type bytes)  {return reinterpret_cast<char *>(bentleyAllocator_malloc(bytes));}
  static void  free(char * const block)       {bentleyAllocator_free(block,0);}
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   04/10
//=======================================================================================
template <class STYPE, int MAXBLOCKSIZE> class DgnMemoryPool : bboost::bpool<Heapzone_allocator>
{
public:
    DgnMemoryPool () : bboost::bpool<Heapzone_allocator>(sizeof(STYPE)) {}
    STYPE* AllocateNode () {if (next_size > MAXBLOCKSIZE) next_size=MAXBLOCKSIZE; return (STYPE*) malloc();}
    void FreeNode (STYPE* node) {free (node);}
    void Clear () {purge_memory();}
    void SetEntrySize (int n, int firstSize) {*((int*) &requested_size) = n; next_size=firstSize;}

    size_t GetMemoryAllocated() const
        {
        size_type total = 0;
        for (bboost::details::PODptr<size_type> ptr = list; ptr.valid(); ptr = ptr.next())
            total += ptr.total_size();

        return total;
        }
};

typedef bboost::bpool<Heapzone_allocator> T_BoostPoolWithMemutilAllocator;

/*=================================================================================**//**
* @bsiclass                                                     Keith.Bentley   10/04
+===============+===============+===============+===============+===============+======*/
struct FixedSizePool1 : private T_BoostPoolWithMemutilAllocator
    {

    DEFINE_T_SUPER(T_BoostPoolWithMemutilAllocator);

    size_type   m_originalNextSize;     // original value of bpool base class's next_size which changes during the lifetime of the bpool

    FixedSizePool1 (uint32_t entrySize, char const* dbgnm = ""): T_BoostPoolWithMemutilAllocator (entrySize)
        {
        m_originalNextSize  = next_size;
        }

    FixedSizePool1 (char const* dbgnm = ""): T_BoostPoolWithMemutilAllocator (8)
        {
        m_originalNextSize  = next_size;
        }

    ~FixedSizePool1 ()
        {
        }

    void    SetName (char const* nm)
        {
        }

    void    SetSize (int entrySize, int chunkSize=32)
        {
        *((int*) &requested_size) = entrySize;
        m_originalNextSize = next_size = chunkSize;
        }

    size_t GetSize () const {return requested_size;}

    size_t GetFreeBytes () const
        {
        size_t nFree = 0;
        for (void* freeBlock = first; NULL != freeBlock; freeBlock = nextof(freeBlock))
            ++nFree;
        return nFree * GetSize();
        }

    size_t GetTotalBytes () const
        {
        size_type total = 0;
        for (bboost::details::PODptr<size_type> ptr = list; ptr.valid(); ptr = ptr.next())
            total += ptr.total_size();

        return total;
        }

    void ReleaseFreeMemory () {release_memory ();}

    void* malloc (size_t sz)
        {
        return T_Super::malloc ();
        }

    void* malloc ()
        {
        return T_Super::malloc ();
        }

    void* ordered_malloc (size_t nChunks, size_t actualSz)
        {
        return T_Super::ordered_malloc (nChunks);
        }

    void* ordered_malloc (size_t nChunks)
        {
        return T_Super::ordered_malloc (nChunks);
        }

    void free (void*p, size_t sz)
        {
        T_Super::free (p);
        }

    void free (void*p)
        {
        T_Super::free (p);
        }

    void ordered_free (void*p, size_t nChunks, size_t actualSize)
        {
        T_Super::ordered_free (p, nChunks);
        }

    void ordered_free (void*p, size_t nChunks)
        {
        T_Super::ordered_free (p, nChunks);
        }

    void purge_memory_and_reinitialize ()
        {
        T_Super::purge_memory ();
        set_next_size(m_originalNextSize);
        }
    };

/*=================================================================================**//**
* @bsiclass                                                     Keith.Bentley   10/04
+===============+===============+===============+===============+===============+======*/
struct HeapZone
{
private:
    enum
        {
        POOL_ENTRY_SIZE   = 8,
        NUM_FIXED_POOLS   = 2*sizeof(void*),
        LARGE_SIZE_CHUNK  = 32, //  32 byte granularity for ordered bpool
        };

    FixedSizePool1   m_fixed[NUM_FIXED_POOLS];
    FixedSizePool1   m_ordered;

static size_t  NumChunks (size_t size)      {return (size+POOL_ENTRY_SIZE-1)/POOL_ENTRY_SIZE;}
static size_t  LargeChunks (size_t size)    {return (size+LARGE_SIZE_CHUNK-1)/LARGE_SIZE_CHUNK;}

public:
    HeapZone (uint32_t sig = 0, bool useMallocForLarge = true, char const* dbgnm = "")
        {
        for (int i=0; i<NUM_FIXED_POOLS; i++)
            {
            m_fixed[i].SetSize ((i+1)*POOL_ENTRY_SIZE);
            }

        // if we're using malloc instead of the ordered bpool, set its size to 0
        m_ordered.SetSize (useMallocForLarge ? 0 : LARGE_SIZE_CHUNK);
        }

    ~HeapZone ()
        {
        }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/2008
+---------------+---------------+---------------+---------------+---------------+------*/
void ReleaseFreeMemory ()
    {
    for (int i=0; i<NUM_FIXED_POOLS; i++)
        m_fixed[i].ReleaseFreeMemory ();

    m_ordered.ReleaseFreeMemory ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/04
+---------------+---------------+---------------+---------------+---------------+------*/
void* Alloc (size_t size)
    {
    if (size<=0)
        return  NULL;

    size_t numChunks = NumChunks(size);

    if (numChunks<=NUM_FIXED_POOLS)
        return  m_fixed[numChunks-1].malloc(size);

    return (0 == m_ordered.GetSize()) ? bentleyAllocator_malloc(size) : m_ordered.ordered_malloc (LargeChunks (size), size);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/04
+---------------+---------------+---------------+---------------+---------------+------*/
void* Realloc (void* ptr, size_t newBytes, size_t oldBytes)
    {
    size_t oldNumChunks = NumChunks(oldBytes);
    size_t newNumChunks = NumChunks(newBytes);

    if (oldNumChunks > NUM_FIXED_POOLS && newNumChunks > NUM_FIXED_POOLS)
        {
        //  newBytes and oldBytes are both large.

        if (0 == m_ordered.GetSize())
            {
            //  We are forwarding large requests to memutil.
            return bentleyAllocator_realloc(ptr, newBytes);
            }

        //  We would use m_ordered for both.
        //  We can reuse the same pointer if both round up to the same number of 32-byte chunks
        if (LargeChunks(newBytes) == LargeChunks(oldBytes))
            {
            return ptr;
            }
        }
    else
        {
        //  newBytes and/or oldBytes is small.

        if (oldNumChunks <= NUM_FIXED_POOLS && newNumChunks <= NUM_FIXED_POOLS)
            {
            //  newBytes and oldBytes are both small => we would use FixedSizePool for both.
            //  We can reuse the same pointer if both round up to the same number of 8-byte chunks
            if (oldNumChunks == newNumChunks)
                {
                return  ptr;
                }
            }
        }

    //  old and new requests are sufficiently different that we must expand

    void*   newPtr = Alloc (newBytes);
    if (NULL ==newPtr)
        return NULL;

    if (ptr)
        {
        memcpy (newPtr, ptr, std::min<size_t>(newBytes, oldBytes));
        Free (ptr, oldBytes);
        }
    return  newPtr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/04
+---------------+---------------+---------------+---------------+---------------+------*/
void Free (void* ptr, size_t size)
    {
    BeAssert (ptr != this);

    size_t numChunks = NumChunks(size);

    if (numChunks<=NUM_FIXED_POOLS)
        m_fixed[numChunks-1].free (ptr, size);
    else
        {
        if (0 == m_ordered.GetSize())
            bentleyAllocator_free(ptr,0);
        else
            m_ordered.ordered_free (ptr, LargeChunks(size), size);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/04
+---------------+---------------+---------------+---------------+---------------+------*/
void EmptyAll()
    {
    for (int i=0; i<NUM_FIXED_POOLS; i++)
        m_fixed[i].purge_memory_and_reinitialize();

    if (0 != m_ordered.GetSize())
        m_ordered.purge_memory_and_reinitialize();
    }
};

END_BENTLEY_NAMESPACE

/** @endcond */
