//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRARasterEditor.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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
    /*
            virtual void*       GotoNext            () = 0;
            virtual void*       GotoStart           () = 0;


            virtual const HGF2DLocation&
                                GetLocation         () const = 0;
            virtual bool       Goto                (const HGF2DLocation& pi_rLoc) = 0;
            virtual bool       GotoNext            () = 0;
            virtual bool       GotoStart           () = 0;
            virtual const HVEShape&
                                GetRegion           () const = 0;
    */
    /*
          // Generic Pixel data access

          virtual HFCPtr<HRPPixelType>
                              GetPixelType        () const = 0;

          virtual const void* GetChannelValue     (size_t pi_Index) const = 0;
          virtual const void* GetCompositeValue   () const = 0;
          virtual const void* GetCompositeValueAt (const HGF2DLocation& pi_rLoc) = 0;
          virtual size_t      GetIndexValue       () const = 0;
          virtual size_t      GetIndexValueAt     (const HGF2DLocation& pi_rLoc,
                                                   bool* po_pValidLocation) = 0;

          virtual const void* GetRawData          () const = 0;
          virtual const void* GetRawDataAt        (const HGF2DLocation& pi_rLoc) = 0;
          virtual const void* GetRunOfRawData     (bool pi_EditMode = false) const = 0;

          virtual void        ResetChannelValue   (size_t pi_ChIndex) = 0;
          virtual void        SetChannelValue     (size_t pi_chIndex, const void* pi_pData) = 0;
          virtual void        SetCompositeValue   (const void* pi_pData) = 0;
          virtual bool       SetCompositeValueAt (const void* pi_pRawData,
                                                   const HGF2DLocation& pi_rPos) = 0;
          virtual void        SetIndexValue       (size_t pi_NewIndex) = 0;
          virtual bool       SetIndexValueAt     (size_t pi_NewIndex,
                                                   const HGF2DLocation& pi_rPos) = 0;
          virtual void        SetRawData          (const void* pi_pRawData) = 0;
          virtual bool       SetRawDataAt        (const void* pi_pRawData,
                                                   const HGF2DLocation& pi_rPos) = 0;

          // RGB Pixel data access

          virtual const void*  GetRGB() const = 0;
          virtual void         SetRGB(const void* pi_pData) = 0;
          virtual void         SetRGB(const HRARgb& pi_rRgb) = 0;

          // Alpha Pixel data access

          virtual const void* GetAlpha() const = 0;
          virtual void        SetAlpha(const void* pi_pData) = 0;
          virtual void        SetAlpha(double pi_Alpha) = 0;

          // Debug function
          virtual void    PrintState(ostream& po_rOutput) const;
    */
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

