//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PrivateApi/ImagePPInternal/gra/HRAImageNode.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#pragma once

#include <Bentley/Bentley.h>
#include <Bentley/RefCounted.h>
#include <ImagePP/h/HmrMacro.h>

#include <ImagePP/all/h/HRAImageOp.h>
#include <ImagePP/all/h/HGF2DExtent.h>
#include <ImagePP/all/h/HGSTypes.h>
#include <ImagePPInternal/gra/ImageCommon.h>
#include <ImagePPInternal/gra/HRAImageSurface.h> 

BEGIN_IMAGEPP_NAMESPACE 

class HRPPixelType;
class HGF2DCoordSys;
class HVEShape;
struct ImageOutputMerger;
class HIMMosaic;
class HRARaster;
class HRATransaction;
struct HRAImageSampler;

//=======================================================================================
// An imageNode have an input and output pixel type.  Some node cannot receive(Leaf) or produce(root), in these case the pixeltype is NULL.
// Pixeltype must be provided at construction and cannot change. Some have fixed pixeltype and some are flexible.
// @bsiclass                                                    
//=======================================================================================
struct ImageNode : public RefCountedBase
{
public:
    typedef std::vector<ImageNodePtr> Children;

    uint32_t GetChildCount() const { return (uint32_t)m_children.size(); }
    
    ImageNodeP GetChildP(uint32_t index) const { return index < GetChildCount() ? m_children[index].get() : NULL; }
    ImageNodeCP GetChildCP(uint32_t index) const {return const_cast<ImageNode*>(this)->GetChildP(index);}

    ImageNodeP GetParentP() { return m_parent; }
    ImageNodeCP GetParentCP() const { return m_parent; }

    ImagePPStatus LinkTo(ImageNodeR child);

    HFCPtr<HGF2DCoordSys>& GetPhysicalCoordSys();
    HGF2DExtent const& GetPhysicalExtent() const;
    HFCPtr<HRPPixelType> GetPixelType();
    ImageTransformNodeP AsImageTransformNodeP();
    ImageSourceNodeP    AsImageSourceNodeP();

    ImagePPStatus PrepareForStrip(HFCInclusiveGrid const& strip);

    //! Image Node and Image sample use the pixel type to differentiate between 1bit and Rle data. Unlike HRABitmap[RLE] that use pixeltype and codec.
    //! This method will create an Rle equivalent pixeltype or return the provided pixeltype if it is already RLE.
    static HFCPtr<HRPPixelType> TransformToRleEquivalent(HFCPtr<HRPPixelType> pixelType);

protected:
    ImageNode();
    virtual ~ImageNode();

    virtual ImageTransformNodeP _AsImageTransformNodeP() { return NULL; }
    virtual ImageSourceNodeP    _AsImageSourceNodeP()    { return NULL; }

    virtual HFCPtr<HRPPixelType> _GetPixelType() = 0;
    virtual HGF2DExtent const& _GetPhysicalExtent() const = 0;
    virtual HFCPtr<HGF2DCoordSys>& _GetPhysicalCoordSys() = 0;
    virtual ImagePPStatus _PrepareForStrip(HFCInclusiveGrid const& strip);

    ImageNodeP m_parent;    // Our parent hold a ref to this.
    Children m_children;    // Hold a ref to our children

private:
    ImageNode(ImageNode&&) = delete;
    ImageNode(ImageNode const&) = delete;
    ImageNode& operator=(ImageNode&&) = delete;
    ImageNode& operator=(ImageNode const&) = delete;

    void AddChild(ImageNodeR child);
};

//=======================================================================================
// @bsiclass                                                    
//=======================================================================================
struct ImageTransformNode : ImageNode
{
public:
    static ImageTransformNodePtr CreateAndLink(ImagePPStatus& status, ImageNodeR parent, HFCPtr<HVEShape>& pShape);

    void AddImageOp(HRAImageOpPtr imageOp, bool atFront);

    void SetResamplingMode(HGSResampling const& resampling);
    
    //! Blend with destination when source has alpha.
    void SetAlphaBlend(bool enableBlend);

    //! The current region to process. Available only after a call to PrepareForStrip
    HFCInclusiveGrid const& GetCurrentRegionGrid() const;

    //! Clip is apply during produce so sample region might be bigger than the effective produced region.
    ImagePPStatus Produce(HRAImageSurfaceR surface, PixelOffset64 const& offset, IImageAllocatorR allocator);

    HFCPtr<HRPPixelType> GetInputPixelType();
    HFCPtr<HRPPixelType> GetOutputPixelType();

    ImageSourceNodeP GetSourceNodeP();
    ImageSourceNodeCP GetSourceNodeCP() const {return const_cast<ImageTransformNode*>(this)->GetSourceNodeP();}

protected:
    virtual ImageTransformNodeP    _AsImageTransformNodeP() override { return this; }
    virtual ImagePPStatus          _PrepareForStrip(HFCInclusiveGrid const& strip) override;
    virtual HGF2DExtent const&     _GetPhysicalExtent() const override;
    virtual HFCPtr<HGF2DCoordSys>& _GetPhysicalCoordSys() override;
    virtual HFCPtr<HRPPixelType> _GetPixelType() override { return GetOutputPixelType(); };

private:
    ImageTransformNode();

    HRAImageSampler* AllocateSampler(HFCPtr<HGF2DTransfoModel>& destToSrcTransfo);

    bool ComputeSourceGridFromDestination(HFCInclusiveGrid* sourceGrid, HFCInclusiveGrid& dest0Grid, HFCInclusiveGrid const& destGrid);

    //! This instance will hold a ref to pShape. The shape must be in physical coordSys.
    void SetClipShape(HFCPtr<HVEShape>& pShape);

    HGSResampling               m_resampling;
    HFCPtr<HVEShape>            m_pClipShape;
    bool                        m_alphaBlend;
    HRAImageOpPipeLine          m_pipe;
    std::unique_ptr<ImageOutputMerger> m_pOutputMerger;
    std::unique_ptr<HRAImageSampler>   m_pSrcSampler;
};

//=======================================================================================
// @bsiclass                                                    
//=======================================================================================
struct ImageSourceNode : ImageNode
{
public:
    //! Return a pixel region for read access. The requested region must be within the physical image extent otherwise an error is returned.
    //! The returned region might be the original pixels or a working copy. To avoid unnecessary copy, the returned sample might be bigger than the 
    //! requested region, 'outOffset' will reflect the effective sample position relative to the source origin.
    ImagePPStatus GetRegion(HRAImageSurfacePtr& pOut, PixelOffset64& outOffset, HFCInclusiveGrid const& region, IImageAllocatorR allocator);
    
    
    //! Sets the scale factor of the image data represented by this node.
    //! This is required for imageOps that needs
    void SetScaleFactorFromMain(double scaleX, double scaleY) 
        { 
        BeAssert(scaleX >= 1.0 && scaleY >= 1.0);
        m_scaleFromMainX = scaleX;
        m_scaleFromMainY = scaleY;
        }

    //! Get the scale factor of the image data represented by this node.
    void GetScaleFactorFromMain(double& scaleX, double& scaleY)
        {
        scaleX = m_scaleFromMainX;
        scaleY = m_scaleFromMainY;
        }

protected:
    ImageSourceNode(HFCPtr<HRPPixelType> pPixelType);
    virtual ImagePPStatus _GetRegion(HRAImageSurfacePtr& pOut, PixelOffset64& outOffset, HFCInclusiveGrid const& region, IImageAllocatorR allocator) = 0;
    
    virtual HFCPtr<HRPPixelType> _GetPixelType() override;

    virtual ImageSourceNode* _AsImageSourceNodeP() override { return this; }

    HFCPtr<HRPPixelType> m_pPixelType;
    double m_scaleFromMainX;
    double m_scaleFromMainY;
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  09/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct ImageSurfaceIterator : public NonCopyableClass
{
public:    
    ImageSurfaceIterator()
        :m_current(NULL),
        m_offset(0,0){}

    ImageSurfaceIterator(HRAImageSurfaceR surface, PixelOffset64 const& offset)
        :m_current(&surface),
        m_offset(offset)
        {
        }

    virtual ~ImageSurfaceIterator(){};

    virtual bool _Next() = 0;

    bool IsValid() const { return m_current != NULL; }

    void Invalidate() { m_current = NULL; }

    //! returned the current sample position in physical coordsys.
    HRAImageSurfaceR GetSurface() { return *m_current; }
    PixelOffset64 const& GetOffset() const { return m_offset; }

protected:
    void SetCurrent(HRAImageSurfaceR pSurface, PixelOffset64 const& offset)
        {
        m_current = &pSurface;
        m_offset = offset;
        }
    PixelOffset64      m_offset;    // The surface offset in relation to the raster it belong to.
    HRAImageSurfacePtr m_current;   // The current surface.
};

//=======================================================================================
// @bsiclass                                                    
//=======================================================================================
struct ImageSinkNode : ImageNode
{
public:
    //! Execute the sink operation.
    ImagePPStatus Execute(IImageAllocatorR allocator);

    HRATransaction* GetTransaction();
    void SetTransaction(HRATransaction* pTransaction);

    ImageSurfaceIterator* GetImageSurfaceIterator(HFCInclusiveGrid& strip);

protected:
    ImageSinkNode(HVEShape const& sinkShape, HFCPtr<HRPPixelType> pInput, HGF2DExtent const& physicalExtent);
    
    //! Provide information about the native block organization. Grid iteration logic will use this information to align ImageSurfaceIterator.
    virtual uint32_t _GetBlockSizeX() = 0;
    virtual uint32_t _GetBlockSizeY() = 0;
    
    virtual ImageSurfaceIterator* _GetImageSurfaceIterator(HFCInclusiveGrid& strip) = 0;
       
    virtual HFCPtr<HRPPixelType> _GetPixelType() override;
    virtual HGF2DExtent const& _GetPhysicalExtent() const override { return m_physicalExtent; }
    virtual HFCPtr<HGF2DCoordSys>& _GetPhysicalCoordSys() override { return const_cast<HFCPtr<HGF2DCoordSys>&>(m_physicalExtent.GetCoordSys()); }
     
    HFCInclusiveGrid        m_sinkGrid;
    HRATransaction*         m_pTransaction;    // undo/redo recording. Might be NULL.
    HFCPtr<HRPPixelType>    m_pPixelType;
    HGF2DExtent             m_physicalExtent;
};

//=======================================================================================
// @bsiclass                                                    
//=======================================================================================
struct MosaicNode : ImageSourceNode
    {
    private:
        HGF2DExtent           m_physicalExtent;
        HFCPtr<HGF2DCoordSys> m_pPhysicalCS;

    public:
        static RefCountedPtr<MosaicNode> Create(HRARaster& mosaic, HFCPtr<HGF2DCoordSys> pPhysicalCS, HGF2DExtent const& physExtent, HFCPtr<HRPPixelType>& pixelType);
        
    protected:
        MosaicNode(HRARaster& mosaic, HFCPtr<HGF2DCoordSys> pPhysicalCS, HGF2DExtent const& physExtent, HFCPtr<HRPPixelType>& pixelType);
        virtual ImagePPStatus _GetRegion(HRAImageSurfacePtr& pOut, PixelOffset64& outOffset, HFCInclusiveGrid const& region, IImageAllocatorR allocator) override;
        virtual HGF2DExtent const& _GetPhysicalExtent() const override;
        virtual HFCPtr<HGF2DCoordSys>& _GetPhysicalCoordSys() override;
    };

END_IMAGEPP_NAMESPACE

