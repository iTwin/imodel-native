//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRADrawOptions.h $
//:>
//:>  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRADrawOptions
//-----------------------------------------------------------------------------

#pragma once

#include "HGFPreDrawOptions.h"
#include "HGFDrawOptions.h"

#include "HFCPtr.h"
#include "HVEShape.h"
#include "HRPPixelType.h"
#include "HRPFilter.h"
#include "HRACopyFromOptions.h"
#include "HRATransaction.h"


class HRADrawOptions : public HGFDrawOptions
    {
    HDECLARE_CLASS_ID(1759, HGFDrawOptions)

public:

    // Primary methods

    _HDLLg                       HRADrawOptions();

    _HDLLg                       HRADrawOptions(const HRADrawOptions& pi_rOptions);
    _HDLLg                       HRADrawOptions(const HGFDrawOptions& pi_rOptions);
    _HDLLg                       HRADrawOptions(const HGFDrawOptions* pi_pOptions);
    _HDLLg                       HRADrawOptions(const HRACopyFromOptions& pi_rCFOptions);

    _HDLLg virtual              ~HRADrawOptions();


    // Operators

    HRADrawOptions&             operator=(const HRADrawOptions& pi_rObj);
    HRADrawOptions&             operator=(const HGFDrawOptions& pi_rObj);
    HRADrawOptions&             operator=(const HGFDrawOptions* pi_pObj);

    //TR 300554 - Temporary fix the problem for STM raster draping only.
    bool                       GetDataDimensionFix() const;
    void                        SetDataDimensionFix(bool pi_fix);

    void                        SetMosaicSupersampling(bool pi_Quality);
    bool                       ApplyMosaicSupersampling() const;

    HFCPtr<HVEShape>            GetShape() const;
    void                        SetShape(const HFCPtr<HVEShape>& pi_rpShape);

    void                        SetGridShape(bool pi_GridShape);
    bool                       ApplyGridShape() const;

    HFCPtr<HGF2DCoordSys>       GetReplacingCoordSys() const;
    void                        SetReplacingCoordSys(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys);

    HFCPtr<HRPPixelType>        GetReplacingPixelType() const;
    void                        SetReplacingPixelType(const HFCPtr<HRPPixelType>& pi_rpPixelType);

    void                        SetTransaction(const HFCPtr<HRATransaction>& pi_rpTransaction);
    HFCPtr<HRATransaction>      GetTransaction() const;

protected:

private:

    HFCPtr<HVEShape>            m_pShape;
    bool                       m_ApplyGridShape;
    HFCPtr<HGF2DCoordSys>       m_pReplacingCoordSys;
    HFCPtr<HRPPixelType>        m_pReplacingPixelType;
    HFCPtr<HRATransaction>      m_pTransaction;

    bool                        m_DataDimensionFix;
    bool                        m_ApplyMosaicSupersampling;
    };

#include "HRADrawOptions.hpp"

