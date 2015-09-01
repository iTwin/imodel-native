//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGF2DTransfoModelAdapter.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

BEGIN_IMAGEPP_NAMESPACE
/** -----------------------------------------------------------------------------
    Returns a constant reference to the internal copy of the adapted model.

    @return a constant reference to the adapted model.
    -----------------------------------------------------------------------------
*/
inline const HFCPtr<HGF2DTransfoModel>& HGF2DTransfoModelAdapter::GetAdaptedTransfoModel() const
    {
    return(m_pAdaptedTransfoModel);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  1/2005
+---------------+---------------+---------------+---------------+---------------+------*/
template<class T, class U> void StudyPrecision
(
    const T&                pi_firstModel,
    const U&                pi_secondModel,
    const HGF2DLiteExtent&  pi_PrecisionArea,
    double                  pi_StepX,
    double                  pi_StepY,
    double*                 po_pMeanError,
    double*                 po_pMaxError
)
    {
    // The extent of area must not be empty
    HPRECONDITION(pi_PrecisionArea.GetWidth() != 0.0);
    HPRECONDITION(pi_PrecisionArea.GetHeight() != 0.0);

    // The step may not be null nor negative
    HPRECONDITION(pi_StepX > 0.0);
    HPRECONDITION(pi_StepY > 0.0);

    // Recipient variables must be provided
    HPRECONDITION(po_pMeanError != 0);
    HPRECONDITION(po_pMaxError != 0);

    // Convert in temporary variables
    double TempX;
    double TempY;

    double MaxDirectError = 0.0;
    double DirectStatSumX = 0.0;
    double DirectStatSumY = 0.0;
    uint32_t DirectStatNumSamples = 0;

    double TempX1;
    double TempY1;

    double CurrentX;
    double CurrentY;

    for (CurrentY = pi_PrecisionArea.GetYMin() ; CurrentY < pi_PrecisionArea.GetYMax() ; CurrentY += pi_StepY)
        {
        for (CurrentX = pi_PrecisionArea.GetXMin() ; CurrentX < pi_PrecisionArea.GetXMax() ; CurrentX += pi_StepX)
            {
            try
                {
                //Ignore point that cannot be converted, we don't want to compute model precision using point outside valid domain
                if ((SUCCESS == pi_firstModel.ConvertDirect(CurrentX, CurrentY, &TempX, &TempY)) && 
                    (SUCCESS == pi_secondModel.ConvertDirect(CurrentX, CurrentY, &TempX1, &TempY1)))
                    {
                    // Compute difference
                    double DeltaX = fabs(TempX - TempX1);
                    double DeltaY = fabs(TempY - TempY1);

                    // Add deltas
                    DirectStatSumX += DeltaX;
                    DirectStatSumY += DeltaY;
                    DirectStatNumSamples++;

                    MaxDirectError = MAX(MaxDirectError, MAX(DeltaX, DeltaY));
                    }
                }
            catch (...)
                {
                //Ignore point that cannot be converted, we don't want to compute model precision using point outside valid domain
                }
            }
        }

    // Compute precision results
    *po_pMaxError = MaxDirectError;
    *po_pMeanError = (DirectStatNumSamples > 0 ? (DirectStatSumX + DirectStatSumY) / (2 * DirectStatNumSamples) : 0.0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  06/2008
+---------------+---------------+---------------+---------------+---------------+------*/
template<class T, class U> void StudyPrecision
(
    const T& pi_firstModel,
    const U& pi_secondModel,
    HGF2DTransfoModelAdapter::ListOfPoints const& pi_points,
    double& po_pMeanError,
    double& po_pMaxError
)
    {
    double MaxDirectError = 0.0;
    double DirectStatSumX = 0.0;
    double DirectStatSumY = 0.0;
    uint32_t DirectStatNumSamples = 0;

    for (HGF2DTransfoModelAdapter::ListOfPoints::const_iterator itr(pi_points.begin()); itr != pi_points.end(); ++itr)
        {
        double TempX;
        double TempY;
        pi_firstModel.ConvertDirect(itr->first, itr->second, &TempX, &TempY);

        // Apply non-linear transformation
        double TempX1;
        double TempY1;
        pi_secondModel.ConvertDirect(itr->first, itr->second, &TempX1, &TempY1);

        // Compute difference
        double DeltaX = fabs(TempX - TempX1);
        double DeltaY = fabs(TempY - TempY1);

        // Add deltas
        DirectStatSumX += DeltaX;
        DirectStatSumY += DeltaY;
        DirectStatNumSamples++;

        MaxDirectError = MAX(MaxDirectError, MAX(DeltaX, DeltaY));
        }

    // Compute precision results
    po_pMaxError = MaxDirectError;
    po_pMeanError = (DirectStatNumSamples > 0 ? (DirectStatSumX + DirectStatSumY) / (2 * DirectStatNumSamples) : 0.0);
    }


END_IMAGEPP_NAMESPACE