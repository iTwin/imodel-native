//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRAImageOp.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#pragma once

#include <Bentley/Bentley.h>
#include <Bentley/RefCounted.h>
#include <ImagePP/h/HmrMacro.h>
#include <ImagePP/all/h/HFCGrid.h>
#include <ImagePP/all/h/HPMPool.h>


BEGIN_IMAGEPP_NAMESPACE 

class HRPPixelType;
class HRPPixelNeighbourhood;
class HRPPixelConverter;
class HRABitmap;
class HRATiledRaster;
class HGF2DTransfoModel;


template <class T>
struct PixelOffset_T
    {
    PixelOffset_T(T const& xIn, T const& yIn) :x(xIn), y(yIn){}
    T x;
    T y;
    };

typedef PixelOffset_T<int64_t>  PixelOffset64;
typedef PixelOffset_T<double> PixelOffset;

/*----------------------------------------------------------------------------+
|struct HRAImageBuffer_0
| An image buffer
|
| TO EVAL:
|   - must be able to support RLE by line. i.e. contiguous or not. 
|       - HRAImageBufferRLE -> RLE allocated by lines or what ever.
|       - HRAImageBufferTiled -> contains tiles.
|       - HRAImageBufferWrapper -> Wrap an existing ImageBuffer with an offset.
|   - Can we support tiled raster here? or maybe it's image sample that holds multiple buffers.
|   - Should we support negative pitch for bottom-up bitmap? We could avoid the cost of flipping the data.
|     before entering the pipeline.       
|   - Do we need to size_t?  I'm not sure we will ever work with more than 4 GB image buffer.
|   - Not sure were the pitch should reside. Since it require the width concept. it must be set
|   - when editing buffer? What is the purpose of HRAImageSample then?
+----------------------------------------------------------------------------*/
struct HRAImageBuffer : public RefCountedBase
{
public:
    //! Get a read/write access to the first scanline of an image,
    Byte* GetDataP(size_t& pitch);

    //! Get a read only access to the first scanline.
    Byte const* GetDataCP(size_t& pitch) const {return const_cast<HRAImageBuffer*>(this)->GetDataP(pitch);}

    HRAImageBufferRleP AsBufferRleP();

protected:
    HRAImageBuffer() = delete;
    HRAImageBuffer(HRAImageBuffer const&) = delete;
    HRAImageBuffer(Byte* pData, size_t bufferSize, size_t pitch);
    virtual ~HRAImageBuffer();
    
    virtual HRAImageBufferRleP _AsBufferRleP() {return NULL;}

    //! Protected because it could be misleading for parasite buffer.  All buffers access should use pitch.
    size_t GetBufferSize() const;  

    // data related members are hold by the base class to ease support in BeImageVisualizer
    Byte*   m_pData;
    size_t  m_bufferSize;
    size_t  m_pitch;
};

/*----------------------------------------------------------------------------+
|struct HRAImageBufferMemory
+----------------------------------------------------------------------------*/
struct HRAImageBufferMemory : public HRAImageBuffer
{
public:
    //! Allocate a new memory buffer. May return IMAGEPP_STATUS_OutOfMemory
    static HRAImageBufferPtr CreateMemoryBuffer(ImagePPStatus& status, size_t sizeInBytes, size_t pitch, IImageAllocatorR allocator);

protected:
    HRAImageBufferMemory(Byte* pData, size_t bufferSize, size_t pitch, IImageAllocatorR allocator);
    virtual ~HRAImageBufferMemory();

    IImageAllocatorR m_allocator;
};

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct HRAImageBufferRle : public HRAImageBufferMemory
{
public:
    //! Allocate a new memory buffer. May return IMAGEPP_STATUS_OutOfMemory
    static HRAImageBufferPtr CreateRleBuffer(ImagePPStatus& status, uint32_t width, uint32_t height, IImageAllocatorR allocator);

    size_t GetLineDataSize(uint32_t line) const;
    void SetLineDataSize(uint32_t line, size_t dataSize);

    void ComputeDataSize(uint32_t line, uint32_t width);
protected:
    virtual HRAImageBufferRleP _AsBufferRleP() override {return this;}

private:
    HRAImageBufferRle(Byte* pData, size_t bufferSize, size_t pitch, uint32_t height, IImageAllocatorR allocator);
    virtual ~HRAImageBufferRle();

    bvector<uint32_t> m_linesDataSize;        // Should be size_t but we use UInt32 to cut the allocated space in half.
};


/*----------------------------------------------------------------------------+
|struct HRAImageSample
| A raster image in whole or part.
|
| TO EVAL:
|   - Can it hold multiple buffers? ex. many tiles?
|   - Would be nice to support a tiled or strip/line sample. ex. with multiple buffers.
|     Sample would have specialized version to access these specialized data organization but
|     would provide a generic GetContiguousBuffer where the buffers would be merge into a single one
|     and all multiple buffers will then be replace by the contiguous version. Doing so would ease 
|     support for contiguous vs non-contiguous data.
+----------------------------------------------------------------------------*/
struct HRAImageSample : public RefCountedBase, public NonCopyableClass
{
public:
    //! Create a new sample using the speficied allocator.
    static HRAImageSamplePtr CreateSample(ImagePPStatus& status, uint32_t width, uint32_t height, HFCPtr<HRPPixelType> pixelType, IImageAllocatorR allocator);

    //! The width in pixels.
    uint32_t GetWidth() const;

    //! The height in pixels.
    uint32_t GetHeight() const;

    HRAImageBufferRleP GetBufferRleP();
    HRAImageBufferRleCP GetBufferRleCP() const {return const_cast<HRAImageSample*>(this)->GetBufferRleP();}

    //! Retrieve image data. May be NULL.
    HRAImageBufferP GetBufferP();

    //! Retrieve image data. May be NULL.
    HRAImageBufferCP GetBufferCP() const {return const_cast<HRAImageSample*>(this)->GetBufferP();}

    //! Attempt to create a reference to the input sample.  May fail if input is too small or pixeltype is not capable of reference(N1, RLE). 
    HRAImageSamplePtr CreateBufferReference(ImagePPStatus& status, uint32_t width, uint32_t height, uint32_t xOrigin, uint32_t yOrigin);

    //! Worst-case buffer size requirement.
    size_t GetMaxScanlineSize() const;  

    HRPPixelType const& GetPixelType() const;

    HFCPtr<HRPPixelType>& GetPixelTypePtr();

#if defined __HMR_DEBUG
    void MarkContour();
#endif
   
    // Do not call. we expose it because we need it in our unit tests.
    void internal_SetBuffer(HRAImageBufferPtr& buffer){ SetBuffer(buffer); }

    //! Used by packet surface that reference outside data.
    static HRAImageSamplePtr internal_CreateSampleFromBuffer(ImagePPStatus& status, uint32_t width, uint32_t height, HFCPtr<HRPPixelType> pixelType, HRAImageBufferR buffer);

    bool ValidateIntegrity() const;

protected:

    //! Allocate a uninitialized buffer that match image sample requirements( N1, N8, RLE...)
    ImagePPStatus AllocateBuffer(IImageAllocatorR allocator);

    //! Set buffer in sample.
    void SetBuffer(HRAImageBufferPtr& buffer);
    HRAImageSample(uint32_t width, uint32_t height, HFCPtr<HRPPixelType>& pixelType);

    uint32_t m_width;
    uint32_t m_height;

    HFCPtr<HRPPixelType> m_pPixelType;
    
    HRAImageBufferPtr m_pBuffer;
};

//=======================================================================================
//! Hold Information about the operation.
//! @bsiclass                                                     
//=======================================================================================
struct ImagepOpParams
{
public:
    ImagepOpParams(HGF2DTransfoModel const& transfo)
        :m_destToSrcTransfo(transfo)
        {
        m_offsetX = m_offsetY = 0;
        }
    
    //! The current input block offset from the whole destination.
    int64_t GetOffsetX() const {return m_offsetX;}
    int64_t GetOffsetY() const {return m_offsetY;}

    void SetOffset(int64_t offsetX, int64_t offsetY) {m_offsetX=offsetX; m_offsetY=offsetY;}

    HGF2DTransfoModel const& GetTransfoModel() const {return m_destToSrcTransfo;}
private:
    //! Can be NULL is Identity.
    HGF2DTransfoModel const& m_destToSrcTransfo;
        
    int64_t m_offsetX;
    int64_t m_offsetY;

    //EN: add the possibility to store imageOp private data.  Anything that should be allocated per thread
    //               could belong here.
    //              Use so sort of imageOp Ids to have direct access to the stored data?
};


/*----------------------------------------------------------------------------+
|struct HRAImageOp
| - Can be called by multiple thread.
| - Ideally, state less. No lock/unlock required.
|   Only member that are valid to all Process operation are acceptable. ex. contrast percentage.  
|   It is invalid to share a temporary working buffer.
| - In some circumstance lock/unlock are required:
|   How to histogram?
|       - Add an histogram accumulator member and an access key.
|       - Process: Accumulate current Process() in a stack allocated object. No lock/unlock during process!
|       - Post: Lock accumulator. add intermediate result(should be fast). unlock accumulator.
|
|   How we link operation:    
|       The most important item is that we expect to preserve the input pixel type as long as possible
|       since the image operations should probably be performed as much as possible upon the native original
|       pixel type so we try to maintain the input pixel type as long as we can. 
|
|       First ImageOP:
|           1) Call SetInputPixelType() with the source pixeltype
|               if failed, add converter to the first available input.
|              
|       All ImageOP:
|           1) Call SetOutputPixelType() with the ImageOP input pixeltype. >>> try to preserve pixeltype.
|               if failed, use the first available output
|           1) Call SetInputPixelType() on the next ImageOp.
|               if failed, try all the available output pixeltype from the previous ImageOp.
|                   if failed, add converter to the first available input.
|
|       Last ImageOp:
|           1) Call SetOutputPixelType() with the destination pixeltype.
|               if failed use first available output. The output merger will do the final conversion.
|
|+----------------------------------------------------------------------------*/
struct HRAImageOp : public RefCountedBase, public NonCopyableClass
    {
    enum ImageOpFlags
        {
        OPPARAM_InPlace = (1<<1),
        //OPPARAM_InPlace = (1<<2),
        };

    //! The current input pixeltype or NULL is not set. 
    HRPPixelType* GetInputPixelType() const;

    //! The current output pixeltype or NULL is not set.
    HRPPixelType* GetOutputPixelType() const;

    //! the pixel neighbourhood required by this operation. Use to provide the appropriate amount of input data.
    HRPPixelNeighbourhood const& GetNeighbourhood() const;

    //! Retrieve supported intput pixelType. 'index' is zero-based and will be incremented to request to next available pixeltype.
    //! Available pixel type must be returned in order of preference. 
    //! 'pixelTypeToMatch' is the desired pixelType. ImageoP should do their best to provide suggestion that will 
    //! preserve bit depth and alpha channel.
    //! @param[out]  pixelType          The available pixel type or NULL.
    //! @param[in]   index              The current request index. Return IMAGEOP_STATUS_NoMorePixelType when done.
    //! @param[in]   pixelTypeToMatch   Try to get a pixelType similar to this one (try to preserve bit depth, alpha channel).
    //! @return IMAGEPP_STATUS_Success  
    //!         IMAGEPP_STATUS_UnknownError               Unknown error.
    //!         IMAGEOP_STATUS_NoMorePixelType     We reach the end of available pixeltype list.
    ImagePPStatus GetAvailableInputPixelType(HFCPtr<HRPPixelType>& pixelType, uint32_t index, const HFCPtr<HRPPixelType> pixelTypeToMatch);
    
    //! Retrieve supported output pixelType. 'index' is zero-based and will be incremented to request to next available pixeltype.
    //! Available pixel type must be returned in order of preference. 
    //! 'pixelTypeToMatch' is the desired pixelType.  ImageoP should do their best to provide suggestion that will 
    //! preserve bit depth and alpha channel.
    //! @param[out]  pixelType          The available pixel type or NULL.
    //! @param[in]   index              The current request index. Return IMAGEOP_STATUS_NoMorePixelType when done.
    //! @param[in]   pixelTypeToMatch   Try to get a pixelType similar to this one (try to preserve bit depth, alpha channel).
    //! @return IMAGEPP_STATUS_Success  
    //!         IMAGEPP_STATUS_UnknownError               Unknown error.
    //!         IMAGEOP_STATUS_NoMorePixelType     We reach the end of available pixeltype list.
    ImagePPStatus GetAvailableOutputPixelType(HFCPtr<HRPPixelType>& pixelType, uint32_t index, const HFCPtr<HRPPixelType> pixelTypeToMatch);

    //! Set the current input pixel type of this operation. If type is not supported, return IMAGEOP_STATUS_InvalidPixelType.
    //! The pixel type can be NULL. In this case, the methods clears the pixel type and every member dependent on the pixel type.
    ImagePPStatus SetInputPixelType(HFCPtr<HRPPixelType> pixelType);

    //! Set the current output pixel type of this operation. If type is not supported, return IMAGEOP_STATUS_InvalidPixelType
    //! The pixel type can be NULL. In this case, the methods clears the pixel type and every member dependent on the pixel type.
    ImagePPStatus SetOutputPixelType(HFCPtr<HRPPixelType> pixelType);

    //! Process pixels here. This is where performance is important. You can assert(debug) for valid input/output but otherwise assumed that everything is valid.
    //! Image sample size may differ but will never exceed the max sample size.
    ImagePPStatus Process(HRAImageSampleR out, HRAImageSampleCR inputData, ImagepOpParams& params);

#if 0 
    // &&Backlog EN:
    //    - MultiThread: If a local temporary buffer is required. How to do it and remain state less?
    //    - OPTIMIZATION: Add inplace support. How ca we tell we have RW to input.
    //    - Prepare/cleanup: We somehow should be able to keep stuff prepared until the source or pipeline operation change. Ex. DEM filter keep
    //      a prepared range map specific to the current input. It is setup in HRADEMFilter constructor and kept until the decoration is dropped. So the 
    //      prepare is executed once and reuse for every subsequent CopyFrom.

    ImagePPStatus ProcessInPlace(HRAImageSampleR imageData, ImagepOpParams& params) {return IMAGEPP_STATUS_UnknownError;}
    
    //! This is where image op should pre-process table look-up and stuff...
    //! ---Thread: Will be called by each working thread. Any non shareable data should be stored in pImageOpData.
    ImagePPStatus Prepare(ImagepOpParams& imageOpParam);
    
    //! Call when all data has been process (or operation aborted) so every filter can free allocated memory.
    //! ---Thread: Will be called by each working thread. 
    ImagePPStatus Cleanup(ImagepOpParams& imageOpParam);
#endif
    
protected:
    virtual ImagePPStatus _GetAvailableInputPixelType(HFCPtr<HRPPixelType>& pixelType, uint32_t index, const HFCPtr<HRPPixelType> pixelTypeToMatch) = 0;
    virtual ImagePPStatus _GetAvailableOutputPixelType(HFCPtr<HRPPixelType>& pixelType, uint32_t index, const HFCPtr<HRPPixelType> pixelTypeToMatch) = 0;

    virtual ImagePPStatus _SetInputPixelType(HFCPtr<HRPPixelType> pixelType) = 0;
    virtual ImagePPStatus _SetOutputPixelType(HFCPtr<HRPPixelType> pixelType) = 0;

    virtual ImagePPStatus _Process(HRAImageSampleR out, HRAImageSampleCR inputData, ImagepOpParams& params) = 0;

    virtual ImagePPStatus _ProcessInPlace(HRAImageSampleR imageData, ImagepOpParams& params) {return IMAGEPP_STATUS_UnknownError;}


    HRAImageOp();
    HRAImageOp(HFCPtr<HRPPixelNeighbourhood> pPixelNeighbourhood);
    virtual ~HRAImageOp();

    HFCPtr<HRPPixelType>            m_pInputPixelType;
    HFCPtr<HRPPixelType>            m_pOutputPixelType;
    HFCPtr<HRPPixelNeighbourhood>   m_pPixelNeighbourhood;
    };

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct HRAImageOpPixelConverter: public HRAImageOp
{
public:

    static HRAImageOpPtr CreatePixelConverter(bool alphaBlend);

protected:
    virtual ImagePPStatus _GetAvailableInputPixelType(HFCPtr<HRPPixelType>& pixelType, uint32_t index, const HFCPtr<HRPPixelType> pixelTypeToMatch) override;

    virtual ImagePPStatus _GetAvailableOutputPixelType(HFCPtr<HRPPixelType>& pixelType, uint32_t index, const HFCPtr<HRPPixelType> pixelTypeToMatch) override;

    virtual ImagePPStatus _SetInputPixelType(HFCPtr<HRPPixelType> pixelType) override;

    virtual ImagePPStatus _SetOutputPixelType(HFCPtr<HRPPixelType> pixelType) override;

    virtual ImagePPStatus _Process(HRAImageSampleR out, HRAImageSampleCR inputData, ImagepOpParams& params) override;

private:

    HRAImageOpPixelConverter(bool alphaBlend);
    virtual ~HRAImageOpPixelConverter();
    
    HFCPtr<HRPPixelConverter> m_pConverter;
    bool                      m_alphaBlend;
};

typedef std::list<HRAImageOpPtr> ImageOpList;
   
/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct HRAImageOpPipeLine
{
public:
    typedef ImageOpList::iterator  ImageOpItr;

    HRAImageOpPipeLine();
    ~HRAImageOpPipeLine();

    //! Adds and image operation at the end of the pipeline.
    //! After adding an image operation, the pipe-line is considered un-prepared.
    ImagePPStatus AddImageOp(HRAImageOpPtr imageOp, bool atFront);

    //! Removes all image operations from the pipeline.
    //! After adding an image operation, the pipe-line is considered un-prepared.
    void Clear();

    //!  Return true if pipeline is empty.
    bool IsEmpty() const;

    //! Returns the number of image operations in the pipeline
    size_t GetCount() const;

    //! This method performs the pipeline preparation based on the information provided. 
    //! The preparation here is applicable to many process calls. It will include computation 
    //! of neighborhood size and pixel type handshaking.
    ImagePPStatus Prepare(HFCPtr<HRPPixelType> inputPixelType, HFCPtr<HRPPixelType> outputPixelType);
    
    //! This method performs the actual process of applying the pipeline operations.
    //! Upon calling the Process() method the input and output image are provided.
    //! Any temporary buffers will be allocated and deallocated according to the
    //! rule implemented in the memory manager.
    HRAImageSamplePtr Process(ImagePPStatus& status, HRAImageSampleR inSample, ImagepOpParams& params, IImageAllocatorR allocator);
   
    //! The neighbourhood of all imageOp.
    HRPPixelNeighbourhood const& GetNeighbourdhood() const;

    //! Returns the output pixel type of the pipeline
    HFCPtr<HRPPixelType> GetOutputPixelType() const;
    
    bool IsReady() const { return m_prepared; }

    ImageOpList const& GetImageOps() const;

    //! Reset all ImageOPs input/output pixeltype, remove PixelConverters and mark the pipeline as not prepared. 
    // *** Not sure it is good idea to remove PixelConverters they might have been added by the caller. 
    //void Reset();
    
private:
    ImagePPStatus AddConverterToFirstAvailableInputPixelType(ImageOpItr& whereItr, HFCPtr<HRPPixelType> pPixelTypeToMatch);

    bool m_prepared;
    ImageOpList m_imageOpList;
    mutable HFCPtr<HRPPixelNeighbourhood> m_pPixelNeighbourhood;

  
};

END_IMAGEPP_NAMESPACE
