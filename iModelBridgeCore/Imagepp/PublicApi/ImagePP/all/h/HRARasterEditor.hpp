//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRARasterEditor.hpp $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Inline methods for class HRARasterEditor
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// public
// Get the edited raster pointer
//-----------------------------------------------------------------------------
inline const HFCPtr<HRARaster>& HRARasterEditor::GetRaster () const
    {
    return m_pRaster;
    }


//-----------------------------------------------------------------------------
// public
// Get the edited raster pointer
//-----------------------------------------------------------------------------
inline const HFCAccessMode HRARasterEditor::GetLockMode () const
    {
    return m_Mode;
    }
