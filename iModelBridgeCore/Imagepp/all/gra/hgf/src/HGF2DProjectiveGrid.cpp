//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hgf/src/HGF2DProjectiveGrid.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HGF2DProjectiveGrid
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HGF2DIdentity.h>
#include <Imagepp/all/h/HGF2DSimilitude.h>
#include <Imagepp/all/h/HGF2DTranslation.h>
#include <Imagepp/all/h/HGF2DStretch.h>
#include <Imagepp/all/h/HGF2DProjective.h>
#include <ImagePPInternal/ext/MatrixFromTiePts/MatrixFromTiePts.h>
#include <Imagepp/all/h/HGF2DPosition.h>

#include <Imagepp/all/h/HGF2DProjectiveGrid.h>


//-----------------------------------------------------------------------------
// Default Constructor
//-----------------------------------------------------------------------------
HGF2DProjectiveGrid::HGF2DProjectiveGrid()
    : HGF2DTransfoModelAdapter(HGF2DIdentity())
    {
    m_pPreTransfoModel      = new HGF2DIdentity();
    m_pPostTransfoModel     = new HGF2DIdentity();

    m_DirectStep = 0;
    m_InverseStep = 0;

    m_NumberOfRows    = 10;
    m_NumberOfColumns = 10;

    m_DirectModelsPresent = false;
    m_InverseModelsPresent = false;

    // Allocate arrays of model pointers
    m_pArrayOfDirectModels  = new HFCPtr<HGF2DTransfoModel>[m_NumberOfRows * m_NumberOfColumns];
    m_pArrayOfInverseModels = new HFCPtr<HGF2DTransfoModel>[m_NumberOfRows * m_NumberOfColumns];


    Prepare();

    HINVARIANTS;
    }


/** -----------------------------------------------------------------------------
    Creates a projective grid transformation model adapter model based on the given model.
    A copy of the given model is performed and kept internally in the
    transformation model adapter.

    The projective grid adapter samples the transformation of the model at need
    to generate a local projective based on a grid of pi_DirectStep by pi_DirectStep
    area in the direct channel direction and the equivalent operation using pi_InverseStep
    for the inverse channel. These steps are used in both X and Y directions. These steps
    must be greater than 0.0

    The adapted model must be non-linear.

    @param pi_rNonLinearTransfoModel Constant reference to a transformation model
                                     to adapt. A copy of this model is done.

    @param pi_DirectStep  The sampling step used in the sampling of the transformation
                          in direct raw unit.

    @param pi_InverseStep  The sampling step used in the sampling of the transformation
                           in inverse raw unit.


    -----------------------------------------------------------------------------
*/
HGF2DProjectiveGrid::HGF2DProjectiveGrid(const HGF2DTransfoModel& pi_rNonLinearTransfoModel,
                                         double                  pi_DirectStep,
                                         double                  pi_InverseStep)
    : HGF2DTransfoModelAdapter(pi_rNonLinearTransfoModel)

    {
    // The steps must not be null nor negative
    HPRECONDITION(pi_DirectStep > 0.0);
    HPRECONDITION(pi_InverseStep > 0.0);

    // The given model must not preserve linearity
    HPRECONDITION(!pi_rNonLinearTransfoModel.PreservesLinearity());

    m_pPreTransfoModel      = new HGF2DIdentity();
    m_pPostTransfoModel     = new HGF2DIdentity();

    m_DirectStep            = pi_DirectStep;
    m_InverseStep           = pi_InverseStep;

    m_NumberOfRows    = 10;
    m_NumberOfColumns = 10;

    // Acceleration attributes
    m_DirectModelsPresent = false;
    m_InverseModelsPresent = false;

    // Allocate arrays of model pointers
    m_pArrayOfDirectModels  = new HFCPtr<HGF2DTransfoModel>[m_NumberOfRows * m_NumberOfColumns];
    m_pArrayOfInverseModels = new HFCPtr<HGF2DTransfoModel>[m_NumberOfRows * m_NumberOfColumns];

    HINVARIANTS;

    }


/** -----------------------------------------------------------------------------
    PROTECTED
    For internal use only.
    Creates a projective grid transformation model adapter model based on the given model.
    A copy of the given model is performed and kept internally in the
    transformation model adapter.

    The projective grid adapter samples the transformation of the model at need
    to generate a local projective based on a grid of pi_DirectStep by pi_DirectStep
    area in the direct channel direction and the equivalent operation using pi_InverseStep
    for the inverse channel. These steps are used in both X and Y directions. These steps
    must be greater than 0.0

    The adapted model must be non-linear.

    Contrarly to public constructor, the method requires a pre and post transformation
    models that must be linearity preserving. These models are used to generate
    composition with other linear models without increasing the complexity
    of the result model. Copies of each of these models is performed.

    @param pi_rNonLinearTransfoModel Constant reference to a transformation model
                                     to adapt. A copy of this model is done.

    @param pi_rPreTransfoModel A linearity preserving transformation model
                               that describes the transformation to apply before
                               the non linear model adapted.

    @param pi_rPostTransfoModel A linearity preserving transformation model
                                that describes the transformation to apply after
                                the non linear model adapted.

    @param pi_DirectStep  The sampling step used in the sampling of the transformation
                          in direct raw unit.

    @param pi_InverseStep  The sampling step used in the sampling of the transformation
                           in inverse raw unit.


    -----------------------------------------------------------------------------
*/
HGF2DProjectiveGrid::HGF2DProjectiveGrid(const HGF2DTransfoModel& pi_rNonLinearTransfoModel,
                                         const HGF2DTransfoModel& pi_rPreTransfo,
                                         const HGF2DTransfoModel& pi_rPostTransfo,
                                         double                  pi_DirectStep,
                                         double                  pi_InverseStep)
    : HGF2DTransfoModelAdapter(pi_rNonLinearTransfoModel)

    {
    // The steps must not be null nor negative
    HPRECONDITION(pi_DirectStep > 0.0);
    HPRECONDITION(pi_InverseStep > 0.0);

    // The pre and post model must preserve linearity
    HPRECONDITION(pi_rPreTransfo.PreservesLinearity());
    HPRECONDITION(pi_rPostTransfo.PreservesLinearity());

    // The given model must not preserve linearity
    HPRECONDITION(!pi_rNonLinearTransfoModel.PreservesLinearity());



    m_pPreTransfoModel       = pi_rPreTransfo.Clone();
    m_pPostTransfoModel      = pi_rPostTransfo.Clone();
    m_DirectStep             = pi_DirectStep;
    m_InverseStep            = pi_InverseStep;

    m_NumberOfRows    = 10;
    m_NumberOfColumns = 10;

    // Acceleration attributes
    m_DirectModelsPresent = false;
    m_InverseModelsPresent = false;


    // Allocate arrays of model pointers
    m_pArrayOfDirectModels  = new HFCPtr<HGF2DTransfoModel>[m_NumberOfRows * m_NumberOfColumns];
    m_pArrayOfInverseModels = new HFCPtr<HGF2DTransfoModel>[m_NumberOfRows * m_NumberOfColumns];

    HINVARIANTS;
    }





/** -----------------------------------------------------------------------------
    This is the copy constructor. It copies the state of the given projective
    grid adapter. Note that copies of all transformation models is performed.

    @param pi_rObj  Constant reference to a projective grid adapter to copy state
                    from.

    -----------------------------------------------------------------------------
*/
HGF2DProjectiveGrid::HGF2DProjectiveGrid(const HGF2DProjectiveGrid& pi_rObj)
    : HGF2DTransfoModelAdapter(pi_rObj)
    {
    Copy (pi_rObj);

    HINVARIANTS;
    }

//-----------------------------------------------------------------------------
// The destructor.
//-----------------------------------------------------------------------------
HGF2DProjectiveGrid::~HGF2DProjectiveGrid()
    {
    // Destroy array of models
    delete [] m_pArrayOfDirectModels;
    delete [] m_pArrayOfInverseModels;
    }

/** -----------------------------------------------------------------------------
    This is the assignment operator. It copies the state of the given projective
    grid adapter. Note that copies of all transformation models is performed.

    @param pi_rObj  Constant reference to a projective grid adapter to copy state from.

    @return Returns a reference to self to be used as an l-value.
    -----------------------------------------------------------------------------
*/
HGF2DProjectiveGrid& HGF2DProjectiveGrid::operator=(const HGF2DProjectiveGrid& pi_rObj)
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
StatusInt HGF2DProjectiveGrid::ConvertDirect(double* pio_pXInOut,
                                             double* pio_pYInOut) const
    {
    HINVARIANTS;

    HPRECONDITION(pio_pXInOut != 0);
    HPRECONDITION(pio_pYInOut != 0);

    // Check if stats are compulsed
    return GetDirectModelFromCoordinate(*pio_pXInOut, *pio_pYInOut)->ConvertDirect(pio_pXInOut, pio_pYInOut);
    }


//-----------------------------------------------------------------------------
// Converter (direct)
//-----------------------------------------------------------------------------
StatusInt HGF2DProjectiveGrid::ConvertDirect(double pi_XIn,
                                             double pi_YIn,
                                             double* po_pXOut,
                                             double* po_pYOut) const
    {
    HINVARIANTS;

    HPRECONDITION(po_pXOut != 0);
    HPRECONDITION(po_pYOut != 0);


    return GetDirectModelFromCoordinate(pi_XIn, pi_YIn)->ConvertDirect(pi_XIn, pi_YIn, po_pXOut, po_pYOut);
    }

//-----------------------------------------------------------------------------
// Converter (direct)
//-----------------------------------------------------------------------------
StatusInt HGF2DProjectiveGrid::ConvertDirect(double    pi_YIn,
                                             double    pi_XInStart,
                                             size_t    pi_NumLoc,
                                             double    pi_XInStep,
                                             double*   po_pXOut,
                                             double*   po_pYOut) const
    {
    // Make sure recipient arrays are provided
    HPRECONDITION(po_pXOut != 0);
    HPRECONDITION(po_pYOut != 0);

    StatusInt status = SUCCESS;

    double  X;
    uint32_t Index;
    double* pCurrentX = po_pXOut;
    double* pCurrentY = po_pYOut;

    for (Index = 0, X = pi_XInStart;
         Index < pi_NumLoc ; ++Index, X+=pi_XInStep, ++pCurrentX, ++pCurrentY)
        {
        StatusInt tempStatus = ConvertDirect(X, pi_YIn, pCurrentX, pCurrentY);

        // We will return the first non SUCCESS return status only yet continue on with all coordinates
        if ((SUCCESS != tempStatus) && (SUCCESS == status))
            status = tempStatus;
        }
    return status;
    }

//-----------------------------------------------------------------------------
// Converter (direct)
//-----------------------------------------------------------------------------
StatusInt HGF2DProjectiveGrid::ConvertDirect(size_t    pi_NumLoc,
                                             double*   pio_aXInOut,
                                             double*   pio_aYInOut) const
    {
    HPRECONDITION(pio_aXInOut != 0);
    HPRECONDITION(pio_aYInOut != 0);

    StatusInt status = SUCCESS;

    for (uint32_t i = 0; i < pi_NumLoc; i++)
        {
        StatusInt tempStatus = ConvertDirect(pio_aXInOut[i], pio_aYInOut[i], pio_aXInOut + i, pio_aYInOut + i);

        // We will return the first non SUCCESS return status only yet continue on with all coordinates
        if ((SUCCESS != tempStatus) && (SUCCESS == status))
            status = tempStatus;
        }
    return status;
    }


//-----------------------------------------------------------------------------
// Converter (inverse)
//-----------------------------------------------------------------------------
StatusInt HGF2DProjectiveGrid::ConvertInverse(double* pio_pXInOut,
                                              double* pio_pYInOut) const
    {
    HINVARIANTS;

    HPRECONDITION(pio_pXInOut != 0);
    HPRECONDITION(pio_pYInOut != 0);

    return GetInverseModelFromCoordinate(*pio_pXInOut, *pio_pYInOut)->ConvertDirect(pio_pXInOut, pio_pYInOut);
    }


//-----------------------------------------------------------------------------
// Converter (inverse)
//-----------------------------------------------------------------------------
StatusInt HGF2DProjectiveGrid::ConvertInverse(double pi_XIn,
                                              double pi_YIn,
                                              double* po_pXOut,
                                              double* po_pYOut) const
    {
    HINVARIANTS;

    HPRECONDITION(po_pXOut != 0);
    HPRECONDITION(po_pYOut != 0);

    return GetInverseModelFromCoordinate(pi_XIn, pi_YIn)->ConvertDirect(pi_XIn, pi_YIn, po_pXOut, po_pYOut);

    }

//-----------------------------------------------------------------------------
// Converter (inverse)
//-----------------------------------------------------------------------------
StatusInt HGF2DProjectiveGrid::ConvertInverse(double    pi_YIn,
                                              double    pi_XInStart,
                                              size_t    pi_NumLoc,
                                              double    pi_XInStep,
                                              double*   po_pXOut,
                                              double*   po_pYOut) const
    {
    // Make sure recipient arrays are provided
    HPRECONDITION(po_pXOut != 0);
    HPRECONDITION(po_pYOut != 0);

    StatusInt status = SUCCESS;

    double  X;
    uint32_t Index;
    double* pCurrentX = po_pXOut;
    double* pCurrentY = po_pYOut;

    for (Index = 0, X = pi_XInStart;
         Index < pi_NumLoc ; ++Index, X+=pi_XInStep, ++pCurrentX, ++pCurrentY)
        {
        StatusInt tempStatus = ConvertInverse(X, pi_YIn, pCurrentX, pCurrentY);

        // We will return the first non SUCCESS return status only yet continue on with all coordinates
        if ((SUCCESS != tempStatus) && (SUCCESS == status))
            status = tempStatus;
        }
    return status;
    }


//-----------------------------------------------------------------------------
// Converter (inverse)
//-----------------------------------------------------------------------------
StatusInt HGF2DProjectiveGrid::ConvertInverse(size_t    pi_NumLoc,
                                              double*   pio_aXInOut,
                                              double*   pio_aYInOut) const
    {
    HPRECONDITION(pio_aXInOut != 0);
    HPRECONDITION(pio_aYInOut != 0);

    StatusInt status = SUCCESS;

    for (uint32_t i = 0; i < pi_NumLoc; i++)
        {
        StatusInt tempStatus = ConvertInverse(pio_aXInOut[i], pio_aYInOut[i], pio_aXInOut + i, pio_aYInOut + i);

        // We will return the first non SUCCESS return status only yet continue on with all coordinates
        if ((SUCCESS != tempStatus) && (SUCCESS == status))
            status = tempStatus;
        }
    return status;
    }


//-----------------------------------------------------------------------------
// PreservesLinearity
// Indicate if the transformation model preserves linearity
//-----------------------------------------------------------------------------
bool   HGF2DProjectiveGrid::PreservesLinearity () const
    {
    HINVARIANTS;

    return(false);
    }

//-----------------------------------------------------------------------------
// PreservesParallelism
// Indicate if the transformation model preserves parallelism
//-----------------------------------------------------------------------------
bool   HGF2DProjectiveGrid::PreservesParallelism() const
    {
    HINVARIANTS;

    return(false);
    }

//-----------------------------------------------------------------------------
// PreservesShape
// Indicate if the transformation model preserves the shape
//-----------------------------------------------------------------------------
bool   HGF2DProjectiveGrid::PreservesShape() const
    {
    HINVARIANTS;

    return(false);
    }

//-----------------------------------------------------------------------------
// PreservesDirection
// Indicate if the transformation model preserves directions
//-----------------------------------------------------------------------------
bool   HGF2DProjectiveGrid::PreservesDirection() const
    {
    HINVARIANTS;

    return(false);
    }


//-----------------------------------------------------------------------------
// CanBeRepresentedByAMatrix
// Indicates if the model can be represented by a transformation matrix
//-----------------------------------------------------------------------------
bool HGF2DProjectiveGrid::CanBeRepresentedByAMatrix() const
    {
    HINVARIANTS;
    return(false);
    }


//-----------------------------------------------------------------------------
//  IsIdentity
//  Returns true if the model contains no transformation
//-----------------------------------------------------------------------------
bool HGF2DProjectiveGrid::IsIdentity () const
    {
    HINVARIANTS;

    return(false);
    }

//-----------------------------------------------------------------------------
//  IsStetchable
//  Returns true if the model contains only scaling and translation
//-----------------------------------------------------------------------------
bool HGF2DProjectiveGrid::IsStretchable (double pi_AngleTolerance) const
    {
    HINVARIANTS;

    return(false);
    }


//-----------------------------------------------------------------------------
//  GetStetchParams
//  Returns the stretch parameters
//-----------------------------------------------------------------------------
void HGF2DProjectiveGrid::GetStretchParams(double* po_pScaleFactorX,
                                           double* po_pScaleFactorY,
                                           HGF2DDisplacement* po_pDisplacement) const
    {
    HINVARIANTS;

    HPRECONDITION(po_pScaleFactorX != 0);
    HPRECONDITION(po_pScaleFactorY != 0);
    HPRECONDITION(po_pDisplacement != 0);

    HFCPtr<HGF2DTransfoModel> pModel;

    // Check if direct models are present
    if (m_DirectModelsPresent)
        {
        // Direct models are present ... we use an available coordinate
        // as the middle of extent
        pModel = GetDirectModelFromCoordinate((m_CurrentDirectExtent.GetXMax() + m_CurrentDirectExtent.GetXMin()) / 2.0,
                                              (m_CurrentDirectExtent.GetYMax() + m_CurrentDirectExtent.GetYMin()) / 2.0);

        }
    else
        {
        // Direct models are absent ... we use 0,0 arbitrarily
        // as the middle of extent
        pModel = GetDirectModelFromCoordinate(0.0, 0.0);

        }
    pModel->GetStretchParams(po_pScaleFactorX, po_pScaleFactorY, po_pDisplacement);

    }



//-----------------------------------------------------------------------------
//  GetMatrix
//  Gets the components of the projective by matrix
//-----------------------------------------------------------------------------
HFCMatrix<3, 3> HGF2DProjectiveGrid::GetMatrix() const
    {
    HINVARIANTS;

    // Should not be called
    HASSERT(0);

    HFCMatrix<3, 3> Matrix;

    return(Matrix);

    }

//-----------------------------------------------------------------------------
// Reverse
// This method reverses the transformation model
//-----------------------------------------------------------------------------
void    HGF2DProjectiveGrid::Reverse()
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

    // Swap steps
    double TempStep;
    TempStep = m_DirectStep;
    m_DirectStep = m_InverseStep;
    m_InverseStep = TempStep;

    // Swap acceleration attributes
    bool ModelsPresent;
    ModelsPresent = m_DirectModelsPresent;
    m_DirectModelsPresent = m_InverseModelsPresent;
    m_InverseModelsPresent = ModelsPresent;

    HFCPtr<HGF2DTransfoModel>* pArrayOfModels;
    pArrayOfModels = m_pArrayOfDirectModels;
    m_pArrayOfDirectModels = m_pArrayOfInverseModels;
    m_pArrayOfInverseModels = pArrayOfModels;


    // Invoque reversing of ancester
    // This call will in turn invoque Prepare()
    HGF2DTransfoModelAdapter::Reverse();
    }



//-----------------------------------------------------------------------------
// ComposeInverseWithDirectOf
// Composes a new transformation model as a combination of self and given
//-----------------------------------------------------------------------------
HFCPtr<HGF2DTransfoModel>  HGF2DProjectiveGrid::ComposeInverseWithDirectOf (const HGF2DTransfoModel& pi_rModel) const
    {
    HINVARIANTS;

    // Recipient
    HFCPtr<HGF2DTransfoModel> pResultModel;

    if (pi_rModel.PreservesLinearity())
        {
        // Since the model preserves linearity, it can be composed with post-model
        HFCPtr<HGF2DTransfoModel> pNewPostModel = m_pPostTransfoModel->ComposeInverseWithDirectOf(pi_rModel);

        pResultModel = new HGF2DProjectiveGrid(*m_pAdaptedTransfoModel,
                                               *m_pPreTransfoModel,
                                               *pNewPostModel,
                                               m_DirectStep,
                                               m_InverseStep);
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
HGF2DTransfoModel* HGF2DProjectiveGrid::Clone () const
    {
    HINVARIANTS;

    // Allocate object as copy and return
    return(new HGF2DProjectiveGrid(*this));
    }


//-----------------------------------------------------------------------------
// ComposeYourself
// PRIVATE
//-----------------------------------------------------------------------------
HFCPtr<HGF2DTransfoModel>  HGF2DProjectiveGrid::ComposeYourself (const HGF2DTransfoModel& pi_rModel) const
    {
    HINVARIANTS;

    // Recipient
    HFCPtr<HGF2DTransfoModel> pResultModel;

    if (pi_rModel.IsIdentity())
        {
        pResultModel = new HGF2DProjectiveGrid(*this);
        }

    // Check if the type of the given model can be represented by a matrix
    else if (pi_rModel.PreservesLinearity())
        {

        HFCMatrix<3, 3> MySelfMatrix(m_pPreTransfoModel->GetMatrix());

        // Compose the two matrix together
        HFCPtr<HGF2DTransfoModel> pResultPreModel = new HGF2DProjective();


        ((HGF2DProjective*)(&(*pResultPreModel)))->SetByMatrix(MySelfMatrix * pi_rModel.GetMatrix());

        pResultModel = new HGF2DProjectiveGrid(*m_pAdaptedTransfoModel,
                                               *pResultPreModel,
                                               *m_pPostTransfoModel,
                                               m_DirectStep,
                                               m_InverseStep);
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
void HGF2DProjectiveGrid::Prepare ()
    {
    // Obtain conversion ratio for direct X to inverse X units

    // Invoque preparation of ancester (required)
    HGF2DTransfoModelAdapter::Prepare();


    }


//-----------------------------------------------------------------------------
//  Copy
//  Copy method
//-----------------------------------------------------------------------------
void HGF2DProjectiveGrid::Copy(const HGF2DProjectiveGrid& pi_rObj)
    {
    // Copy master data
    m_pPreTransfoModel       = pi_rObj.m_pPreTransfoModel->Clone();
    m_pPostTransfoModel      = pi_rObj.m_pPostTransfoModel->Clone();

    m_DirectStep = pi_rObj.m_DirectStep;
    m_InverseStep = pi_rObj.m_InverseStep;

    m_NumberOfRows    = pi_rObj.m_NumberOfRows;
    m_NumberOfColumns = pi_rObj.m_NumberOfColumns;

    // The copy does not include acceleration arrays.
    m_DirectModelsPresent = false;
    m_InverseModelsPresent = false;

    // Allocate arrays of model pointers
    m_pArrayOfDirectModels  = new HFCPtr<HGF2DTransfoModel>[m_NumberOfRows * m_NumberOfColumns];
    m_pArrayOfInverseModels = new HFCPtr<HGF2DTransfoModel>[m_NumberOfRows * m_NumberOfColumns];

    }


//-----------------------------------------------------------------------------
// CreateSimplifiedModel
//-----------------------------------------------------------------------------
HFCPtr<HGF2DTransfoModel> HGF2DProjectiveGrid::CreateSimplifiedModel() const
    {
    HINVARIANTS;

    // Declare recipient variable
    HFCPtr<HGF2DTransfoModel> pSimplifiedModel;

    // If we get here, no simplification is possible.
    return pSimplifiedModel;
    }


/** -----------------------------------------------------------------------------
    PRIVATE METHOD
    This method returns the applicable direct projective for the area
    the given coordinate falls in. If the projective in question does not
    exist yet it is generated and kept in the grid buffer.

    @param pi_X The X value of the coordinate to obtain transformation model
                 approximation from in direct value.

    @param pi_Y The Y value of the coordinate to obtain transformation model
                 approximation from in direct value.

    @return Returns the generated projective transformation model.
    -----------------------------------------------------------------------------
*/
const HFCPtr<HGF2DTransfoModel>& HGF2DProjectiveGrid::GetDirectModelFromCoordinate(double pi_X, double pi_Y) const
    {
    // Check if coordinate is part of current extent
    if (!m_DirectModelsPresent || !m_CurrentDirectExtent.IsPointIn(HGF2DPosition(pi_X, pi_Y)))
        {
        // The coordinate is not in current extent ... we must re-create models
        // Clear models
        ClearDirectModels();

        // Calculate the new full extent
        double FullXExtent = m_NumberOfColumns * m_DirectStep;
        double FullYExtent = m_NumberOfRows * m_DirectStep;

        // Compute the extent
        HGF2DLiteExtent CurrentExtent(pi_X - FullXExtent / 2.0,
                                      pi_Y - FullYExtent / 2.0,
                                      pi_X + FullXExtent / 2.0,
                                      pi_Y + FullYExtent / 2.0);

        m_CurrentDirectExtent = CurrentExtent;

        // Indicate models are present
        m_DirectModelsPresent = true;
        }

    // Calculate the row and column specific to coordinate
    int32_t Row = (int32_t)((pi_Y - m_CurrentDirectExtent.GetYMin()) / m_DirectStep);
    int32_t Col = (int32_t)((pi_X - m_CurrentDirectExtent.GetXMin()) / m_DirectStep);

    // Check if model is allocated
    if (m_pArrayOfDirectModels[Row * m_NumberOfColumns + Col] == 0)
        {
        // Model is not allocated ... create
        // Create extent
        HGF2DLiteExtent MyModelExtent(m_CurrentDirectExtent.GetXMin() + (Col * m_DirectStep),
                                      m_CurrentDirectExtent.GetYMin() + (Row * m_DirectStep),
                                      m_CurrentDirectExtent.GetXMin() + ((Col + 1) * m_DirectStep),
                                      m_CurrentDirectExtent.GetYMin() + ((Row + 1) * m_DirectStep));

        m_pArrayOfDirectModels[Row * m_NumberOfColumns + Col] = CreateDirectModelFromExtent(MyModelExtent);
        }

    return(m_pArrayOfDirectModels[Row * m_NumberOfColumns + Col]);
    }

/** -----------------------------------------------------------------------------
    PRIVATE METHOD
    This method returns the applicable inverse projective for the area
    the given coordinate falls in. If the projective in question does not
    exist yet it is generated and kept in the grid buffer.

    @param pi_X The X value of the coordinate to obtain transformation model
                 approximation from in inverse value.

    @param pi_Y The Y value of the coordinate to obtain transformation model
                 approximation from in inverse value.

    @return Returns the generated projective transformation model.
    -----------------------------------------------------------------------------
*/
const HFCPtr<HGF2DTransfoModel>& HGF2DProjectiveGrid::GetInverseModelFromCoordinate(double pi_X, double pi_Y) const
    {
    // Check if coordinate is part of current extent
    if (!m_InverseModelsPresent || !m_CurrentInverseExtent.IsPointIn(HGF2DPosition(pi_X, pi_Y)))
        {
        // The coordinate is not in current extent ... we must re-create models
        // Clear models
        ClearInverseModels();

        // Calculate the new full extent
        double FullXExtent = m_NumberOfColumns * m_InverseStep;
        double FullYExtent = m_NumberOfRows * m_InverseStep;

        // Compute the extent
        HGF2DLiteExtent CurrentExtent(pi_X - FullXExtent / 2.0,
                                      pi_Y - FullYExtent / 2.0,
                                      pi_X + FullXExtent / 2.0,
                                      pi_Y + FullYExtent / 2.0);

        m_CurrentInverseExtent = CurrentExtent;

        // Indicate models are present
        m_InverseModelsPresent = true;
        }

    // Calculate the row and column specific to coordinate
    int32_t Row = (int32_t)((pi_Y - m_CurrentInverseExtent.GetYMin()) / m_InverseStep);
    int32_t Col = (int32_t)((pi_X - m_CurrentInverseExtent.GetXMin()) / m_InverseStep);

    // Check if model is allocated
    if (m_pArrayOfInverseModels[Row * m_NumberOfColumns + Col] == 0)
        {
        // Model is not allocated ... create
        // Create extent
        HGF2DLiteExtent MyModelExtent(m_CurrentInverseExtent.GetXMin() + (Col * m_InverseStep),
                                      m_CurrentInverseExtent.GetYMin() + (Row * m_InverseStep),
                                      m_CurrentInverseExtent.GetXMin() + ((Col + 1) * m_InverseStep),
                                      m_CurrentInverseExtent.GetYMin() + ((Row + 1) * m_InverseStep));

        m_pArrayOfInverseModels[Row * m_NumberOfColumns + Col] = CreateInverseModelFromExtent(MyModelExtent);
        }

    return(m_pArrayOfInverseModels[Row * m_NumberOfColumns + Col]);
    }



/** -----------------------------------------------------------------------------
    PRIVATE METHOD
    This method performs the generation of a direct projective approximation
    by sampling of the transformation of the non-linear model on the four corners
    of given extent.

    @param pi_rExtent The application area over which sampling is performed expressed
                      in direct coordinates.

    @return Returns the generated projective transformation model.
    -----------------------------------------------------------------------------
*/
HFCPtr<HGF2DTransfoModel> HGF2DProjectiveGrid::CreateDirectModelFromExtent(const HGF2DLiteExtent& pi_rExtent) const
    {
    double TiePoints[24];

    // Process lower left corner
    TiePoints[0] = pi_rExtent.GetXMin();
    TiePoints[1] = pi_rExtent.GetYMin();
    TiePoints[2] = 0.0;

    // Apply the pre-transformation
    double TempX;
    double TempY;


    m_pPreTransfoModel->ConvertDirect(TiePoints[0], TiePoints[1], &TempX, &TempY);

    // Apply geographic transformation
    m_pAdaptedTransfoModel->ConvertDirect(&TempX, &TempY);

    // Apply the post-transformation
    m_pPostTransfoModel->ConvertDirect(&TempX, &TempY);

    TiePoints[3] = TempX;
    TiePoints[4] = TempY;
    TiePoints[5] = 0.0;

    // Process upper left corner
    TiePoints[6] = pi_rExtent.GetXMin();
    TiePoints[7] = pi_rExtent.GetYMax();
    TiePoints[8] = 0.0;

    // Apply the pre-transformation
    m_pPreTransfoModel->ConvertDirect(TiePoints[6], TiePoints[7], &TempX, &TempY);

    // Apply geographic transformation
    m_pAdaptedTransfoModel->ConvertDirect(&TempX, &TempY);

    // Apply the post-transformation
    m_pPostTransfoModel->ConvertDirect(&TempX, &TempY);

    TiePoints[9] = TempX;
    TiePoints[10] = TempY;
    TiePoints[11] = 0.0;

    // Process upper right corner
    TiePoints[12] = pi_rExtent.GetXMax();
    TiePoints[13] = pi_rExtent.GetYMax();
    TiePoints[14] = 0.0;

    // Apply the pre-transformation
    m_pPreTransfoModel->ConvertDirect(TiePoints[12], TiePoints[13], &TempX, &TempY);

    // Apply geographic transformation
    // Apply geographic transformation
    m_pAdaptedTransfoModel->ConvertDirect(&TempX, &TempY);

    // Apply the post-transformation
    m_pPostTransfoModel->ConvertDirect(&TempX, &TempY);

    TiePoints[15] = TempX;
    TiePoints[16] = TempY;
    TiePoints[17] = 0.0;


    // Process lower right corner
    TiePoints[18] = pi_rExtent.GetXMax();
    TiePoints[19] = pi_rExtent.GetYMin();
    TiePoints[20] = 0.0;

    // Apply the pre-transformation
    m_pPreTransfoModel->ConvertDirect(TiePoints[18], TiePoints[19], &TempX, &TempY);

    // Apply geographic transformation
    // Apply geographic transformation
    m_pAdaptedTransfoModel->ConvertDirect(&TempX, &TempY);

    // Apply the post-transformation
    m_pPostTransfoModel->ConvertDirect(&TempX, &TempY);

    TiePoints[21] = TempX;
    TiePoints[22] = TempY;
    TiePoints[23] = 0.0;

    double MyFlatMatrix[4][4];

    // Create matrix
    GetProjectiveTransfoMatrixFromScaleAndTiePts(MyFlatMatrix, 24, TiePoints);

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

    // create model
    HFCPtr<HGF2DTransfoModel> pModel = new HGF2DProjective(MyMatrix);

    return pModel;
    }

/** -----------------------------------------------------------------------------
    PRIVATE METHOD
    This method performs the generation of a inverse projective approximation
    by sampling of the transformation of the non-linear model on the four corners
    of given extent.

    @param pi_rExtent The application area over which sampling is performed expressed
                      in inverse coordinates.

    @return Returns the generated projective transformation model.
    -----------------------------------------------------------------------------
*/
HFCPtr<HGF2DTransfoModel> HGF2DProjectiveGrid::CreateInverseModelFromExtent(const HGF2DLiteExtent& pi_rExtent) const
    {
#if (0)
    double TiePoints[24];

    // Process lower left corner
    TiePoints[0] = pi_rExtent.GetXMin();
    TiePoints[1] = pi_rExtent.GetYMin();
    TiePoints[2] = 0.0;

    // Apply the pre-transformation
    double TempX;
    double TempY;

    m_pPostTransfoModel->ConvertInverse(TiePoints[0], TiePoints[1], &TempX, &TempY);

    // Apply geographic transformation
    m_pAdaptedTransfoModel->ConvertInverse(&TempX, &TempY);

    // Apply the post-transformation
    m_pPreTransfoModel->ConvertInverse(&TempX, &TempY);

    TiePoints[3] = TempX;
    TiePoints[4] = TempY;
    TiePoints[5] = 0.0;

    // Process upper left corner
    TiePoints[6] = pi_rExtent.GetXMin();
    TiePoints[7] = pi_rExtent.GetYMax();
    TiePoints[8] = 0.0;

    // Apply the pre-transformation
    m_pPostTransfoModel->ConvertInverse(TiePoints[6], TiePoints[7], &TempX, &TempY);

    // Apply geographic transformation
    m_pAdaptedTransfoModel->ConvertInverse(&TempX, &TempY);

    // Apply the post-transformation
    m_pPreTransfoModel->ConvertInverse(&TempX, &TempY);


    TiePoints[9] = TempX;
    TiePoints[10] = TempY;
    TiePoints[11] = 0.0;

    // Process upper right corner
    TiePoints[12] = pi_rExtent.GetXMax();
    TiePoints[13] = pi_rExtent.GetYMax();
    TiePoints[14] = 0.0;

    // Apply the pre-transformation
    m_pPostTransfoModel->ConvertInverse(TiePoints[12], TiePoints[13], &TempX, &TempY);

    // Apply geographic transformation
    m_pAdaptedTransfoModel->ConvertInverse(&TempX, &TempY);

    // Apply the post-transformation
    m_pPreTransfoModel->ConvertInverse(&TempX, &TempY);

    TiePoints[15] = TempX;
    TiePoints[16] = TempY;
    TiePoints[17] = 0.0;


    // Process lower right corner
    TiePoints[18] = pi_rExtent.GetXMax();
    TiePoints[19] = pi_rExtent.GetYMin();
    TiePoints[20] = 0.0;

    // Apply the pre-transformation
    m_pPostTransfoModel->ConvertInverse(TiePoints[18], TiePoints[19], &TempX, &TempY);

    // Apply geographic transformation
    m_pAdaptedTransfoModel->ConvertInverse(&TempX, &TempY);

    // Apply the post-transformation
    m_pPreTransfoModel->ConvertInverse(&TempX, &TempY);

    TiePoints[21] = TempX;
    TiePoints[22] = TempY;
    TiePoints[23] = 0.0;

    double MyFlatMatrix[4][4];

    // Create matrix
    GetProjectiveTransfoMatrixFromScaleAndTiePts(MyFlatMatrix, 24, TiePoints);

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

    // create model
    HFCPtr<HGF2DTransfoModel> pModel = new HGF2DProjective(MyMatrix);

    return pModel;
#else

    double TempX;
    double TempY;

    // Obtain the center of extent coordinate
    double ExtentCenterX = ((pi_rExtent.GetXMax() + pi_rExtent.GetXMin()) / 2.0);
    double ExtentCenterY = ((pi_rExtent.GetYMax() + pi_rExtent.GetYMin()) / 2.0);

    // Transform center using non-linear model
    m_pPostTransfoModel->ConvertInverse(ExtentCenterX, ExtentCenterY, &TempX, &TempY);

    // Apply geographic transformation
    m_pAdaptedTransfoModel->ConvertInverse(&TempX, &TempY);

    // Apply the post-transformation
    m_pPreTransfoModel->ConvertInverse(&TempX, &TempY);

    // Obtain transformation model direct at this position
    const HFCPtr<HGF2DTransfoModel>& rpDirectModel = HGF2DProjectiveGrid::GetDirectModelFromCoordinate(TempX, TempY);

    // Clone model
    HFCPtr<HGF2DTransfoModel> pNewModel = rpDirectModel->Clone();

    // Reverse model
    pNewModel->Reverse();

    return(pNewModel);
#endif
    }




/** -----------------------------------------------------------------------------
    PRIVATE METHOD
    This method clears the projective grid cache for all direct projectives
    already calculated
    -----------------------------------------------------------------------------
*/
void HGF2DProjectiveGrid::ClearDirectModels() const
    {
    HFCPtr<HGF2DTransfoModel> NullPointer;

    // Prepare for each blocks
    for (uint32_t j = 0 ; j < m_NumberOfRows ; ++j)
        {
        // Fill in the new row
        for (uint32_t i = 0 ; i < m_NumberOfColumns ; ++i)
            {
            // Check if there is a model defined
            if (m_pArrayOfDirectModels[j * m_NumberOfColumns + i] != NullPointer)
                {
                // The model is present ... destroy
                m_pArrayOfDirectModels[j * m_NumberOfColumns + i] = NullPointer;
                }
            }
        }

    m_DirectModelsPresent = false;
    }

/** -----------------------------------------------------------------------------
    PRIVATE METHOD
    This method clears the projective grid cache for all inverse projectives
    already calculated
    -----------------------------------------------------------------------------
*/
void HGF2DProjectiveGrid::ClearInverseModels() const
    {
    HFCPtr<HGF2DTransfoModel> NullPointer;

    // Prepare for each blocks
    for (uint32_t j = 0 ; j < m_NumberOfRows ; ++j)
        {
        // Fill in the new row
        for (uint32_t i = 0 ; i < m_NumberOfColumns ; ++i)
            {
            // Check if there is a model defined
            if (m_pArrayOfInverseModels[j * m_NumberOfColumns + i] != NullPointer)
                {
                // The model is present ... destroy
                m_pArrayOfInverseModels[j * m_NumberOfColumns + i] = NullPointer;
                }
            }
        }

    m_InverseModelsPresent = false;
    }


