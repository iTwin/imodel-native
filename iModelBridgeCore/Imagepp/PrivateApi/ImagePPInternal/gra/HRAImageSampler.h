//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
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
    SingleBlockAllocator() : m_size(0)
#if defined(__HMR_DEBUG)
    , m_isAvailable(true)
#endif
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

    enum {IsThreadSafe = 0};

private:
    SingleBlockAllocator(SingleBlockAllocator const&) = delete;
    SingleBlockAllocator& operator=(SingleBlockAllocator const&) = delete;

    unique_ptr<Byte[]> m_buf;
    size_t m_size;
#if defined(__HMR_DEBUG) // otherwise flagged as unused
    bool m_isAvailable;
#endif
};

//----------------------------------------------------------------------------------------
// @bsiclass                                                     Mathieu.Marchand  1/2016
//----------------------------------------------------------------------------------------
struct SequentialLineExecutor
    {    
    typedef SingleBlockAllocator Allocator;

    SequentialLineExecutor(SingleBlockAllocator& allocator):m_allocator(allocator){}

    Allocator& GetAllocator() {return m_allocator;}

    template <typename lineFunction>
    void ForEachLine(uint32_t count, const lineFunction& func)
        {
        for (uint32_t line = 0; line < count; ++line)
            func(line);
        }

    Allocator& m_allocator;
    };

#ifdef HAVE_CONCURRENCY_RUNTIME
//----------------------------------------------------------------------------------------
// @bsiclass                                                   Mathieu.Marchand  1/2016
//----------------------------------------------------------------------------------------
struct ConcurrencyLineExecutor
    {
    struct Allocator
        {
        public:
            Allocator() { ; }

            Byte* AllocMemory(size_t memorySize)
                {
                return (Byte*)Concurrency::Alloc(memorySize); 
                }
            void FreeMemory(Byte* pMem)
                {
                Concurrency::Free(pMem);
                }
        private:
            Allocator(Allocator const&) = delete;
            Allocator& operator=(Allocator const&) = delete;
        };

    Allocator& GetAllocator() {return m_allocator;}

    template <typename lineFunction>
    static void ForEachLine(uint32_t count, const lineFunction& func)
        {
        Concurrency::parallel_for<uint32_t>(0, count, func);
        }

    Allocator m_allocator;
    };
#endif

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
    IMAGEPPTEST_EXPORT ImagePPStatus test_Stretch(HRAImageSampleR outData, PixelOffset const& outOffset, HRAImageSurfaceR inData, PixelOffset const& inOffset);
    IMAGEPPTEST_EXPORT ImagePPStatus test_Warp(HRAImageSampleR outData, PixelOffset const& outOffset, HRAImageSurfaceR inData, PixelOffset const& inOffset);
    
    //! Samplers create methods. NULL is returned when pixeltype is not supported.
    IMAGEPPTEST_EXPORT static HRAImageSampler* CreateNearestRle(HFCPtr<HGF2DTransfoModel>& destToSrcTransfo, HRPPixelType const& pixelType);
    IMAGEPPTEST_EXPORT static HRAImageSampler* CreateNearestN1(HFCPtr<HGF2DTransfoModel>& destToSrcTransfo, HRPPixelType const& pixelType);
    IMAGEPPTEST_EXPORT static HRAImageSampler* CreateNearestN8(HFCPtr<HGF2DTransfoModel>& destToSrcTransfo, HRPPixelType const& pixelType);
    IMAGEPPTEST_EXPORT static HRAImageSampler* CreateBilinear(HFCPtr<HGF2DTransfoModel>& destToSrcTransfo, HRPPixelType const& pixelType);
    IMAGEPPTEST_EXPORT static HRAImageSampler* CreateBicubic(HFCPtr<HGF2DTransfoModel>& destToSrcTransfo, HRPPixelType const& pixelType);

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
    SingleBlockAllocator m_singleBlockAllocator;
    // ***** End Not-thread safe *****

    bool m_enableMultiThreading;

private:
    HRAImageSampler();
    HRAImageSampler(HRAImageSampler const&) = delete;
    HRAImageSampler& operator=(HRAImageSampler const&) = delete;
};

END_IMAGEPP_NAMESPACE
