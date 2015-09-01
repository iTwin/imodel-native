//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRAReferenceToRasterEditor.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRAReferenceToRasterEditor
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

#pragma once

#include "HFCPtr.h"
#include "HGF2DLocation.h"
#include "HRARasterEditor.h"
#include "HRARaster.h"
#include "HRAReferenceToRaster.h"

BEGIN_IMAGEPP_NAMESPACE
class HRARaster;
class HVEShape;

class HRAReferenceToRasterEditor : public HRARasterEditor
    {
public:

    // Class ID for this class.
    HDECLARE_CLASS_ID(HRAReferenceToRasterEditorId, HRARasterEditor)

    // Type used in type Error to specify error type
    /*
            typedef struct
            {
                enum
                {
                    INVALID_ROLE,
                    INVALID_INDEX,
                    INVALID_POS,
                    NO_ALPHA
                } m_ErrorType;
            } Error;
    */
    // Primary methods

    HRAReferenceToRasterEditor (const HFCPtr<HRAReferenceToRaster>& pi_pReference,
                                const HFCAccessMode                 pi_Mode);
    HRAReferenceToRasterEditor (const HFCPtr<HRAReferenceToRaster>& pi_pReference,
                                const HVEShape&                     pi_rRegion,
                                const HFCAccessMode                 pi_Mode);
    virtual         ~HRAReferenceToRasterEditor();
    /*
          // Debug function
    #ifdef __HMR_PRINTSTATE
          virtual void    PrintState(ostream& po_rOutput) const;
    #endif
    */
protected:


private:

    // Use a raster editor to work on the source internally
    HRARasterEditor*    m_pSourceEditor;

    // Computation locations (in GetLocation, Goto, etc...)
    HGF2DLocation        m_SourceLocation;
    HGF2DLocation        m_TempLocation;

    // Reference's coordinate system. Cached here for speed...
    HFCPtr<HGF2DCoordSys>
    m_RefCoordSys;

    // Methods

    // Disable methods
    HRAReferenceToRasterEditor (const HRAReferenceToRasterEditor& pi_rObj);
    HRAReferenceToRasterEditor&
    operator=(const HRAReferenceToRasterEditor& pi_rObj);

    };
END_IMAGEPP_NAMESPACE



