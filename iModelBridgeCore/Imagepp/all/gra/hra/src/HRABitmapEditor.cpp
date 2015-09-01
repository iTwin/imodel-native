/*--------------------------------------------------------------------------------------+
|
|     $Source: all/gra/hra/src/HRABitmapEditor.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//-----------------------------------------------------------------------------
// Methods for class HRABitmapEditor
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HRABitmapEditor.h>
#include <Imagepp/all/h/HRABitmapBase.h>
#include <Imagepp/all/h/HRASurface.h>
#include <Imagepp/all/h/HGSSurfaceDescriptor.h>
#include <Imagepp/all/h/HGF2DGrid.h>
#include <Imagepp/all/h/HRAEditor.h>
#include <Imagepp/all/h/HGSRegion.h>


//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
HRABitmapEditor::HRABitmapEditor(const HFCPtr<HRABitmapBase>&   pi_pRaster,
                                 const HFCAccessMode        pi_Mode)

    : HRARasterEditor ((HFCPtr<HRARaster>&) pi_pRaster, pi_Mode)
    {
    InitObject(pi_pRaster->GetSurfaceDescriptor());
    }

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
HRABitmapEditor::HRABitmapEditor(const HFCPtr<HRABitmapBase>& pi_pRaster,
                                 const HVEShape&          pi_rShape,
                                 const HFCAccessMode      pi_Mode)
    : HRARasterEditor ((HFCPtr<HRARaster>&) pi_pRaster, pi_Mode)
    {
    InitObject(pi_pRaster->GetSurfaceDescriptor(), &pi_rShape);
    }

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
HRABitmapEditor::HRABitmapEditor(const HFCPtr<HRABitmapBase>& pi_pRaster,
                                 const HGFScanLines&      pi_rScanLines,
                                 const HFCAccessMode      pi_Mode)
    : HRARasterEditor ((HFCPtr<HRARaster>&) pi_pRaster, pi_Mode)
    {
    InitObject(pi_pRaster->GetSurfaceDescriptor(), 0, &pi_rScanLines);
    }

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
HRABitmapEditor::~HRABitmapEditor()
    {
    // update the bitmap if required
//     if(GetLockMode().m_HasWriteAccess)
//         {
//         m_pEditor.reset();
//         m_pSurface.reset();
//         }

    // assure to complete the buffer
    m_pEditor.reset();
    m_pSurface.reset();
    }

//-----------------------------------------------------------------------------
// private
// InitObject
//-----------------------------------------------------------------------------
void HRABitmapEditor::InitObject(const HFCPtr<HGSSurfaceDescriptor>& pi_rpSurDescriptor,
                                 const HVEShape*          pi_pShape,
                                 const HGFScanLines*      pi_pScanlines)
    {
    HPRECONDITION(pi_pShape == 0 || pi_pScanlines == 0);
    HPRECONDITION(pi_pShape == 0 || !pi_pShape->IsEmpty()); // Either no shape is provided or this provided shape is not empty

    HFCPtr<HGSRegion> pClipRegion;

    if (pi_pShape != 0)
        {
        HFCPtr<HVEShape> pTmpShape(new HVEShape(*pi_pShape));
        pTmpShape->ChangeCoordSys(((HFCPtr<HRABitmapBase>&)GetRaster())->GetPhysicalCoordSys());

        // set the clip region on the destination surface only
        // test if this is a simple rectangle covering the entire surface
        bool Clip = true;
        if(pTmpShape->IsRectangle())
            {
            HGF2DGrid Grid(pTmpShape->GetExtent());

            // if yes, do not clip
            if(Grid.GetWidth() == pi_rpSurDescriptor->GetWidth() && Grid.GetHeight() == pi_rpSurDescriptor->GetHeight())
                Clip = false;
            }
        if(Clip)
            {
            pClipRegion = new HGSRegion(pTmpShape, pTmpShape->GetCoordSys());
            }
        }
    else if (pi_pScanlines != 0)
        {
        pClipRegion = new HGSRegion(pi_pScanlines);
        }
        
    // create a surface from the descriptor and toolbox
    m_pSurface.reset(new HRASurface(pi_rpSurDescriptor.GetPtr()));
    if (pClipRegion != 0)
        m_pSurface->SetRegion(pClipRegion);

    // create an editor on this surface
    m_pEditor.reset(new HRAEditor(*m_pSurface));
    }

//-----------------------------------------------------------------------------
// public
// GetSurfaceEditor
//-----------------------------------------------------------------------------
HRAEditor* HRABitmapEditor::GetSurfaceEditor()
    {
    return m_pEditor.get();
    }

