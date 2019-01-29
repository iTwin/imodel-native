//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRARasterEditor.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Inline methods for class HRARasterEditor
//-----------------------------------------------------------------------------


BEGIN_IMAGEPP_NAMESPACE
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
END_IMAGEPP_NAMESPACE
