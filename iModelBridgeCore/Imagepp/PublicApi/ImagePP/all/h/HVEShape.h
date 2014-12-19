//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HVEShape.h $
//:>
//:>  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HVEShape
//-----------------------------------------------------------------------------
// Description of a shape in two-dimensional coordinate system.
//-----------------------------------------------------------------------------
#pragma once

#include <Imagepp/all/h/HGFGraphicObject.h>
#include "HGF2DExtent.h"
#include "HVE2DShape.h"
#include "HGF2DShape.h"

class HGF2DCoordSys;
class HVEShapeRasterLine;
class HGF2DTransfoModel;

// This section registers some types for HPM persistence

#ifdef __HMR_DEBUG_MEMBER
//#define HVESHAPE_DEBUG_CODE 1
#endif

#ifdef HVESHAPE_DEBUG_CODE
#define HVESHAPE_SYNCH_DEBUG_CODE debug_SynchExtent();

#else
#define HVESHAPE_SYNCH_DEBUG_CODE
#endif


// Class declaration

class HVEShape : public HGFGraphicObject
    {
    HPM_DECLARE_CLASS_DLL(_HDLLg,  1009)

public:

    // Primary methods
    _HDLLg                  HVEShape();
    _HDLLg                  HVEShape(const HFCPtr<HGF2DCoordSys>& pi_pCoordSys);
    _HDLLg                  HVEShape(const HGF2DShape& pi_rFence,
                                     const HFCPtr<HGF2DCoordSys>& pi_pCoordSys);
    _HDLLg                  HVEShape(const HVEShape& pi_rObj);
    _HDLLg                  HVEShape(const HGF2DExtent& pi_rExtent);
    _HDLLg                  HVEShape(double pi_x1, double pi_y1,
                                     double pi_x2, double pi_y2,
                                     const HFCPtr<HGF2DCoordSys>& pi_pCoordSys);
    _HDLLg                 HVEShape(const HVE2DShape& pi_RawShape);
    _HDLLg                 HVEShape(size_t*                      po_pBufferLength,
                                    double*                     pi_pBuffer,
                                    const HFCPtr<HGF2DCoordSys>& pi_pCoordSys);

    _HDLLg virtual         ~HVEShape();
    HVEShape&      operator=(const HVEShape& pi_rObj);
    bool              operator==(const HVEShape& pi_rObj) const;


    // Graphic object methods

    _HDLLg virtual void        Move(const HGF2DDisplacement& pi_rDisplacement);
    _HDLLg virtual void        Rotate(double               pi_Angle,
                                      const HGF2DLocation& pi_rOrigin);
    _HDLLg virtual void        Scale(double pi_ScaleFactor,
                                     const HGF2DLocation& pi_rOrigin);
    _HDLLg virtual void        Scale(double pi_ScaleFactorX,
                                     double pi_ScaleFactorY,
                                     const HGF2DLocation& pi_rOrigin);

    _HDLLg virtual void       TransformDirect(HGF2DTransfoModel const& pi_rTransfo);
    _HDLLg virtual void       TransformInverse(HGF2DTransfoModel const& pi_rTransfo);


    _HDLLg virtual HGFGraphicObject::Location
    Locate(const HGF2DLocation& pi_rPoint) const;

    _HDLLg virtual HGF2DShape*    GetLightShape() const;


    // Unit management

    void            ChangeCoordSys(const HFCPtr<HGF2DCoordSys>& pi_pCoordSys);

    // Shape methods
         _HDLLg void     Intersect (const HVEShape& pi_rObj);
    void            Differentiate (const HVEShape& pi_rObj);
         _HDLLg void     Unify (const HVEShape& pi_rObj);

    bool           IsEmpty     () const;
    _HDLLg bool           IsRectangle () const;

    bool           IsPointIn (const HGF2DLocation& pi_rPoint) const;

    bool           IsPointOn (const HGF2DLocation& pi_rPoint) const;

    void            MakeEmpty();

    HGF2DExtent     GetExtent() const;

    _HDLLg virtual void    PrintState (ostream& po_rOutput) const;


    const HVE2DShape*    GetShapePtr() const;

    void                   GenerateScanLines(HGFScanLines& pio_rScanLines) const;

    bool                   Matches(const HVEShape& pi_rObj) const;

    _HDLLg virtual void    SetStrokeTolerance(HFCPtr<HGFTolerance> & pi_Tolerance);
    _HDLLg virtual HFCPtr<HGFTolerance>
    GetStrokeTolerance() const;

#ifdef HVESHAPE_DEBUG_CODE
    // NOTE: m_debug_shape and m_debug_extent are not valid anymore after a SetCoordSys() because the call is implemented
    //       by HGFGraphicObject and we can add specific code for HVEShape.
    //       The call is virtual and can be overwritten.
    _HDLLg virtual void        SetCoordSys (const HFCPtr<HGF2DCoordSys>& pi_pCoordSys) {
        HGFGraphicObject::SetCoordSys(pi_pCoordSys);
        HVESHAPE_SYNCH_DEBUG_CODE
        };

    static  HFCPtr<HGF2DCoordSys> s_debug_refCoordSys;
    mutable HAutoPtr<HVE2DShape>  m_debug_shape;
    mutable HGF2DExtent           m_debug_extent;
    void  debug_SynchExtent() const;
#endif

protected:

    _HDLLg virtual void    SetCoordSysImplementation(const HFCPtr<HGF2DCoordSys>& pi_pCoordSys);

private:

    // Members

    // The internal shape...
    HVE2DShape*      m_pShape;
    };

#include "HVEShape.hpp"





