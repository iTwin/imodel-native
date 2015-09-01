//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PrivateApi/ImagePPInternal/gra/HRAImageSampler.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#pragma once

#include <ImagePP/all/h/HRAImageOp.h>
#include <ImagePP/all/h/HRPPixelNeighbourhood.h>

BEGIN_IMAGEPP_NAMESPACE

class HGF2DTransfoModel;
//class IHPMMemoryManager;
class HGF2DCoordSys;

struct HRASampleN1Surface;
struct HRASampleN8Surface;
struct HRASampleRleSurface;
struct HRAPacketN1Surface;
struct HRAPacketN8Surface;
struct HRAPacketCodecRleSurface;
struct HRAPacketRleSurface;
   
/*---------------------------------------------------------------------------------**//**
* SingleBlockAllocator
* Simple allocator that reuse previously allocated memory.
* This allocator is not thread safe.
+---------------+---------------+---------------+---------------+---------------+------*/
struct SingleBlockAllocator
{
public:
    SingleBlockAllocator() : m_size(0), m_isAvailable(true)
        {
        }
    ~SingleBlockAllocator()
        {
        HDEBUGCODE(BeAssert(m_isAvailable););
        }

    Byte* AllocMemory(size_t memorySize)
        {
        HDEBUGCODE(BeAssert(m_isAvailable););
        if (memorySize > m_size)
            {
            m_buf.reset(new Byte[memorySize]);
            m_size = memorySize;
            }

        HDEBUGCODE(m_isAvailable = false;);
        return m_buf.get();
        }

    void FreeMemory(Byte* pMem)
        {
        BeAssert(pMem == m_buf.get());
        HDEBUGCODE(BeAssert(!m_isAvailable););
        HDEBUGCODE(m_isAvailable = true;);
        }

private:
    SingleBlockAllocator(SingleBlockAllocator const&) = delete;
    SingleBlockAllocator& operator=(SingleBlockAllocator const&) = delete;

    unique_ptr<Byte[]> m_buf;
    size_t m_size;
    bool m_isAvailable;
};

/*---------------------------------------------------------------------------------**//**
* ConcurrencyAllocator
+---------------+---------------+---------------+---------------+---------------+------*/
struct ConcurrencyAllocator
{
public:
    ConcurrencyAllocator() { ; }

    Byte* AllocMemory(size_t memorySize)
        {
#if defined (ANDROID) || defined (__APPLE__)
        return (Byte*)malloc(memorySize);   //DM-Android
#elif defined (_WIN32)
        return (Byte*)Concurrency::Alloc(memorySize); 
#endif

        }
    void FreeMemory(Byte* pMem)
        {
#if defined (ANDROID) || defined (__APPLE__)
        free(pMem);   //DM-Android
#elif defined (_WIN32)
        Concurrency::Free(pMem);
#endif
        }

private:
    ConcurrencyAllocator(ConcurrencyAllocator const&) = delete;
    ConcurrencyAllocator& operator=(ConcurrencyAllocator const&) = delete;
};


/*---------------------------------------------------------------------------------**//**
* RawBuffer_T
+---------------+---------------+---------------+---------------+---------------+------*/
template <class Type_T, class Allocator_T>
struct RawBuffer_T
{
public:
    typedef  Type_T&       reference;
    typedef  Type_T const& const_reference;

    RawBuffer_T(size_t count, Allocator_T& allocator) : m_allocator(allocator)
        {
        m_buf = (Type_T*)m_allocator.AllocMemory(count*sizeof(Type_T));
        }

    ~RawBuffer_T()
        {
        m_allocator.FreeMemory((Byte*)(m_buf));
        }

    reference       At(size_t pos)       { return m_buf[pos]; }
    const_reference At(size_t pos) const { return m_buf[pos]; }

    Type_T*       GetBufferP()        { return m_buf; }
    Type_T const* GetBufferCP() const { return m_buf; }

private:
    RawBuffer_T() = delete;
    RawBuffer_T(RawBuffer_T const&) = delete;
    RawBuffer_T& operator= (RawBuffer_T const&) = delete;

    Allocator_T& m_allocator;
    Type_T*      m_buf;
};

/*----------------------------------------------------------------------------+
|struct HRAImageSampler
+----------------------------------------------------------------------------*/
struct HRAImageSampler
{
public:
    HRAImageSampler(HRPPixelNeighbourhood const& neighbourhood, HFCPtr<HGF2DTransfoModel>& destToSrcTransfo);
    virtual ~HRAImageSampler();

    //! Compute a new sample. The returned sample can reference the original surface so it must be considered as a READ-ONLY sample.
    HRAImageSamplePtr ComputeSample(ImagePPStatus& status, uint32_t width, uint32_t height, PixelOffset const& outOffset, 
                                    HRAImageSurfaceR inData, PixelOffset const& inOffset,
                                    IImageAllocatorR allocator);
    
    HRPPixelNeighbourhood const& GetNeighbourdhood() const;

    HGF2DTransfoModel const& GetTransfoModel() const;

    bool IsStretchable() const;
    bool PreservesLinearity() const;

    // ** For google tests only **
    ImagePPStatus test_Stretch(HRAImageSampleR outData, PixelOffset const& outOffset, HRAImageSurfaceR inData, PixelOffset const& inOffset);
    ImagePPStatus test_Warp(HRAImageSampleR outData, PixelOffset const& outOffset, HRAImageSurfaceR inData, PixelOffset const& inOffset);
    
    //! Samplers create methods. NULL is returned when pixeltype is not supported.
    static HRAImageSampler* CreateNearestRle(HFCPtr<HGF2DTransfoModel>& destToSrcTransfo, HRPPixelType const& pixelType);
    static HRAImageSampler* CreateNearestN1(HFCPtr<HGF2DTransfoModel>& destToSrcTransfo, HRPPixelType const& pixelType);
    static HRAImageSampler* CreateNearestN8(HFCPtr<HGF2DTransfoModel>& destToSrcTransfo, HRPPixelType const& pixelType);
    static HRAImageSampler* CreateBilinear(HFCPtr<HGF2DTransfoModel>& destToSrcTransfo, HRPPixelType const& pixelType);
    static HRAImageSampler* CreateBicubic(HFCPtr<HGF2DTransfoModel>& destToSrcTransfo, HRPPixelType const& pixelType);

protected:
    //! Fill the output sample. Assumed that buffer is already allocated.
    virtual ImagePPStatus _Stretch(HRAImageSampleR outData, PixelOffset const& outOffset, HRAImageSurfaceR inData, PixelOffset const& inOffset) = 0;

    //! Fill the output sample. Assumed that buffer is already allocated.
    virtual ImagePPStatus _Warp(HRAImageSampleR outData, PixelOffset const& outOffset, HRAImageSurfaceR inData, PixelOffset const& inOffset) = 0;

    HRAImageSamplePtr Translate(ImagePPStatus& status, uint32_t width, uint32_t height, HRAImageSurfaceR inData, PixelOffset const& offset, IImageAllocatorR allocator);

    HFCPtr<HGF2DTransfoModel> m_pDestToSrcTransfo;
    HRPPixelNeighbourhood     m_neighbourhood;

    bool   m_isStretchable;
    double m_offsetX;       // valid only when m_isStretchable is true.
    double m_offsetY;       // valid only when m_isStretchable is true.
    double m_scaleX;        // valid only when m_isStretchable is true.
    double m_scaleY;        // valid only when m_isStretchable is true.
    bool   m_preservesLinearity;
    bool   m_hasTranslationOnly;

    // ***** Begin Not-thread safe *****
    // If we want sampler to be threadable, this memory will need to come from somewhere else
    SingleBlockAllocator m_allocator; // Not thread safe
    // ***** End Not-thread safe *****

    bool m_enableMultiThreading;

private:
    HRAImageSampler();
    HRAImageSampler(HRAImageSampler const&) = delete;
    HRAImageSampler& operator=(HRAImageSampler const&) = delete;
};

END_IMAGEPP_NAMESPACE
