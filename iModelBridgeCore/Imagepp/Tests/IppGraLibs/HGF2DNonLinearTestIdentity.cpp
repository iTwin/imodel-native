//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Tests/IppGraLibs/HGF2DNonLinearTestIdentity.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HGF2DNonLinearTestIdentity
//-----------------------------------------------------------------------------


#include "../imagepptestpch.h"
#include "HGF2DNonLinearTestIdentity.h"

//-----------------------------------------------------------------------------
// Default Constructor
//-----------------------------------------------------------------------------
HGF2DNonLinearTestIdentity::HGF2DNonLinearTestIdentity() 
    {
    Prepare();
    }

//-----------------------------------------------------------------------------
// Copy constructor.
//-----------------------------------------------------------------------------
HGF2DNonLinearTestIdentity::HGF2DNonLinearTestIdentity(const HGF2DNonLinearTestIdentity& pi_rObj)
    : HGF2DTransfoModel (pi_rObj)
    {
    Prepare();
    }

//-----------------------------------------------------------------------------
// The destructor.
//-----------------------------------------------------------------------------
HGF2DNonLinearTestIdentity::~HGF2DNonLinearTestIdentity()
    {
    }

//-----------------------------------------------------------------------------
// operator=
// Assignment operator.
//-----------------------------------------------------------------------------
HGF2DNonLinearTestIdentity& HGF2DNonLinearTestIdentity::operator=(const HGF2DNonLinearTestIdentity& pi_rObj)
    {
    // Check that object to copy is not self
    if (this != &pi_rObj)
        {
        // Call ancester copy
        HGF2DTransfoModel::operator=(pi_rObj);
        // Update attributes
        Prepare();
        }

    // Return reference to self
    return (*this);
    }

//-----------------------------------------------------------------------------
// Converter (direct)
//-----------------------------------------------------------------------------
StatusInt HGF2DNonLinearTestIdentity::ConvertDirect(double* pio_pXInOut,
                                                    double* pio_pYInOut) const
    {
    // Make sure variables are provided
    HPRECONDITION(pio_pXInOut != 0);
    HPRECONDITION(pio_pYInOut != 0);

    return SUCCESS;
    }

//-----------------------------------------------------------------------------
// Converter (direct)
//-----------------------------------------------------------------------------
StatusInt HGF2DNonLinearTestIdentity::ConvertDirect 
(
double    pi_YIn,
double    pi_XInStart,
size_t    pi_NumLoc,
double    pi_XInStep,
double*   po_aXOut,
double*   po_aYOut
) const
    {
    // Check arrays are provided
    HPRECONDITION(po_aXOut != 0);
    HPRECONDITION(po_aYOut != 0);

    uint32_t   Index;
    double  X;
    double* pCurrentX = po_aXOut;
    double* pCurrentY = po_aYOut;

    for (Index = 0, X = pi_XInStart;
        Index < pi_NumLoc ; ++Index, X+=pi_XInStep, ++pCurrentX, ++pCurrentY)
        {
        *pCurrentX = X;
        *pCurrentY = pi_YIn;
        }

    return SUCCESS;
    }


//-----------------------------------------------------------------------------
// Converter (direct)
//-----------------------------------------------------------------------------
StatusInt HGF2DNonLinearTestIdentity::ConvertDirect 
(
size_t    pi_NumLoc,
double*   pio_aXInOut,
double*   pio_aYInOut
) const
    {
    // Check arrays are provided
    HPRECONDITION(pio_aXInOut != 0);
    HPRECONDITION(pio_aYInOut != 0);
    //Nothing to do, because the arguments are input/output arrays
    return SUCCESS;
    }

//-----------------------------------------------------------------------------
// Converter (direct)
//-----------------------------------------------------------------------------
StatusInt HGF2DNonLinearTestIdentity::ConvertDirect(double   pi_XIn,
                                                    double   pi_YIn,
                                                    double*  po_pXOut,
                                                    double*  po_pYOut) const
    {
    // Make sure recipient variables are provided
    HPRECONDITION(po_pXOut != 0);
    HPRECONDITION(po_pYOut != 0);

    *po_pXOut = pi_XIn;
    *po_pYOut = pi_YIn;

    return SUCCESS;
    }

//-----------------------------------------------------------------------------
// Converter (inverse)
//-----------------------------------------------------------------------------
StatusInt HGF2DNonLinearTestIdentity::ConvertInverse(double* pio_pXInOut,
                                                     double* pio_pYInOut) const
    {
    // Make sure recipient variables are provided
    HPRECONDITION(pio_pXInOut != 0);
    HPRECONDITION(pio_pYInOut != 0);

    return SUCCESS;
    }

//-----------------------------------------------------------------------------
// Converter (inverse)
//-----------------------------------------------------------------------------
StatusInt HGF2DNonLinearTestIdentity::ConvertInverse 
(
double    pi_YIn,
double    pi_XInStart,
size_t    pi_NumLoc,
double    pi_XInStep,
double*   po_aXOut,
double*   po_aYOut
) const
    {
    // Make sure recipient variables are provided
    HPRECONDITION(po_aXOut != 0);
    HPRECONDITION(po_aYOut != 0);

    uint32_t   Index;
    double  X;
    double* pCurrentX = po_aXOut;
    double* pCurrentY = po_aXOut;

    for (Index = 0, X = pi_XInStart;
        Index < pi_NumLoc ; ++Index, X+=pi_XInStep, ++pCurrentX, ++pCurrentY)
        {
        *pCurrentX = X;
        *pCurrentY = pi_YIn;
        }

    return SUCCESS;
    }

//-----------------------------------------------------------------------------
// Converter (inverse)
//-----------------------------------------------------------------------------
StatusInt HGF2DNonLinearTestIdentity::ConvertInverse
(
size_t    pi_NumLoc,
double*   pio_aXInOut,
double*   pio_aYInOut
) const
    {
    // Check arrays are provided
    HPRECONDITION(pio_aXInOut != 0);
    HPRECONDITION(pio_aYInOut != 0);
    //Nothing to do, because the arguments are input/output arrays
    return SUCCESS;
    }

//-----------------------------------------------------------------------------
// Converter (inverse)
//-----------------------------------------------------------------------------
StatusInt HGF2DNonLinearTestIdentity::ConvertInverse(double  pi_XIn,
                                                     double  pi_YIn,
                                                     double* po_pXOut,
                                                     double* po_pYOut) const
    {
    // Make sure recipient variables are provided
    HPRECONDITION(po_pXOut != 0);
    HPRECONDITION(po_pYOut != 0);

    *po_pXOut = pi_XIn;
    *po_pYOut = pi_YIn;

    return SUCCESS;
    }

//-----------------------------------------------------------------------------
// IsStretchable
//-----------------------------------------------------------------------------
bool   HGF2DNonLinearTestIdentity::IsStretchable (double pi_AngleTolerance) const
    {
    return(false);
    }

//-----------------------------------------------------------------------------
// IsIdentity
//-----------------------------------------------------------------------------
bool   HGF2DNonLinearTestIdentity::IsIdentity () const
    {
    return(false);
    }

//-----------------------------------------------------------------------------
// PreservesLinearity
// Indicate if the transformation model preserves linearity
//-----------------------------------------------------------------------------
bool   HGF2DNonLinearTestIdentity::PreservesLinearity () const
    {
    return(false);
    }

//-----------------------------------------------------------------------------
// PreservesParallelism
// Indicate if the transformation model preserves parallelism
//-----------------------------------------------------------------------------
bool   HGF2DNonLinearTestIdentity::PreservesParallelism() const
    {
    return(false);
    }

//-----------------------------------------------------------------------------
// PreservesShape
// Indicate if the transformation model preserves the shape
//-----------------------------------------------------------------------------
bool   HGF2DNonLinearTestIdentity::PreservesShape() const
    {
    return(false);
    }

//-----------------------------------------------------------------------------
// PreservesDirection
// Indicate if the transformation model preserves directions
//-----------------------------------------------------------------------------
bool   HGF2DNonLinearTestIdentity::PreservesDirection() const
    {
    return(false);
    }

//-----------------------------------------------------------------------------
// GetStretchParams
//-----------------------------------------------------------------------------
void    HGF2DNonLinearTestIdentity::GetStretchParams (double*  po_pScaleFactorX,
                                                      double*  po_pScaleFactorY,
                                                      HGF2DDisplacement* po_pDisplacement) const
    {
    HPRECONDITION(po_pScaleFactorX != NULL);
    HPRECONDITION(po_pScaleFactorY != NULL);
    HPRECONDITION(po_pDisplacement != NULL);

    // Return stretch params for neutral model
    *po_pScaleFactorX = 1.0;
    *po_pScaleFactorY = 1.0;
    po_pDisplacement->SetDeltaX(0.0);
    po_pDisplacement->SetDeltaY(0.0);
    }

//-----------------------------------------------------------------------------
// Prepare
// Prepares transformation parameters
//-----------------------------------------------------------------------------
void HGF2DNonLinearTestIdentity::Prepare ()
    {
 
    }
//-----------------------------------------------------------------------------
// Reverse
// This method reverses the transformation model
//-----------------------------------------------------------------------------
void HGF2DNonLinearTestIdentity::Reverse()
    {
    // Invoque reversing of ancester
    HGF2DTransfoModel::Reverse();

    // Prepare
    Prepare();

    }


//-----------------------------------------------------------------------------
// CanBeRepresentedByAMatrix
// Indicates if the model can be represented by a transformation matrix
//-----------------------------------------------------------------------------
bool HGF2DNonLinearTestIdentity::CanBeRepresentedByAMatrix() const
    {
    return(false);
    }

//-----------------------------------------------------------------------------
//  GetMatrix
//  Gets the components of the model by matrix
//-----------------------------------------------------------------------------
HFCMatrix<3, 3> HGF2DNonLinearTestIdentity::GetMatrix() const
    {
    HFCMatrix<3, 3> ReturnedMatrix;

    return(GetMatrix(ReturnedMatrix));
    }

//-----------------------------------------------------------------------------
//  GetMatrix
//  Gets the components of the model by matrix
//-----------------------------------------------------------------------------
HFCMatrix<3, 3>& HGF2DNonLinearTestIdentity::GetMatrix(HFCMatrix<3, 3>& po_rRecipient) const
    {
    po_rRecipient[0][0] = 1.0;
    po_rRecipient[0][1] = 0.0;
    po_rRecipient[0][2] = 0.0;
    po_rRecipient[1][0] = 0.0;
    po_rRecipient[1][1] = 1.0;
    po_rRecipient[1][2] = 0.0;
    po_rRecipient[2][0] = 0.0;
    po_rRecipient[2][1] = 0.0;
    po_rRecipient[2][2] = 1.0;

    return(po_rRecipient);
    }

//-----------------------------------------------------------------------------
// ComposeInverseWithDirectOf
// Composes a new transformation model as a combination of self and given
//-----------------------------------------------------------------------------
HFCPtr<HGF2DTransfoModel>  HGF2DNonLinearTestIdentity::ComposeInverseWithDirectOf(const HGF2DTransfoModel& pi_rModel) const
    {
     HFCPtr<HGF2DTransfoModel> pResultModel(pi_rModel.Clone());

    return (pResultModel);
    }


//-----------------------------------------------------------------------------
// Clone
// This method allocates a copy of self. The caller is responsible for
// the deletion of this object.
//-----------------------------------------------------------------------------
HGF2DTransfoModel* HGF2DNonLinearTestIdentity::Clone () const
    {
    // Allocate object as copy and return
    return (new HGF2DNonLinearTestIdentity(*this));
    }

//-----------------------------------------------------------------------------
// ComposeYourself
// PRIVATE
//-----------------------------------------------------------------------------
HFCPtr<HGF2DTransfoModel>  HGF2DNonLinearTestIdentity::ComposeYourself(const HGF2DTransfoModel& pi_rModel) const
    {
    // Recipient
    HFCPtr<HGF2DTransfoModel> pResultModel;

    // Type is not known ... build a complex
    // To do this we call the ancester ComposeYourself
    pResultModel = HGF2DTransfoModel::ComposeYourself(pi_rModel);

    return (pResultModel);
    }

