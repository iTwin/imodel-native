/*--------------------------------------------------------------------------------------+
|
|     $Source: all/gra/hra/src/ImageAllocator.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ImagePPInternal/hstdcpp.h>
#include <ImagePPInternal/gra/ImageAllocator.h>



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ImageAllocatorPool& ImagePool::GetDefaultPool()
    {
    static ImageAllocatorPool s_defaultPool;

    return s_defaultPool;
    }

/*---------------------------------------------------------------------------------**//**
* ImageAllocator
+---------------+---------------+---------------+---------------+---------------+------*/
struct ImageAllocatorReuseAlreadyAllocatedBlocks : public IImageAllocator
{
public:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Stephane.Poulin                 11/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    ImageAllocatorReuseAlreadyAllocatedBlocks(size_t alignment, uint32_t numberOfBlocks) 
        : m_alignment(alignment), m_first(NULL), m_last(NULL), m_count(0), m_maxNumberOfBlocks(numberOfBlocks)
        {
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Stephane.Poulin                 11/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual ~ImageAllocatorReuseAlreadyAllocatedBlocks()
        {
        }

protected:
    /*---------------------------------------------------------------------------------**//**
    * MemNode
    +---------------+---------------+---------------+---------------+---------------+------*/
    struct MemNode
        {
        size_t   m_size;
        Byte*    m_prev;
        Byte*    m_next;
        };

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Stephane.Poulin                 11/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    ImageAllocatorReuseAlreadyAllocatedBlocks() = delete;
    ImageAllocatorReuseAlreadyAllocatedBlocks(ImageAllocatorReuseAlreadyAllocatedBlocks const&) = delete;
    ImageAllocatorReuseAlreadyAllocatedBlocks& operator=(ImageAllocatorReuseAlreadyAllocatedBlocks const&) = delete;
    
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Stephane.Poulin                 11/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual Byte* _AllocMemory(size_t memorySize) override
        {
        if (m_alignment != 0)
            memorySize = (((memorySize - 1) / m_alignment) + 1) * m_alignment;

        Byte* found = NULL;
        Byte* current = m_first;
        while (current != NULL)
            {
            if (GetSize(current) == memorySize)
                {
                found = current;
                break;
                }

            if (GetSize(current) >= memorySize)
                {
                if (found && GetSize(found) > GetSize(current))
                    found = current;
                else
                    found = current;
                }

            current = GetNext(current);
            }

        if (found)
            {
            BeAssert(!m_first || !GetPrev(m_first));
            BeAssert(!m_last || !GetNext(m_last));
            RemoveNode(found);

            BeAssert(!m_first || !GetPrev(m_first));
            BeAssert(!m_last || !GetNext(m_last));

            return GetAddress(found);
            }


        // Must align memory to point to MemNode?
        Byte* pNewBuf = reinterpret_cast<Byte*> (::operator new(memorySize + sizeof(MemNode)));
        InitNode((Byte*)pNewBuf, memorySize);
        return GetAddress((Byte*)pNewBuf);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Stephane.Poulin                 11/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual void _FreeMemory(Byte* pBuf) override
        {
        BeAssert(NULL != pBuf);
        Byte* pMem = GetNode(pBuf);

        BeAssert(!m_first || !GetPrev(m_first));
        BeAssert(!m_last || !GetNext(m_last));
        AddNode(pMem);

        BeAssert(!m_first || !GetPrev(m_first));
        BeAssert(!m_last || !GetNext(m_last));

        if (m_count < m_maxNumberOfBlocks)
            return;

        Byte* lastBk = m_last; // m_last will be changed in RemoveNode
        RemoveNode(m_last);
        BeAssert(!m_first || !GetPrev(m_first));
        BeAssert(!m_last || !GetNext(m_last));
        ::operator delete(lastBk);
        }

private:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Stephane.Poulin                 11/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    size_t GetSize(Byte const* pNode) const { return reinterpret_cast<MemNode const*>(pNode)->m_size; }
    Byte* GetPrev(Byte const* pNode) const { return reinterpret_cast<MemNode const*>(pNode)->m_prev; }
    Byte* GetNext(Byte const* pNode) const { return reinterpret_cast<MemNode const*>(pNode)->m_next; }
    Byte* GetNode(Byte* pBuf) const { return pBuf - sizeof(MemNode); }
    Byte* GetAddress(Byte* pMem) const { return pMem + sizeof(MemNode); }
    void SetPrev(Byte* pMem, Byte* pNode) const { reinterpret_cast<MemNode*>(pMem)->m_prev = pNode; }
    void SetNext(Byte* pMem, Byte* pNode) const { reinterpret_cast<MemNode*>(pMem)->m_next = pNode; }
    void SetSize(Byte* pMem, size_t const size) const { reinterpret_cast<MemNode*>(pMem)->m_size = size; }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Stephane.Poulin                 11/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    void InitNode(Byte* pNode, size_t size)
        {
        BeAssert(NULL != pNode);
        SetSize(pNode, size);
        SetPrev(pNode, NULL);
        SetNext(pNode, NULL);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Stephane.Poulin                 11/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    void AddNode(Byte* pNode)
        {
        BeAssert(NULL != pNode);

        SetPrev(pNode, NULL);

        if (!m_first)
            {
            BeAssert(!m_last);
            SetNext(pNode, NULL);
            m_first = pNode;
            m_last = pNode;
            m_count = 1;
            return;
            }

        SetNext(pNode, m_first);
        SetPrev(m_first, pNode);
        m_first = pNode;
        ++m_count;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Stephane.Poulin                 11/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    void RemoveNode(Byte const* pNode)
        {
        BeAssert(NULL != pNode);
        Byte* prev = GetPrev(pNode);
        Byte* next = GetNext(pNode);

        if (!prev && !next)
            {
            BeAssert(m_count == 1);
            m_first = NULL;
            m_last = NULL;
            m_count = 0;
            return;
            }

        if (prev)
            SetNext(prev, next);

        if (next)
            SetPrev(next, prev);

        if (!prev)
            m_first = next;

        if (!next)
            m_last = prev;

        --m_count;
        }

    Byte* m_first;
    Byte* m_last;
    uint32_t m_count;
    const size_t m_alignment;
    const uint32_t m_maxNumberOfBlocks;
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  12/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ImageAllocatorRefPtr ImageAllocatorRef::Create(ImageAllocatorPool& pool)
    {
    ImageAllocatorRefPtr pNew = new ImageAllocatorRef(pool);

    if(pNew->m_allocator == NULL)
        return NULL;

    return pNew;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  12/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ImageAllocatorRef::ImageAllocatorRef(ImageAllocatorPool& pool)
    :m_pool(pool),
     m_allocator(pool.PopAllocator())
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  12/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ImageAllocatorRef::~ImageAllocatorRef()
    {
    if(m_allocator != NULL)
        m_pool.PushAllocator(m_allocator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ImageAllocatorPool::ImageAllocatorPool(uint32_t maxAllocators, uint32_t blockAlignment, uint32_t maxBlocksPerAllocator) 
 :m_maxAllocators(maxAllocators), m_alignment(blockAlignment), m_maxBlocsPerAllocator(maxBlocksPerAllocator)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ImageAllocatorPool::~ImageAllocatorPool()
    {
    HFCMonitor monitor(m_key);
    m_allocators.clear();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  12/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ImageAllocatorRefPtr ImageAllocatorPool::GetAllocatorRef()
    {
    return ImageAllocatorRef::Create(*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
unique_ptr<IImageAllocator> ImageAllocatorPool::PopAllocator()
    {
    HFCMonitor monitor(m_key);

    if (!m_allocators.empty())
        {
        unique_ptr<IImageAllocator> pAllocator = std::move(m_allocators.front());
        m_allocators.pop_front();
        return pAllocator;
        }

    return unique_ptr<IImageAllocator>(new ImageAllocatorReuseAlreadyAllocatedBlocks(m_alignment, m_maxBlocsPerAllocator));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  12/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void ImageAllocatorPool::PushAllocator(unique_ptr<IImageAllocator>& allocator)
    {
    HFCMonitor monitor(m_key);

    m_allocators.emplace_front(std::move(allocator));

    if (m_allocators.size() > m_maxAllocators)
        m_allocators.pop_back();
    }




