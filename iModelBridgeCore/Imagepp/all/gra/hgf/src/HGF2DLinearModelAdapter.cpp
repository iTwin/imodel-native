//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hgf/src/HGF2DLinearModelAdapter.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HGF2DLinearModelAdapter
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HGF2DIdentity.h>
#include <Imagepp/all/h/HGF2DSimilitude.h>
#include <Imagepp/all/h/HGF2DTranslation.h>
#include <Imagepp/all/h/HGF2DStretch.h>
#include <Imagepp/all/h/HGF2DAffine.h>
#include <Imagepp/all/h/HGF2DProjective.h>
#include <ImagePPInternal/ext/MatrixFromTiePts/MatrixFromTiePts.h>

#include <Imagepp/all/h/HGF2DLinearModelAdapter.h>

//-----------------------------------------------------------------------------
// Default Constructor
//-----------------------------------------------------------------------------
HGF2DLinearModelAdapter::HGF2DLinearModelAdapter()
    : HGF2DTransfoModelAdapter(HGF2DIdentity())

    {
    m_pPreTransfoModel      = new HGF2DIdentity();
    m_pPostTransfoModel     = new HGF2DIdentity();

    m_ApplicationArea = HGF2DLiteExtent(0.0, 0.0, 1.0, 1.0);
    m_Step = 1.0;

    m_pLinearModel = CreateDirectModelFromExtent(m_ApplicationArea, m_Step);

    m_AdaptAsAffine = false;

    Prepare();

    HINVARIANTS;
    }


/** -----------------------------------------------------------------------------
    Creates an linear model transformation adapter model based on the given model.
    A copy of the given model is performed and kept internally in the
    transformation model adapter.

    The linear model adapter samples the transformation of the model to adapt
    in the indicated region at every step in both X and Y directions. The
    given extent may not represent an empty area and the step must be twice as
    small as both the extent width and height.

    The adapted model must be non-linear.

    The units of the present adapter are those of the non-linear model adapted

    @param pi_rNonLinearTransfoModel Constant reference to a transformation model
                                     to adapt. A copy of this model is done.

    @param pi_rApplicationArea The extent in direct channel coordinates
                               to create an affine or projective approximation of the
                               non-linear model. Transformation is sample in this
                               area using provided step.

    @param pi_Step  The sampling step used in the sampling of the transformation
                    in direct raw unit. This step must be greater than 0.0.

    @param pi_AdaptAsAffine Optional parameter indicating if the adaptation must
                            must restrict the approximation to a simple affine
                            and not compute projection parameters. The default is
                            to adapt as a projective.

    -----------------------------------------------------------------------------
*/
HGF2DLinearModelAdapter::HGF2DLinearModelAdapter(const HGF2DTransfoModel& pi_rNonLinearTransfoModel,
                                                 const HGF2DLiteExtent&   pi_rApplicationArea,
                                                 double                  pi_Step,
                                                 bool                    pi_AdaptAsAffine)
    : HGF2DTransfoModelAdapter(pi_rNonLinearTransfoModel)
    {
    // The steps must not be null nor negative
    HPRECONDITION(pi_Step > 0.0);

    // The given model must not preserve linearity
    HPRECONDITION(!pi_rNonLinearTransfoModel.PreservesLinearity());

    // The width nor the height of extent may be null
    HPRECONDITION(pi_rApplicationArea.GetWidth() > 0.0);
    HPRECONDITION(pi_rApplicationArea.GetHeight() > 0.0);

    // The step must be at least twice as small as either width or height
    // of extent
    HPRECONDITION((pi_Step * 2.0) < pi_rApplicationArea.GetWidth());
    HPRECONDITION((pi_Step * 2.0) < pi_rApplicationArea.GetHeight());

    m_pPreTransfoModel      = new HGF2DIdentity();
    m_pPostTransfoModel     = new HGF2DIdentity();


    m_ApplicationArea = pi_rApplicationArea;
    m_Step = pi_Step;
    m_AdaptAsAffine = pi_AdaptAsAffine;
    m_pLinearModel = CreateDirectModelFromExtent(m_ApplicationArea, m_Step);

    HINVARIANTS;
    }


/** -----------------------------------------------------------------------------
    PROTECTED
    For internal use only.
    Creates a affine or projective transformation model adapter model based on the given model.
    A copy of the given model is performed and kept internally in the
    transformation model adapter.

    The linear model adapter samples the transformation of the model to adapt
    in the indicated region at every step in both X and Y directions. The
    given extent may not represent an empty area and the step must be twice as
    small as both the extent width and height.

    The adapted model must be non-linear.

    The units of the present adapter are those of the non-linear model adapted

    Contrarly to public constructor, the method requires a pre and post transformation
    models that must be linearity preserving. These models are used to generate
    composition with other linear models without increasing the complexity
    of the result model. Copies of each of these models is performed.

    @param pi_rNonLinearTransfoModel Constant reference to a transformation model
                                     to adapt. A copy of this model is done.

    @param pi_rPreTransfoModel A linearity and paralelism preserving transformation model
                               that describes the transformation to apply before
                               the non linear model adapted.

    @param pi_rPostTransfoModel A linearity and paralelism preserving transformation model
                                that describes the transformation to apply after
                                the non linear model adapted.

    @param pi_rApplicationArea The extent in direct channel coordinates
                               to create an affine or projective approximation of the
                               non-linear model. Transformation is sample in this
                               area using provided step.

    @param pi_Step  The sampling step used in the sampling of the transformation
                    in direct raw unit. This step must be greater than 0.0.

    @param pi_AdaptAsAffine Optional parameter indicating if the adaptation must
                            must restrict the approximation to a simple affine
                            and not compute projection parameters. The default is
                            to adapt as a projective.

    -----------------------------------------------------------------------------
*/
HGF2DLinearModelAdapter::HGF2DLinearModelAdapter(const HGF2DTransfoModel& pi_rNonLinearTransfoModel,
                                                 const HGF2DTransfoModel& pi_rPreTransfo,
                                                 const HGF2DTransfoModel& pi_rPostTransfo,
                                                 const HGF2DLiteExtent&   pi_rApplicationArea,
                                                 double                  pi_Step,
                                                 bool                    pi_AdaptAsAffine)
    : HGF2DTransfoModelAdapter(pi_rNonLinearTransfoModel)
    {
    // The steps must not be null nor negative
    HPRECONDITION(pi_Step > 0.0);

    // The pre and post model must preserve linearity
    HPRECONDITION(pi_rPreTransfo.PreservesLinearity());
    HPRECONDITION(pi_rPostTransfo.PreservesLinearity());

    // The given model must not preserve linearity
    HPRECONDITION(!pi_rNonLinearTransfoModel.PreservesLinearity());

    // The width nor the height of extent may be null
    HPRECONDITION(pi_rApplicationArea.GetWidth() > 0.0);
    HPRECONDITION(pi_rApplicationArea.GetHeight() > 0.0);

    // The step must be at least twice as small as either width or height
    // of extent
    HPRECONDITION((pi_Step * 2.0) < pi_rApplicationArea.GetWidth());
    HPRECONDITION((pi_Step * 2.0) < pi_rApplicationArea.GetHeight());

    m_pPreTransfoModel       = pi_rPreTransfo.Clone();
    m_pPostTransfoModel      = pi_rPostTransfo.Clone();


    m_ApplicationArea = pi_rApplicationArea;
    m_Step = pi_Step;
    m_AdaptAsAffine = pi_AdaptAsAffine;
    m_pLinearModel = CreateDirectModelFromExtent(m_ApplicationArea, m_Step);

    HINVARIANTS;
    }


/** -----------------------------------------------------------------------------
    PROTECTED
    For internal use only.
    Creates a linear model transformation model adapter model based on the given model.
    A copy of the given model is performed and kept internally in the
    transformation model adapter.

    The linear model adapter samples the transformation of the model to adapt
    in the indicated region at every step in both X and Y directions. The
    given extent may not represent an empty area and the step must be twice as
    small as both the extent width and height.

    The adapted model must be non-linear.

    The units of the present model are those of the linear model approximation
    not those of the adapter model

    Contrarly to public constructor, the method requires a pre and post transformation
    models that must be linearity preserving. These models are used to generate
    composition with other linear models without increasing the complexity
    of the result model. Copies of each of these models is performed.

    In addition, this constructor received the already computed adpater that
    results from composition. This implies that non-linear model,
    application area nor step are used since the approximation is already
    resloved.

    @param pi_rNonLinearTransfoModel Constant reference to a transformation model
                                     to adapt. A copy of this model is done.

    @param pi_rLinearApproximation The already compute linear model approximation. A
                                   copy of this model is done.

    @param pi_rPreTransfoModel A linearity and paralelism preserving transformation model
                               that describes the transformation to apply before
                               the non linear model adapted.

    @param pi_rPostTransfoModel A linearity and paralelism preserving transformation model
                                that describes the transformation to apply after
                                the non linear model adapted.

    @param pi_rApplicationArea The extent in direct channel coordinates
                               to create a linear model approximation of the
                               non-linear model. Transformation is sample in this
                               area using provided step.

    @param pi_Step  The sampling step used in the sampling of the transformation
                    in direct raw unit. This step must be greater than 0.0.

    @param pi_AdaptAsAffine Optional parameter indicating if the adaptation must
                            must restrict the approximation to a simple affine
                            and not compute projection parameters. The default is
                            to adapt as a projective.

    -----------------------------------------------------------------------------
*/
HGF2DLinearModelAdapter::HGF2DLinearModelAdapter(const HGF2DTransfoModel& pi_rNonLinearTransfoModel,
                                                 const HGF2DTransfoModel& pi_rLinearApproximation,
                                                 const HGF2DTransfoModel& pi_rPreTransfo,
                                                 const HGF2DTransfoModel& pi_rPostTransfo,
                                                 const HGF2DLiteExtent&   pi_rApplicationArea,
                                                 double                  pi_Step,
                                                 bool                    pi_AdaptAsAffine)
    : HGF2DTransfoModelAdapter(pi_rNonLinearTransfoModel)
    {
    // The steps must not be null nor negative
    HPRECONDITION(pi_Step > 0.0);

    // The pre and post model must preserve linearity
    HPRECONDITION(pi_rPreTransfo.PreservesLinearity());
    HPRECONDITION(pi_rPostTransfo.PreservesLinearity());

    HPRECONDITION(pi_rLinearApproximation.PreservesLinearity());
    HPRECONDITION(!pi_AdaptAsAffine || pi_rLinearApproximation.PreservesParallelism());

    // The given model must not preserve linearity
    HPRECONDITION(!pi_rNonLinearTransfoModel.PreservesLinearity());

    // The width nor the height of extent may be null
    HPRECONDITION(pi_rApplicationArea.GetWidth() > 0.0);
    HPRECONDITION(pi_rApplicationArea.GetHeight() > 0.0);

    // The step must be at least twice as small as either width or height
    // of extent
    HPRECONDITION((pi_Step * 2.0) < pi_rApplicationArea.GetWidth());
    HPRECONDITION((pi_Step * 2.0) < pi_rApplicationArea.GetHeight());


    m_pPreTransfoModel       = pi_rPreTransfo.Clone();
    m_pPostTransfoModel      = pi_rPostTransfo.Clone();
    m_pLinearModel           = pi_rLinearApproximation.Clone();




    m_ApplicationArea = pi_rApplicationArea;
    m_Step = pi_Step;
    m_AdaptAsAffine = pi_AdaptAsAffine;


    HINVARIANTS;
    }







/** -----------------------------------------------------------------------------
    This is the copy constructor. It copies the state of the given linear model
    adapter. Note that copies of all transformation models is performed.

    @param pi_rObj  Constant reference to a linear model adapter to copy state from.

    -----------------------------------------------------------------------------
*/
HGF2DLinearModelAdapter::HGF2DLinearModelAdapter(const HGF2DLinearModelAdapter& pi_rObj)
    : HGF2DTransfoModelAdapter(pi_rObj)
    {
    Copy (pi_rObj);

    HINVARIANTS;
    }

//-----------------------------------------------------------------------------
// The destructor.
//-----------------------------------------------------------------------------
HGF2DLinearModelAdapter::~HGF2DLinearModelAdapter()
    {
    }

/** -----------------------------------------------------------------------------
    This is the assignment operator. It copies the state of the given linear model
    adapter. Note that copies of all transformation models is performed.

    @param pi_rObj  Constant reference to a linear model adapter to copy state from.

    @return Returns a reference to self to be used as an l-value.
    -----------------------------------------------------------------------------
*/
HGF2DLinearModelAdapter& HGF2DLinearModelAdapter::operator=(const HGF2DLinearModelAdapter& pi_rObj)
    {
    HINVARIANTS;

    // Check that object to copy is not self
    if (this != &pi_rObj)
        {
        // Call ancestor operator=
        HGF2DTransfoModelAdapter::operator=(pi_rObj);
        Copy (pi_rObj);
        }

    // Return reference to self
    return (*this);
    }


//-----------------------------------------------------------------------------
// Converter (direct)
//-----------------------------------------------------------------------------
StatusInt HGF2DLinearModelAdapter::ConvertDirect(double* pio_pXInOut,
                                            double* pio_pYInOut) const
    {
    HINVARIANTS;

    HPRECONDITION(pio_pXInOut != 0);
    HPRECONDITION(pio_pYInOut != 0);

    return m_pLinearModel->ConvertDirect(pio_pXInOut, pio_pYInOut);
    }


//-----------------------------------------------------------------------------
// Converter (direct)
//-----------------------------------------------------------------------------
StatusInt HGF2DLinearModelAdapter::ConvertDirect(double pi_XIn,
                                            double pi_YIn,
                                            double* po_pXOut,
                                            double* po_pYOut) const
    {
    HINVARIANTS;

    HPRECONDITION(po_pXOut != 0);
    HPRECONDITION(po_pYOut != 0);


    return m_pLinearModel->ConvertDirect(pi_XIn, pi_YIn, po_pXOut, po_pYOut);
    }

//-----------------------------------------------------------------------------
// Converter (direct)
//-----------------------------------------------------------------------------
StatusInt HGF2DLinearModelAdapter::ConvertDirect(double    pi_YIn,
                                                 double    pi_XInStart,
                                                 size_t     pi_NumLoc,
                                                 double    pi_XInStep,
                                                 double*   po_pXOut,
                                                 double*   po_pYOut) const
    {
    HINVARIANTS;

    // Make sure recipient arrays are provided
    HPRECONDITION(po_pXOut != 0);
    HPRECONDITION(po_pYOut != 0);

    return m_pLinearModel->ConvertDirect(pi_YIn, pi_XInStart, pi_NumLoc, pi_XInStep, po_pXOut, po_pYOut);
    }

//-----------------------------------------------------------------------------
// Converter (direct)
//-----------------------------------------------------------------------------
StatusInt HGF2DLinearModelAdapter::ConvertDirect(size_t pi_NumLoc, 
                                                 double* pio_aXInOut, 
                                                 double* pio_aYInOut) const
    {
    HINVARIANTS;

    // Make sure recipient arrays are provided
    HPRECONDITION(pio_aXInOut != 0);
    HPRECONDITION(pio_aYInOut != 0);

    return m_pLinearModel->ConvertDirect(pi_NumLoc, pio_aXInOut, pio_aYInOut);
    }


//-----------------------------------------------------------------------------
// Converter (inverse)
//-----------------------------------------------------------------------------
StatusInt HGF2DLinearModelAdapter::ConvertInverse(double* pio_pXInOut,
                                                  double* pio_pYInOut) const
    {
    HINVARIANTS;

    HPRECONDITION(pio_pXInOut != 0);
    HPRECONDITION(pio_pYInOut != 0);

    return m_pLinearModel->ConvertInverse(pio_pXInOut, pio_pYInOut);
    }


//-----------------------------------------------------------------------------
// Converter (inverse)
//-----------------------------------------------------------------------------
StatusInt HGF2DLinearModelAdapter::ConvertInverse(double pi_XIn,
                                                  double pi_YIn,
                                                  double* po_pXOut,
                                                  double* po_pYOut) const
    {
    HINVARIANTS;

    HPRECONDITION(po_pXOut != 0);
    HPRECONDITION(po_pYOut != 0);

    return m_pLinearModel->ConvertInverse(pi_XIn, pi_YIn, po_pXOut, po_pYOut);
    }

//-----------------------------------------------------------------------------
// Converter (inverse)
//-----------------------------------------------------------------------------
StatusInt HGF2DLinearModelAdapter::ConvertInverse(double    pi_YIn,
                                                  double    pi_XInStart,
                                                  size_t    pi_NumLoc,
                                                  double    pi_XInStep,
                                                  double*   po_pXOut,
                                                  double*   po_pYOut) const
    {
    HINVARIANTS;

    // Make sure recipient arrays are provided
    HPRECONDITION(po_pXOut != 0);
    HPRECONDITION(po_pYOut != 0);

    return m_pLinearModel->ConvertInverse(pi_YIn, pi_XInStart, pi_NumLoc, pi_XInStep, po_pXOut, po_pYOut);
    }


//-----------------------------------------------------------------------------
// Converter (Inverse)
//-----------------------------------------------------------------------------
StatusInt HGF2DLinearModelAdapter::ConvertInverse(size_t pi_NumLoc, 
                                                  double* pio_aXInOut, 
                                                  double* pio_aYInOut) const
    {
    HINVARIANTS;

    // Make sure recipient arrays are provided
    HPRECONDITION(pio_aXInOut != 0);
    HPRECONDITION(pio_aYInOut != 0);

    return m_pLinearModel->ConvertInverse(pi_NumLoc, pio_aXInOut, pio_aYInOut);
    }


//-----------------------------------------------------------------------------
// PreservesLinearity
// Indicate if the transformation model preserves linearity
//-----------------------------------------------------------------------------
bool   HGF2DLinearModelAdapter::PreservesLinearity () const
    {
    HINVARIANTS;

    return(m_pLinearModel->PreservesLinearity());
    }

//-----------------------------------------------------------------------------
// PreservesParallelism
// Indicate if the transformation model preserves parallelism
//-----------------------------------------------------------------------------
bool   HGF2DLinearModelAdapter::PreservesParallelism() const
    {
    HINVARIANTS;

    return(m_pLinearModel->PreservesParallelism());
    }

//-----------------------------------------------------------------------------
// PreservesShape
// Indicate if the transformation model preserves the shape
//-----------------------------------------------------------------------------
bool   HGF2DLinearModelAdapter::PreservesShape() const
    {
    HINVARIANTS;

    return(m_pLinearModel->PreservesShape());
    }

//-----------------------------------------------------------------------------
// PreservesDirection
// Indicate if the transformation model preserves directions
//-----------------------------------------------------------------------------
bool   HGF2DLinearModelAdapter::PreservesDirection() const
    {
    HINVARIANTS;

    return(m_pLinearModel->PreservesDirection());
    }


//-----------------------------------------------------------------------------
// CanBeRepresentedByAMatrix
// Indicates if the model can be represented by a transformation matrix
//-----------------------------------------------------------------------------
bool HGF2DLinearModelAdapter::CanBeRepresentedByAMatrix() const
    {
    HINVARIANTS;

    return(m_pLinearModel->CanBeRepresentedByAMatrix());
    }


//-----------------------------------------------------------------------------
//  IsIdentity
//  Returns true if the model contains no transformation
//-----------------------------------------------------------------------------
bool HGF2DLinearModelAdapter::IsIdentity () const
    {
    HINVARIANTS;

    return(m_pLinearModel->IsIdentity());
    }

//-----------------------------------------------------------------------------
//  IsStetchable
//  Returns true if the model contains only scaling and translation
//-----------------------------------------------------------------------------
bool HGF2DLinearModelAdapter::IsStretchable (double pi_AngleTolerance) const
    {
    HINVARIANTS;

    return(m_pLinearModel->IsStretchable(pi_AngleTolerance));
    }


//-----------------------------------------------------------------------------
//  GetStetchParams
//  Returns the stretch parameters
//-----------------------------------------------------------------------------
void HGF2DLinearModelAdapter::GetStretchParams(double* po_pScaleFactorX,
                                               double* po_pScaleFactorY,
                                               HGF2DDisplacement* po_pDisplacement) const
    {
    HINVARIANTS;

    HPRECONDITION(po_pScaleFactorX != 0);
    HPRECONDITION(po_pScaleFactorY != 0);
    HPRECONDITION(po_pDisplacement != 0);


    m_pLinearModel->GetStretchParams(po_pScaleFactorX, po_pScaleFactorY, po_pDisplacement);
    }



//-----------------------------------------------------------------------------
//  GetMatrix
//  Gets the components of the model by matrix
//-----------------------------------------------------------------------------
HFCMatrix<3, 3> HGF2DLinearModelAdapter::GetMatrix() const
    {
    HINVARIANTS;

    return(m_pLinearModel->GetMatrix());

    }


//-----------------------------------------------------------------------------
// Reverse
// This method reverses the transformation model
//-----------------------------------------------------------------------------
void    HGF2DLinearModelAdapter::Reverse()
    {
    HINVARIANTS;

    // Swap pre and post transformations
    HFCPtr<HGF2DTransfoModel> pTempModel;
    pTempModel = m_pPreTransfoModel;
    m_pPreTransfoModel = m_pPostTransfoModel;
    m_pPostTransfoModel = pTempModel;

    // Reverse transformation models
    m_pPreTransfoModel->Reverse();
    m_pPostTransfoModel->Reverse();

    // Compute center of application area
    m_pLinearModel->Reverse();

    // Invoque reversing of ancester
    // This call will in turn invoque Prepare()
    HGF2DTransfoModelAdapter::Reverse();
    }



//-----------------------------------------------------------------------------
// ComposeInverseWithDirectOf
// Composes a new transformation model as a combination of self and given
//-----------------------------------------------------------------------------
HFCPtr<HGF2DTransfoModel>  HGF2DLinearModelAdapter::ComposeInverseWithDirectOf (const HGF2DTransfoModel& pi_rModel) const
    {
    HINVARIANTS;

    // Recipient
    HFCPtr<HGF2DTransfoModel> pResultModel;

    if (pi_rModel.PreservesLinearity())
        {
        HFCPtr<HGF2DTransfoModel> pNewPostModel = m_pPostTransfoModel->ComposeInverseWithDirectOf(pi_rModel);

        HFCPtr<HGF2DTransfoModel> pNewLinearModel = m_pLinearModel->ComposeInverseWithDirectOf(pi_rModel);

        pResultModel = new HGF2DLinearModelAdapter(*m_pAdaptedTransfoModel,
                                                   *pNewLinearModel,
                                                   *m_pPreTransfoModel,
                                                   *pNewPostModel,
                                                   m_ApplicationArea,
                                                   m_Step);

        }
    else
        {
        // Model is not known ... ask other
        pResultModel = CallComposeOf(pi_rModel);
        }

    return (pResultModel);
    }




//-----------------------------------------------------------------------------
// Clone
// This method allocates a copy of self. The caller is responsible for
// the deletion of this object.
//-----------------------------------------------------------------------------
HGF2DTransfoModel* HGF2DLinearModelAdapter::Clone () const
    {
    HINVARIANTS;

    // Allocate object as copy and return
    return(new HGF2DLinearModelAdapter(*this));
    }


//-----------------------------------------------------------------------------
// ComposeYourself
// PRIVATE
//-----------------------------------------------------------------------------
HFCPtr<HGF2DTransfoModel>  HGF2DLinearModelAdapter::ComposeYourself (const HGF2DTransfoModel& pi_rModel) const
    {
    HINVARIANTS;

    // Recipient
    HFCPtr<HGF2DTransfoModel> pResultModel;

    if (pi_rModel.IsIdentity())
        {
        pResultModel = new HGF2DLinearModelAdapter(*this);
        }
    // Check if the type of the given model can be represented by a matrix
    else if (pi_rModel.PreservesLinearity() &&
             pi_rModel.PreservesParallelism())
        {

        // Compute the new pre-model
        HFCMatrix<3, 3> MySelfMatrix(m_pPreTransfoModel->GetMatrix());

        HFCPtr<HGF2DTransfoModel> pResultPreModel;

        if (m_AdaptAsAffine)
            {
            // Compose the two matrix together
            pResultPreModel = new HGF2DAffine();
            }
        else
            {
            // Compose the two matrix together
            pResultPreModel = new HGF2DProjective();
            }



        HFCMatrix<3, 3> ResMatrix = MySelfMatrix * pi_rModel.GetMatrix();

        // The projection parameters must be null
        HASSERT(ResMatrix[2][0] == 0.0);
        HASSERT(ResMatrix[2][1] == 0.0);

        if (m_AdaptAsAffine)
            {
            ((HGF2DAffine*)(&(*pResultPreModel)))->SetByMatrixParameters(ResMatrix[0][2],
                                                                         ResMatrix[0][0],
                                                                         ResMatrix[0][1],
                                                                         ResMatrix[1][2],
                                                                         ResMatrix[1][0],
                                                                         ResMatrix[1][1]);
            }
        else
            {
            ((HGF2DProjective*)(&(*pResultPreModel)))->SetByMatrix(MySelfMatrix * pi_rModel.GetMatrix());
            }

        // Compute the new affine or projective
        MySelfMatrix = m_pLinearModel->GetMatrix();

        HFCPtr<HGF2DTransfoModel> pResultLinModel;

        if (m_AdaptAsAffine)
            {
            // Compose the two matrix together
            pResultLinModel = new HGF2DAffine();
            }
        else
            {
            // Compose the two matrix together
            pResultLinModel = new HGF2DProjective();
            }


        ResMatrix = MySelfMatrix * pi_rModel.GetMatrix();

        if (m_AdaptAsAffine)
            {
            // The projection parameters must be null
            HASSERT(ResMatrix[2][0] == 0.0);
            HASSERT(ResMatrix[2][1] == 0.0);

            ((HGF2DAffine*)(&(*pResultLinModel)))->SetByMatrixParameters(ResMatrix[0][2],
                                                                         ResMatrix[0][0],
                                                                         ResMatrix[0][1],
                                                                         ResMatrix[1][2],
                                                                         ResMatrix[1][0],
                                                                         ResMatrix[1][1]);
            }
        else
            {
            ((HGF2DProjective*)(&(*pResultLinModel)))->SetByMatrix(MySelfMatrix * pi_rModel.GetMatrix());
            }

        // Convert center area coordinates
        pResultModel = new HGF2DLinearModelAdapter(*m_pAdaptedTransfoModel,
                                                   *pResultLinModel,
                                                   *pResultPreModel,
                                                   *m_pPostTransfoModel,
                                                   m_ApplicationArea,
                                                   m_Step);
        }
    else
        {
        // Type is not known ... build a complex
        // To do this we call the ancester ComposeYourself
        pResultModel = HGF2DTransfoModel::ComposeYourself(pi_rModel);
        }

    return (pResultModel);
    }




//-----------------------------------------------------------------------------
//  Prepare
//  This methods prepares the conversion parameters from the basic
//  model attribute
//-----------------------------------------------------------------------------
void HGF2DLinearModelAdapter::Prepare ()
    {
    // Obtain conversion ratio for direct X to inverse X units

    // Invoque preparation of ancester (required)
    HGF2DTransfoModelAdapter::Prepare();

    }


//-----------------------------------------------------------------------------
//  Copy
//  Copy method
//-----------------------------------------------------------------------------
void HGF2DLinearModelAdapter::Copy(const HGF2DLinearModelAdapter& pi_rObj)
    {
    // Copy master data
    m_pPreTransfoModel       = pi_rObj.m_pPreTransfoModel->Clone();
    m_pPostTransfoModel      = pi_rObj.m_pPostTransfoModel->Clone();

    m_Step = pi_rObj.m_Step;

    m_AdaptAsAffine = pi_rObj.m_AdaptAsAffine;

    m_pLinearModel = pi_rObj.m_pLinearModel->Clone();
    m_ApplicationArea = pi_rObj.m_ApplicationArea;
    }


//-----------------------------------------------------------------------------
// CreateSimplifiedModel
//-----------------------------------------------------------------------------
HFCPtr<HGF2DTransfoModel> HGF2DLinearModelAdapter::CreateSimplifiedModel() const
    {
    HINVARIANTS;

    HFCPtr<HGF2DTransfoModel> pResultModel;

    pResultModel = m_pLinearModel->CreateSimplifiedModel();

    // If simplification of model is not possible then return the linear adapter.
    if (pResultModel == 0)
        pResultModel = m_pLinearModel->Clone();

    return(pResultModel);
    }


/** -----------------------------------------------------------------------------
    PRIVATE METHOD
    This method performs the generation of a affine or projective by sampling of the
    transformation of the non-linear model.

    @param pi_rExtent The application area over which sampling is performed.

    @param pi_Step The sampling step used in both X and Y directions.

    @return Returns the generated affine or projective linear transformation model.
    -----------------------------------------------------------------------------
*/
HFCPtr<HGF2DTransfoModel> HGF2DLinearModelAdapter::CreateDirectModelFromExtent(const HGF2DLiteExtent& pi_rExtent, double pi_Step) const
    {
    // The extent of area must not be empty
    HPRECONDITION(pi_rExtent.GetWidth() != 0.0);
    HPRECONDITION(pi_rExtent.GetHeight() != 0.0);

    // The step may not be null nor negative
    HPRECONDITION(pi_Step > 0.0);

    // Calculate the number of tie points needed
    uint32_t NumberOfPoints = (uint32_t)(((pi_rExtent.GetHeight() / pi_Step) + 2) * ((pi_rExtent.GetWidth() / pi_Step) + 2));


    // Allocate Tie Points (6 double per tie points pair)
    HArrayAutoPtr<double> pTiePoints(new double[NumberOfPoints * 6]);

    uint32_t CurrentTiePointVal = 0;
    double CurrentX;
    double CurrentY;
    double TempX;
    double TempY;
    for (CurrentY = pi_rExtent.GetYMin() ; CurrentY < pi_rExtent.GetYMax() ; CurrentY += pi_Step)
        {
        for (CurrentX = pi_rExtent.GetXMin() ; CurrentX < pi_rExtent.GetXMax() ; CurrentX += pi_Step)
            {
            pTiePoints[CurrentTiePointVal] = CurrentX;
            ++CurrentTiePointVal;
            pTiePoints[CurrentTiePointVal] = CurrentY;
            ++CurrentTiePointVal;
            pTiePoints[CurrentTiePointVal] = 0.0;
            ++CurrentTiePointVal;

            m_pPreTransfoModel->ConvertDirect(CurrentX, CurrentY, &TempX, &TempY);

            // Apply non-linear transformation
            m_pAdaptedTransfoModel->ConvertDirect(&TempX, &TempY);

            // Apply the post-transformation
            m_pPostTransfoModel->ConvertDirect(&TempX, &TempY);


            pTiePoints[CurrentTiePointVal] = TempX;
            ++CurrentTiePointVal;
            pTiePoints[CurrentTiePointVal] = TempY;
            ++CurrentTiePointVal;
            pTiePoints[CurrentTiePointVal] = 0.0;
            ++CurrentTiePointVal;
            }
        }



    double MyFlatMatrix[4][4];

    if (m_AdaptAsAffine)
        {
        // Create matrix
        GetAffineTransfoMatrixFromScaleAndTiePts(MyFlatMatrix, (unsigned short)CurrentTiePointVal, pTiePoints);
        }
    else
        {
        // Create matrix
        GetProjectiveTransfoMatrixFromScaleAndTiePts(MyFlatMatrix, (unsigned short)CurrentTiePointVal, pTiePoints);
        }

    HFCMatrix<3, 3> MyMatrix;
    MyMatrix[0][0] = MyFlatMatrix[0][0];
    MyMatrix[0][1] = MyFlatMatrix[0][1];
    MyMatrix[0][2] = MyFlatMatrix[0][3];
    MyMatrix[1][0] = MyFlatMatrix[1][0];
    MyMatrix[1][1] = MyFlatMatrix[1][1];
    MyMatrix[1][2] = MyFlatMatrix[1][3];
    MyMatrix[2][0] = MyFlatMatrix[3][0];
    MyMatrix[2][1] = MyFlatMatrix[3][1];
    MyMatrix[2][2] = MyFlatMatrix[3][3];

    HFCPtr<HGF2DTransfoModel> pModel;

    if (m_AdaptAsAffine)
        {
        // The projection parameters must be null
        HASSERT(MyMatrix[2][0] == 0.0);
        HASSERT(MyMatrix[2][1] == 0.0);

        // create model
        HFCPtr<HGF2DAffine> pAffine = new HGF2DAffine();
        pAffine->SetByMatrixParameters(MyMatrix[0][2],
                                       MyMatrix[0][0],
                                       MyMatrix[0][1],
                                       MyMatrix[1][2],
                                       MyMatrix[1][0],
                                       MyMatrix[1][1]);


        //    HFCPtr<HGF2DTransfoModel> pModel = new HGF2DAffine(MyMatrix);
        pModel = pAffine;
        }
    else
        {
        // create model
        HFCPtr<HGF2DProjective> pProjective = new HGF2DProjective();
        pProjective->SetByMatrix(MyMatrix);

        //    HFCPtr<HGF2DTransfoModel> pModel = new HGF2DProjective(MyMatrix);
        pModel = pProjective;
        }

    return pModel;
    }

