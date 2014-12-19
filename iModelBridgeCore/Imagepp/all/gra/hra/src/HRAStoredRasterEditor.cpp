//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hra/src/HRAStoredRasterEditor.cpp $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Methods for class HRAStoredRasterEditor
//-----------------------------------------------------------------------------

#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>

#include <Imagepp/all/h/HRAStoredRaster.h>
#include <Imagepp/all/h/HRAStoredRasterEditor.h>

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
HRAStoredRasterEditor::HRAStoredRasterEditor(const HFCPtr<HRAStoredRaster>& pi_pRaster,
                                             const HFCAccessMode            pi_Mode)

    : HRARasterEditor ( (HFCPtr<HRARaster>&) pi_pRaster, pi_Mode)
    {
    // Nothing to do
    }


//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
HRAStoredRasterEditor::~HRAStoredRasterEditor()
    {
    ////////////////////////////////////////////////////////////////////
    //
    // The following code has been commented out because the real
    // editor (HRABitmapEditor) does this job when calling the
    // UpdateFromSurfaceDescriptor method. This must be done in
    // future editors if they are not a child of HRABitmapEditor.
    //
    ////////////////////////////////////////////////////////////////////

    // If the Raster is possible modify, send message ContentChange and
    // set object dirty.
//    if (GetLockMode() != HFC_READ_ONLY)
//    {
//        HFCPtr<HRAStoredRaster>& pRaster = (HFCPtr<HRAStoredRaster>&) GetRaster();

    // invalidate the representative palette cache
//        pRaster->InvalidateRepPalCache();

    // Set object modified
//        pRaster->SetModificationState ();

    // Raster's content has probably changed.
    // Notify it, so it can notify its registered rasters.
//        pRaster->Propagate(HRAContentChangedMsg(*pRaster->GetEffectiveShape()));
//    }
    }
/*
//-----------------------------------------------------------------------------
// Print the state of the object
//-----------------------------------------------------------------------------
void HRAStoredRasterEditor::PrintState (ostream& po_rOutput) const
{
#ifdef __HMR_PRINTSTATE

    // Call the parent
    HRARasterEditor::PrintState (po_rOutput);

    po_rOutput

    << "HRAStoredRasterEditor"
    << endl;

#endif
}
*/