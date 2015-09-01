//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hgf/src/HGF2DIdentity.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HGF2DIdentity
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HGF2DIdentity.h>
#include <Imagepp/all/h/HGF2DDisplacement.h>

/** -----------------------------------------------------------------------------
    Default Constructor
    Initializes the model with units for all channels set to meters.
    -----------------------------------------------------------------------------
*/
HGF2DIdentity::HGF2DIdentity()
    {
    // We let the default units and Update unit conversion accelerator attributes
    Prepare();
    }


//-----------------------------------------------------------------------------
// Copy constructor.
//-----------------------------------------------------------------------------
HGF2DIdentity::HGF2DIdentity(const HGF2DIdentity& pi_rObj)
    : HGF2DTransfoModel ()
    {
    // Update unit conversion accelerator attributes
    Prepare();
    }

//-----------------------------------------------------------------------------
// The destructor.
//-----------------------------------------------------------------------------
HGF2DIdentity::~HGF2DIdentity()
    {
    }

//-----------------------------------------------------------------------------
// operator=
// Assignment operator.
//-----------------------------------------------------------------------------
HGF2DIdentity& HGF2DIdentity::operator=(const HGF2DIdentity& pi_rObj)
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Alexandre.Gariepy               06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool HGF2DIdentity::IsConvertDirectThreadSafe() const 
    { 
    return true; 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Alexandre.Gariepy               06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool HGF2DIdentity::IsConvertInverseThreadSafe() const 
    { 
    return true; 
    }

//-----------------------------------------------------------------------------
// Converter (direct)
//-----------------------------------------------------------------------------
StatusInt HGF2DIdentity::ConvertDirect(double* pio_pXInOut,
                                       double* pio_pYInOut) const
    {
    // Make sure that recipient variables are provided
    HPRECONDITION(pio_pXInOut != 0);
    HPRECONDITION(pio_pYInOut != 0);

    return SUCCESS;
    }

//-----------------------------------------------------------------------------
// Converter (direct)
//-----------------------------------------------------------------------------
StatusInt HGF2DIdentity::ConvertDirect(double   pi_XIn,
                                       double   pi_YIn,
                                       double*  po_pXOut,
                                       double*  po_pYOut) const
    {
    // Make sure that recipient variables are provided
    HPRECONDITION(po_pXOut != 0);
    HPRECONDITION(po_pYOut != 0);

    *po_pXOut = pi_XIn;
    *po_pYOut = pi_YIn;

    return SUCCESS;
    }

//-----------------------------------------------------------------------------
// Converter (direct)
//-----------------------------------------------------------------------------
StatusInt HGF2DIdentity::ConvertDirect(double    pi_YIn,
                                       double    pi_XInStart,
                                       size_t    pi_NumLoc,
                                       double    pi_XInStep,
                                       double*   po_aXOut,
                                       double*   po_aYOut) const
    {
    // Make sure recipient arrays are provided
    HPRECONDITION(po_aXOut != 0);
    HPRECONDITION(po_aYOut != 0);

    double  X;
    uint32_t Index;
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
StatusInt HGF2DIdentity::ConvertDirect(size_t    pi_NumLoc,
                                       double*   pio_aXInOut,
                                       double*   pio_aYInOut) const
    {
    HPRECONDITION(pio_aXInOut != 0);
    HPRECONDITION(pio_aYInOut != 0);
    //Nothing to do because the arrays in argument are input/output arrays
    return SUCCESS;
    }

//-----------------------------------------------------------------------------
// Converter (inverse)
//-----------------------------------------------------------------------------
StatusInt HGF2DIdentity::ConvertInverse(double* pio_pXInOut,
                                        double* pio_pYInOut) const
    {
    // Make sure that recipient variables are provided
    HPRECONDITION(pio_pXInOut != 0);
    HPRECONDITION(pio_pYInOut != 0);

    return SUCCESS;
    }

//-----------------------------------------------------------------------------
// Converter (inverse)
//-----------------------------------------------------------------------------
StatusInt HGF2DIdentity::ConvertInverse(double  pi_XIn,
                                        double  pi_YIn,
                                        double* po_pXOut,
                                        double* po_pYOut) const
    {
    // Make sure that recipient variables are provided
    HPRECONDITION(po_pXOut != 0);
    HPRECONDITION(po_pYOut != 0);

    *po_pXOut = pi_XIn;
    *po_pYOut = pi_YIn;

    return SUCCESS;
    }

//-----------------------------------------------------------------------------
// Converter (inverse)
//-----------------------------------------------------------------------------
StatusInt HGF2DIdentity::ConvertInverse(double    pi_YIn,
                                        double    pi_XInStart,
                                        size_t    pi_NumLoc,
                                        double    pi_XInStep,
                                        double*   po_aXOut,
                                        double*   po_aYOut) const
    {
    // Make sure recipient arrays are provided
    HPRECONDITION(po_aXOut != 0);
    HPRECONDITION(po_aYOut != 0);

    uint32_t Index;
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
// Converter (inverse)
//-----------------------------------------------------------------------------
StatusInt HGF2DIdentity::ConvertInverse(size_t    pi_NumLoc,
                                        double*   pio_aXInOut,
                                        double*   pio_aYInOut) const
    {
    HPRECONDITION(pio_aXInOut != 0);
    HPRECONDITION(pio_aYInOut != 0);
    //Nothing to do because the arrays in argument are input/output arrays
    return SUCCESS;
    }


//-----------------------------------------------------------------------------
// IsStretchable
//-----------------------------------------------------------------------------
bool   HGF2DIdentity::IsStretchable (double pi_AngleTolerance) const
    {
    return (true);
    }

//-----------------------------------------------------------------------------
// IsIdentity
//-----------------------------------------------------------------------------
bool   HGF2DIdentity::IsIdentity () const
    {
    return (true);
    }

//-----------------------------------------------------------------------------
// PreservesLinearity
// Indicate if the transformation model preserves linearity
//-----------------------------------------------------------------------------
bool   HGF2DIdentity::PreservesLinearity () const
    {
    return (true);
    }

//-----------------------------------------------------------------------------
// PreservesParallelism
// Indicate if the transformation model preserves parallelism
//-----------------------------------------------------------------------------
bool   HGF2DIdentity::PreservesParallelism() const
    {
    return (true);
    }

//-----------------------------------------------------------------------------
// PreservesShape
// Indicate if the transformation model preserves the shape
//-----------------------------------------------------------------------------
bool   HGF2DIdentity::PreservesShape() const
    {
    return (true);
    }

//-----------------------------------------------------------------------------
// PreservesDirection
// Indicate if the transformation model preserves directions
//-----------------------------------------------------------------------------
bool   HGF2DIdentity::PreservesDirection() const
    {
    return (true);
    }

//-----------------------------------------------------------------------------
// GetStretchParams
//-----------------------------------------------------------------------------
void    HGF2DIdentity::GetStretchParams (double*  po_pScaleFactorX,
                                         double*  po_pScaleFactorY,
                                         HGF2DDisplacement* po_pDisplacement) const
    {
    HPRECONDITION(po_pScaleFactorX != 0);
    HPRECONDITION(po_pScaleFactorY != 0);
    HPRECONDITION(po_pDisplacement != 0);

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
void HGF2DIdentity::Prepare ()
    {
    }

//-----------------------------------------------------------------------------
// Reverse
// This method reverses the transformation model
//-----------------------------------------------------------------------------
void HGF2DIdentity::Reverse()
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
bool HGF2DIdentity::CanBeRepresentedByAMatrix() const
    {
    return true;
    }

//-----------------------------------------------------------------------------
//  GetMatrix
//  Gets the components of the model by matrix
//-----------------------------------------------------------------------------
HFCMatrix<3, 3> HGF2DIdentity::GetMatrix() const
    {
    HFCMatrix<3, 3> ReturnedMatrix;

    return(GetMatrix(ReturnedMatrix));
    }

//-----------------------------------------------------------------------------
//  GetMatrix
//  Gets the components of the model by matrix
//-----------------------------------------------------------------------------
HFCMatrix<3, 3>& HGF2DIdentity::GetMatrix(HFCMatrix<3, 3>& po_rRecipient) const
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
HFCPtr<HGF2DTransfoModel>  HGF2DIdentity::ComposeInverseWithDirectOf (const HGF2DTransfoModel& pi_rModel) const
    {
    // Recipient
    HFCPtr<HGF2DTransfoModel> pResultModel(pi_rModel.Clone());

    return (pResultModel);
    }


//-----------------------------------------------------------------------------
// Clone
// This method allocates a copy of self. The caller is responsible for
// the deletion of this object.
//-----------------------------------------------------------------------------
HGF2DTransfoModel* HGF2DIdentity::Clone () const
    {
    // Allocate object as copy and return
    return (new HGF2DIdentity (*this));
    }




