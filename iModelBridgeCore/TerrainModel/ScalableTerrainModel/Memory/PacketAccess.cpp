/*--------------------------------------------------------------------------------------+
|
|     $Source: ScalableTerrainModel/Memory/PacketAccess.cpp $
|    $RCSfile: PacketAccess.cpp,v $
|   $Revision: 1.4 $
|       $Date: 2011/08/19 13:50:29 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ScalableTerrainModelPCH.h>


#include <ScalableTerrainModel/Memory/PacketAccess.h>
#include <ScalableTerrainModel/Memory/Allocation.h>

BEGIN_BENTLEY_MRDTM_MEMORY_NAMESPACE


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
RawPacket::RawPacket                      (const MemoryAllocator&  pi_memoryAllocator)
    :   m_pAllocator(pi_memoryAllocator.Clone()),
        m_pObjLifeCycle(new PODObjectsLifeCycle(0)), // Create void POD life-cycle. This signals that packet has not been edited yet.
        m_pBufferBegin(0),
        m_pBufferEnd(0),
        m_pEnd(0)
    {

    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
RawPacket::~RawPacket                     ()
    {
    Clear();
    m_pAllocator->Deallocate(m_pBufferBegin);
    // TDORAY: Make exception safe?
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
size_t RawPacket::GetCapacity () const
    {
    return (const byte*)m_pBufferEnd - (const byte*)m_pBufferBegin;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void RawPacket::Reserve (size_t pi_NewCapacity)
    {

    if (pi_NewCapacity <= GetCapacity())
        return;

    struct SafeAllocator
        {
        const MemoryAllocator& m_rAllocator;

        void* m_pBufferBegin;
        void* m_pBufferEnd;


        explicit SafeAllocator (const MemoryAllocator& pi_rAllocator,
                                size_t pi_capacity) 
                :   m_rAllocator(pi_rAllocator), 
                    m_pBufferBegin(m_rAllocator.Allocate(pi_capacity)),
                    m_pBufferEnd((byte*)m_pBufferBegin + pi_capacity)
            {
            }

        ~SafeAllocator () 
            {
            m_rAllocator.Deallocate(m_pBufferBegin);
            }

        void SwapWith (void*& pio_pBufferBegin, void*& pio_pBufferEnd) 
            {
            using std::swap;
            swap(m_pBufferBegin, pio_pBufferBegin);
            swap(m_pBufferEnd, pio_pBufferEnd);
            }
        };

    struct SafeConstructor 
        {
        const ObjectsLifeCycle& m_rLifeCycle;
        const SafeAllocator& m_rAllocator;
        void* m_pEnd;

        explicit SafeConstructor (const ObjectsLifeCycle& pi_rLifeCycle, const SafeAllocator& pi_rAllocator, 
                                  const void* pi_pSrcBegin, const void* pi_pSrcEnd)
            :   m_rLifeCycle(pi_rLifeCycle),
                m_rAllocator(pi_rAllocator),
                m_pEnd(m_rAllocator.m_pBufferBegin)
            {
            m_rLifeCycle.ConstructFrom(m_rAllocator.m_pBufferBegin, m_pEnd, pi_pSrcBegin, pi_pSrcEnd);
            }

        ~SafeConstructor ()
            {
            m_rLifeCycle.Destroy(m_rAllocator.m_pBufferBegin, m_pEnd);
            }

        void SwapWith (void*& pio_pEnd) 
            {
            using std::swap;
            swap(m_pEnd, pio_pEnd);
            }
        };  

    SafeAllocator allocator(*m_pAllocator, pi_NewCapacity);
    SafeConstructor constructor(*m_pObjLifeCycle, allocator, m_pBufferBegin, m_pEnd);

    allocator.SwapWith(m_pBufferBegin, m_pBufferEnd);
    constructor.SwapWith(m_pEnd);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void RawPacket::Clear ()
    {
    m_pObjLifeCycle->Destroy(m_pBufferBegin, m_pEnd);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool RawPacket::IsPODLifeCycleCompatibleWith (size_t pi_typeSize) const
    {
    if (m_pObjLifeCycle->m_typeSize == pi_typeSize)
        {
        return true;
        }
    else if (m_pObjLifeCycle->m_typeSize < pi_typeSize)
        {
        assert(0 != m_pObjLifeCycle->m_typeSize);
        return 0 == pi_typeSize % m_pObjLifeCycle->m_typeSize;
        }
    else 
        {
        assert(0 != pi_typeSize);
        return 0 == (m_pObjLifeCycle->m_typeSize % pi_typeSize) && 
               (0 < GetSize() ? (0 == GetSize() % pi_typeSize) : true);
        }
    }



/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
RawPacket::ObjectsLifeCycle::ObjectsLifeCycle (size_t  pi_typeSize,
                                               ClassID pi_classID) 
    :   m_typeSize(pi_typeSize),
        m_classID(pi_classID)
    {

    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
RawPacket::PODObjectsLifeCycle::PODObjectsLifeCycle (size_t pi_typeSize)
    :   ObjectsLifeCycle(pi_typeSize, s_GetClassID())
    {

    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void RawPacket::PODObjectsLifeCycle::Construct     (void*   pi_begin,
                                                    void*&  pio_end,
                                                    size_t  pi_count) const
    {
    assert(pi_begin == pio_end);
    // No constructor to call
    pio_end = ((byte*)pi_begin) + pi_count*m_typeSize;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void RawPacket::PODObjectsLifeCycle::ConstructFrom     (void*           pi_begin,
                                                        void*&          pio_end,
                                                        const void*     pi_srcBegin,
                                                        const void*     pi_srcEnd) const
    {
    assert(pi_begin == pio_end);
    // No constructor to call

    size_t size = (const byte*)pi_srcEnd - (const byte*)pi_srcBegin;

    memcpy(pi_begin, pi_srcBegin, size);

    pio_end = (byte*)pi_begin + size;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void RawPacket::PODObjectsLifeCycle::Destroy   (void*   pi_begin,
                                                void*&  pio_end) const
    {
    // No destructor to call
    pio_end = pi_begin;
    }



END_BENTLEY_MRDTM_MEMORY_NAMESPACE
