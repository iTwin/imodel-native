//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hra/src/HRAStoredRaster.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Methods for class HRAStoredRaster
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HFCMath.h>
#include <Imagepp/all/h/HRAStoredRaster.h>
#include <Imagepp/all/h/HRARasterIterator.h>
#include <Imagepp/all/h/HRPPixelTypeI1R8G8B8RLE.h>
#include <Imagepp/all/h/HRPPixelTypeI1R8G8B8A8RLE.h>
#include <Imagepp/all/h/HRPPixelTypeV24R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeV32R8G8B8A8.h>
#include <Imagepp/all/h/HRPPixelTypeFactory.h>
#include <Imagepp/all/h/HGF2DIdentity.h>
#include <Imagepp/all/h/HGF2DTranslation.h>
#include <Imagepp/all/h/HGF2DSimilitude.h>
#include <Imagepp/all/h/HGF2DStretch.h>
#include <Imagepp/all/h/HRPMessages.h>
#include <Imagepp/all/h/HGFMessages.h>
#include <Imagepp/all/h/HRARepPalParms.h>
#include <Imagepp/all/h/HRAHistogramOptions.h>
#include <Imagepp/all/h/HRADrawOptions.h>
#include <Imagepp/all/h/HRADrawProgressIndicator.h>
#include <Imagepp/all/h/HRABitmapBase.h>
#include <Imagepp/all/h/HRATransaction.h>
#include <Imagepp/all/h/HRATransactionRecorder.h>
#include <Imagepp/all/h/HCDPacket.h>
#include <Imagepp/all/h/HRAEditor.h>
#include <Imagepp/all/h/HRABitmapEditor.h>
#include <Imagepp/all/h/HRSObjectStore.h>
#include <Imagepp/all/h/HMDContext.h>
#include <Imagepp/all/h/HGFMappedSurface.h>
#include <Imagepp/all/h/HVE2DRectangle.h>
#include <Imagepp/all/h/HGSSurfaceDescriptor.h>
#include <ImagePPInternal/gra/HRACopyToOptions.h>
#include <ImagePPInternal/gra/ImageAllocator.h>
#include <ImagePPInternal/gra/HRAImageNode.h>

HPM_REGISTER_ABSTRACT_CLASS(HRAStoredRaster, HRARaster)

//-----------------------------------------------------------------------------
// class HRAStoredRasterTransaction
//-----------------------------------------------------------------------------
class ImagePP::HRAStoredRasterTransaction : public HFCShareableObject<HRAStoredRasterTransaction>
    {
    public:
        HRAStoredRasterTransaction(const HFCPtr<HRATransaction>& pi_pUndo, const HFCPtr<HRATransaction>& pi_pRedo = 0);
        virtual ~HRAStoredRasterTransaction();

        bool                    HasUndoTransaction() const;
        HFCPtr<HRATransaction>  GetUndoTransaction() const;
        void                    SetUndoTransaction(const HFCPtr<HRATransaction>& pi_rpUndo);

        bool                    HasRedoTransaction() const;
        HFCPtr<HRATransaction>  GetRedoTransaction() const;
        void                    SetRedoTransaction(const HFCPtr<HRATransaction>& pi_rpRedo);

        void                    Clear();

        void                    SetSaveBookmark();
        void                    RemoveSaveBookmark();
        bool                    GetSaveBookmark() const;

    private:

        bool                        m_SavedBookmark;
        HFCPtr<HRATransaction>      m_pUndo;
        HFCPtr<HRATransaction>      m_pRedo;
    };


//-----------------------------------------------------------------------------
// public : HRAStoredRasterTransaction
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// public
// Constructor
//-----------------------------------------------------------------------------
HRAStoredRasterTransaction::HRAStoredRasterTransaction(const HFCPtr<HRATransaction>& pi_pUndo,
                                                       const HFCPtr<HRATransaction>& pi_pRedo)
    : m_pUndo(pi_pUndo),
      m_pRedo(pi_pRedo),
      m_SavedBookmark(false)
    {

    }


//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HRAStoredRasterTransaction::~HRAStoredRasterTransaction()
    {

    }


//-----------------------------------------------------------------------------
// public
// HasUndoTransaction
//-----------------------------------------------------------------------------
bool HRAStoredRasterTransaction::HasUndoTransaction() const
    {
    return (m_pUndo != 0);
    }


//-----------------------------------------------------------------------------
// public
// GetUndoTransaction
//-----------------------------------------------------------------------------
HFCPtr<HRATransaction> HRAStoredRasterTransaction::GetUndoTransaction() const
    {
    return m_pUndo;
    }


//-----------------------------------------------------------------------------
// public
// SetUndoTransaction
//-----------------------------------------------------------------------------
void HRAStoredRasterTransaction::SetUndoTransaction(const HFCPtr<HRATransaction>& pi_rpUndo)
    {
    m_pUndo = pi_rpUndo;
    }


//-----------------------------------------------------------------------------
// public
// HasRedoTransaction
//-----------------------------------------------------------------------------
bool HRAStoredRasterTransaction::HasRedoTransaction() const
    {
    return (m_pRedo != 0);
    }


//-----------------------------------------------------------------------------
// public
// GetRedoTransaction
//-----------------------------------------------------------------------------
HFCPtr<HRATransaction> HRAStoredRasterTransaction::GetRedoTransaction() const
    {
    return m_pRedo;
    }


//-----------------------------------------------------------------------------
// public
// SetRedoTransaction
//-----------------------------------------------------------------------------
void HRAStoredRasterTransaction::SetRedoTransaction(const HFCPtr<HRATransaction>& pi_rpRedo)
    {
    m_pRedo = pi_rpRedo;
    }


//-----------------------------------------------------------------------------
// public
// Clear
//-----------------------------------------------------------------------------
void HRAStoredRasterTransaction::Clear()
    {
    if (m_pUndo != 0)
        m_pUndo->Clear();

    if (m_pRedo != 0)
        m_pRedo->Clear();
    }


//-----------------------------------------------------------------------------
// public
// SetSaveBookmark
//-----------------------------------------------------------------------------
void HRAStoredRasterTransaction::SetSaveBookmark()
    {
    m_SavedBookmark = true;
    }


//-----------------------------------------------------------------------------
// public
// RemoveSaveBookmark
//-----------------------------------------------------------------------------
void HRAStoredRasterTransaction::RemoveSaveBookmark()
    {
    m_SavedBookmark = false;
    }


//-----------------------------------------------------------------------------
// public
// GetSaveBookmark
//-----------------------------------------------------------------------------
bool HRAStoredRasterTransaction::GetSaveBookmark() const
    {
    return m_SavedBookmark;
    }


//-----------------------------------------------------------------------------
// public
// Default constructor. Create a HRABitmap (1,1)
//                      pixel (24 bits) if not specify.
//-----------------------------------------------------------------------------
HRAStoredRaster::HRAStoredRaster (HRPPixelType* pi_pPixelType,
                                  bool         pi_Resizable)
    : HRARaster (new HGF2DCoordSys ())
    {
    // Keep information
    m_Resizable = pi_Resizable;

    if (pi_pPixelType)
        m_pPixelType        = (HRPPixelType*)pi_pPixelType->Clone();
    else
        m_pPixelType        = new HRPPixelTypeV24R8G8B8 ();
    HASSERT(m_pPixelType != 0);

    // Save model between Logical and physical CoordSys
    m_pPhysicalCoordSys = GetCoordSys();

    m_pPhysicalRect = new HVEShape(0.0, 0.0, 1.0, 1.0, m_pPhysicalCoordSys);

    if (m_Resizable == true)
        {
        m_pRasterPhysicalRect = new HVEShape(0.0, 0.0, 1.0, 1.0, m_pPhysicalCoordSys);
        }
    else
        {
        m_pRasterPhysicalRect = m_pPhysicalRect;
        }

    // Set a quarter of a pixel tolerance
    double CenterX = 0.5;
    double CenterY = 0.5;
    HFCPtr<HGFTolerance> pTol (new HGFTolerance(CenterX - DEFAULT_PIXEL_TOLERANCE,
                                                CenterY - DEFAULT_PIXEL_TOLERANCE,
                                                CenterX + DEFAULT_PIXEL_TOLERANCE,
                                                CenterY + DEFAULT_PIXEL_TOLERANCE,
                                                m_pPhysicalCoordSys));
    m_pPhysicalRect->SetStrokeTolerance(pTol);
    m_pRasterPhysicalRect->SetStrokeTolerance(pTol);


    // Default shape = all raster's extent
    // Set the shape on the raster
    // Optimize, it skips over local implementation that performs
    // an intersection.
    HRARaster::SetShapeImpl (*m_pPhysicalRect);
    m_HasSetShape = false;

    m_pEffectiveShape = m_pPhysicalRect;

    //link to the pixel type
    LinkTo(m_pPixelType);
    }

//-----------------------------------------------------------------------------
// The constructor.
//-----------------------------------------------------------------------------
HRAStoredRaster::HRAStoredRaster   (uint64_t                    pi_WidthPixels,
                                    uint64_t                    pi_HeightPixels,
                                    const HGF2DTransfoModel*     pi_pModelCSp_CSl,
                                    const HFCPtr<HGF2DCoordSys>& pi_rpLogicalCoordSys,
                                    const HFCPtr<HRPPixelType>&  pi_rpType,
                                    bool                        pi_Resizable)
    : HRARaster(pi_rpLogicalCoordSys)
    {
    // Keep information
    m_Resizable         = pi_Resizable;
    m_pPixelType        = pi_rpType;

    // Set Physical Shape
    CHECK_HUINT64_TO_HDOUBLE_CONV(pi_WidthPixels)
    CHECK_HUINT64_TO_HDOUBLE_CONV(pi_HeightPixels)

    // Save model between Logical and physical CoordSys
    m_pTransfoModel = pi_pModelCSp_CSl == 0 ? new HGF2DIdentity() : pi_pModelCSp_CSl->Clone();
        
    m_pPhysicalCoordSys = new HGF2DCoordSys (*m_pTransfoModel, pi_rpLogicalCoordSys);

    m_pPhysicalRect = new HVEShape(0.0, 0.0, (double)pi_WidthPixels, (double)pi_HeightPixels, m_pPhysicalCoordSys);

    if (pi_Resizable)
        {
        m_pRasterPhysicalRect = new HVEShape(0.0, 0.0, (double)pi_WidthPixels, (double)pi_HeightPixels, m_pPhysicalCoordSys);
        }
    else
        {
        m_pRasterPhysicalRect = m_pPhysicalRect;
        }       

    // Set a quarter of a pixel tolerance
    double CenterX = pi_WidthPixels / 2.0;
    double CenterY = pi_HeightPixels / 2.0;
    HFCPtr<HGFTolerance> pTol (new HGFTolerance(CenterX - DEFAULT_PIXEL_TOLERANCE,
                                                CenterY - DEFAULT_PIXEL_TOLERANCE,
                                                CenterX + DEFAULT_PIXEL_TOLERANCE,
                                                CenterY + DEFAULT_PIXEL_TOLERANCE,
                                                m_pPhysicalCoordSys));
    m_pPhysicalRect->SetStrokeTolerance(pTol);
    m_pRasterPhysicalRect->SetStrokeTolerance(pTol);

    // Default shape = all raster's extent
    // Set the shape on the raster
    // Optimize, it skips over local implementation that performs
    // an intersection.
    HRARaster::SetShapeImpl (*m_pPhysicalRect);
    m_HasSetShape   = false;
    m_pEffectiveShape = m_pPhysicalRect;

    //link to the pixel type
    LinkTo(m_pPixelType);
    }


//-----------------------------------------------------------------------------
// The copy constructor.
//-----------------------------------------------------------------------------
HRAStoredRaster::HRAStoredRaster   (const HRAStoredRaster& pi_rObj)
    : HRARaster(pi_rObj),
      m_HasSetShape(pi_rObj.m_HasSetShape)
    {
    DeepCopy (pi_rObj);

    LinkTo(m_pPixelType);
    }


//-----------------------------------------------------------------------------
// The destructor.
//-----------------------------------------------------------------------------
HRAStoredRaster::~HRAStoredRaster()
    {
    UnlinkFrom(m_pPixelType);

    DeepDelete();


    if (m_pTransactionRecorder != 0)
        {
        // save the transaction stack into the recorder
        size_t UndoStackSize = 2 * m_UndoStack.size()+ 1;  // add 1 entry for the stack size
        HArrayAutoPtr<uint32_t> pTransactionStack(new uint32_t[UndoStackSize]);
        *pTransactionStack = (uint32_t)m_UndoStack.size();   // save the stack size
        uint32_t* pTransaction = pTransactionStack + 1;
        HFCPtr<HRAStoredRasterTransaction> pStoredRasterTransaction;

        if (!m_UndoStack.empty())
            {
            // serialize the undo stack
            for (size_t i = 0; i < *pTransactionStack; i++)
                {
                pStoredRasterTransaction = m_UndoStack.top();
                m_UndoStack.pop();
                *pTransaction++ = pStoredRasterTransaction->GetUndoTransaction()->GetID();
                *pTransaction++ = (pStoredRasterTransaction->HasRedoTransaction() ? pStoredRasterTransaction->GetRedoTransaction()->GetID() : -1);
                }
            }

        // save the undo stack into the recorder
        m_pTransactionRecorder->PutTransactionStack(HRATransactionRecorder::UNDO,
                                                    pTransactionStack,
                                                    UndoStackSize * sizeof(uint32_t));

        // serialize the redo stack
        size_t RedoStackSize = 2 * m_RedoStack.size() + 2;  // add 2 entries, stack size and saved bookmark
        if (UndoStackSize < RedoStackSize)
            pTransactionStack = new uint32_t[RedoStackSize];

        *pTransactionStack = (uint32_t)m_RedoStack.size();   // save the stack size
        *(pTransactionStack + 1) = -1;                      // initialize save bookmark
        pTransaction = pTransactionStack + 2;

        if (!m_RedoStack.empty())
            {
            for (size_t i = 0; i < *pTransactionStack; i++)
                {
                pStoredRasterTransaction = m_RedoStack.top();
                m_RedoStack.pop();
                HPRECONDITION(pStoredRasterTransaction->HasUndoTransaction() && pStoredRasterTransaction->HasRedoTransaction());

                if (*(pTransactionStack + 1) == -1 && pStoredRasterTransaction->GetSaveBookmark())
                    *(pTransactionStack + 1) = (uint32_t)m_RedoStack.size();     // write the save bookmark

                *pTransaction++ = pStoredRasterTransaction->GetUndoTransaction()->GetID();  // write the undo transaction
                *pTransaction++ = pStoredRasterTransaction->GetRedoTransaction()->GetID();  // write the redo transaction
                }
            }

        // save the redo stack into the recorder
        m_pTransactionRecorder->PutTransactionStack(HRATransactionRecorder::REDO,
                                                    pTransactionStack,
                                                    RedoStackSize * sizeof(uint32_t));

        }
    }

//-----------------------------------------------------------------------------
// public
// Saved
//-----------------------------------------------------------------------------
void HRAStoredRaster::Saved()
    {
    if (!m_RedoStack.empty())
        {
        m_RedoStack.top()->SetSaveBookmark();
        }
    }

//-----------------------------------------------------------------------------
// The assignment operator.
//-----------------------------------------------------------------------------
HRAStoredRaster& HRAStoredRaster::operator=(const HRAStoredRaster& pi_rObj)
    {
    if (this != &pi_rObj)
        {
        UnlinkFrom(m_pPixelType);

        HRARaster::operator=(pi_rObj);

        // Delete currently allocated memory for the object
        DeepDelete();

        DeepCopy (pi_rObj);

        m_HasSetShape   = pi_rObj.m_HasSetShape;

        LinkTo(m_pPixelType);
        }

    return *this;
    }


//-----------------------------------------------------------------------------
// public
// SetShape
//-----------------------------------------------------------------------------
void HRAStoredRaster::SetShape (const HVEShape& pi_rShape)
    {
    m_HasSetShape = true;
    HRARaster::SetShape(pi_rShape);
    }

//-----------------------------------------------------------------------------
// protected
// SetCoordSysImplementation
//-----------------------------------------------------------------------------
void HRAStoredRaster::SetCoordSysImplementation(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys)
    {
    // Set our model before calling HRARaster's implementation.
    // Why? Because we need to update our PhysicalRect before. HRARaster's
    // implementation will end up calling our UpdateEffectiveShape method,
    // and we'll need our PhysicalRect to do the job...
    SetModelCSp_CSl (*m_pTransfoModel, false);

    HRARaster::SetCoordSysImplementation(pi_rpCoordSys);
    }

//-----------------------------------------------------------------------------
// public
// SetTransfoModel - This method Set a new model
//-----------------------------------------------------------------------------
void HRAStoredRaster::SetTransfoModel (const HGF2DTransfoModel& pi_rModelCSp_CSl)
    {
    SetModelCSp_CSl (pi_rModelCSp_CSl, true);

    // Send coordinate system change notification
    Propagate(HGFGeometryChangedMsg());
    }

//-----------------------------------------------------------------------------
// public
// SetTransfoModel - This method Set a new model and a new Logical CoordSys
//-----------------------------------------------------------------------------
void HRAStoredRaster::SetTransfoModel (const HGF2DTransfoModel& pi_rModelCSp_CSl,
                                       const HFCPtr<HGF2DCoordSys>& pi_rpLogicalCoordSys)
    {
    SetTransfoModel (pi_rModelCSp_CSl);

    // Do nothing if same coordSys ?
    if (pi_rpLogicalCoordSys != GetCoordSys())
        {
        SetCoordSys(pi_rpLogicalCoordSys);
        }
    }


//-----------------------------------------------------------------------------
// public
// InitPhysicalExtent - This method initialize the physical extent
//-----------------------------------------------------------------------------
void HRAStoredRaster::InitSize(uint64_t pi_WidthPixels, uint64_t pi_HeightPixels)
    {
    CHECK_HUINT64_TO_HDOUBLE_CONV(pi_WidthPixels)
    CHECK_HUINT64_TO_HDOUBLE_CONV(pi_HeightPixels)

    m_pPhysicalRect = new HVEShape(0.0,
                                   0.0,
                                   (double)pi_WidthPixels,
                                   (double)pi_HeightPixels,
                                   GetPhysicalCoordSys());

    if (m_Resizable == true)
        {
        m_pRasterPhysicalRect = new HVEShape(0.0,
                                             0.0,
                                             (double)pi_WidthPixels,
                                             (double)pi_HeightPixels,
                                             GetPhysicalCoordSys());
        }
    else
        {
        m_pRasterPhysicalRect = m_pPhysicalRect;
        }

    double CenterX = pi_WidthPixels / 2.0;
    double CenterY = pi_HeightPixels / 2.0;
    HFCPtr<HGFTolerance> pTol (new HGFTolerance(CenterX - DEFAULT_PIXEL_TOLERANCE,
                                                CenterY - DEFAULT_PIXEL_TOLERANCE,
                                                CenterX + DEFAULT_PIXEL_TOLERANCE,
                                                CenterY + DEFAULT_PIXEL_TOLERANCE,
                                                GetPhysicalCoordSys()));
    m_pPhysicalRect->SetStrokeTolerance(pTol);
    m_pRasterPhysicalRect->SetStrokeTolerance(pTol);

    HRARaster::SetShapeImpl (*m_pPhysicalRect);
    m_HasSetShape   = false;

    // Set new shape
    m_pEffectiveShape = m_pPhysicalRect;

    // invalidate the representative palette cache
    InvalidateRepPalCache();
    }

//-----------------------------------------------------------------------------
// public
// SetRasterEffectiveExtent
//-----------------------------------------------------------------------------
void HRAStoredRaster::SetRasterExtent(const HGF2DExtent& pi_rRasterExtent)
    {
    HPRECONDITION(IsResizable());
    HPRECONDITION(pi_rRasterExtent.GetCoordSys() == m_pPhysicalCoordSys);


    int64_t Left   = (int64_t)(m_pRasterPhysicalRect->GetExtent().GetXMin() - pi_rRasterExtent.GetXMin());
    int64_t Right  = (int64_t)(pi_rRasterExtent.GetXMax() - m_pRasterPhysicalRect->GetExtent().GetXMax());
    int64_t Bottom = (int64_t)(m_pRasterPhysicalRect->GetExtent().GetYMin() - pi_rRasterExtent.GetYMin());
    int64_t Top    = (int64_t)(pi_rRasterExtent.GetYMax() - m_pRasterPhysicalRect->GetExtent().GetYMax());

    m_pRasterPhysicalRect = new HVEShape(pi_rRasterExtent);

    double CenterX = pi_rRasterExtent.GetWidth() / 2.0;
    double CenterY = pi_rRasterExtent.GetHeight() / 2.0;
    HFCPtr<HGFTolerance> pTol (new HGFTolerance(CenterX - DEFAULT_PIXEL_TOLERANCE,
                                                CenterY - DEFAULT_PIXEL_TOLERANCE,
                                                CenterX + DEFAULT_PIXEL_TOLERANCE,
                                                CenterY + DEFAULT_PIXEL_TOLERANCE,
                                                GetPhysicalCoordSys()));
    m_pRasterPhysicalRect->SetStrokeTolerance(pTol);

    m_HasSetShape   = false;

    // Set new shape
    m_pEffectiveShape = m_pRasterPhysicalRect;

    // invalidate the representative palette cache
    InvalidateRepPalCache();

    HPMObjectStore* pStore = GetStore();

    if (pStore != 0 && pStore->IsCompatibleWith(HRSObjectStore::CLASS_ID))
        ((HRSObjectStore*)pStore)->RasterSizeChanged(Top, Bottom, Left, Right);
    }       

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ImagePPStatus HRAStoredRaster::_CopyFrom(HRARaster& srcRaster, HRACopyFromOptions const& pi_rOptions)
    {
    HPRECONDITION(pi_rOptions.GetEffectiveCopyRegion() != NULL);
    HPRECONDITION(!pi_rOptions.GetEffectiveCopyRegion()->IsEmpty());

    // Intersect copy region with physical extent. 
    HFCPtr<HVEShape> totalCopyRegion(new HVEShape(GetPhysicalExtent()));
    totalCopyRegion->Intersect(*pi_rOptions.GetEffectiveCopyRegion());
    if(totalCopyRegion->IsEmpty())
        return COPYFROM_STATUS_VoidRegion;

    HASSERT(totalCopyRegion->GetCoordSys().GetPtr() == GetPhysicalCoordSys().GetPtr());
        
    HRACopyToOptions copyToOpts;
    copyToOpts.SetAlphaBlend(pi_rOptions.ApplyAlphaBlend());    
    copyToOpts.SetShape(totalCopyRegion.GetPtr());
    copyToOpts.SetResamplingMode(pi_rOptions.GetResamplingMode());
    
    ImagePPStatus status = IMAGEPP_STATUS_UnknownError;

    // Create destination root node.
    ImageSinkNodePtr pSinkNode = GetSinkNode(status, *totalCopyRegion, pi_rOptions.GetDestReplacingPixelType());
    if (IMAGEPP_STATUS_Success != status)
        return status;
   
    // undo/redo recording.
    if (GetCurrentTransaction() != NULL)
        pSinkNode->SetTransaction(GetCurrentTransaction());
   
    // Convert to destination physical coordinates  //&&OPTIMIZATION can we avoid change CS??
    totalCopyRegion->ChangeCoordSys(pSinkNode->GetPhysicalCoordSys());

    ImageTransformNodePtr pTrfNode = ImageTransformNode::CreateAndLink(status, *pSinkNode, totalCopyRegion);
    if(pTrfNode == NULL || status != IMAGEPP_STATUS_Success)
        return status;

    pTrfNode->SetResamplingMode(copyToOpts.GetResamplingMode());
    pTrfNode->SetAlphaBlend(pi_rOptions.ApplyAlphaBlend());
 
    if(IMAGEPP_STATUS_Success != (status = srcRaster.BuildCopyToContext(*pTrfNode, copyToOpts)))
        return status;

    ImageAllocatorRefPtr allocatorRefPtr = ImagePool::GetDefaultPool().GetAllocatorRef();

    if (allocatorRefPtr == NULL || pTrfNode->GetChildCount() == 0)
        return IMAGEPP_STATUS_UnknownError;

    status = pSinkNode->Execute(allocatorRefPtr->GetAllocator());

    return status;
    }


//-----------------------------------------------------------------------------
// public
// CopyFromLegacy
//-----------------------------------------------------------------------------
void HRAStoredRaster::CopyFromLegacy(const HFCPtr<HRARaster>& pi_pSrcRaster, const HRACopyFromLegacyOptions& pi_rOptions)
    {
    HRADrawOptions Options(pi_rOptions);
    Options.SetTransaction(GetCurrentTransaction());

    // Prepare the region we'll copy.
    //
    HFCPtr<HVEShape> pTotalCopyShape(new HVEShape(*GetEffectiveShape()));
    if (pi_rOptions.GetDestShape())
        pTotalCopyShape->Intersect(*pi_rOptions.GetDestShape());

    if (pi_rOptions.ApplySourceClipping())
        pTotalCopyShape->Intersect(*pi_pSrcRaster->GetEffectiveShape());

    Options.SetShape(pTotalCopyShape);

    if (!pTotalCopyShape->IsEmpty())
        {

        HAutoPtr<HRARasterIterator> pDstIterator(CreateIterator (HRAIteratorOptions(pTotalCopyShape,
                                                                                    GetPhysicalCoordSys(),
                                                                                    false)));

        // parse the rasters in the destination
        HFCPtr<HRARaster> pDstRaster;
        while (((pDstRaster = (*pDstIterator)()) != 0) && HRADrawProgressIndicator::GetInstance()->ContinueIteration())
            {
            HASSERT(pDstRaster->IsCompatibleWith(HRABitmapBase::CLASS_ID));
            HFCPtr<HRABitmapBase> pBitmap = (HFCPtr<HRABitmapBase>&)pDstRaster;

            // get the surface descriptor for this raster.
            HFCPtr<HGSSurfaceDescriptor> pDescriptor;
            if(pi_rOptions.GetDestReplacingPixelType() != 0)
                {
                HFCPtr<HRPPixelType> PixelReplace(pi_rOptions.GetDestReplacingPixelType());
                pDescriptor = pBitmap->GetSurfaceDescriptor(&PixelReplace);
                }
            else
                pDescriptor = pBitmap->GetSurfaceDescriptor();

            // create a surface for the destination
            HGFMappedSurface desSurface(pDescriptor, pBitmap->GetPhysicalCoordSys());

            pi_pSrcRaster->Draw(desSurface, Options);

            // Notify the bitmap of its change
            pBitmap->Updated(Options.GetShape().GetPtr());

            pDstIterator->Next();
            }
        }

    // invalidate the representative palette cache
    InvalidateRepPalCache();
    }

//-----------------------------------------------------------------------------
// public
// CopyFromLegacy
//-----------------------------------------------------------------------------
void HRAStoredRaster::CopyFromLegacy(const HFCPtr<HRARaster>& pi_pSrcRaster)
    {
    CopyFromLegacy(pi_pSrcRaster, HRACopyFromLegacyOptions());
    }

//-----------------------------------------------------------------------------
// public
// GetAveragePixelSize - Default implementation...
//-----------------------------------------------------------------------------
HGF2DExtent HRAStoredRaster::GetAveragePixelSize () const
    {
    return HGF2DExtent (0.0, 0.0, 1.0, 1.0, GetPhysicalCoordSys());
    }

//-----------------------------------------------------------------------------
// public
// GetPixelSizeRange - Default implementation...
//-----------------------------------------------------------------------------
void HRAStoredRaster::GetPixelSizeRange(HGF2DExtent& po_rMinimum, HGF2DExtent& po_rMaximum) const
    {
    uint64_t size_x, size_y;
    GetSize(&size_x, &size_y);

    po_rMinimum = HGF2DExtent (size_x / 2.0 - 0.5,
                               size_y / 2.0 - 0.5,
                               size_x / 2.0 + 0.5,
                               size_y / 2.0 + 0.5,
                               GetPhysicalCoordSys());

    po_rMaximum = po_rMinimum;
    }

//-----------------------------------------------------------------------------
// Move the raster object
//-----------------------------------------------------------------------------
void HRAStoredRaster::Move(const HGF2DDisplacement& pi_rDisplacement)
    {
    // Create a new model and Set
    SetTransfoModel (*GetTransfoModel()->ComposeInverseWithDirectOf(
                         HGF2DTranslation(pi_rDisplacement)));
    }


//-----------------------------------------------------------------------------
// Rotate the raster object around a specific point
//-----------------------------------------------------------------------------
void HRAStoredRaster::Rotate(double      pi_Angle,
                             const HGF2DLocation& pi_rOrigin)
    {
    // Model with translation.
    HGF2DSimilitude Rotation;
    HGF2DLocation LogicalLocation(pi_rOrigin, GetCoordSys());
    Rotation.AddRotation(pi_Angle, LogicalLocation.GetX(), LogicalLocation.GetY());

    // Create a new model and Set
    SetTransfoModel(*GetTransfoModel()->ComposeInverseWithDirectOf(Rotation));
    }


//-----------------------------------------------------------------------------
// Scale the raster object
//-----------------------------------------------------------------------------
void HRAStoredRaster::Scale(double pi_ScaleFactorX,
                            double pi_ScaleFactorY,
                            const HGF2DLocation& pi_rOrigin)
    {
    // Scaling model
    HGF2DStretch Scale;
    HGF2DLocation LogicalLocation(pi_rOrigin, GetCoordSys());
    Scale.AddAnisotropicScaling(pi_ScaleFactorX, pi_ScaleFactorY,
                                LogicalLocation.GetX(), LogicalLocation.GetY());

    // Create a new model and Set
    SetTransfoModel(*GetTransfoModel()->ComposeInverseWithDirectOf(Scale));
    }


//-----------------------------------------------------------------------------
// public
// GetRepresentativePalette .
//-----------------------------------------------------------------------------
unsigned short HRAStoredRaster::GetRepresentativePalette(
    HRARepPalParms* pio_pRepPalParms)
    {
    HPRECONDITION(pio_pRepPalParms != 0);

    unsigned short CountUsed;

    // get the representative palette cache if updated
    if(pio_pRepPalParms->UseCache() && (pio_pRepPalParms->GetHistogram() == 0))
        CountUsed = HRARaster::GetRepresentativePaletteCache(pio_pRepPalParms);
    else
        CountUsed = 0;

    // test if not updated
    if(CountUsed == 0)
        {
        HRPPixelType* pPixelType = pio_pRepPalParms->GetPixelType();

        HPRECONDITION(pPixelType != 0 && pPixelType->CountIndexBits() > 0);

        if (!pPixelType->GetPalette().IsReadOnly())
            {
            HFCPtr<HRPPixelType> pSrcType;
            // test if we must use the source pixel type or a pixel type replacer
            if(pio_pRepPalParms->GetSamplingOptions().GetSrcPixelTypeReplacer() != 0)
                pSrcType = pio_pRepPalParms->GetSamplingOptions().GetSrcPixelTypeReplacer();
            else
                pSrcType = GetPixelType();
            // we verify if the stored raster is by index or by value
            if(pSrcType->CountIndexBits() != 0 && pSrcType->CountValueBits() == 0)
                {
                // If the source palette is same size or smaller than destination palette to build,
                // simply build the palette using all entries from the source.

                if (pSrcType->CountIndexBits() <= pPixelType->CountIndexBits() &&
                    pio_pRepPalParms->GetMaxEntries() == 0 ||
                    pSrcType->GetPalette().GetMaxEntries() <=
                    MIN(pPixelType->GetPalette().GetMaxEntries(), pio_pRepPalParms->GetMaxEntries()))
                    {
                    // get the palette of the raster
                    const HRPPixelPalette& rSrcPalette = pSrcType->GetPalette();

                    // get the number of entries in the palette
                    CountUsed = (unsigned short)rSrcPalette.CountUsedEntries();

                    Byte Value[HRPPixelType::MAX_PIXEL_BYTES];

                    // Copy entries from source to destination palette, with conversion
                    HFCPtr<HRPPixelType> pDestPalettePixelType = HRPPixelTypeFactory::GetInstance()->Create(pPixelType->GetPalette().GetChannelOrg(), 0);
                    HFCPtr<HRPPixelConverter> pSrcIndexToDestCompositeValue = pSrcType->GetConverterTo(pDestPalettePixelType);
                    HRPPixelPalette* pPaletteToUpdate = &pPixelType->LockPalette();
                    uint32_t DstCountUsed = pPaletteToUpdate->CountUsedEntries();

                    // Information use, if the palette destination has a locked entry
                    //
                    size_t      EntryLockSize=0;
                    const void* pEntryLockData = NULL;
                    int32_t    EntryLocked = pPaletteToUpdate->GetLockedEntryIndex();
                    if (EntryLocked != -1)
                        {
                        EntryLockSize  = pPaletteToUpdate->GetPixelEntrySize();
                        pEntryLockData = pPaletteToUpdate->GetCompositeValue(EntryLocked);
                        }

                    // TR 159986: RLE need a raw value in RLE.
                    if(pSrcType->IsCompatibleWith(HRPPixelTypeI1R8G8B8RLE::CLASS_ID) || pSrcType->IsCompatibleWith(HRPPixelTypeI1R8G8B8A8RLE::CLASS_ID))
                        {
                        unsigned short IndexToConvert = 0;
                        unsigned short IndexDst = 0;
                        for (; IndexToConvert < 2; ++IndexToConvert,++IndexDst)
                            {
                            unsigned short SourceRawValue = IndexToConvert ? 0 : 1;
                            pSrcIndexToDestCompositeValue->Convert(&SourceRawValue, Value);

                            // Palette destination has a locked entry ?
                            // If the value of the locked entry is not present in the source
                            // palette we lost an index.
                            if (EntryLocked != -1)
                                {
                                // If the current Value == the lock entry
                                // we do possibly nothing, because the locked entry already have
                                // the good value, and  can't be changed.
                                if (memcmp(Value, pEntryLockData, EntryLockSize) == 0)
                                    {
                                    // If the entry is not the locked entry
                                    // keep the same index for the next look.
                                    if (EntryLocked != IndexDst)
                                        --IndexDst;         // do not increment
                                    }
                                else
                                    {
                                    // Skip the lock entry
                                    if (EntryLocked == IndexDst)
                                        ++IndexDst;

                                    if (IndexDst >= DstCountUsed)
                                        --IndexDst;
                                    pPaletteToUpdate->SetCompositeValue(IndexDst, Value);
                                    }
                                }
                            else
                                pPaletteToUpdate->SetCompositeValue(IndexDst, Value);
                            }
                        }
                    else if (pSrcType->CountIndexBits() < 8)
                        {
                        unsigned short IndexToConvert = 0;
                        unsigned short IndexDst = 0;
                        for (; IndexToConvert < CountUsed; ++IndexToConvert,++IndexDst)
                            {
                            Byte SourceRawValue = CONVERT_TO_BYTE(IndexToConvert << (8 - pSrcType->CountIndexBits()));
                            pSrcIndexToDestCompositeValue->Convert(&SourceRawValue, Value);

                            // Palette destination has a locked entry ?
                            // If the value of the locked entry is not present in the source
                            // palette we lost an index.
                            if (EntryLocked != -1)
                                {
                                // If the current Value == the lock entry
                                // we do possibly nothing, because the locked entry already have
                                // the good value, and  can't be changed.
                                if (memcmp(Value, pEntryLockData, EntryLockSize) == 0)
                                    {
                                    // If the entry is not the locked entry
                                    // keep the same index for the next look.
                                    if (EntryLocked != IndexDst)
                                        --IndexDst;         // do not increment
                                    }
                                else
                                    {
                                    // Skip the lock entry
                                    if (EntryLocked == IndexDst)
                                        ++IndexDst;

                                    if (IndexDst >= DstCountUsed)
                                        --IndexDst;
                                    pPaletteToUpdate->SetCompositeValue(IndexDst, Value);
                                    }
                                }
                            else
                                pPaletteToUpdate->SetCompositeValue(IndexDst, Value);
                            }
                        }
                    else
                        {
                        unsigned short IndexToConvert = 0;
                        unsigned short IndexDst = 0;
                        for (; IndexToConvert < CountUsed; ++IndexToConvert,++IndexDst)
                            {
                            pSrcIndexToDestCompositeValue->Convert(&IndexToConvert, Value);

                            // Palette destination has a locked entry ?
                            // If the value of the locked entry is not present in the source
                            // palette we lost an index.
                            if (EntryLocked != -1)
                                {
                                // If the current Value == the lock entry
                                // we do possibly nothing, because the locked entry already have
                                // the good value, and  can't be changed.
                                if (memcmp(Value, pEntryLockData, EntryLockSize) == 0)
                                    {
                                    // If the entry is not the locked entry
                                    // keep the same index for the next loop.
                                    if (EntryLocked != IndexDst)
                                        --IndexDst;         // do not increment
                                    }
                                else
                                    {
                                    // Skip the lock entry
                                    if (EntryLocked == IndexDst)
                                        ++IndexDst;

                                    if (IndexDst >= DstCountUsed)
                                        --IndexDst;
                                    pPaletteToUpdate->SetCompositeValue(IndexDst, Value);
                                    }
                                }
                            else
                                pPaletteToUpdate->SetCompositeValue(IndexDst, Value);
                            }
                        }
                    pPixelType->UnlockPalette();

                    // test if we have an histogram and compute it if yes
                    if(pio_pRepPalParms->GetHistogram() != 0)
                        {
                        HRAHistogramOptions Histo (pio_pRepPalParms->GetHistogram(), pPixelType, pio_pRepPalParms->GetSamplingOptions());
                        ComputeHistogram(&Histo);
                        }
                    }
                }
            else
                {
                // Is the number of values possible in the source pixel type
                // less than the number of entries in the palette?
                if(pSrcType->CountPixelRawDataBits() < pPixelType->CountIndexBits() && (pio_pRepPalParms->GetHistogram() == 0) &&
                   pSrcType->CountValueBits() == 1)
                    {
                    HASSERT(pSrcType->CountIndexBits() == 0 && pSrcType->CountPixelRawDataBits() == 1);

                    HRPPixelPalette& rPalette = pPixelType->LockPalette();

                    // Create a value pixeltype using the palette channel org.
                    HFCPtr<HRPPixelType> pValuePixelType = HRPPixelTypeFactory::GetInstance()->Create(rPalette.GetChannelOrg(), 0);
                    if(pValuePixelType)
                        {
                        HFCPtr<HRPPixelConverter> pConverter = pValuePixelType->GetConverterFrom(pSrcType.GetPtr());
                        HArrayAutoPtr<Byte>     outCompValue(new Byte[(pValuePixelType->CountPixelRawDataBits() + 7) / 8]);
                        Byte                    srcValue;

                        srcValue = 0x00;    // first bit OFF
                        pConverter->Convert(&srcValue, outCompValue);
                        rPalette.SetCompositeValue(0, outCompValue);

                        srcValue = 0x80;    // first bit ON
                        pConverter->Convert(&srcValue, outCompValue);
                        rPalette.SetCompositeValue(1, outCompValue);

                        CountUsed = 2;
                        }

                    pPixelType->UnlockPalette();
                    }
                }

            // if we computed a representative palette at this level,
            // we update the cache
            if(CountUsed != 0 && pio_pRepPalParms->UseCache())
                UpdateRepPalCache(CountUsed, pPixelType->GetPalette());
            }
        }

    return CountUsed;
    }



//-----------------------------------------------------------------------------
// public
// SetTransactionRecorder
//-----------------------------------------------------------------------------
void HRAStoredRaster::SetTransactionRecorder(HFCPtr<HRATransactionRecorder>& pi_rpTransactionRecorder)
    {
    m_pTransactionRecorder = pi_rpTransactionRecorder;

    // check if the recorder contains an UNDO stack
    void* pStackPtr;
    size_t   StackSize;
    m_pTransactionRecorder->GetTransactionStack(HRATransactionRecorder::UNDO, (void**)&pStackPtr, &StackSize);
    HArrayAutoPtr<uint32_t> pStack((uint32_t*)pStackPtr);

    if (pStack != 0)
        {
        // the stack is a list of UInt32 where the first entry is the element count
        // each entry contains the UNDO transaction ID and the REDO transaction ID
        HPRECONDITION(StackSize >= sizeof(uint32_t));
        HPRECONDITION((StackSize % sizeof(uint32_t)) == 0);
        HPRECONDITION(((StackSize / sizeof(uint32_t)) % 2) == 1);

        uint32_t* pTransaction(pStack + StackSize / sizeof(uint32_t) - 1);
        HFCPtr<HRATransaction> pUndoTransaction;
        HFCPtr<HRATransaction> pRedoTransaction;
        for(size_t i = 0; i < *pStack; i++)
            {
            if (*pTransaction != -1)
                {
                // the redo transaction exist, create it
                pRedoTransaction = m_pTransactionRecorder->CreateTransaction(HRATransactionRecorder::REDO,
                                                                             *pTransaction--);
                }
            else
                {
                // no redo transaction
                pRedoTransaction = 0;
                --pTransaction;
                }

            // create the redo transaction
            pUndoTransaction = m_pTransactionRecorder->CreateTransaction(HRATransactionRecorder::UNDO,
                                                                         *pTransaction--);

            // push the transaction on the undo stack
            m_UndoStack.push(new HRAStoredRasterTransaction(pUndoTransaction, pRedoTransaction));
            }
        }


    // redo stack
    m_pTransactionRecorder->GetTransactionStack(HRATransactionRecorder::REDO, (void**)&pStackPtr, &StackSize);
    pStack = (uint32_t*)pStackPtr;

    if (pStack != 0)
        {
        HPRECONDITION(StackSize >= sizeof(uint32_t));
        HPRECONDITION((StackSize % sizeof(uint32_t)) == 0);
        HPRECONDITION(((StackSize / sizeof(uint32_t)) % 2) == 0);    // the redo stack contains 2 extra values, the stack size and the saved bookmark

        uint32_t* pTransaction(pStack + StackSize / sizeof(uint32_t) - 1);
        HFCPtr<HRATransaction> pUndoTransaction;
        HFCPtr<HRATransaction> pRedoTransaction;
        uint32_t SavedBookmark = *(pStack + 1);
        for(size_t i = 0; i < *pStack; i++)
            {
            pRedoTransaction = m_pTransactionRecorder->CreateTransaction(HRATransactionRecorder::REDO,
                                                                         *pTransaction--);
            // the undo transaction always exist
            pUndoTransaction = m_pTransactionRecorder->CreateTransaction(HRATransactionRecorder::UNDO,
                                                                         *pTransaction--);

            // push the transactio on the redo stack
            m_RedoStack.push(new HRAStoredRasterTransaction(pUndoTransaction, pRedoTransaction));

            if (i == SavedBookmark)
                m_RedoStack.top()->SetSaveBookmark();
            }
        }

    // apply redo transaciton until the save bookmark has reached
    while (!m_RedoStack.empty())
        {
        if (m_RedoStack.top()->GetSaveBookmark())
            break;

        Redo();
        }
    }

//-----------------------------------------------------------------------------
// public
// SetTransactionRecorder
//-----------------------------------------------------------------------------
const HFCPtr<HRATransactionRecorder>& HRAStoredRaster::GetTransactionRecorder() const
    {
    return m_pTransactionRecorder;
    }


//-----------------------------------------------------------------------------
// public
// StartTransaction
//-----------------------------------------------------------------------------
HSTATUS HRAStoredRaster::StartTransaction()
    {
    HPRECONDITION(m_pTransactionRecorder != 0);
    HPRECONDITION(m_pCurrentTransaction == 0);

    m_pCurrentTransaction = m_pTransactionRecorder->CreateNewTransaction(HRATransactionRecorder::UNDO);

    return (m_pCurrentTransaction != 0 ? H_SUCCESS : H_ERROR);
    }

//-----------------------------------------------------------------------------
// public
// EndTransaction
//-----------------------------------------------------------------------------
HSTATUS HRAStoredRaster::EndTransaction()
    {
    HSTATUS RetValue = H_EMPTY_TRANSACTION;
    HPRECONDITION(m_pCurrentTransaction != 0);

    if (!m_pCurrentTransaction->IsEmpty())
        {
        m_pCurrentTransaction->Commit();

        if (!m_RedoStack.empty())
            while (m_RedoStack.size() != 0)
                {
                m_RedoStack.top()->Clear();
                m_RedoStack.pop();
                }
                

        m_UndoStack.push(new HRAStoredRasterTransaction(m_pCurrentTransaction));

        RetValue = H_SUCCESS;
        }

    m_pCurrentTransaction = 0;

    return RetValue;
    }


//-----------------------------------------------------------------------------
// public
// Undo
//-----------------------------------------------------------------------------
void HRAStoredRaster::Undo(bool pi_RecordRedo)
    {
    HPRECONDITION(m_pCurrentTransaction == 0);

    if (!m_UndoStack.empty())
        {
        HFCPtr<HRAStoredRasterTransaction> pTransaction(m_UndoStack.top());
        m_UndoStack.pop();

        if (pi_RecordRedo && !pTransaction->HasRedoTransaction())
            {
            // start a new transaction to record the undo operation
            m_pCurrentTransaction = m_pTransactionRecorder->CreateNewTransaction(HRATransactionRecorder::REDO);
            ApplyTransaction(pTransaction->GetUndoTransaction());
            m_pCurrentTransaction->Commit();
            pTransaction->SetRedoTransaction(m_pCurrentTransaction);
            m_pCurrentTransaction = 0;
            }
        else
            ApplyTransaction(pTransaction->GetUndoTransaction());

        m_RedoStack.push(pTransaction);
        }
    }


//-----------------------------------------------------------------------------
// public
// Redo
//-----------------------------------------------------------------------------
void HRAStoredRaster::Redo()
    {
    HPRECONDITION(m_pCurrentTransaction == 0);

    if (!m_RedoStack.empty())
        {
        HFCPtr<HRAStoredRasterTransaction> pTransaction(m_RedoStack.top());
        m_RedoStack.pop();
        HPOSTCONDITION(pTransaction->HasRedoTransaction());
        ApplyTransaction(pTransaction->GetRedoTransaction());
        m_UndoStack.push(pTransaction);
        }
    }


//-----------------------------------------------------------------------------
// Print the state of the object
//-----------------------------------------------------------------------------
void HRAStoredRaster::PrintState(ostream& po_rOutput) const
    {
#ifdef __HMR_PRINTSTATE

    // Call the parent
    HRARaster::PrintState (po_rOutput);

    po_rOutput

            << "HRAStoredRaster"
            << endl;

#endif
    }



//-----------------------------------------------------------------------------
// protected
// RecalculateEffectiveShape - Intersection between the physical and logical
//                             shape.
//-----------------------------------------------------------------------------
void HRAStoredRaster::RecalculateEffectiveShape ()
    {
    // If the Raster has no SHape, the Effective is already compute...
    if (m_HasSetShape)
        {
        m_pEffectiveShape = new HVEShape (*m_pRasterPhysicalRect);

        // Intersect Physical and Logical Shape
        // Note: For performance, m_pEffectiveShape must be in physicalCoorSys
        //
        m_pEffectiveShape->Intersect (GetShape());
        }
    }


//-------------------------------------------------------------------- Privates

//-----------------------------------------------------------------------------
// private
// DeepCopy
//-----------------------------------------------------------------------------
void HRAStoredRaster::DeepCopy (const HRAStoredRaster& pi_rObj)
    {
    m_Resizable           = pi_rObj.m_Resizable;
    m_pPixelType          = pi_rObj.m_pPixelType;           //DMx Copy object???
    m_pPhysicalCoordSys   = pi_rObj.m_pPhysicalCoordSys;
    m_pTransfoModel       = pi_rObj.m_pTransfoModel;
    m_pPhysicalRect       = new HVEShape(*pi_rObj.m_pPhysicalRect);
    m_pRasterPhysicalRect = new HVEShape(*pi_rObj.m_pRasterPhysicalRect);
    m_pEffectiveShape     = new HVEShape(*pi_rObj.m_pEffectiveShape);
    HASSERT(m_pEffectiveShape != 0);
    }


//-----------------------------------------------------------------------------
// private
// DeepDelete
//-----------------------------------------------------------------------------
void HRAStoredRaster::DeepDelete ()
    {
    }


//-----------------------------------------------------------------------------
// private
// SetModelCSp_CSl : Set the model beween Physical CoordSys and the Logical
//-----------------------------------------------------------------------------
void HRAStoredRaster::SetModelCSp_CSl (const HGF2DTransfoModel& pi_rModel,
                                       bool                    pi_AdjustLogicalShape)
    {
    // Create a new physical coordinate system by composing the new translation
    // with the old transformation between physical and logical.
    HFCPtr<HGF2DCoordSys> NewPhysical(new HGF2DCoordSys (pi_rModel, GetCoordSys()));

    // Keep the physical and Effective Shape in new Physical CoordSys
    // Not need to call ChangeCoord, because these Shapes must be always in
    // PhysicalCoordSys
    m_pPhysicalRect->SetCoordSys(NewPhysical);
    m_pRasterPhysicalRect->SetCoordSys(NewPhysical);
    m_pEffectiveShape->SetCoordSys(NewPhysical);

    // Got to bring logical shape in physical CS...
    if (pi_AdjustLogicalShape)
        {
        HVEShape LogicalShape(GetShape());
        LogicalShape.ChangeCoordSys(m_pPhysicalCoordSys);
        LogicalShape.SetCoordSys(NewPhysical);
        SetShapeImpl(LogicalShape);
        }

    // Change for the new physical CS
    m_pPhysicalCoordSys = NewPhysical;
    m_pPhysicalCoordSys->SetModificationState();    // CoordSys Modified

    // Reset model between Logical and physical CoordSys
    m_pTransfoModel = pi_rModel.Clone();
    }

//-----------------------------------------------------------------------------
// private
// ApplyTransaction
//-----------------------------------------------------------------------------
void HRAStoredRaster::ApplyTransaction(const HFCPtr<HRATransaction>& pi_rpTransaction)
    {
    uint64_t PosX;
    uint64_t PosY;
    uint32_t Width;
    uint32_t Height;
    size_t  DataSize;

    if (pi_rpTransaction->PopEntry(&PosX,
                                   &PosY,
                                   &Width,
                                   &Height,
                                   &DataSize,
                                   true))   // first call
        {
        HCDPacket NewData;
        NewData.SetBufferOwnership(true);


        HCDPacket CurrentData;
        CurrentData.SetBufferOwnership(true);

        HFCPtr<HRABitmapBase> pLastBitmap;
        HAutoPtr<HGF2DExtent> pLastBitmapExtent;
        HFCPtr<HRARaster> pDstRaster;
        HAutoPtr<HRARasterEditor> pBitmapEditor;
        HRAEditor* pEditor = NULL;

        do
            {
            if (NewData.GetBufferSize() < DataSize)
                {
                NewData.SetBuffer(new Byte[DataSize], DataSize);
                CurrentData.SetBuffer(new Byte[DataSize], DataSize);
                }

            //TR 270241 - Instead of throwing (no exception should be thrown from HRA objects),
            //            just go to the next entry for now, since the application using this
            //            functionality (MS) hasn't the concept of undo/redo failure.
            if (pi_rpTransaction->ReadEntryData(DataSize, NewData.GetBufferAddress()) != DataSize)
                {
                continue;
                }

            memcpy(CurrentData.GetBufferAddress(), NewData.GetBufferAddress(), DataSize);
            CurrentData.SetDataSize(DataSize);


            HGF2DExtent Extent((double)PosX,
                               (double)PosY,
                               (double)(PosX + Width),
                               (double)(PosY + Height),
                               GetPhysicalCoordSys());

            // check if the new entry is on the same block
            if (pLastBitmap != 0 && pLastBitmapExtent->OuterContains(Extent))
                {
                // apply the transaction data into the bitmap
                pEditor->MergeRuns((HUINTX)PosX,
                                   (HUINTX)PosY,
                                   Width,
                                   Height,
                                   NewData.GetBufferAddress(),
                                   m_pCurrentTransaction);
                }
            else
                {
                // find the block on which the entry must be apply
                HFCPtr<HVEShape> pExtent(new HVEShape(Extent));
                HAutoPtr<HRARasterIterator> pDstIterator(CreateIterator (HRAIteratorOptions(pExtent,
                                                                                            GetPhysicalCoordSys(),
                                                                                            false)));

                while (((pDstRaster = (*pDstIterator)()) != 0) && HRADrawProgressIndicator::GetInstance()->ContinueIteration())
                    {
                    HASSERT(pDstRaster->IsCompatibleWith(HRABitmapBase::CLASS_ID));
                    HFCPtr<HRABitmapBase> pBitmap = (HFCPtr<HRABitmapBase>&)pDstRaster;

                    // the new block is different than the last one
                    if (pBitmap != pLastBitmap)
                        {
                        if (pLastBitmap != 0)
                            pLastBitmap->Updated();

                        pLastBitmap = pBitmap;
                        pLastBitmapExtent = new HGF2DExtent(pBitmap->GetPhysicalExtent());
                        pLastBitmapExtent->ChangeCoordSys(GetPhysicalCoordSys());
                        pBitmapEditor = pBitmap->CreateEditor(HFC_READ_WRITE);
                        pEditor = ((HRABitmapEditor*)pBitmapEditor.get())->GetSurfaceEditor();
                        }


                    // apply the transaction data into the bitmap
                    pEditor->MergeRuns((HUINTX)PosX,
                                       (HUINTX)PosY,
                                       Width,
                                       Height,
                                       NewData.GetBufferAddress(),
                                       m_pCurrentTransaction);

                    pDstIterator->Next();
                    }
                }
            }
        while(pi_rpTransaction->PopEntry(&PosX, &PosY, &Width, &Height, &DataSize));

        if (pLastBitmap != 0)
            pLastBitmap->Updated();

        // invalidate the representative palette cache
        InvalidateRepPalCache();
        }
    }


//-----------------------------------------------------------------------------
// public
// GetContext : Get the context
//-----------------------------------------------------------------------------
HFCPtr<HMDContext> HRAStoredRaster::GetContext()
    {
    return m_pContext;
    }

//-----------------------------------------------------------------------------
// public
// SetContext : Set the context
//-----------------------------------------------------------------------------
void HRAStoredRaster::SetContext(const HFCPtr<HMDContext>& pi_rpContext)
    {
    m_pContext = pi_rpContext;
    }

//-----------------------------------------------------------------------------
// public
// GetSize -  Return the current physical shape.
//-----------------------------------------------------------------------------
void HRAStoredRaster::GetSize(uint64_t* po_pWidthPixels, uint64_t* po_pHeightPixels) const
    {
    HPRECONDITION(m_pPhysicalRect->IsRectangle());

    double XMin;
    double YMin;
    double XMax;
    double YMax;

    ((HVE2DRectangle*)m_pPhysicalRect->GetShapePtr())->GetRectangle(&XMin, &YMin, &XMax, &YMax);
    *po_pWidthPixels     = (uint64_t)(XMax+0.5); // Do not use standard round as it trunk with type long.
    *po_pHeightPixels    = (uint64_t)(YMax+0.5); // Do not use standard round as it trunk with type long.
    }

//-----------------------------------------------------------------------------
// public
// ContainsPixelsWithCannel
//  - Tells if the stored raster contains pixels with the specified channel.
//-----------------------------------------------------------------------------
bool HRAStoredRaster::ContainsPixelsWithChannel(
    HRPChannelType::ChannelRole pi_Role,
    Byte                      pi_Id) const
    {
    return (m_pPixelType->GetChannelOrg().GetChannelIndex(pi_Role, pi_Id) != HRPChannelType::FREE);
    }

//-----------------------------------------------------------------------------
// public
// GetPixelType - Returns the pixel type.
//-----------------------------------------------------------------------------
HFCPtr<HRPPixelType> HRAStoredRaster::GetPixelType() const
    {
    return (m_pPixelType);
    }

//-----------------------------------------------------------------------------
// public
// ClearAllRecordedData
//-----------------------------------------------------------------------------
void HRAStoredRaster::ClearAllRecordedData()
    {
    HPRECONDITION(m_pTransactionRecorder != 0);

    m_pTransactionRecorder->ClearAllRecordedData();
    while (m_UndoStack.size() != 0)
        m_UndoStack.pop();

    while (m_RedoStack.size() != 0)
        m_RedoStack.pop();
    }

//-----------------------------------------------------------------------------
// public
// GetRasterSize -  Return the current raster physical shape.
//-----------------------------------------------------------------------------
void HRAStoredRaster::GetRasterSize(uint64_t*     po_pWidthPixels,
    uint64_t*     po_pHeightPixels,
    uint64_t*     po_pPosX,
    uint64_t*     po_pPosY) const
    {
    HPRECONDITION(po_pWidthPixels != 0);
    HPRECONDITION(po_pHeightPixels != 0);

    if (!m_Resizable)
        {
        GetSize(po_pWidthPixels, po_pHeightPixels);
        if (po_pPosX != 0)
            *po_pPosX = 0;
        if (po_pPosY != 0)
            *po_pPosY = 0;
        }
    else
        {
        HPRECONDITION(m_pPhysicalRect->IsRectangle());

        *po_pWidthPixels = (uint64_t)(m_pRasterPhysicalRect->GetExtent().GetWidth()) + 1;
        *po_pHeightPixels = (uint64_t)(m_pRasterPhysicalRect->GetExtent().GetHeight()) + 1;

        if (po_pPosX != 0)
            *po_pPosX = (uint64_t)(m_pRasterPhysicalRect->GetExtent().GetXMin() + 0.5); // Do not use standard round as it trunk with type long.

        if (po_pPosY != 0)
            *po_pPosY = (uint64_t)(m_pRasterPhysicalRect->GetExtent().GetYMin() + 0.5); // Do not use standard round as it trunk with type long.
        }
    }

//-----------------------------------------------------------------------------
// public
// SetCurrentTransaction
//-----------------------------------------------------------------------------
void HRAStoredRaster::SetCurrentTransaction(HFCPtr<HRATransaction>& pi_rpTransaction)
    {
    m_pCurrentTransaction = pi_rpTransaction;
    }

//-----------------------------------------------------------------------------
// public
// GetCurrentTransaction
//-----------------------------------------------------------------------------
HFCPtr<HRATransaction>& HRAStoredRaster::GetCurrentTransaction()
    {
    return m_pCurrentTransaction;
    }

//-----------------------------------------------------------------------------
// public
// CanUndo
//-----------------------------------------------------------------------------
bool HRAStoredRaster::CanUndo() const
    {
    return !m_UndoStack.empty();
    }

//-----------------------------------------------------------------------------
// public
// GetCurrentTransaction
//-----------------------------------------------------------------------------
bool HRAStoredRaster::CanRedo() const
    {
    return !m_RedoStack.empty();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
double HRAStoredRaster::EvaluateScaleFactor(HFCPtr<HGF2DCoordSys> const& srcCS, HFCPtr<HGF2DCoordSys> const& dstCS, HVEShape const& shape)
    {
    HFCPtr<HGF2DTransfoModel> pTransfo = srcCS->GetTransfoModelTo(dstCS);
    double factor = 1.0;

    // Check if we can extract Scaling
    // Here we check if the model represents a parallelism preserving relation
    // With parallelism preserving relation, even though the model cannot be properly
    // represented by a stretch, insures that the scaling factors are not dependant upon the
    // position into space.
    if (pTransfo->PreservesParallelism())
        {
        double scaleFactorX, scaleFactorY;
        HGF2DDisplacement displacement;
        pTransfo->GetStretchParams(&scaleFactorX, &scaleFactorY, &displacement);
        factor = MIN(fabs(scaleFactorX), fabs(scaleFactorY));
        }
    else
        {
        // We will simply use the extent
        HGF2DExtent shapeExtent(shape.GetExtent());

        // Check if raster has a size
        if (shapeExtent.IsDefined())
            {
            // Obtain extent four corners
            HGF2DLocation lowerLeft(shapeExtent.GetLowerLeft(), srcCS);
            HGF2DLocation lowerRight(shapeExtent.GetLowerRight(), srcCS);
            HGF2DLocation upperLeft(shapeExtent.GetUpperLeft(), srcCS);
            HGF2DLocation upperRight(shapeExtent.GetUpperRight(), srcCS);

            HGF2DLocation otherLowerLeft(lowerLeft, dstCS);
            HGF2DLocation otherLowerRight(lowerRight, dstCS);
            HGF2DLocation otherUpperLeft(upperLeft, dstCS);
            HGF2DLocation otherUpperRight(upperRight, dstCS);

            // Compute scales
            // Extent may not be undefined
            HASSERT((lowerLeft - lowerRight).CalculateLength() != 0.0);
            HASSERT((upperLeft - upperRight).CalculateLength() != 0.0);
            HASSERT((upperLeft - lowerLeft).CalculateLength() != 0.0);
            HASSERT((upperRight - lowerRight).CalculateLength() != 0.0);

            double lowerScale = (otherLowerLeft - otherLowerRight).CalculateLength() / (lowerLeft - lowerRight).CalculateLength();
            double upperScale = (otherUpperLeft - otherUpperRight).CalculateLength() / (upperLeft - upperRight).CalculateLength();
            double leftScale = (otherUpperLeft - otherLowerLeft).CalculateLength() / (upperLeft - lowerLeft).CalculateLength();
            double rightScale = (otherUpperRight - otherLowerRight).CalculateLength() / (upperRight - lowerRight).CalculateLength();

            double scale = MAX(fabs(lowerScale), MAX(fabs(upperScale), MAX(fabs(leftScale), fabs(rightScale))));

            BeAssert(scale != 0.0);

            factor = scale;
            }
        }

    return factor;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t HRAStoredRaster::EvaluateScaleFactorPowerOf2(HFCPtr<HGF2DCoordSys> const& srcCS, HFCPtr<HGF2DCoordSys> const& dstCS, HVEShape const& shape)
    {
    double scaleFactor = EvaluateScaleFactor(srcCS, dstCS, shape);
    double logBase2 = floor(log(fabs(scaleFactor)) / log(2.0));
    uint32_t shift = logBase2 > 0 ? (uint32_t)logBase2 : 0;

    return shift;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  07/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ImageSinkNodePtr HRAStoredRaster::GetSinkNode(ImagePPStatus& status, HVEShape const& sinkShape, HFCPtr<HRPPixelType> pReplacingPixelType)
    {
    if (pReplacingPixelType != NULL && GetPixelType()->CountPixelRawDataBits() != pReplacingPixelType->CountPixelRawDataBits())
        {
        BeAssert(GetPixelType()->CountPixelRawDataBits() == pReplacingPixelType->CountPixelRawDataBits());
        status = COPYFROM_STATUS_IncompatiblePixelTypeReplacer;
        return NULL;
        }    
   
    return _GetSinkNode(status, sinkShape, pReplacingPixelType); 
    }
