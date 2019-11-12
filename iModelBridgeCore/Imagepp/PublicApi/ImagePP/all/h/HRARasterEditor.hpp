//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
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
