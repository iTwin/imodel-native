//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGF2DTransfoModelAdapter.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HGF2DTransfoModelAdapter
//-----------------------------------------------------------------------------
// Description of 2D TransfoModel transformation model.
//-----------------------------------------------------------------------------

#pragma once

#include "HGF2DTransfoModel.h"
#include "HGF2DLiteExtent.h"

BEGIN_IMAGEPP_NAMESPACE
/** -----------------------------------------------------------------------------
    @version 1.0
    @author Alain Robert 

    The purpose of the transfo model adapter is to envelop a non-linear, imperfectly
    bi-directional or slow transformation model under an approximation other
    transformation model either linear, efficient or bi-directionnal
    for performance or precision reason.

    The present abstract class defines only the statistics collection
    and study of precision.
    Since the adaptation is an approximation of the transformation model,
    an error may be introduced by adaptation using such a TransfoModel
    adapter. The error can be studied using the StudyPrecisionOver() method.
    Study of precision can be performed on any area.

    -----------------------------------------------------------------------------
*/
class HGF2DTransfoModelAdapter : public HGF2DTransfoModel
    {
    HDECLARE_CLASS_ID(HGF2DTransfoModelId_Adapter, HGF2DTransfoModel)

public:
    typedef  list <pair<double, double> > ListOfPoints;

    // Primary methods
    HGF2DTransfoModelAdapter();
    virtual         ~HGF2DTransfoModelAdapter();

    const HFCPtr<HGF2DTransfoModel>&
    GetAdaptedTransfoModel() const;

    IMAGEPP_EXPORT void     StudyPrecisionOver(const HGF2DLiteExtent& pi_PrecisionArea,
                                       double                pi_Step,
                                       double*               po_pMeanError,
                                       double*               po_pMaxError) const;

    void            Reverse();


protected:

    HGF2DTransfoModelAdapter(const HGF2DTransfoModel& pi_rAdaptedTransfoModel);
    HGF2DTransfoModelAdapter(const HGF2DTransfoModelAdapter& pi_rObj);
    HGF2DTransfoModelAdapter&    operator=(const HGF2DTransfoModelAdapter& pi_rObj);

    HFCPtr<HGF2DTransfoModel> m_pAdaptedTransfoModel;

    virtual void    Prepare ();

#ifdef HVERIFYCONTRACT
    void               ValidateInvariants() const
        {
        }
#endif

private:

    };


template <class T, class U> void StudyPrecision
(
    const T& pi_firstModel,
    const U& pi_secondModel,
    const HGF2DLiteExtent&   pi_PrecisionArea,
    double                  pi_StepX,
    double                  pi_StepY,
    double*                 po_pMeanError,
    double*                 po_pMaxError
);

template<class T, class U> void StudyPrecision
(
    const T& pi_firstModel,
    const U& pi_secondModel,
    HGF2DTransfoModelAdapter::ListOfPoints const& pi_points,
    double& po_pMeanError,
    double& po_pMaxError
);

END_IMAGEPP_NAMESPACE
#include "HGF2DTransfoModelAdapter.hpp"

