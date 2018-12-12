//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGF2DLocalProjectiveGrid.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#include "HGF2DComplexTransfoModel.h"

BEGIN_IMAGEPP_NAMESPACE
//-----------------------------------------------------------------------------
// GetDirectGridWidth
//-----------------------------------------------------------------------------
inline double HGF2DLocalProjectiveGrid::GetDirectGridWidth() const
    {
    return m_directExtent.GetWidth() / m_nColumns;
    }

//-----------------------------------------------------------------------------
// GetDirectGridHeight
//-----------------------------------------------------------------------------
inline double HGF2DLocalProjectiveGrid::GetDirectGridHeight() const
    {
    return m_directExtent.GetHeight() / m_nRows;
    }

//-----------------------------------------------------------------------------
// GetNumberOfRow
//-----------------------------------------------------------------------------
inline uint32_t HGF2DLocalProjectiveGrid::GetNumberOfRow() const
    {
    return m_nRows;
    }

//-----------------------------------------------------------------------------
// GetNumberOfColumn
//-----------------------------------------------------------------------------
inline uint32_t HGF2DLocalProjectiveGrid::GetNumberOfColumn() const
    {
    return m_nColumns;
    }

//-----------------------------------------------------------------------------
// GetGlobalAffine
//-----------------------------------------------------------------------------
inline const HFCPtr<HGF2DTransfoModel> HGF2DLocalProjectiveGrid::GetGlobalAffine() const
    {
    if (m_pGlobalAffine == NULL)
        m_pGlobalAffine = ComputeGlobalAffineModel();

    return m_pGlobalAffine;
    }

//-----------------------------------------------------------------------------
// GetComposedGlobalAffine
//-----------------------------------------------------------------------------
inline HFCPtr<HGF2DTransfoModel> HGF2DLocalProjectiveGrid::GetComposedGlobalAffine() const
    {
    if (m_pComposedAffine == NULL)
        m_pComposedAffine = m_pPreTransfoModel->ComposeInverseWithDirectOf(*GetGlobalAffine())->ComposeInverseWithDirectOf(*m_pPostTransfoModel);

    HASSERT (m_pComposedAffine != NULL);

    return m_pComposedAffine;
    }

//-----------------------------------------------------------------------------
// GetComposedAdaptedModel
//-----------------------------------------------------------------------------
inline HFCPtr<HGF2DTransfoModel> HGF2DLocalProjectiveGrid::GetComposedAdaptedModel() const
    {
    if (m_pComposedAdaptedModel == NULL)
        m_pComposedAdaptedModel = m_pPreTransfoModel->ComposeInverseWithDirectOf(*m_pAdaptedTransfoModel)->ComposeInverseWithDirectOf(*m_pPreTransfoModel);

    HASSERT (m_pComposedAdaptedModel != NULL);

    return m_pComposedAdaptedModel;
    }

//-----------------------------------------------------------------------------
// GetExtent
//-----------------------------------------------------------------------------
inline const HGF2DLiteExtent& HGF2DLocalProjectiveGrid::GetExtent() const
    {
    return m_directExtent;
    }

//-----------------------------------------------------------------------------
// GetUseApproximation
//-----------------------------------------------------------------------------
inline bool HGF2DLocalProjectiveGrid::GetUseApproximation() const
    {
    return m_useGlobalAffineApproximation;
    }

//-----------------------------------------------------------------------------
// SetUseApproximation
//-----------------------------------------------------------------------------
inline void  HGF2DLocalProjectiveGrid::SetUseApproximation(bool pi_Value) const
    {
    m_useGlobalAffineApproximation = pi_Value;
    }
END_IMAGEPP_NAMESPACE