//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hcp/src/HCPGCoordContiguousModelAdapter.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>
#include <Imagepp/all/h/HCPGCoordContiguousModelAdapter.h>

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
HCPGCoordContiguousModelAdapter::HCPGCoordContiguousModelAdapter(const HCPGCoordModel&  pi_ExactTransfoModel,
                                                                 double pi_MinXLongDirect, double pi_MinXLongInverse)
    : HGF2DTransfoModelAdapter(pi_ExactTransfoModel)
    {
    m_MinXLongDirect = pi_MinXLongDirect - 5 /*magic number*/;
    m_MinXLongInverse = pi_MinXLongInverse - 5;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
HCPGCoordContiguousModelAdapter::HCPGCoordContiguousModelAdapter(const HCPGCoordContiguousModelAdapter& pi_rObj)
    : HGF2DTransfoModelAdapter(pi_rObj)
    {
    Copy (pi_rObj);

    HINVARIANTS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
HCPGCoordContiguousModelAdapter::~HCPGCoordContiguousModelAdapter()
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
HCPGCoordContiguousModelAdapter& HCPGCoordContiguousModelAdapter::operator=(const HCPGCoordContiguousModelAdapter& pi_rObj)
    {
    HINVARIANTS;

    // Check that object to copy is not self
    if (this != &pi_rObj)
        {
        // Call ancestor operator=
        HCPGCoordContiguousModelAdapter::operator=(pi_rObj);
        Copy (pi_rObj);
        }

    // Return reference to self
    return (*this);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
void HCPGCoordContiguousModelAdapter::Copy (const HCPGCoordContiguousModelAdapter& pi_rObj)
    {
    m_MinXLongDirect = pi_rObj.m_MinXLongDirect;
    m_MinXLongInverse = pi_rObj.m_MinXLongInverse;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
void HCPGCoordContiguousModelAdapter::Reverse ()
    {
    HGF2DTransfoModelAdapter::Reverse();
    double temp = m_MinXLongDirect;
    m_MinXLongDirect = m_MinXLongInverse;
    m_MinXLongInverse = temp;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
StatusInt HCPGCoordContiguousModelAdapter::ConvertDirect(double* pio_pXInOut, double* pio_pYInOut) const
    {
    StatusInt status = m_pAdaptedTransfoModel->ConvertDirect(pio_pXInOut, pio_pYInOut);

    HCPGCoordModel const* adaptedModel = dynamic_cast<HCPGCoordModel const*>(m_pAdaptedTransfoModel.GetPtr());
    if(adaptedModel->GetDestinationGEOCS().GetProjectionCode() == 1 && *pio_pXInOut < m_MinXLongInverse)
        {
        *pio_pXInOut += 360;
        }

    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
StatusInt HCPGCoordContiguousModelAdapter::ConvertDirect(double pi_YIn, double pi_XInStart, size_t pi_NumLoc, double pi_XInStep,
                                                         double* po_pXOut, double* po_pYOut) const
    {
    uint32_t Index;
    double X;
    StatusInt status = SUCCESS;

    for (Index = 0, X = pi_XInStart; Index < pi_NumLoc ; ++Index, X+=pi_XInStep)
        {
        StatusInt currentStatus = ConvertDirect(X, pi_YIn, po_pXOut + Index, po_pYOut + Index);
        if(currentStatus != SUCCESS)
            status = currentStatus;
        }
    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
StatusInt HCPGCoordContiguousModelAdapter::ConvertDirect(double pi_XIn, double pi_YIn, double* po_pXOut, double* po_pYOut) const
    {
    *po_pXOut = pi_XIn;
    *po_pYOut = pi_YIn;

    StatusInt status = m_pAdaptedTransfoModel->ConvertDirect(po_pXOut, po_pYOut);

    HCPGCoordModel const* adaptedModel = dynamic_cast<HCPGCoordModel const*>(m_pAdaptedTransfoModel.GetPtr());
    if(adaptedModel->GetDestinationGEOCS().GetProjectionCode() == 1 && *po_pXOut < m_MinXLongInverse)
        {
        *po_pXOut += 360;
        }

    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
StatusInt HCPGCoordContiguousModelAdapter::ConvertDirect(size_t pi_NumLoc, double* pio_aXInOut, double* pio_aYInOut) const
    {
    StatusInt status = SUCCESS;
    for(size_t i = 0; i < pi_NumLoc; ++i)
        {
        StatusInt currentStatus = ConvertDirect(pio_aXInOut + i, pio_aYInOut + i);
        if(currentStatus != SUCCESS)
            status = currentStatus;
        }
    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
StatusInt HCPGCoordContiguousModelAdapter::ConvertInverse(double* pio_pXInOut, double* pio_pYInOut) const
    {
    StatusInt status = m_pAdaptedTransfoModel->ConvertInverse(pio_pXInOut, pio_pYInOut);

    HCPGCoordModel const* adaptedModel = dynamic_cast<HCPGCoordModel const*>(m_pAdaptedTransfoModel.GetPtr());
    if(adaptedModel->GetSourceGEOCS().GetProjectionCode() == 1 && *pio_pXInOut < m_MinXLongDirect)
        {
        *pio_pXInOut += 360;
        }

    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
StatusInt HCPGCoordContiguousModelAdapter::ConvertInverse(double pi_YIn, double pi_XInStart, size_t pi_NumLoc, double pi_XInStep,
                                                          double* po_pXOut, double* po_pYOut) const
    {
    uint32_t Index;
    double X;
    StatusInt status = SUCCESS;

    for (Index = 0, X = pi_XInStart; Index < pi_NumLoc ; ++Index, X+=pi_XInStep)
        {
        StatusInt currentStatus = ConvertInverse(X, pi_YIn, po_pXOut + Index, po_pYOut + Index);
        if(currentStatus != SUCCESS)
            status = currentStatus;
        }
    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
StatusInt HCPGCoordContiguousModelAdapter::ConvertInverse(double pi_XIn, double pi_YIn, double* po_pXOut, double* po_pYOut) const
    {
    *po_pXOut = pi_XIn;
    *po_pYOut = pi_YIn;

    StatusInt status = m_pAdaptedTransfoModel->ConvertInverse(po_pXOut, po_pYOut);

    HCPGCoordModel const* adaptedModel = dynamic_cast<HCPGCoordModel const*>(m_pAdaptedTransfoModel.GetPtr());
    if(adaptedModel->GetSourceGEOCS().GetProjectionCode() == 1 && *po_pXOut < m_MinXLongDirect)
        {
        *po_pXOut += 360;
        }


    return status;
    //HCPGCoordModel const* adaptedModel = dynamic_cast<HCPGCoordModel const*>(m_pAdaptedTransfoModel.GetPtr());
    //*po_pXOut = pi_XIn;
    //*po_pYOut = pi_YIn;

    //if(adaptedModel->GetDestinationGEOCS().GetProjectionCode() == 1)
        //ToGeographicDomain(*po_pXOut);

    //m_pAdaptedTransfoModel->ConvertInverse(po_pXOut, po_pYOut);

    //if(adaptedModel->GetSourceGEOCS().GetProjectionCode() == 1)
        //ToContiguousDomain(*po_pXOut);

    //return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Alexandre.Gariepy   08/15
//+---------------+---------------+---------------+---------------+---------------+------
StatusInt HCPGCoordContiguousModelAdapter::ConvertInverse(size_t pi_NumLoc, double* pio_aXInOut, double* pio_aYInOut) const
    {
    StatusInt status = SUCCESS;
    for(size_t i = 0; i < pi_NumLoc; ++i)
        {
        StatusInt currentStatus = ConvertInverse(pio_aXInOut + i, pio_aYInOut + i);
        if(currentStatus != SUCCESS)
            status = currentStatus;
        }
    return status;
    }

////---------------------------------------------------------------------------------------
//// @bsimethod                                                 Alexandre.Gariepy   08/15
////+---------------+---------------+---------------+---------------+---------------+------
//void HCPGCoordContiguousModelAdapter::ToGeographicDomain(double& x) const
    //{
    //if(x <= -180)
        //{
        //double adjustment = ceil((-180.0 - x) / 360.0) * 360.0;
        //x += adjustment;
        //}
    //else if(x > 180)
        //{
        //double adjustment = ceil((x - 180.0) / 360.0) * 360.0;
        //x += adjustment;
        //}
    //}

////---------------------------------------------------------------------------------------
//// @bsimethod                                                 Alexandre.Gariepy   08/15
////+---------------+---------------+---------------+---------------+---------------+------
//void HCPGCoordContiguousModelAdapter::ToContiguousDomain(double& x) const
    //{
    //if(m_UsePositiveDomain && x < 0)
        //{
        //double adjustment = ceil((-1 * x) / 360.0) * 360.0;
        //x += adjustment;
        //}
    //else if(!m_UsePositiveDomain && x > 0)
        //{
        //double adjustment = ceil(x / 360.0) * 360.0;
        //x -= adjustment;
        //}
    //}

void HCPGCoordContiguousModelAdapter::GetStretchParams(double* po_pScaleFactorX, double* po_pScaleFactorY, HGF2DDisplacement* po_pDisplacement) const
    {
    HGF2DTransfoModel::GetStretchParamsAt(po_pScaleFactorX, po_pScaleFactorY, po_pDisplacement, 0.0, 0.0);
    }

HFCMatrix<3, 3> HCPGCoordContiguousModelAdapter::GetMatrix() const
    {
    HFCMatrix<3, 3> m;
    return m;
    }

HGF2DTransfoModel* HCPGCoordContiguousModelAdapter::Clone () const
    {
    return(new HCPGCoordContiguousModelAdapter(*this));
    }
