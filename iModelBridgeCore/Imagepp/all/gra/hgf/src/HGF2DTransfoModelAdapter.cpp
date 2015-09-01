//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hgf/src/HGF2DTransfoModelAdapter.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Methods for class HGF2DTransfoModelAdapter
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HGF2DIdentity.h>

#include <Imagepp/all/h/HGF2DTransfoModelAdapter.h>


//-----------------------------------------------------------------------------
// Default Constructor
//-----------------------------------------------------------------------------
HGF2DTransfoModelAdapter::HGF2DTransfoModelAdapter()
    : HGF2DTransfoModel()
    {
    m_pAdaptedTransfoModel = new HGF2DIdentity();

    HINVARIANTS;
    }


/** -----------------------------------------------------------------------------
    Creates a transformation model adapter model based on the given model.
    Since this is an abstract class, the present constructor cannot be called
    directly but only by derived classes. A copy of the given model
    is performed and kept internally in the transformation model adapter.

    @param pi_rAdaptedTransfoModel Constant reference to a transformation model
                                   to be adapted. A copy of this model is done.

    -----------------------------------------------------------------------------
*/
HGF2DTransfoModelAdapter::HGF2DTransfoModelAdapter(const HGF2DTransfoModel& pi_rAdaptedTransfoModel)
    : HGF2DTransfoModel(pi_rAdaptedTransfoModel)
    {
    m_pAdaptedTransfoModel = pi_rAdaptedTransfoModel.Clone();

    HINVARIANTS;
    }



/** -----------------------------------------------------------------------------
    Copy constructor for an adapter transformation model
    A copy of the adapted model of given model
    is performed and kept internally in the new transformation model adapter.

    @param pi_rObj A transformation model adapter to copy construct from.
    -----------------------------------------------------------------------------
*/
HGF2DTransfoModelAdapter::HGF2DTransfoModelAdapter(const HGF2DTransfoModelAdapter& pi_rObj)
    : HGF2DTransfoModel(pi_rObj)
    {
    // Copy model
    m_pAdaptedTransfoModel = pi_rObj.m_pAdaptedTransfoModel->Clone();

    HINVARIANTS;
    }

//-----------------------------------------------------------------------------
// The destructor.
//-----------------------------------------------------------------------------
HGF2DTransfoModelAdapter::~HGF2DTransfoModelAdapter()
    {
    }

/** -----------------------------------------------------------------------------
    Assignment operator. This operation sets the present object to the
    state of given transformation model adapter. A copy of the adapted model
    is performed.

    @param pi_rObj The transformation model adapter to copy state of.

    @return Returns a reference to self to be used as an l-value.
    -----------------------------------------------------------------------------
*/
HGF2DTransfoModelAdapter& HGF2DTransfoModelAdapter::operator=(const HGF2DTransfoModelAdapter& pi_rObj)
    {
    HINVARIANTS;

    // Check that object to copy is not self
    if (this != &pi_rObj)
        {
        // Call ancestor operator=
        HGF2DTransfoModel::operator=(pi_rObj);
        m_pAdaptedTransfoModel = pi_rObj.m_pAdaptedTransfoModel->Clone();
        }

    // Return reference to self
    return (*this);
    }



void    HGF2DTransfoModelAdapter::Reverse()
    {
    HINVARIANTS;

    // Reverse non-linear model
    m_pAdaptedTransfoModel->Reverse();

    // Invoque reversing of ancester
    // This call will in turn invoque Prepare()
    HGF2DTransfoModel::Reverse();
    }



/** -----------------------------------------------------------------------------
    This method allows studying of the precision in the direct direction of
    transformation over a specified area using indicated step. The method enables
    to understand the impact of the TransfoModel adapter approximation of a
    model. The method generates a grid entirely included in the given area
    and samples transformation differences between adapted model and adapter
    model. The mean and maximum error obtained are returned.

    @param pi_rPrecisionArea Constant reference to extent to perform study over.

    @param pi_Step The step of the grid used in both X and Y directions.

    @param po_pMeanError Pointer to double that receives the mean error evaluated.

    @param po_pMaxError Pointer to double that receives the maximum error evaluated.
    -----------------------------------------------------------------------------
*/
void HGF2DTransfoModelAdapter::StudyPrecisionOver(const HGF2DLiteExtent& pi_PrecisionArea,
                                                  double                pi_Step,
                                                  double*               po_pMeanError,
                                                  double*               po_pMaxError) const
    {
    HINVARIANTS;
    StudyPrecision(*this, *m_pAdaptedTransfoModel, pi_PrecisionArea, pi_Step, pi_Step, po_pMeanError, po_pMaxError);
    }

//-----------------------------------------------------------------------------
//  Prepare
//  This methods prepares the conversion parameters from the basic
//  model attribute
//-----------------------------------------------------------------------------
void HGF2DTransfoModelAdapter::Prepare ()
    {
    // Obtain conversion ratio for direct X to inverse X units

    }

