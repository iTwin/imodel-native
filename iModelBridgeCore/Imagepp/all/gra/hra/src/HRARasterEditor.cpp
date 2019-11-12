//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Methods for class HRARasterEditor
//-----------------------------------------------------------------------------

#include <ImageppInternal.h>


#include <ImagePP/all/h/HRARasterEditor.h>

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
HRARasterEditor::HRARasterEditor(const HFCPtr<HRARaster>& pi_pRaster,
                                 const HFCAccessMode      pi_Mode)
    {
    // Take a copy of the raster pointer
    m_pRaster = pi_pRaster;
    m_Mode    = pi_Mode;
    }

//-----------------------------------------------------------------------------
// Copy constructor
//-----------------------------------------------------------------------------
HRARasterEditor::HRARasterEditor(const HRARasterEditor& pi_rObj)
    {
    // Take a copy of the raster pointer
    m_pRaster = pi_rObj.m_pRaster;
    m_Mode    = pi_rObj.m_Mode;
    }

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
HRARasterEditor::~HRARasterEditor()
    {
    // The raster's content has probably changed...
    // But don't make the notification here, because some types of rasters
    // like HIMReferenceToRaster and HIMMosaic will already receive notification
    // from their source rasters.
    }

//-----------------------------------------------------------------------------
// Assignment operator.
//-----------------------------------------------------------------------------
HRARasterEditor& HRARasterEditor::operator=(const HRARasterEditor& pi_rObj)
    {
    if (this != &pi_rObj)
        {
        // Take a copy of the raster pointer
        m_pRaster = pi_rObj.m_pRaster;
        m_Mode    = pi_rObj.m_Mode;
        }

    return *this;
    }


/*
//-----------------------------------------------------------------------------
// Print the state of the object
//-----------------------------------------------------------------------------
void HRARasterEditor::PrintState (ostream& po_rOutput) const
{
#ifdef __HMR_PRINTSTATE

    po_rOutput

    << "HRARasterEditor"
    << endl;

#endif
}
*/
