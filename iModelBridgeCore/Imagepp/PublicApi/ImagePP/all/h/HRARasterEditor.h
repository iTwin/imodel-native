//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRARasterEditor
//-----------------------------------------------------------------------------
// Raster editors are objects to use to access and manipulate raster data at
// the pixel level, and can be used to scan this data like using an iterator.
// This is an abstract class.
//-----------------------------------------------------------------------------

#pragma once

#include "HFCPtr.h"
#include "HRPPixelType.h"
#include "HRARaster.h"

#include "HRPFilter.h"

BEGIN_IMAGEPP_NAMESPACE
class HRARaster;
class HGF2DLocation;
class HVEShape;
class HRABitmap;

class HRARasterEditor
    {
public:

    // Class ID for this class.
    HDECLARE_BASECLASS_ID(HRARasterEditorId_Base)


    // Primary methods

    HRARasterEditor         (const HFCPtr<HRARaster>& pi_pRaster,
                             const HFCAccessMode        pi_Mode=HFC_READ_ONLY);
    virtual         ~HRARasterEditor        ();

    // Editor control

    virtual const HFCPtr<HRARaster>&
    GetRaster           () const;
    virtual const HFCAccessMode
    GetLockMode         () const;

protected:

private:

    // Disable methods
    HRARasterEditor    (const HRARasterEditor& pi_rObj);
    HRARasterEditor& operator=(const HRARasterEditor& pi_rObj);


    // The edited raster object
    HFCPtr<HRARaster>
    m_pRaster;

    // Lock mode
    HFCAccessMode   m_Mode;

    };
END_IMAGEPP_NAMESPACE

#include "HRARasterEditor.hpp"

