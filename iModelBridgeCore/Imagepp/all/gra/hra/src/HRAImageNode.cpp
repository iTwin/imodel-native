//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hra/src/HRAImageNode.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <ImagePPInternal/gra/ImageCommon.h>
#include <ImagePPInternal/gra/HRAImageNode.h>
// #include <ImagePPInternal/gra/HRAImageSampler.h>
#include <Imagepp/all/h/HRPPixelType.h>
#include <ImagePP/all/h/HVEShape.h>
#include <ImagePP/all/h/HRPPixelConverter.h>
#include <ImagePP/all/h/HRPPixelNeighbourhood.h>
#include <ImagePP/all/h/HGF2DStretch.h>
// #include <ImagePP/all/h/HPMPool.h>
// #include <ImagePP/all/h/HGF2DTransfoModel.h>
// #include <ImagePP/all/h/HGF2DIdentity.h>
// #include <Imagepp/all/h/HRABitmap.h>
// #include <ImagePP/all/h/HCDPacket.h>
#include <ImagePP/all/h/HGFScanLines.h>
#include <ImagePPInternal/gra/HRAImageNearestSamplerN8.h>
#include <ImagePPInternal/gra/HRAImageBilinearSamplerN8.h>
#include <ImagePPInternal/gra/HRAImageBicubicSamplerN8.h>
#include <ImagePPInternal/gra/HRAImageNearestSamplerRLE.h>
#include <ImagePPInternal/gra/HRAImageNearestSamplerN1.h>
#include <ImagePP/all/h/HVE2DVoidShape.h>
#include <ImagePP/all/h/HVE2DUniverse.h>
#include <ImagePPInternal/gra/HRAImageSurface.h>
#include <ImagePP/all/h/HRPPixelTypeI1R8G8B8.h>
#include <ImagePP/all/h/HRPPixelTypeI1R8G8B8RLE.h>
#include <ImagePP/all/h/HRPPixelTypeI1R8G8B8A8.h>
#include <ImagePP/all/h/HRPPixelTypeI1R8G8B8A8RLE.h>
#include <ImagePPInternal/gra/HRAImageEditor.h>

//#define _Mark_Contour 1

#if defined __HMR_DEBUG && defined _Mark_Contour
#define MarkSampleContour(sampleP) sampleP->MarkContour();
#else
#define MarkSampleContour(sampleP)
#endif

#define MAX_STRIP_HEIGHT 256



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  12/2014
+---------------+---------------+---------------+---------------+---------------+------*/
static bool s_IsRLEPixelType(HRPPixelType const& pixelType)
    {
    return pixelType.IsCompatibleWith(HRPPixelTypeI1R8G8B8RLE::CLASS_ID) || pixelType.IsCompatibleWith(HRPPixelTypeI1R8G8B8A8RLE::CLASS_ID);
    }

typedef std::pair<uint64_t, uint64_t>   LeftRightPair;
typedef std::vector<LeftRightPair>  Scanline;
typedef std::vector<Scanline>       Scanlines;


/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct OutputMergerProcessor
    {
    OutputMergerProcessor(Scanlines const& scanlines, HFCInclusiveGrid const& grid, HRPPixelConverter const& converter)
    :m_scanLines(scanlines), m_grid(grid), m_converter(converter){}
    virtual ~OutputMergerProcessor(){};

    virtual ImagePPStatus _Process(PixelOffset64 const& outOffset, HRAImageSampleR inputData, PixelOffset64 const& inOffset) = 0;
    
protected:
    Scanlines const&            m_scanLines;
    HFCInclusiveGrid            m_grid;
    HRPPixelConverter const&    m_converter;
    };

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename Surface_T, class InputEditor_T, bool NeedCompose_T>
struct OutputMergerProcessorOptimizedN8 : public OutputMergerProcessor
{
public:
    OutputMergerProcessorOptimizedN8(Surface_T& outSurface, Scanlines const& scanlines, HFCInclusiveGrid const& grid, HRPPixelConverter const& converter)
    :OutputMergerProcessor(scanlines, grid, converter),
     m_outEditor(outSurface)
        {
        BeAssert(m_converter.GetDestinationPixelType()->GetClassID() == outSurface.GetPixelType().GetClassID());
        BeAssert(outSurface.GetPixelType().CountPixelRawDataBits() % 8 == 0);
        }

    virtual ~OutputMergerProcessorOptimizedN8(){};

    virtual ImagePPStatus _Process(PixelOffset64 const& outOffset, HRAImageSampleR inputData, PixelOffset64 const& inOffset) override;

private:
    typename Surface_T::ImageEditor m_outEditor;
};

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename Surface_T, class InputEditor_T, bool NeedCompose_T>
struct OutputMergerProcessorNormal : public OutputMergerProcessor
{
public:
    OutputMergerProcessorNormal(Surface_T& outSurface, Scanlines const& scanlines, HFCInclusiveGrid const& grid, HRPPixelConverter const& converter)
    :OutputMergerProcessor(scanlines, grid, converter),
     m_outEditor(outSurface)
        {
        BeAssert(m_converter.GetDestinationPixelType()->GetClassID() == outSurface.GetPixelType().GetClassID());
        }

    virtual ~OutputMergerProcessorNormal(){};

    virtual ImagePPStatus _Process(PixelOffset64 const& outOffset, HRAImageSampleR inputData, PixelOffset64 const& inOffset) override;

private:
    typename Surface_T::ImageEditor m_outEditor;
};

/*-------------------------------------------------------------------------------------------------------------*/
/*----------------------------------------------ImageOutputMerger----------------------------------------------*/
/*-------------------------------------------------------------------------------------------------------------*/
struct ImagePP::ImageOutputMerger
    {
    ImageOutputMerger(HRPPixelType const& inputPixelType, HRPPixelType const& outputPixelType, bool alphaBlend, HFCPtr<HVEShape>& pShape);

    ~ImageOutputMerger(){}
      
    HFCInclusiveGrid const& GetCurrentGrid() const {return m_currentGrid;}

    ImagePPStatus PrepareStrip(HFCInclusiveGrid const& stripGrid);

    bool HasScanlines() const {return GetCurrentGrid().GetWidth() != 0 && GetCurrentGrid().GetHeight() != 0; }

    unique_ptr<OutputMergerProcessor> CreateProcessor(HRAImageSurfaceR outSurface) const;

private:
    
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                   Mathieu.Marchand  10/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    template<bool NeedCompose_T, class InputEditor_T>
    struct MergerProcessorCreator : public SurfaceVisitor
        {
        MergerProcessorCreator(ImageOutputMerger const& merger) :m_merger(merger){}

        virtual ~MergerProcessorCreator(){};

        unique_ptr<OutputMergerProcessor> ReleaseProcessor() {return unique_ptr<OutputMergerProcessor>(m_pProcessor.release());}

        virtual ImagePPStatus _Visit(HRASampleN1Surface& surface) override {return ProcessNormal(surface);}
        virtual ImagePPStatus _Visit(HRASampleRleSurface& surface) override {return ProcessNormal(surface);}
        virtual ImagePPStatus _Visit(HRAPacketN1Surface& surface) override {return ProcessNormal(surface);}
        virtual ImagePPStatus _Visit(HRAPacketRleSurface& surface) override {return ProcessNormal(surface);}
        virtual ImagePPStatus _Visit(HRAPacketCodecRleSurface& surface) override {return IMAGEPP_STATUS_NoImplementation;}
        virtual ImagePPStatus _Visit(HRAPacketN8Surface& surface) override {return ProcessOptimizedN8(surface);}
        virtual ImagePPStatus _Visit(HRASampleN8Surface& surface) override {return ProcessOptimizedN8(surface);}

        /*---------------------------------------------------------------------------------**//**
        * @bsimethod                                                   Mathieu.Marchand  10/2014
        +---------------+---------------+---------------+---------------+---------------+------*/
        template<typename Surface_T>
        ImagePPStatus ProcessNormal(Surface_T& surface)
            {
            m_pProcessor.reset(new OutputMergerProcessorNormal<Surface_T, InputEditor_T, NeedCompose_T>(
                surface, m_merger.m_scanLines, m_merger.m_currentGrid, *m_merger.m_pConverter));

            return IMAGEPP_STATUS_Success;
            }

        /*---------------------------------------------------------------------------------**//**
        * @bsimethod                                                   Mathieu.Marchand  10/2014
        +---------------+---------------+---------------+---------------+---------------+------*/
        template<typename Surface_T>
        ImagePPStatus ProcessOptimizedN8(Surface_T& surface)
            {
            m_pProcessor.reset(new OutputMergerProcessorOptimizedN8<Surface_T, InputEditor_T, NeedCompose_T>(
                surface, m_merger.m_scanLines, m_merger.m_currentGrid, *m_merger.m_pConverter));

            return IMAGEPP_STATUS_Success;
            }
    
        ImageOutputMerger const& m_merger;
        unique_ptr<OutputMergerProcessor> m_pProcessor;
        };

    template<bool NeedCompose_T, class InputEditor_T>
    unique_ptr<OutputMergerProcessor> CreateProcessor_T(HRAImageSurfaceR outSurface) const;

    ImagePPStatus CopyScanlines(HGFScanLines& scl);

    HFCPtr<HVEShape>            m_pShape;       // can be NULL. Assumed to be in physical CS of the samples we will receive.
    Scanlines                   m_scanLines;
    HFCInclusiveGrid            m_currentGrid;
    HFCPtr<HRPPixelConverter>   m_pConverter;
    bool                        m_alphaBlend;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ImageOutputMerger::ImageOutputMerger(HRPPixelType const& inputPixelType, HRPPixelType const& outputPixelType, bool alphaBlend, HFCPtr<HVEShape>& pShape)
:m_alphaBlend(alphaBlend),
 m_pShape(pShape),
 m_pConverter(NULL)
    {  
    // Output merger is expecting a shape it is required to make sure we are not copying undesired clamp data.
    // The shape must include the source physical rect.
    BeAssert(pShape != NULL);       

// Negative origin are OK since the shape is express in dest CS. The source might be bigger than the dest.  
// The negatives will be remove in OutputMerger::PrepareStrip because the shape will be intersected with the dest region
//     BeAssert(pShape != NULL ? HDOUBLE_GREATER_OR_EQUAL_EPSILON(pShape->GetExtent().GetOrigin().GetX(), 0) : true);
//     BeAssert(pShape != NULL ? HDOUBLE_GREATER_OR_EQUAL_EPSILON(pShape->GetExtent().GetOrigin().GetY(), 0) : true);
    BeAssert(pShape != NULL ? !pShape->IsCompatibleWith(HVE2DVoidShape::CLASS_ID) : true);
    BeAssert(pShape != NULL ? !pShape->IsCompatibleWith(HVE2DUniverse::CLASS_ID) : true);
   
    m_pConverter = inputPixelType.GetConverterTo(&outputPixelType);
    BeAssert(m_pConverter != NULL);

    m_alphaBlend  = alphaBlend && inputPixelType.GetChannelOrg().GetChannelIndex(HRPChannelType::ALPHA, 0) != HRPChannelType::FREE;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
unique_ptr<OutputMergerProcessor> ImageOutputMerger::CreateProcessor(HRAImageSurfaceR outSurface) const
    {
    BeAssert(m_pConverter != NULL);
    //BeAssert(m_converter.GetSourcePixelType()->GetClassID() == inputData.GetPixelType().GetClassID());
    BeAssert(m_pConverter->GetDestinationPixelType()->GetClassID() == outSurface.GetPixelType().GetClassID());
    //BeAssert(outOffset.x >= 0 && outOffset.y >= 0);
    //BeAssert(inOffset.x >= 0 && inOffset.y >= 0);
    BeAssert(m_currentGrid.GetYMin() >= 0); // Scanlines stored in m_scanLines should have a positive offset.
    BeAssert(m_currentGrid.GetXMin() >= 0);  // We assumed strips, so no x offset.
    BeAssert(m_currentGrid.GetWidth() > 0 && m_currentGrid.GetHeight() > 0); // How that happen? why are we call here if we have nothing to copy.
     
    if (m_pConverter->GetSourcePixelType()->CountPixelRawDataBits() % 8 == 0) 
        {
        if (m_alphaBlend)
           return CreateProcessor_T<true, ImageEditorN8>(outSurface);

        return CreateProcessor_T<false, ImageEditorN8>(outSurface);
        }

    if (m_pConverter->GetSourcePixelType()->IsCompatibleWith(HRPPixelTypeI1R8G8B8RLE::CLASS_ID) || m_pConverter->GetSourcePixelType()->IsCompatibleWith(HRPPixelTypeI1R8G8B8A8RLE::CLASS_ID))
        {
        if (m_alphaBlend)
            return CreateProcessor_T<true, ImageEditorSampleRle>(outSurface);

        return CreateProcessor_T<false, ImageEditorSampleRle>(outSurface);
        }

    if (m_pConverter->GetSourcePixelType()->CountPixelRawDataBits() == 1)
        {
        if (m_alphaBlend)
            return CreateProcessor_T<true, ImageEditorN1>(outSurface);

        return CreateProcessor_T<false, ImageEditorN1>(outSurface);
        }

    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  12/2014
+---------------+---------------+---------------+---------------+---------------+------*/
template<bool NeedCompose_T, class InputEditor_T>
unique_ptr<OutputMergerProcessor> ImageOutputMerger::CreateProcessor_T(HRAImageSurfaceR outSurface) const
    {
    MergerProcessorCreator<NeedCompose_T, InputEditor_T> creator(*this);

    if(IMAGEPP_STATUS_Success != outSurface.Accept(creator))
        return NULL;

    return creator.ReleaseProcessor();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ImagePPStatus ImageOutputMerger::PrepareStrip(HFCInclusiveGrid const& stripGrid)
    {
    HASSERT(stripGrid.GetXMin() >= 0 && stripGrid.GetYMin() >= 0);

    m_scanLines.clear();
    m_currentGrid = stripGrid; // Default to whole strip.

    if (m_pShape == NULL)
        return IMAGEPP_STATUS_Success;
        
    //&&OPTIMIZATION, avoid intersection with the grid, and implement the a partial scan line generation.
    //       Also generate in a way that we do not need to copy them or compute extent.
    // *** N.B. With mosaic m_pShape might have an negative origin. It is important to intersect with the stripGrid, which cannot have negative value. 
    HVEShape gridShape((double)stripGrid.GetXMin(), (double)stripGrid.GetYMin(), (double)(stripGrid.GetXMin() + stripGrid.GetWidth()), (double)(stripGrid.GetYMin() + stripGrid.GetHeight()), m_pShape->GetCoordSys());
    gridShape.Intersect(*m_pShape);

    if (gridShape.IsEmpty())
        {
        m_currentGrid.InitEmpty();
        return IMAGEPP_STATUS_Success; // No intersection
        }

    // Scanlines cannot have negative values.
    BeAssert(HDOUBLE_GREATER_OR_EQUAL_EPSILON(gridShape.GetExtent().GetOrigin().GetX(), 0));
    BeAssert(HDOUBLE_GREATER_OR_EQUAL_EPSILON(gridShape.GetExtent().GetOrigin().GetY(), 0));

    // By default generate scanlines will have 0.5 strategy(gridMode=false). 
    // Grid=true : if the pixel is touched it is filled. For mosaic, the last raster to touch the pixel will fill it.
    // Grid=false: The pixel need 50% or more to be filled. Precision error and raster alignment might generate a blank or bad pixel selection.
    static bool s_gridMode = true;  // True: A pixel we touch is a pixel we copy. False: A pixel need to fill 50% to be copied.
    // Using GridMode=False, will have the effect of returning no scanline in some cases(ex: x: 0 - 0.48). This was the old way of
    // doing things in an attempt to minimize pixel selection errors between raster tiles.  Right now, we have decided to always grid since the
    // new copyFrom is truly destination driven(instead of per tile destination driven) so bad pixel selection between tiles is not problem anymore. 
    // However with a mosaic, on raster border the destination pixel will be filled by the top most raster no matter what is the fill percentage.
    HGFScanLines scanLines(s_gridMode);
    gridShape.GenerateScanLines(scanLines);
   
    return CopyScanlines(scanLines);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ImagePPStatus ImageOutputMerger::CopyScanlines(HGFScanLines& scl)
    {
    if (!scl.GotoFirstRun())
        {
        //No scanlines means we no intersection. Nothing to copy.
        m_currentGrid.InitEmpty();
        return IMAGEPP_STATUS_Success;      
        }        

    BeAssert(scl.GetScanLineStart() >= 0);

    if (scl.IsRectangle())
        {
        size_t pixelCount;
        HSINTX startPosX, startPosY;
        scl.GetCurrentRun(&startPosX, &startPosY, &pixelCount);
        
        m_currentGrid.InitFromLenght((double)startPosX, (double)scl.GetScanLineStart(), pixelCount, scl.GetScanlineCount());
        return IMAGEPP_STATUS_Success;
        }

    m_scanLines.resize(scl.GetScanlineCount());

    int64_t startPixelPosX = LLONG_MAX;
    int64_t endPixelPosX = LLONG_MIN;

    do  {
        size_t pixelCount;
        HSINTX startPosX, startPosY;
        scl.GetCurrentRun(&startPosX, &startPosY, &pixelCount);

        // compute MIN, MAX
        if (startPosX < startPixelPosX)
            startPixelPosX = startPosX;

        if ((startPosX + (int64_t)pixelCount) > endPixelPosX)
            endPixelPosX = startPosX + pixelCount;

        BeAssert(startPosX >= 0);
        m_scanLines[startPosY - scl.GetScanLineStart()].emplace_back(make_pair(startPosX, pixelCount));
        } while (scl.GotoNextRun());


    BeAssert(scl.GetScanLineStart() >= 0);
    m_currentGrid.InitFromLenght((double)startPixelPosX, (double)scl.GetScanLineStart(), endPixelPosX - startPixelPosX, scl.GetScanlineCount());

    return IMAGEPP_STATUS_Success;
    }

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ImageTransformNodeP    ImageNode::AsImageTransformNodeP()        { return _AsImageTransformNodeP(); }
ImageSourceNodeP       ImageNode::AsImageSourceNodeP()           { return _AsImageSourceNodeP(); }
HFCPtr<HGF2DCoordSys>& ImageNode::GetPhysicalCoordSys()          { return _GetPhysicalCoordSys(); }
HGF2DExtent const&     ImageNode::GetPhysicalExtent() const      { return _GetPhysicalExtent(); }
HFCPtr<HRPPixelType>   ImageNode::GetPixelType()                 { return _GetPixelType(); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ImageNode::ImageNode()
    :m_parent(NULL)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ImageNode::~ImageNode()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ImagePPStatus ImageNode::LinkTo(ImageNodeR child)
    {
    AddChild(child);
    return IMAGEPP_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void ImageNode::AddChild(ImageNodeR child)
    {
    child.m_parent = this;
    m_children.push_back(&child);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ImagePPStatus ImageNode::PrepareForStrip(HFCInclusiveGrid const& strip)
    {
    return _PrepareForStrip(strip);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ImagePPStatus ImageNode::_PrepareForStrip(HFCInclusiveGrid const& strip)
    {
    ImagePPStatus status = IMAGEPP_STATUS_Success;

    for (auto pChild : m_children)
        {
        if (pChild->GetPhysicalCoordSys() != GetPhysicalCoordSys())
            {
            HGF2DExtent srcExtent((double)strip.GetXMin(), (double)strip.GetYMin(), (double)(strip.GetXMax() + 1), (double)(strip.GetYMax() + 1), GetPhysicalCoordSys());
            srcExtent.ChangeCoordSys(pChild->GetPhysicalCoordSys());

            HFCInclusiveGrid inChildCSGrid(srcExtent.GetXMin(), srcExtent.GetYMin(), srcExtent.GetXMax(), srcExtent.GetYMax());

            if (IMAGEPP_STATUS_Success != (status = pChild->PrepareForStrip(inChildCSGrid)))
                break;
            }
        else
            {
            if (IMAGEPP_STATUS_Success != (status = pChild->PrepareForStrip(strip)))
                break;
            }
        }
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
HFCPtr<HRPPixelType> ImageNode::TransformToRleEquivalent(HFCPtr<HRPPixelType> pixelType)
    {
    if (pixelType->CountPixelRawDataBits() != 1)
        return NULL;

    if (pixelType->IsCompatibleWith(HRPPixelTypeI1R8G8B8RLE::CLASS_ID) || pixelType->IsCompatibleWith(HRPPixelTypeI1R8G8B8A8RLE::CLASS_ID))
        return pixelType;

    HFCPtr<HRPPixelType> pRlePixelType;

    if (pixelType->IsCompatibleWith(HRPPixelTypeI1R8G8B8::CLASS_ID))
        {
        pRlePixelType = new HRPPixelTypeI1R8G8B8RLE(pixelType->GetPalette());
        }
    else if (pixelType->IsCompatibleWith(HRPPixelTypeI1R8G8B8A8::CLASS_ID))
        {
        pRlePixelType = new HRPPixelTypeI1R8G8B8A8RLE(pixelType->GetPalette());
        }
    else
        {
        pRlePixelType = new HRPPixelTypeI1R8G8B8RLE();
        }
   
    HRPPixelType1BitInterface const* p1BitInterface = pixelType->Get1BitInterface();
    if (NULL != p1BitInterface && p1BitInterface->IsForegroundStateDefined())
        pRlePixelType->Get1BitInterface()->SetForegroundState(p1BitInterface->GetForegroundState());
        
    return pRlePixelType;
    }

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
ImageSourceNodeP       ImageTransformNode::GetSourceNodeP() { return GetChildP(0)->AsImageSourceNodeP(); }
HFCPtr<HGF2DCoordSys>& ImageTransformNode::_GetPhysicalCoordSys() { BeAssert(NULL != GetParentP()); return GetParentP()->GetPhysicalCoordSys(); }
HFCPtr<HRPPixelType>   ImageTransformNode::GetInputPixelType() { BeAssert(NULL != GetSourceNodeP());  return GetSourceNodeP()->GetPixelType(); }
HFCPtr<HRPPixelType>   ImageTransformNode::GetOutputPixelType() { BeAssert(NULL != GetParentP());  return GetParentP()->GetPixelType(); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ImageTransformNodePtr ImageTransformNode::CreateAndLink(ImagePPStatus& status, ImageNodeR parent, HFCPtr<HVEShape>& pShape)
    {
    BeAssert(pShape != NULL && parent.GetPhysicalCoordSys() == pShape->GetCoordSys());

    ImageTransformNodePtr pNode = new ImageTransformNode();

    // Link prior to set the shape. We need the parent to enable coordSys validation.
    if (IMAGEPP_STATUS_Success != (status = parent.LinkTo(*pNode)))
        return NULL;

    // We always need the clip to make sure the OutputMerger have all the information it needs to copy the right pixels.
    pNode->SetClipShape(pShape);

    status = IMAGEPP_STATUS_Success;

    return pNode;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ImageTransformNode::ImageTransformNode()    
:ImageNode(),
  m_pipe(),
  m_resampling(HGSResampling::NEAREST_NEIGHBOUR)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void ImageTransformNode::AddImageOp(HRAImageOpPtr imageOp, bool atFront)
    {
    m_pipe.AddImageOp(imageOp, atFront);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void ImageTransformNode::SetResamplingMode(HGSResampling const& resampling)
    {
    m_resampling = resampling;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void ImageTransformNode::SetClipShape(HFCPtr<HVEShape>& pShape)
    {
    HPRECONDITION(GetParentP() != NULL ? pShape->GetCoordSys() == GetPhysicalCoordSys() : true);
    
// Negative origin are OK since the shape is express in dest CS. The source might be bigger than the dest.  
// The negatives will be remove in OutputMerger::PrepareStrip because the shape will be intersected with the dest region.
//     BeAssert(pShape != NULL ? HDOUBLE_GREATER_OR_EQUAL_EPSILON(pShape->GetExtent().GetOrigin().GetX(), 0) : true);
//     BeAssert(pShape != NULL ? HDOUBLE_GREATER_OR_EQUAL_EPSILON(pShape->GetExtent().GetOrigin().GetY(), 0) : true);
    BeAssert(pShape != NULL ? !pShape->IsCompatibleWith(HVE2DVoidShape::CLASS_ID) : true);
    BeAssert(pShape != NULL ? !pShape->IsCompatibleWith(HVE2DUniverse::CLASS_ID) : true);

    m_pClipShape = pShape;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void ImageTransformNode::SetAlphaBlend(bool enableBlend)
    {
    m_alphaBlend = enableBlend;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ImagePPStatus ImageTransformNode::_PrepareForStrip(HFCInclusiveGrid const& strip)
    {
    if (GetSourceNodeP() == NULL)
        return IMAGEPP_STATUS_UnknownError;

    // * Sampler stage. Init on first call.
    if (m_pSrcSampler.get() == NULL)
        {
        HFCPtr<HGF2DTransfoModel> pDstPhysicalToSrcPhysical = GetPhysicalCoordSys()->GetTransfoModelTo(GetSourceNodeP()->GetPhysicalCoordSys());
        HFCPtr<HGF2DTransfoModel> pSimplModel = pDstPhysicalToSrcPhysical->CreateSimplifiedModel();
        if (NULL != pSimplModel.GetPtr())
            pDstPhysicalToSrcPhysical = pSimplModel;

        m_pSrcSampler.reset(AllocateSampler(pDstPhysicalToSrcPhysical));
        if (m_pSrcSampler.get() == NULL)
            return IMAGEPP_STATUS_UnknownError;
        }

    // * Pixel filtering stage. Init on first call.
    ImagePPStatus status;
    if(IMAGEPP_STATUS_Success != (status = m_pipe.Prepare(GetInputPixelType(), GetOutputPixelType())))
        return status;

    HFCPtr<HRPPixelType> pEffectivePixelType(m_pipe.IsEmpty() ? GetInputPixelType() : m_pipe.GetOutputPixelType());

    // * Output merger stage. Init on first call.
    if (m_pOutputMerger.get() == NULL)
        {
        if (m_pClipShape == NULL)
            {
            HFCPtr<HVEShape> pSrcPhysical = new HVEShape(GetSourceNodeP()->GetPhysicalExtent());
            pSrcPhysical->ChangeCoordSys(GetPhysicalCoordSys());        // Make sure it is in physical CS of what we will output.
            m_pOutputMerger.reset(new ImageOutputMerger(*pEffectivePixelType, *GetOutputPixelType(), m_alphaBlend, m_pClipShape));
            }
        else
            {
            HPRECONDITION(m_pClipShape->GetCoordSys() == GetPhysicalCoordSys());
            m_pOutputMerger.reset(new ImageOutputMerger(*pEffectivePixelType, *GetOutputPixelType(), m_alphaBlend, m_pClipShape));
            }
        }

    // Tell the output merger that we are working on this strip.
    if (IMAGEPP_STATUS_Success != (status = m_pOutputMerger->PrepareStrip(strip)))
        return status;

    // Compute required source grid only if we have scanlines to output.
    HFCInclusiveGrid requiredSourceGrid, effectiveDestGrid;
    if(m_pOutputMerger->HasScanlines())
        ComputeSourceGridFromDestination(&requiredSourceGrid, effectiveDestGrid, strip);
    
    // Dispatch to children.
    for (auto pChild : m_children)
        {
        if (IMAGEPP_STATUS_Success != (status = pChild->PrepareForStrip(requiredSourceGrid)))
            break;
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool ImageTransformNode::ComputeSourceGridFromDestination(HFCInclusiveGrid* sourceGrid, HFCInclusiveGrid& dest0Grid, HFCInclusiveGrid const& destGrid)
    {
    HGF2DExtent const& sourcePhysical = GetSourceNodeP()->GetPhysicalExtent();

    // 1) Intersect with the current region. That includes clipping extent and physical extent of the source.
    //    This intersect helps reduce the amount of clamp data that will be generated with mosaic and others multiple source node.
    dest0Grid.InitFromIntersectionOf(destGrid, GetCurrentRegionGrid());
    //BeAssert(dest0Grid.GetWidth() != 0 && dest0Grid.GetHeight() != 0);  // do what???
    if (dest0Grid.GetWidth() == 0 || dest0Grid.GetHeight() == 0)
        return false;

    // 2) Filtering neighborhood is affected by scale. Add it before we convert to source coordinate.
    HRPPixelNeighbourhood filteringNeighbourhood = m_pipe.GetNeighbourdhood();
    PixelOffset dest0Offset((double)(dest0Grid.GetXMin() - filteringNeighbourhood.GetXOrigin()), (double)(dest0Grid.GetYMin() - filteringNeighbourhood.GetYOrigin()));
    dest0Grid.InitFromLenght(dest0Offset.x, dest0Offset.y, 
                             dest0Grid.GetWidth() + (filteringNeighbourhood.GetWidth() - 1), 
                             dest0Grid.GetHeight() + (filteringNeighbourhood.GetHeight() - 1));

    // No need to continue if sourceGrid is not provided
    if (NULL == sourceGrid)
        return true;

    // 3) Convert the dest sample into source coordinates
    //    We need to go thru HVEShape because of complex models that do not preserve linearity. In these cases, HVEShape will generate extra segment.
    HGF2DExtent srcSampleExtent(dest0Offset.x, dest0Offset.y, dest0Offset.x + dest0Grid.GetWidth(), dest0Offset.y + dest0Grid.GetHeight(), GetPhysicalCoordSys());
    srcSampleExtent.ChangeCoordSys(GetSourceNodeP()->GetPhysicalCoordSys());


    HDEBUGCODE
        (
        HVEShape srcSampleExtentShape(dest0Offset.x, dest0Offset.y, dest0Offset.x + dest0Grid.GetWidth(), dest0Offset.y + dest0Grid.GetHeight(), GetPhysicalCoordSys());
        srcSampleExtentShape.ChangeCoordSys(GetSourceNodeP()->GetPhysicalCoordSys()); srcSampleExtentShape;

        BeAssert(srcSampleExtentShape.GetExtent().IsEqualTo(srcSampleExtent, HGLOBAL_EPSILON));

        HVEShape srcPhysicalInDestCS(GetSourceNodeP()->GetPhysicalExtent());
        srcPhysicalInDestCS.ChangeCoordSys(GetPhysicalCoordSys()); srcPhysicalInDestCS;
        srcPhysicalInDestCS.GetExtent();
        )

    // 4) Intersection with source physical extent:
    //  a-)In case where there is a sampler the src doesn't need to produce clamping since the sampler will do it.  It is also
    //    more effective in case of extreme zoom out.
    //    ex. whole src covert only 1/10000 of a dest pixel. In this case, the required source will be 10000x10000 to generate 1 dest pixel.
    //    If we clip to src only 1x1 will be generated and sampler will clamp if required.
    //  b-)OPTIMIZATION: When no sampling is required, if would be faster to ask the source to generate the clamping and skip the sampler. 
    //          >>> Not all get region are capable of clamping.
    srcSampleExtent.Intersect(sourcePhysical);
    HASSERT(srcSampleExtent.GetCoordSys() == GetChildP(0)->GetPhysicalCoordSys());

    // No intersection scenario:
    //      1) Warp: It occurs since we iterate over intersecting destination extent.
    //      2) Also some cases with Mosaic since we intersect over the destination region of all images.
    // In these cases, we simply ignore the requested region and return early. The destination is untouched.
    static bool s_AssertOnUndefinedExtent = false;
    HASSERT(s_AssertOnUndefinedExtent ? srcSampleExtent.IsDefined() : true);
    if (!srcSampleExtent.IsDefined())
        return false;

    HFCInclusiveGrid srcSampleGrid(srcSampleExtent.GetXMin(), srcSampleExtent.GetYMin(), srcSampleExtent.GetXMax(), srcSampleExtent.GetYMax());
    if(srcSampleGrid.GetWidth() == 0 || srcSampleGrid.GetHeight() == 0)
        return false;

    // Sampler neighbourhood is not affected by scale. Add it to the required source area.
    HRPPixelNeighbourhood samplerNeighbourhood(1, 1, 0, 0);
    if (m_pSrcSampler.get() != NULL)
        samplerNeighbourhood = m_pSrcSampler->GetNeighbourdhood();

    srcSampleGrid.InitFromLenght((double)(srcSampleGrid.GetXMin() - samplerNeighbourhood.GetXOrigin()), (double)(srcSampleGrid.GetYMin() - samplerNeighbourhood.GetYOrigin()),
                                 srcSampleGrid.GetWidth() + (samplerNeighbourhood.GetWidth() - 1), srcSampleGrid.GetHeight() + (samplerNeighbourhood.GetHeight() - 1));
   
    // Never return return a grid outside the physical extent. There is at least output merger that cannot handle negative values when generating scanlines.
    HFCInclusiveGrid physicalGrid(sourcePhysical.GetXMin(), sourcePhysical.GetYMin(), sourcePhysical.GetXMax(), sourcePhysical.GetYMax());
    sourceGrid->InitFromIntersectionOf(srcSampleGrid, physicalGrid);

    return sourceGrid->GetWidth() != 0 && sourceGrid->GetHeight() != 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ImagePPStatus ImageTransformNode::Produce(HRAImageSurfaceR destSurface, PixelOffset64 const& offset, IImageAllocatorR allocator)
    {
    BeAssert(GetParentP()->GetPhysicalCoordSys().GetPtr() == GetPhysicalCoordSys().GetPtr());
    BeAssert(m_pipe.IsReady());
    BeAssert(m_pOutputMerger.get() != NULL);
    
    HFCInclusiveGrid destGrid;
    destGrid.InitFromLenght((double)(offset.x), (double)(offset.y), destSurface.GetWidth(), destSurface.GetHeight());

    // Compute the scale from the main resolution of the source to the destination
    double resolutionScaleX, resolutionScaleY;
    GetSourceNodeP()->GetScaleFactorFromMain(resolutionScaleX, resolutionScaleY);
    HGF2DStretch stretchModel(HGF2DDisplacement(), resolutionScaleX, resolutionScaleY);
    HFCPtr<HGF2DTransfoModel> pMainToDst(stretchModel.ComposeInverseWithDirectOf(m_pSrcSampler->GetTransfoModel()));
    HFCPtr<HGF2DTransfoModel> pSimplModel(pMainToDst->CreateSimplifiedModel());
    if (NULL != pSimplModel.GetPtr())
        pMainToDst = pSimplModel;

    std::unique_ptr<OutputMergerProcessor> pOutputMergerProcessor(m_pOutputMerger->CreateProcessor(destSurface));
    if(pOutputMergerProcessor.get() == NULL)
        return IMAGEPP_STATUS_NoImplementation;

    // Split the destination into manageable block.
    // &&OPTIMIZATION
    //      - This should take into account the size and organization of both the source and destination.
    //      - Snap to source block to avoid construction of contiguous samples.
    //      - see note in destTessellation loop about manageable size and strip height.
    uint32_t manageableBlockSizeX = 512;
    uint32_t manageableBlockSizeY = MAX_STRIP_HEIGHT;

    // It is slow to process large binary in small block so we decided increase the block width when stretchable.
    // When warping(ex. rotation) the required source size could be huge so keep block small.
    if(m_pSrcSampler->IsStretchable() ||
       s_IsRLEPixelType(*GetSourceNodeP()->GetPixelType()) && s_IsRLEPixelType(destSurface.GetPixelType()))
        {
        manageableBlockSizeX = 4096;
        }   
    
    // Compute the effective destination region to iterate over.
    HFCInclusiveGrid effectiveDstGrid(HFCInclusiveGrid::FromIntersectionOf(destGrid, GetCurrentRegionGrid()));

    // Generate the tessellation and iterate over the destination
    SquareTessellation destTessellation(effectiveDstGrid, manageableBlockSizeX, manageableBlockSizeY, (uint32_t)destGrid.GetXMin(), (uint32_t)destGrid.GetYMin());

    ImagePPStatus status = IMAGEPP_STATUS_Success;

    for (auto dstGridItr : destTessellation)
        {
        HFCInclusiveGrid srcGrid, dest1Grid;
        if (!ComputeSourceGridFromDestination(&srcGrid, dest1Grid, dstGridItr))
            {
            // If manageable block size is smaller than the strip height then we need to continue our 
            // processing because we might have an intersection with subsequent block. 
            if(manageableBlockSizeY < MAX_STRIP_HEIGHT || !m_pSrcSampler->PreservesLinearity())
                continue;
            
            // return since the manageableBlockSizeY is higher then the strip height and we won't have other intersection.
            return IMAGEPP_STATUS_Success;   // No intersection, this is not an error.
            }

        // *** Source query stage. Could be a tiled raster, bitmap, mosaic....
        HRAImageSurfacePtr pSrcSurface;
        PixelOffset64 srcOffset(srcGrid.GetXMin(), srcGrid.GetYMin()); // Note that srcOffset might be updated by GetRegion(...)
        if(IMAGEPP_STATUS_Success != (status = GetSourceNodeP()->GetRegion(pSrcSurface, srcOffset, srcGrid, allocator)))
            return status;      

        // *** Sampling stage.
        PixelOffset dest0Offset((double)dest1Grid.GetXMin(), (double)dest1Grid.GetYMin());
        HRAImageSamplePtr pDestSample0 = m_pSrcSampler->ComputeSample(status, (uint32_t)dest1Grid.GetWidth(), (uint32_t)dest1Grid.GetHeight(), dest0Offset, *pSrcSurface, PixelOffset((double)srcOffset.x, (double)srcOffset.y), allocator);
        if (IMAGEPP_STATUS_Success != status)
            return status;

        ImagepOpParams imageOpParams(*pMainToDst);
        imageOpParams.SetOffset((int64_t)dest0Offset.x, (int64_t)dest0Offset.y);
        
        // *** Pixel filtering stage.
        if (!m_pipe.IsEmpty())
            pDestSample0 = m_pipe.Process(status, *pDestSample0, imageOpParams, allocator);

        // *** Final stage. Shape, blend(compose) or convert in the destination. 
        //      Must use offset from imageOpParams since pipe might update it for non-identity filter(ex. convolution)
        status = pOutputMergerProcessor->_Process(offset, *pDestSample0, PixelOffset64(imageOpParams.GetOffsetX(), imageOpParams.GetOffsetY()));
        }
        
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
HRAImageSampler* ImageTransformNode::AllocateSampler(HFCPtr<HGF2DTransfoModel>& destToSrcTransfo)
    {
    unique_ptr<HRAImageSampler> pSampler;

    switch (m_resampling.GetResamplingMethod())
        {
        case HGSResampling::CUBIC_CONVOLUTION:
            pSampler.reset(HRAImageSampler::CreateBicubic(destToSrcTransfo, *GetInputPixelType()));
            if (pSampler.get() != NULL)
                break;
        // *** CONTINUE to next method.
        case HGSResampling::AVERAGE:        // Our implementation of bilinear is faster and better looking. Average make sense only for 1:N scaling.
        case HGSResampling::BILINEAR:
            pSampler.reset(HRAImageSampler::CreateBilinear(destToSrcTransfo, *GetInputPixelType()));
            if (pSampler.get() != NULL)
                break;
        // *** CONTINUE to next method.
        default:
            pSampler.reset(HRAImageSampler::CreateNearestRle(destToSrcTransfo, *GetInputPixelType()));
            if (pSampler.get() != NULL)
                break;

            pSampler.reset(HRAImageSampler::CreateNearestN1(destToSrcTransfo, *GetInputPixelType()));
            if (pSampler.get() != NULL)
                break;

            pSampler.reset(HRAImageSampler::CreateNearestN8(destToSrcTransfo, *GetInputPixelType()));
            break;
        }

    return pSampler.release();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
HGF2DExtent const& ImageTransformNode::_GetPhysicalExtent()  const { BeAssert(NULL != GetSourceNodeCP());  return GetSourceNodeCP()->GetPhysicalExtent(); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
HFCInclusiveGrid const& ImageTransformNode::GetCurrentRegionGrid() const
    {
    BeAssert(m_pOutputMerger.get() != NULL);

    if (m_pOutputMerger.get() != NULL)
        return m_pOutputMerger->GetCurrentGrid();

    static HFCInclusiveGrid s_emptyGrid(0, 0, 0, 0);
    return s_emptyGrid;
    }

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ImageSourceNode::ImageSourceNode(HFCPtr<HRPPixelType> pPixelType)
:ImageNode(), 
m_pPixelType(pPixelType), m_scaleFromMainX(1.0), m_scaleFromMainY(1.0)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 09/2014
+---------------+---------------+---------------+---------------+---------------+------*/
HFCPtr<HRPPixelType> ImageSourceNode::_GetPixelType() { return m_pPixelType; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  09/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ImagePPStatus ImageSourceNode::GetRegion(HRAImageSurfacePtr& pOut, PixelOffset64& outOffset, HFCInclusiveGrid const& region, IImageAllocatorR allocator)
    {
    return _GetRegion(pOut, outOffset, region, allocator);
    }


///////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ImageSinkNode::ImageSinkNode(HVEShape const& sinkShape, HFCPtr<HRPPixelType> pInput, HGF2DExtent const& physicalExtent)
:ImageNode(),
 m_sinkGrid(sinkShape.GetExtent().GetXMin(), sinkShape.GetExtent().GetYMin(), sinkShape.GetExtent().GetXMax(), sinkShape.GetExtent().GetYMax()),
 m_pTransaction(NULL),
 m_pPixelType(pInput),
 m_physicalExtent(physicalExtent)
    {
    BeAssert(m_physicalExtent.IsDefined());
    BeAssert(sinkShape.GetCoordSys().GetPtr() == physicalExtent.GetCoordSys().GetPtr());
    BeAssert(NULL != pInput.GetPtr());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 09/2014
+---------------+---------------+---------------+---------------+---------------+------*/
HFCPtr<HRPPixelType> ImageSinkNode::_GetPixelType() {return m_pPixelType; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
HRATransaction* ImageSinkNode::GetTransaction() { return m_pTransaction; }
void ImageSinkNode::SetTransaction(HRATransaction* pTransaction){ m_pTransaction = pTransaction; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  09/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ImageSurfaceIterator* ImageSinkNode::GetImageSurfaceIterator(HFCInclusiveGrid& strip){ return _GetImageSurfaceIterator(strip); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  07/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ImagePPStatus ImageSinkNode::Execute(IImageAllocatorR allocator)
    {
    if (GetChildP(0) == NULL || GetChildP(0)->AsImageTransformNodeP() == NULL)
        return IMAGEPP_STATUS_UnknownError;

    ImagePPStatus status = IMAGEPP_STATUS_Success;

    ImageTransformNodeP pTrfNode = GetChildP(0)->AsImageTransformNodeP();

    uint32_t blockHeight(_GetBlockSizeY());
    if(blockHeight < MAX_STRIP_HEIGHT)
        blockHeight = (MAX_STRIP_HEIGHT / blockHeight)*MAX_STRIP_HEIGHT;  // Align close to MAX_STRIP_HEIGHT.
    else if(blockHeight > MAX_STRIP_HEIGHT)
        blockHeight = MAX_STRIP_HEIGHT;
    
    HFCGridStripper gridStripper(m_sinkGrid, blockHeight);

    for (auto sinkStrip : gridStripper)
        {
        if (IMAGEPP_STATUS_Success != (status = PrepareForStrip(sinkStrip)))
            break;

        // Update to the effective area.
        HFCInclusiveGrid srcStripGrid = pTrfNode->GetCurrentRegionGrid();
        if (srcStripGrid.GetWidth() == 0 || srcStripGrid.GetHeight() == 0)
            continue;

        // Iterator over all destination blocks that intersect with the current strip.
        for (unique_ptr<ImageSurfaceIterator> pSurfaceItr(GetImageSurfaceIterator(srcStripGrid)); pSurfaceItr->IsValid(); pSurfaceItr->_Next())
            {
            ImagePPStatus produceStatus = pTrfNode->Produce(pSurfaceItr->GetSurface(), pSurfaceItr->GetOffset(), allocator);
            // Report first encountered error but keep going.  This is what we did before.
            if(IMAGEPP_STATUS_Success == status)
                status = produceStatus;

            MarkSampleContour(pDestSample);
            }
        }
  
    return status;
    }

    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename Surface_T, class InputEditor_T, bool NeedCompose_T>
ImagePPStatus OutputMergerProcessorOptimizedN8<Surface_T, InputEditor_T, NeedCompose_T>::_Process(
    PixelOffset64 const& outOffset, HRAImageSampleR inputData, PixelOffset64 const& inOffset)
{
    BeAssert(m_converter.GetSourcePixelType()->GetClassID() == inputData.GetPixelType().GetClassID());
    BeAssert(outOffset.x >= 0 && outOffset.y >= 0);
    BeAssert(inOffset.x >= 0 && inOffset.y >= 0);

    // OutBlock corners
    const uint64_t outBlockOriginX = outOffset.x;
    const uint64_t outBlockOriginY = outOffset.y;
    const uint64_t outBlockCornerX = outBlockOriginX + m_outEditor.GetWidth();
    const uint64_t outBlockCornerY = outBlockOriginY + m_outEditor.GetHeight();

    // InBlock corners
    const uint64_t inBlockOriginX = inOffset.x;
    const uint64_t inBlockOriginY = inOffset.y;
    const uint64_t inBlockCornerX = inBlockOriginX + inputData.GetWidth();
    const uint64_t inBlockCornerY = inBlockOriginY + inputData.GetHeight();

    // Intersect outBlock, inBlock and currentGrid. That gives us the region that needs copying.
    uint64_t firstPixel = MAX((uint64_t)m_grid.GetXMin(), MAX(outBlockOriginX, inBlockOriginX));
    uint64_t endPixel = MIN((uint64_t)m_grid.GetXMax() + 1, MIN(outBlockCornerX, inBlockCornerX));

    uint64_t firstScanline = MAX((uint64_t)m_grid.GetYMin(), MAX(outBlockOriginY, inBlockOriginY));
    uint64_t endScanline = MIN((uint64_t)m_grid.GetYMax() + 1, MIN(outBlockCornerY, inBlockCornerY));

    uint32_t pixelCount = (uint32_t)(endPixel - firstPixel);

    BeAssert((firstPixel - inOffset.x) >= 0);
    BeAssert((firstPixel - inOffset.x) + pixelCount <= inputData.GetWidth());
    BeAssert((firstPixel - outOffset.x) >= 0);
    BeAssert((firstPixel - outOffset.x) + pixelCount <= m_outEditor.GetWidth());

    InputEditor_T inEditor(inputData);

    // Rectangles have no scanline, copy the intersection.
    if (m_scanLines.empty())
        {
        for (uint64_t scanLine = firstScanline; scanLine < endScanline; ++scanLine)
            {
            BeAssert(IN_RANGE(scanLine - inOffset.y, 0, inputData.GetHeight()));
            BeAssert(IN_RANGE(scanLine - outOffset.y, 0, m_outEditor.GetHeight()));

            if (NeedCompose_T)  // *** Will be resolve at compile time
                {
                m_converter.Compose(inEditor.GetPixels((uint32_t)(firstPixel - inOffset.x), (uint32_t)(scanLine - inOffset.y), pixelCount),
                                      m_outEditor.LockPixels((uint32_t)(firstPixel - outOffset.x), (uint32_t)(scanLine - outOffset.y), pixelCount),
                                      pixelCount);
                }
            else
                {
                m_converter.Convert(inEditor.GetPixels((uint32_t)(firstPixel - inOffset.x), (uint32_t)(scanLine - inOffset.y), pixelCount),
                                      m_outEditor.LockPixels((uint32_t)(firstPixel - outOffset.x), (uint32_t)(scanLine - outOffset.y), pixelCount),
                                      pixelCount);
                }

            m_outEditor.UnLockPixels();
            }
        }
    else
        {
        uint64_t scanlineOffset = m_grid.GetYMin();
        for (uint64_t scanLine = firstScanline; scanLine < endScanline; ++scanLine)
            {
            BeAssert(IN_RANGE(scanLine - inOffset.y, 0, inputData.GetHeight()));
            BeAssert(IN_RANGE(scanLine - outOffset.y, 0, m_outEditor.GetHeight()));
            BeAssert(IN_RANGE(scanLine-scanlineOffset, 0, m_scanLines.size()-1));
            
            Scanline const& currentScanline = m_scanLines[(uint32_t)(scanLine-scanlineOffset)];
            for (auto runItr : currentScanline)
                {
                if (runItr.first > endPixel) 
                    break; // this run and the subsequent runs are not overlapping

                //&&OPTIMIZATION:  We should be looking for the first scanline in separate loop and then test only the exit condition in the process loop. 
                if ((runItr.first + runItr.second) < firstPixel)
                    continue; // this run is not overlapping, but maybe next runs will...

                const int64_t beginRun = MAX(firstPixel, runItr.first);
                const int64_t endRun = MIN(endPixel, runItr.first + runItr.second);

                BeAssert((beginRun - inOffset.x) + (endRun - beginRun)/*pixelToCopy*/ <= inputData.GetWidth());
                BeAssert((beginRun - outOffset.x) + (endRun - beginRun)/*pixelToCopy*/ <= m_outEditor.GetWidth());

                if (NeedCompose_T)  // *** Will be resolve at compile time
                    {
                    m_converter.Compose(inEditor.GetPixels((uint32_t)(beginRun - inOffset.x), (uint32_t)(scanLine - inOffset.y), (uint32_t)(endRun - beginRun)),
                                          m_outEditor.LockPixels((uint32_t)(beginRun - outOffset.x), (uint32_t)(scanLine - outOffset.y), (uint32_t)(endRun - beginRun)),
                                          (uint32_t)(endRun - beginRun));
                    }
                else
                    {
                    m_converter.Convert(inEditor.GetPixels((uint32_t)(beginRun - inOffset.x), (uint32_t)(scanLine - inOffset.y), (uint32_t)(endRun - beginRun)),
                                          m_outEditor.LockPixels((uint32_t)(beginRun - outOffset.x), (uint32_t)(scanLine - outOffset.y), (uint32_t)(endRun - beginRun)),
                                          (uint32_t)(endRun - beginRun));
                    }

                m_outEditor.UnLockPixels();
                // N8 optimized code:
//                 m_converter.Convert(pInBuffer + (scanLine - inOffset.y)*inPitch + (beginRun - inOffset.x)*inBytesPerPixel,
//                                       pOutBuffer + (scanLine - outOffset.y)*outPitch + (beginRun - outOffset.x)*outBytesPerPixel, 
//                                       endRun - beginRun);
                }
            }
        }

    return IMAGEPP_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename Surface_T, class InputEditor_T, bool NeedCompose_T>
ImagePPStatus OutputMergerProcessorNormal<Surface_T, InputEditor_T, NeedCompose_T>::_Process(
    PixelOffset64 const& outOffset, HRAImageSampleR inputData, PixelOffset64 const& inOffset)
{
    BeAssert(m_converter.GetSourcePixelType()->GetClassID() == inputData.GetPixelType().GetClassID());
    BeAssert(outOffset.x >= 0 && outOffset.y >= 0);
    BeAssert(inOffset.x >= 0 && inOffset.y >= 0);

    // OutBlock corners
    const uint64_t outBlockOriginX = outOffset.x;
    const uint64_t outBlockOriginY = outOffset.y;
    const uint64_t outBlockCornerX = outBlockOriginX + m_outEditor.GetWidth();
    const uint64_t outBlockCornerY = outBlockOriginY + m_outEditor.GetHeight();

    // InBlock corners
    const uint64_t inBlockOriginX = inOffset.x;
    const uint64_t inBlockOriginY = inOffset.y;
    const uint64_t inBlockCornerX = inBlockOriginX + inputData.GetWidth();
    const uint64_t inBlockCornerY = inBlockOriginY + inputData.GetHeight();

    // Intersect outBlock, inBlock and currentGrid. That gives us the region that needs copying.
    uint64_t firstPixel = MAX((uint64_t)m_grid.GetXMin(), MAX(outBlockOriginX, inBlockOriginX));
    uint64_t endPixel = MIN((uint64_t)m_grid.GetXMax() + 1, MIN(outBlockCornerX, inBlockCornerX));

    uint64_t firstScanline = MAX((uint64_t)m_grid.GetYMin(), MAX(outBlockOriginY, inBlockOriginY));
    uint64_t endScanline = MIN((uint64_t)m_grid.GetYMax() + 1, MIN(outBlockCornerY, inBlockCornerY));

    uint32_t pixelCount = (uint32_t)(endPixel - firstPixel);

    BeAssert((firstPixel - inOffset.x) >= 0);
    BeAssert((firstPixel - inOffset.x) + pixelCount <= inputData.GetWidth());
    BeAssert((firstPixel - outOffset.x) >= 0);
    BeAssert((firstPixel - outOffset.x) + pixelCount <= m_outEditor.GetWidth());

    InputEditor_T inEditor(inputData);

    unique_ptr<Byte[]> pWorkingBuffer;

    if (!NeedCompose_T)  // *** Will be resolve at compile time
        pWorkingBuffer.reset(new Byte[m_outEditor.GetWorkingBufferSize()]);       //&&OPTIMIZATION avoid new at each call

    // Rectangles have no scanline, copy the intersection.
    if (m_scanLines.empty())
        {
        for (uint64_t scanLine = firstScanline; scanLine < endScanline; ++scanLine)
            {
            BeAssert(IN_RANGE(scanLine - inOffset.y, 0, inputData.GetHeight()));
            BeAssert(IN_RANGE(scanLine - outOffset.y, 0, m_outEditor.GetHeight()));
            
            if (NeedCompose_T)  // *** Will be resolve at compile time
                {
                m_converter.Compose(inEditor.GetPixels((uint32_t)(firstPixel - inOffset.x), (uint32_t)(scanLine - inOffset.y), pixelCount),
                    m_outEditor.LockPixels((uint32_t)(firstPixel - outOffset.x), (uint32_t)(scanLine - outOffset.y), pixelCount),
                    pixelCount);

                m_outEditor.UnLockPixels();
                }
            else
                {
                m_converter.Convert(inEditor.GetPixels((uint32_t)(firstPixel - inOffset.x), (uint32_t)(scanLine - inOffset.y), pixelCount),
                                      pWorkingBuffer.get(), pixelCount);

                m_outEditor.SetPixels((uint32_t)(firstPixel - outOffset.x), (uint32_t)(scanLine - outOffset.y), pixelCount, pWorkingBuffer.get());
                }           
            }
        }
    else
        {
        uint64_t scanlineOffset = m_grid.GetYMin();
        for (uint64_t scanLine = firstScanline; scanLine < endScanline; ++scanLine)
            {
            BeAssert(IN_RANGE(scanLine - inOffset.y, 0, inputData.GetHeight()));
            BeAssert(IN_RANGE(scanLine - outOffset.y, 0, m_outEditor.GetHeight()));
            BeAssert(IN_RANGE(scanLine-scanlineOffset, 0, m_scanLines.size()-1));
            
            for (auto runItr : m_scanLines[(uint32_t)(scanLine-scanlineOffset)])
                {
                if (runItr.first > endPixel)
                    break; // this run and the subsequent runs are not overlapping
                if ((runItr.first + runItr.second) < firstPixel)
                    continue; // this run is not overlapping, but maybe next runs will...

                const int64_t beginRun = MAX(firstPixel, runItr.first);
                const int64_t endRun = MIN(endPixel, runItr.first + runItr.second);

                BeAssert((beginRun - inOffset.x) + (endRun - beginRun)/*pixelToCopy*/ <= inputData.GetWidth());
                BeAssert((beginRun - outOffset.x) + (endRun - beginRun)/*pixelToCopy*/ <= m_outEditor.GetWidth());
                
                if (NeedCompose_T)  // *** Will be resolve at compile time
                    {
                    m_converter.Compose(inEditor.GetPixels((uint32_t)(beginRun - inOffset.x), (uint32_t)(scanLine - inOffset.y), (uint32_t)(endRun - beginRun)),
                                          m_outEditor.LockPixels((uint32_t)(beginRun - outOffset.x), (uint32_t)(scanLine - outOffset.y), (uint32_t)(endRun - beginRun)),
                                          (uint32_t)(endRun - beginRun));

                    m_outEditor.UnLockPixels();
                    }
                else
                    {
                    m_converter.Convert(inEditor.GetPixels((uint32_t)(beginRun - inOffset.x), (uint32_t)(scanLine - inOffset.y), (uint32_t)(endRun - beginRun)),
                                          pWorkingBuffer.get(), (uint32_t)(endRun - beginRun));

                    m_outEditor.SetPixels((uint32_t)(beginRun - outOffset.x), (uint32_t)(scanLine - outOffset.y), (uint32_t)(endRun - beginRun), pWorkingBuffer.get());
                    }
                }
            }
        }

    return IMAGEPP_STATUS_Success;
    }
