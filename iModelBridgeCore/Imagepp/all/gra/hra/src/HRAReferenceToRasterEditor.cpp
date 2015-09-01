//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hra/src/HRAReferenceToRasterEditor.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Methods for class HRAReferenceToRasterEditor
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>
#include <Imagepp/all/h/HRAReferenceToRasterEditor.h>

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
HRAReferenceToRasterEditor::HRAReferenceToRasterEditor(const HFCPtr<HRAReferenceToRaster>& pi_pReference,
                                                       const HFCAccessMode                 pi_Mode)

    : HRARasterEditor( (HFCPtr<HRARaster>&) pi_pReference, pi_Mode),
      m_SourceLocation(pi_pReference->GetCoordSys()),
      m_TempLocation(pi_pReference->GetCoordSys()),
      m_RefCoordSys(pi_pReference->GetCoordSys())
    {
    // Set source location's coordinate system correctly
    m_SourceLocation.SetCoordSys(pi_pReference->GetSource()->GetCoordSys());

    m_pSourceEditor = pi_pReference->GetSource()->CreateEditor(pi_Mode);
    }


//-----------------------------------------------------------------------------
// Constructor with a region
//-----------------------------------------------------------------------------
HRAReferenceToRasterEditor::HRAReferenceToRasterEditor(
    const HFCPtr<HRAReferenceToRaster>& pi_pReference,
    const HVEShape&                     pi_rRegion,
    const HFCAccessMode                 pi_Mode)
    : HRARasterEditor( (HFCPtr<HRARaster>&) pi_pReference, pi_Mode),
      m_SourceLocation(pi_pReference->GetCoordSys()),
      m_TempLocation(pi_pReference->GetCoordSys()),
      m_RefCoordSys(pi_pReference->GetCoordSys())
    {
    // Set source location's coordinate system correctly
    m_SourceLocation.SetCoordSys(pi_pReference->GetSource()->GetCoordSys());

    m_pSourceEditor = pi_pReference->GetSource()->CreateEditor(pi_rRegion, pi_Mode);
    }



//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
HRAReferenceToRasterEditor::~HRAReferenceToRasterEditor()
    {
    // Delete internal raster editor
    delete m_pSourceEditor;
    }



/*
//-----------------------------------------------------------------------------
// Print the object's state
//-----------------------------------------------------------------------------
#ifdef __HMR_PRINTSTATE
void HRAReferenceToRasterEditor::PrintState(ostream& po_rOutput) const
{
    // Call ancestor
    HRARasterEditor::PrintState(po_rOutput);

    po_rOutput
        << "HRAReferenceToRasterEditor"
        << endl;
}
#endif
*/