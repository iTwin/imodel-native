//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hgf/src/HGF3DTransfoModel.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HGF3DTransfoModel (inline methods)
//-----------------------------------------------------------------------------

#include "hstdcpp.h"

#include "HGF3DTransfoModel.h"

/** -----------------------------------------------------------------------------
    Default constructor
    -----------------------------------------------------------------------------
*/
HGF3DTransfoModel::HGF3DTransfoModel()
    {
    }


/** -----------------------------------------------------------------------------
    Destroyer
    -----------------------------------------------------------------------------
*/
HGF3DTransfoModel::~HGF3DTransfoModel()
    {
    }


/** -----------------------------------------------------------------------------
    Studies the reversibility of the model over a region using the given step.
    The deviation from the original value is used in the
    calculation of mean and maximum error which are returned.

    @param pi_rPrecisionArea An extent over which to perform the study. The
                             area may not be empty.

    @param pi_Step The step used in X and Y for sampling. This value must be
                   greater than 0.0


    @param po_pMeanError Pointer to double that receives the mean error.

    @param po_pMaxError  Pointer to double that receives the maximum error.


    -----------------------------------------------------------------------------
*/
void HGF3DTransfoModel::StudyReversibilityPrecisionOver(const HGF2DLiteExtent& pi_PrecisionArea,
                                                        double                pi_ZMin,
                                                        double                pi_ZMax,
                                                        double                pi_Step,
                                                        double*               po_pMeanError,
                                                        double*               po_pMaxError) const
    {
    // The extent of area must not be empty
    HPRECONDITION(pi_PrecisionArea.GetWidth() != 0.0);
    HPRECONDITION(pi_PrecisionArea.GetHeight() != 0.0);

    // The step may not be null nor negative
    HPRECONDITION(pi_Step > 0.0);

    // Recipient variables must be provided
    HPRECONDITION(po_pMeanError != NULL);
    HPRECONDITION(po_pMaxError != NULL);

    // Convert in temporary variables
    double TempX;
    double TempY;
    double TempZ;

    double MaxError = 0.0;
    double StatSumX = 0.0;
    double StatSumY = 0.0;
    double StatSumZ = 0.0;
    uint32_t StatNumSamples = 0;

    double TempX1;
    double TempY1;
    double TempZ1;

    double CurrentX;
    double CurrentY;
    double CurrentZ;

    for (CurrentZ = pi_ZMin ; CurrentZ < pi_ZMax ; CurrentZ += pi_Step)
        {

        for (CurrentY = pi_PrecisionArea.GetYMin() ;
             CurrentY < pi_PrecisionArea.GetYMax()  ;
             CurrentY += pi_Step)
            {
            bool Initialized = false;
            for (CurrentX = pi_PrecisionArea.GetXMin() ;
                 CurrentX < pi_PrecisionArea.GetXMax() ;
                 CurrentX += pi_Step)
                {
                // Convert one way
                ConvertDirect(CurrentX, CurrentY, CurrentZ, &TempX, &TempY, &TempZ);



                // Convert back
                ConvertInverse(TempX, TempY, TempZ, &TempX1, &TempY1, &TempZ1);

                // Compute difference (drift)
                double DeltaX = fabs(CurrentX - TempX1);
                double DeltaY = fabs(CurrentY - TempY1);
                double DeltaZ = fabs(CurrentZ - TempZ1);

                // Add deltas
                StatSumX += DeltaX;
                StatSumY += DeltaY;
                StatSumZ += DeltaZ;
                StatNumSamples++;

                MaxError = MAX(MaxError, MAX(DeltaX, MAX(DeltaY, DeltaZ)));
                }
            }
        }

    // Compute precision results
    *po_pMaxError = MaxError;
    *po_pMeanError = (StatNumSamples > 0 ? (StatSumX + StatSumY + StatSumZ) / (3 * StatNumSamples) : 0.0);

    }



