/*--------------------------------------------------------------------------------------+
|
|     $Source: PrivateApi/ImagePPInternal/gra/ImageAllocator.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <ImagePP/all/h/HFCMacros.h>

BEGIN_IMAGEPP_NAMESPACE

struct ImageAllocatorPool;

/*---------------------------------------------------------------------------------**//**
* ImagePool
+---------------+---------------+---------------+---------------+---------------+------*/
struct ImagePool
{
public:
    static ImageAllocatorPool& GetDefaultPool();

private:
    ImagePool() = delete;
    ImagePool(ImagePool const&) = delete;
    ImagePool& operator=(ImagePool const&) = delete;
};

/*---------------------------------------------------------------------------------**//**
* IImageAllocator
+---------------+---------------+---------------+---------------+---------------+------*/
struct IImageAllocator
    {
    virtual Byte*        _AllocMemory(size_t memorySize) = 0;
    virtual void         _FreeMemory(Byte* pMem) = 0;
    };

/*---------------------------------------------------------------------------------**//**
* SystemAllocator
+---------------+---------------+---------------+---------------+---------------+------*/
struct SystemAllocator : public IImageAllocator
    {
    virtual ~SystemAllocator() {};

    virtual Byte* _AllocMemory(size_t memorySize) override 
        {
        return new Byte[memorySize];
        }
    virtual void _FreeMemory(Byte* pMem) override 
        {
        delete [] pMem;
        }
    };

/*---------------------------------------------------------------------------------**//**
* SystemAllocator
+---------------+---------------+---------------+---------------+---------------+------*/
struct KeepLastBlockAllocator : public IImageAllocator
{
public:
    KeepLastBlockAllocator(){m_pFreeBlock = NULL;}
    virtual ~KeepLastBlockAllocator() {}

    typedef uint32_t BlockHeader;

    virtual Byte* _AllocMemory(size_t memorySize) override 
        {
        if(GetFreeBlockSize() < memorySize)
            {
            AllocateNewBlock(memorySize);
            if(m_pFreeBlock == NULL)
                return NULL;
            }

        return m_pFreeBlock.release() + sizeof(BlockHeader);
        }

    virtual void _FreeMemory(Byte* pMem) override 
        {
        m_pFreeBlock.reset(pMem - sizeof(BlockHeader));
        }
private:
    size_t GetFreeBlockSize() const
        {
        if(m_pFreeBlock == NULL)
            return 0;

        return *(BlockHeader*)m_pFreeBlock.get();
        }

    void AllocateNewBlock(size_t size)
        {
        m_pFreeBlock.reset(new Byte[sizeof(BlockHeader) + size]);
        *(BlockHeader*)m_pFreeBlock.get() = (BlockHeader)size;
        }

    unique_ptr<Byte[]> m_pFreeBlock;
};

/*---------------------------------------------------------------------------------**//**
*  ImageAllocatorRef
* Hold a reference to an allocator that is managed by an ImageAllocatorPool
+---------------+---------------+---------------+---------------+---------------+------*/
struct ImageAllocatorRef : public RefCountedBase, public NonCopyableClass
{
    static ImageAllocatorRefPtr Create(ImageAllocatorPool& pool);

    IImageAllocatorR GetAllocator() {return *m_allocator;}

protected:
    ImageAllocatorRef() = delete;

    ImageAllocatorRef(ImageAllocatorPool& pool);
    ~ImageAllocatorRef();

    ImageAllocatorPool& m_pool;
    unique_ptr<IImageAllocator> m_allocator;
};

/*---------------------------------------------------------------------------------**//**
* ImageAllocatorPool
+---------------+---------------+---------------+---------------+---------------+------*/
struct ImageAllocatorPool
    {
    friend ImageAllocatorRef;

    public:
        ImageAllocatorPool(uint32_t maxAllocators = 5, uint32_t blockAlignment = 64 * 64, uint32_t maxBlocksPerAllocator = 5);
        ~ImageAllocatorPool();

        ImageAllocatorRefPtr GetAllocatorRef();

    protected:
        // to be used by ImageAllocatorRef to get access to allocators.
        unique_ptr<IImageAllocator> PopAllocator();
        void PushAllocator(unique_ptr<IImageAllocator>& allocator);

    private:
        ImageAllocatorPool(ImageAllocatorPool const&) = delete;
        ImageAllocatorPool& operator=(ImageAllocatorPool const&) = delete;


        HFCExclusiveKey               m_key;
        std::list<unique_ptr<IImageAllocator> > m_allocators;
        uint32_t                      m_maxAllocators;
        uint32_t                      m_maxBlocsPerAllocator;
        size_t                        m_alignment;
    };

END_IMAGEPP_NAMESPACE
