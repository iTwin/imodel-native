//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRADrawOptions.hpp $
//:>
//:>  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// inline methods for class : HRADrawOptions
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// public
// GetShape
//-----------------------------------------------------------------------------
inline HFCPtr<HVEShape> HRADrawOptions::GetShape() const
    {
    return m_pShape;
    }

//-----------------------------------------------------------------------------
// public
// SetShape
//-----------------------------------------------------------------------------
inline void HRADrawOptions::SetShape(const HFCPtr<HVEShape>& pi_rpShape)
    {
    m_pShape = pi_rpShape;
    }

//-----------------------------------------------------------------------------
// public
// SetGridShape
//-----------------------------------------------------------------------------
inline void HRADrawOptions::SetGridShape(bool pi_GridShape)
    {
    m_ApplyGridShape = pi_GridShape;
    }

//-----------------------------------------------------------------------------
// public
// ApplyGridShape
//-----------------------------------------------------------------------------
inline bool HRADrawOptions::ApplyGridShape() const
    {
    return m_ApplyGridShape;
    }

//-----------------------------------------------------------------------------
// public
// GetReplacingCoordSys
//-----------------------------------------------------------------------------
inline HFCPtr<HGF2DCoordSys> HRADrawOptions::GetReplacingCoordSys() const
    {
    return m_pReplacingCoordSys;
    }

//-----------------------------------------------------------------------------
// public
// SetReplacingCoordSys
//-----------------------------------------------------------------------------
inline void HRADrawOptions::SetReplacingCoordSys(const HFCPtr<HGF2DCoordSys>& pi_rpReplacingCoordSys)
    {
    m_pReplacingCoordSys = pi_rpReplacingCoordSys;
    }

//-----------------------------------------------------------------------------
// public
// GetReplacingPixelType
//-----------------------------------------------------------------------------
inline HFCPtr<HRPPixelType> HRADrawOptions::GetReplacingPixelType() const
    {
    return m_pReplacingPixelType;
    }

//-----------------------------------------------------------------------------
// public
// SetReplacingPixelType
//-----------------------------------------------------------------------------
inline void HRADrawOptions::SetReplacingPixelType(const HFCPtr<HRPPixelType>& pi_rpReplacingPixelType)
    {
    m_pReplacingPixelType = pi_rpReplacingPixelType;
    }

//-----------------------------------------------------------------------------
// public
// ApplyMosaicSupersampling
//-----------------------------------------------------------------------------
inline bool HRADrawOptions::ApplyMosaicSupersampling() const
    {
    return m_ApplyMosaicSupersampling;
    }


//-----------------------------------------------------------------------------
// public
// SetMosaicSupersampling
//-----------------------------------------------------------------------------
inline void HRADrawOptions::SetMosaicSupersampling(bool pi_Quality)
    {
    m_ApplyMosaicSupersampling = pi_Quality;
    }

//-----------------------------------------------------------------------------
// public
// SetTransaction
//-----------------------------------------------------------------------------
inline void HRADrawOptions::SetTransaction(const HFCPtr<HRATransaction>& pi_rpTransaction)
    {
    m_pTransaction = pi_rpTransaction;
    }

//-----------------------------------------------------------------------------
// public
// GetTransaction
//-----------------------------------------------------------------------------
inline HFCPtr<HRATransaction> HRADrawOptions::GetTransaction() const
    {
    return m_pTransaction;
    }
