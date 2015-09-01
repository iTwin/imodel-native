//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hgf/src/HGF2DLocalProjectiveGrid.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HGF2DLocalProjectiveGrid
//-----------------------------------------------------------------------------
#include <ImagePPInternal/hstdcpp.h>
#define NO_IODEFS



#include <Imagepp/all/h/HGF2DLocalProjectiveGrid.h>
#include <Imagepp/all/h/HGF2DIdentity.h>
#include <Imagepp/all/h/HGF2DProjective.h>
#include <ImagePPInternal/ext/MatrixFromTiePts/MatrixFromTiePts.h>
#include <Imagepp/all/h/HGFException.h>
#include <Imagepp/all/h/HGF2DGridModel.h>
#include <Imagepp/all/h/HGF2DBoundaryModel.h>
#include <Imagepp/all/h/HGF2DPieceWiseModel.h>
#include <Imagepp/all/h/HGF2DGridModelBooster.h>
#include <Imagepp/all/h/HGF2DBoundaryModelBooster.h>


#define DEFAULT_NUMBER_OF_ROWS (10)
#define DEFAULT_NUMBER_OF_COLS (10)

//-----------------------------------------------------------------------------
// Default Constructor
//-----------------------------------------------------------------------------
HGF2DLocalProjectiveGrid::HGF2DLocalProjectiveGrid()
    : HGF2DTransfoModelAdapter(HGF2DIdentity())
    {
    m_Direct = true;
    m_useGlobalAffineApproximation = true;

    m_pPreTransfoModel      = new HGF2DIdentity();
    m_pPostTransfoModel     = new HGF2DIdentity();

    m_pGlobalAffine = NULL;

    // Default extent
    m_directExtent = HGF2DLiteExtent(0.0, 0.0, 1.0, 1.0);

    m_nColumns = (DEFAULT_NUMBER_OF_COLS);
    m_nRows    = (DEFAULT_NUMBER_OF_ROWS);

    m_pGridModel            = new HGF2DGridModel(HGF2DIdentity(), m_directExtent, DEFAULT_NUMBER_OF_ROWS, DEFAULT_NUMBER_OF_COLS);
    m_pGridModelBooster     = m_pGridModel;
    m_pBoundaryModel        = new HGF2DBoundaryModel(m_directExtent, ComputeTransitExtent(), m_pGridModel, GetGlobalAffine(), m_nColumns, m_nRows);
    m_pBoundaryModelBooster = m_pBoundaryModel;

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
HGF2DLocalProjectiveGrid::HGF2DLocalProjectiveGrid(const HGF2DTransfoModel& pi_rNonLinearTransfoModel,
                                                   const HGF2DLiteExtent&   pi_rAdaptedDirectExtent,
                                                   uint32_t                 pi_NumOfTilesX,
                                                   uint32_t                 pi_NumOfTilesY)
    : HGF2DTransfoModelAdapter(pi_rNonLinearTransfoModel)
    {
    // Number of tiles must be > 0
    HPRECONDITION (pi_NumOfTilesX > 0);
    HPRECONDITION (pi_NumOfTilesY > 0);

    // The given model must not preserve linearity
    HPRECONDITION(!pi_rNonLinearTransfoModel.PreservesLinearity());

    m_Direct = true;
    m_useGlobalAffineApproximation = true;

    m_pPreTransfoModel      = new HGF2DIdentity();
    m_pPostTransfoModel     = new HGF2DIdentity();

    m_pGlobalAffine = NULL;
    m_directExtent  = pi_rAdaptedDirectExtent;
    m_nColumns = pi_NumOfTilesX;
    m_nRows    = pi_NumOfTilesY;

    // Create Grid model
    m_pGridModel            = new HGF2DGridModel(pi_rNonLinearTransfoModel, m_directExtent, pi_NumOfTilesX, pi_NumOfTilesY);
    m_pGridModelBooster     = m_pGridModel;
    m_pBoundaryModel        = new HGF2DBoundaryModel(m_directExtent, ComputeTransitExtent(), m_pGridModel, GetGlobalAffine(), pi_NumOfTilesX, pi_NumOfTilesY);
    m_pBoundaryModelBooster = m_pBoundaryModel;

    HINVARIANTS;
    }

/** -----------------------------------------------------------------------------
    Creates a projective grid transformation model adapter model.

    @param pi_rGlobalAffine
    @param pi_rAdaptedDirectExtent  The grid extent

    @param pi_NumOfTilesX           The sampling step used in the sampling of the
                                    transformation in direct raw unit.

    @param pi_NumOfTilesY           The sampling step used in the sampling of the
                                    transformation in inverse raw unit.

    @param pi_rModelList            List of projectives. The list must be set by row,
                                    Proj00, Proj10, Proj20...Projn0,
                                    Proj01, Proj11, Proj21...Projn1
                                    ...
                                    Proj0m, Proj1m, Proj2m...Projnm

                                    where n = pi_NumOfTilesX - 1
                                          m = pi_NumOfTilesY - 1

    -----------------------------------------------------------------------------
*/
HGF2DLocalProjectiveGrid::HGF2DLocalProjectiveGrid(const HGF2DAffine&       pi_rGlobalAffine,
                                                   const HGF2DLiteExtent&   pi_rAdaptedDirectExtent,
                                                   uint32_t                 pi_NumOfTilesX,
                                                   uint32_t                 pi_NumOfTilesY,
                                                   const list<HFCPtr<HGF2DTransfoModel> >&
                                                   pi_rModelList)
    : HGF2DTransfoModelAdapter(HGF2DIdentity())
    {
    HPRECONDITION(pi_NumOfTilesX > 0 && pi_NumOfTilesY > 0);
    HPRECONDITION(pi_rModelList.size() == pi_NumOfTilesX * pi_NumOfTilesY);

    m_Direct = true;
    m_useGlobalAffineApproximation = true;

    m_pPreTransfoModel      = new HGF2DIdentity();
    m_pPostTransfoModel     = new HGF2DIdentity();

    m_pGlobalAffine = pi_rGlobalAffine.Clone();

    m_directExtent = pi_rAdaptedDirectExtent;
    m_nColumns = pi_NumOfTilesX;
    m_nRows    = pi_NumOfTilesY;

    m_pGridModel            = new HGF2DGridModel(m_directExtent, pi_NumOfTilesX, pi_NumOfTilesY, pi_rModelList);
    m_pGridModelBooster     = m_pGridModel;
    m_pBoundaryModel        = new HGF2DBoundaryModel(m_directExtent, ComputeTransitExtent(), m_pGridModel, GetGlobalAffine(), pi_NumOfTilesX, pi_NumOfTilesY);
    m_pBoundaryModelBooster = m_pBoundaryModel;

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
HGF2DLocalProjectiveGrid::HGF2DLocalProjectiveGrid (bool                     pi_Mode,
                                                    const HGF2DTransfoModel& pi_rNonLinearTransfoModel,
                                                    const HGF2DTransfoModel& pi_rPreTransfo,
                                                    const HGF2DTransfoModel& pi_rPostTransfo,
                                                    HFCPtr<HGF2DGridModel>   pi_gridModel,
                                                    HFCPtr<HGF2DBoundaryModel> pi_boundaryModel,
                                                    HGF2DLiteExtent          pi_AdaptedDirectExtent,
                                                    uint32_t                 pi_NumOfTilesX,
                                                    uint32_t                 pi_NumOfTilesY)
    : HGF2DTransfoModelAdapter(pi_rNonLinearTransfoModel)
    {
    // Number of tiles must be > 0
    HPRECONDITION (pi_NumOfTilesX > 0);
    HPRECONDITION (pi_NumOfTilesY > 0);

    // The pre and post model must preserve linearity
    HPRECONDITION(pi_rPreTransfo.PreservesLinearity());
    HPRECONDITION(pi_rPostTransfo.PreservesLinearity());

    // The given model must not preserve linearity
    HPRECONDITION(!pi_rNonLinearTransfoModel.PreservesLinearity());

    m_Direct = pi_Mode;
    m_useGlobalAffineApproximation = true;

    m_pPreTransfoModel       = pi_rPreTransfo.Clone();
    m_pPostTransfoModel      = pi_rPostTransfo.Clone();
    m_pGlobalAffine = NULL;

    m_directExtent = pi_AdaptedDirectExtent;
    m_nColumns = pi_NumOfTilesX;
    m_nRows    = pi_NumOfTilesY;

    m_pGridModel        = pi_gridModel;
    m_pGridModelBooster = new HGF2DGridModelBooster(m_pGridModel, m_pPreTransfoModel, m_pPostTransfoModel);

    m_pBoundaryModel        = pi_boundaryModel;
    m_pBoundaryModelBooster = new HGF2DBoundaryModelBooster(m_pBoundaryModel, m_pPreTransfoModel, m_pPostTransfoModel);


    HINVARIANTS;
    }


/** -----------------------------------------------------------------------------
    This is the copy constructor. It copies the state of the given projective
    grid adapter. Note that copies of all transformation models is performed.

    @param pi_rObj  Constant reference to a projective grid adapter to copy state
                    from.

    -----------------------------------------------------------------------------
*/
HGF2DLocalProjectiveGrid::HGF2DLocalProjectiveGrid(const HGF2DLocalProjectiveGrid& pi_rObj)
    : HGF2DTransfoModelAdapter(pi_rObj)
    {
    Copy (pi_rObj);

    HINVARIANTS;
    }

//-----------------------------------------------------------------------------
// The destructor.
//-----------------------------------------------------------------------------
HGF2DLocalProjectiveGrid::~HGF2DLocalProjectiveGrid()
    {
    }

/** -----------------------------------------------------------------------------
    This is the assignment operator. It copies the state of the given projective
    grid adapter. Note that copies of all transformation models is performed.

    @param pi_rObj  Constant reference to a projective grid adapter to copy state from.

    @return Returns a reference to self to be used as an l-value.
    -----------------------------------------------------------------------------
*/
HGF2DLocalProjectiveGrid& HGF2DLocalProjectiveGrid::operator=(const HGF2DLocalProjectiveGrid& pi_rObj)
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
StatusInt HGF2DLocalProjectiveGrid::ConvertDirect(double* pio_pXInOut,
                                                  double* pio_pYInOut) const
    {
    HINVARIANTS;

    HPRECONDITION(pio_pXInOut != 0);
    HPRECONDITION(pio_pYInOut != 0);

    if (m_Direct)
        {
        return GetComposedModelFromCoordinate(*pio_pXInOut, *pio_pYInOut)->ConvertDirect(pio_pXInOut, pio_pYInOut);
        }
    else
        {
        return GetComposedModelFromInverseCoordinate(*pio_pXInOut, *pio_pYInOut)->ConvertInverse(pio_pXInOut, pio_pYInOut);
        }
    }


//-----------------------------------------------------------------------------
// Converter (direct)
//-----------------------------------------------------------------------------
StatusInt HGF2DLocalProjectiveGrid::ConvertDirect(double pi_XIn,
                                                  double pi_YIn,
                                                  double* po_pXOut,
                                                  double* po_pYOut) const
    {
    HINVARIANTS;

    HPRECONDITION(po_pXOut != 0);
    HPRECONDITION(po_pYOut != 0);

    if (m_Direct)
        {
        return GetComposedModelFromCoordinate(pi_XIn, pi_YIn)->ConvertDirect(pi_XIn, pi_YIn, po_pXOut, po_pYOut);
        }
    else
        {
        return GetComposedModelFromInverseCoordinate(pi_XIn, pi_YIn)->ConvertInverse(pi_XIn, pi_YIn, po_pXOut, po_pYOut);
        }
    }

//-----------------------------------------------------------------------------
// Converter (direct)
//-----------------------------------------------------------------------------
StatusInt HGF2DLocalProjectiveGrid::ConvertDirect(double    pi_YIn,
                                                  double    pi_XInStart,
                                                  size_t    pi_NumLoc,
                                                  double    pi_XInStep,
                                                  double*   po_pXOut,
                                                  double*   po_pYOut) const
    {
    // Make sure recipient arrays are provided
    HPRECONDITION(po_pXOut != 0);
    HPRECONDITION(po_pYOut != 0);

    double  X;
    uint32_t Index;
    double* pCurrentX = po_pXOut;
    double* pCurrentY = po_pYOut;

    StatusInt status = SUCCESS;

    for (Index = 0, X = pi_XInStart;
         Index < pi_NumLoc;
         ++Index, X+=pi_XInStep, ++pCurrentX, ++pCurrentY)
        {
        StatusInt tempStatus = ConvertDirect(X, pi_YIn, pCurrentX, pCurrentY);

        if ((SUCCESS != tempStatus) && (SUCCESS == status))
            status = tempStatus;
        }

    return status;
    }

//-----------------------------------------------------------------------------
// Converter (direct)
//-----------------------------------------------------------------------------
StatusInt HGF2DLocalProjectiveGrid::ConvertDirect(size_t pi_NumLoc, 
                                                  double* pio_aXInOut,
                                                  double* pio_aYInOut) const
    {
    // Make sure recipient arrays are provided
    HPRECONDITION(pio_aXInOut != 0);
    HPRECONDITION(pio_aYInOut != 0);

    StatusInt status = SUCCESS;

    for(uint32_t i = 0; i < pi_NumLoc; i++)
        {
        StatusInt tempStatus = ConvertDirect(pio_aXInOut[i], pio_aYInOut[i], pio_aXInOut + i, pio_aYInOut + i);

        if ((SUCCESS != tempStatus) && (SUCCESS == status))
            status = tempStatus;
        }

    return status;
    }


//-----------------------------------------------------------------------------
// Converter (inverse)
//-----------------------------------------------------------------------------
StatusInt HGF2DLocalProjectiveGrid::ConvertInverse(double* pio_pXInOut,
                                                   double* pio_pYInOut) const
    {
    HINVARIANTS;

    HPRECONDITION(pio_pXInOut != 0);
    HPRECONDITION(pio_pYInOut != 0);

    if (m_Direct)
        {
        return GetComposedModelFromInverseCoordinate(*pio_pXInOut, *pio_pYInOut)->ConvertInverse(pio_pXInOut, pio_pYInOut);
        }
    else
        {
        return GetComposedModelFromCoordinate(*pio_pXInOut, *pio_pYInOut)->ConvertDirect(pio_pXInOut, pio_pYInOut);
        }
    }


//-----------------------------------------------------------------------------
// Converter (inverse)
//-----------------------------------------------------------------------------
StatusInt HGF2DLocalProjectiveGrid::ConvertInverse(double   pi_XIn,
                                                   double   pi_YIn,
                                                   double*  po_pXOut,
                                                   double*  po_pYOut) const
    {
    HINVARIANTS;

    HPRECONDITION(po_pXOut != 0);
    HPRECONDITION(po_pYOut != 0);

    if (m_Direct)
        {
        return GetComposedModelFromInverseCoordinate(pi_XIn, pi_YIn)->ConvertInverse(pi_XIn, pi_YIn, po_pXOut, po_pYOut);
        }
    else
        {
        return GetComposedModelFromCoordinate(pi_XIn, pi_YIn)->ConvertDirect(pi_XIn, pi_YIn, po_pXOut, po_pYOut);
        }
    }

//-----------------------------------------------------------------------------
// Converter (inverse)
//-----------------------------------------------------------------------------
StatusInt HGF2DLocalProjectiveGrid::ConvertInverse(double    pi_YIn,
                                                   double    pi_XInStart,
                                                   size_t    pi_NumLoc,
                                                   double    pi_XInStep,
                                                   double*   po_pXOut,
                                                   double*   po_pYOut) const
    {
    // Make sure recipient arrays are provided
    HPRECONDITION(po_pXOut != 0);
    HPRECONDITION(po_pYOut != 0);

    double  X;
    uint32_t Index;
    double* pCurrentX = po_pXOut;
    double* pCurrentY = po_pYOut;

    StatusInt status = SUCCESS;

    for (Index = 0, X = pi_XInStart;
         Index < pi_NumLoc ; ++Index, X+=pi_XInStep, ++pCurrentX, ++pCurrentY)
        {
        StatusInt tempStatus = ConvertInverse(X, pi_YIn, pCurrentX, pCurrentY);

        if ((SUCCESS != tempStatus) && (SUCCESS == status))
            status = tempStatus;
        }

    return status;
    }


//-----------------------------------------------------------------------------
// Converter (inverse)
//-----------------------------------------------------------------------------
StatusInt HGF2DLocalProjectiveGrid::ConvertInverse(size_t pi_NumLoc, 
                                                   double* pio_aXInOut,
                                                   double* pio_aYInOut) const
    {
    // Make sure recipient arrays are provided
    HPRECONDITION(pio_aXInOut != 0);
    HPRECONDITION(pio_aYInOut != 0);

    StatusInt status = SUCCESS;

    for(uint32_t i = 0; i < pi_NumLoc; i++)
        {
        StatusInt tempStatus = ConvertInverse(pio_aXInOut[i], pio_aYInOut[i], pio_aXInOut + i, pio_aYInOut + i);

        if ((SUCCESS != tempStatus) && (SUCCESS == status))
            status = tempStatus;
        }

    return status;
    }


//-----------------------------------------------------------------------------
//  IsIdentity
//  Returns true if the model contains no transformation
//-----------------------------------------------------------------------------
bool HGF2DLocalProjectiveGrid::IsIdentity () const
    {
    HINVARIANTS;

    return(false);
    }

//-----------------------------------------------------------------------------
//  IsStetchable
//  Returns true if the model contains only scaling and translation
//-----------------------------------------------------------------------------
bool HGF2DLocalProjectiveGrid::IsStretchable (double pi_AngleTolerance) const
    {
    HINVARIANTS;

    return(false);
    }

//-----------------------------------------------------------------------------
//  GetStetchParams
//  Returns the stretch parameters
//-----------------------------------------------------------------------------
void HGF2DLocalProjectiveGrid::GetStretchParams(double*           po_pScaleFactorX,
                                                double*           po_pScaleFactorY,
                                                HGF2DDisplacement* po_pDisplacement) const
    {
    HINVARIANTS;

    HPRECONDITION(po_pScaleFactorX != 0);
    HPRECONDITION(po_pScaleFactorY != 0);
    HPRECONDITION(po_pDisplacement != 0);

    HFCPtr<HGF2DTransfoModel> pModel;

    // Direct models are present ... we use an available coordinate
    // as the middle of extent

    if (m_Direct)
        {
        pModel = GetComposedModelFromCoordinate((m_directExtent.GetXMax() + m_directExtent.GetXMin()) / 2.0,
                                                (m_directExtent.GetYMax() + m_directExtent.GetYMin()) / 2.0);
        }
    else
        {
        pModel = GetComposedModelFromInverseCoordinate((m_directExtent.GetXMax() + m_directExtent.GetXMin()) / 2.0,
                                                       (m_directExtent.GetYMax() + m_directExtent.GetYMin()) / 2.0);
        pModel = pModel->Clone();
        pModel->Reverse();
        }

    pModel->GetStretchParams(po_pScaleFactorX, po_pScaleFactorY, po_pDisplacement);
    }

//-----------------------------------------------------------------------------
// Clone
// This method allocates a copy of self. The caller is responsible for
// the deletion of this object.
//-----------------------------------------------------------------------------
HGF2DTransfoModel* HGF2DLocalProjectiveGrid::Clone () const
    {
    HINVARIANTS;

    // Allocate object as copy and return
    return(new HGF2DLocalProjectiveGrid(*this));
    }

//-----------------------------------------------------------------------------
// ComposeInverseWithDirectOf
// Composes a new transformation model as a combination of self and given
//-----------------------------------------------------------------------------
HFCPtr<HGF2DTransfoModel>  HGF2DLocalProjectiveGrid::ComposeInverseWithDirectOf(const HGF2DTransfoModel& pi_rModel) const
    {
    HINVARIANTS;

    // Recipient
    HFCPtr<HGF2DTransfoModel> pResultModel;

    if (GetAdaptedTransfoModel()->IsCompatibleWith(HGF2DIdentity::CLASS_ID))
        {
        pResultModel = HGF2DTransfoModelAdapter::ComposeInverseWithDirectOf(pi_rModel);
        }
    else
        {
        if (m_Direct)
            {
            if (pi_rModel.PreservesLinearity())
                {
                // Since the model preserves linearity, it can be composed with post-model
                HFCPtr<HGF2DTransfoModel> pNewPostModel = m_pPostTransfoModel->ComposeInverseWithDirectOf(pi_rModel);

                pResultModel = new HGF2DLocalProjectiveGrid(m_Direct,
                                                            *m_pAdaptedTransfoModel,
                                                            *m_pPreTransfoModel,
                                                            *pNewPostModel,
                                                            m_pGridModel,
                                                            m_pBoundaryModel,
                                                            m_directExtent,
                                                            m_nColumns,
                                                            m_nRows);
                }
            else
                {
                // Model is not known ... ask other
                pResultModel = CallComposeOf(pi_rModel);
                }
            }
        else
            {
            if (pi_rModel.PreservesLinearity())
                {
                // Since the model preserves linearity, it can be composed with post-model
                HFCPtr<HGF2DTransfoModel> pInversePreModel (m_pPreTransfoModel->Clone());
                pInversePreModel->Reverse();

                HFCPtr<HGF2DTransfoModel> pNewPreModel = pInversePreModel->ComposeInverseWithDirectOf(pi_rModel);
                pNewPreModel->Reverse();

                pResultModel = new HGF2DLocalProjectiveGrid(m_Direct,
                                                            *m_pAdaptedTransfoModel,
                                                            *pNewPreModel,
                                                            *m_pPostTransfoModel,
                                                            m_pGridModel,
                                                            m_pBoundaryModel,
                                                            m_directExtent,
                                                            m_nColumns,
                                                            m_nRows);
                }
            else
                {
                // Model is not known ... ask other
                pResultModel = CallComposeOf(pi_rModel);
                }
            }
        }
    return (pResultModel);
    }

//-----------------------------------------------------------------------------
// CanBeRepresentedByAMatrix
// Indicates if the model can be represented by a transformation matrix
//-----------------------------------------------------------------------------
bool HGF2DLocalProjectiveGrid::CanBeRepresentedByAMatrix() const
    {
    HINVARIANTS;
    return(false);
    }

//-----------------------------------------------------------------------------
//  GetMatrix
//  Gets the components of the projective by matrix
//-----------------------------------------------------------------------------
HFCMatrix<3, 3> HGF2DLocalProjectiveGrid::GetMatrix() const
    {
    HINVARIANTS;

    // Should not be called
    HASSERT(0);

    HFCMatrix<3, 3> Matrix;

    return(Matrix);

    }

//-----------------------------------------------------------------------------
// CreateSimplifiedModel
//-----------------------------------------------------------------------------
HFCPtr<HGF2DTransfoModel> HGF2DLocalProjectiveGrid::CreateSimplifiedModel() const
    {
    HINVARIANTS;

    // Declare recipient variable
    HFCPtr<HGF2DTransfoModel> pSimplifiedModel;

    // If we get here, no simplification is possible.
    return pSimplifiedModel;
    }


//-----------------------------------------------------------------------------
// PreservesLinearity
// Indicate if the transformation model preserves linearity
//-----------------------------------------------------------------------------
bool   HGF2DLocalProjectiveGrid::PreservesLinearity () const
    {
    HINVARIANTS;

    return(false);
    }

//-----------------------------------------------------------------------------
// PreservesParallelism
// Indicate if the transformation model preserves parallelism
//-----------------------------------------------------------------------------
bool   HGF2DLocalProjectiveGrid::PreservesParallelism() const
    {
    HINVARIANTS;

    return(false);
    }

//-----------------------------------------------------------------------------
// PreservesShape
// Indicate if the transformation model preserves the shape
//-----------------------------------------------------------------------------
bool   HGF2DLocalProjectiveGrid::PreservesShape() const
    {
    HINVARIANTS;

    return(false);
    }

//-----------------------------------------------------------------------------
// PreservesDirection
// Indicate if the transformation model preserves directions
//-----------------------------------------------------------------------------
bool   HGF2DLocalProjectiveGrid::PreservesDirection() const
    {
    HINVARIANTS;

    return(false);
    }


//-----------------------------------------------------------------------------
// Reverse
// This method reverses the transformation model
//-----------------------------------------------------------------------------
void    HGF2DLocalProjectiveGrid::Reverse()
    {
    HINVARIANTS;
    m_Direct = !m_Direct;

    // Invoque reversing of ancester
    // This call will in turn invoque Prepare()
    // HCHK: Here we bypass HGF2DTransfoModelAdapter::Reverse
    HGF2DTransfoModel::Reverse();
    }


//-----------------------------------------------------------------------------
//  Prepare
//  This methods prepares the conversion parameters from the basic
//  model attribute
//-----------------------------------------------------------------------------
void HGF2DLocalProjectiveGrid::Prepare ()
    {
    // Obtain conversion ratio for direct X to inverse X units


    // Invoque preparation of ancester (required)
    HGF2DTransfoModelAdapter::Prepare();
    }

//-----------------------------------------------------------------------------
// ComposeYourself
// PRIVATE
//-----------------------------------------------------------------------------
HFCPtr<HGF2DTransfoModel>  HGF2DLocalProjectiveGrid::ComposeYourself (const HGF2DTransfoModel& pi_rModel) const
    {
    HINVARIANTS;

    // Recipient
    HFCPtr<HGF2DTransfoModel> pResultModel;

    if (GetAdaptedTransfoModel()->IsCompatibleWith(HGF2DIdentity::CLASS_ID))
        {
        pResultModel = HGF2DTransfoModelAdapter::ComposeYourself(pi_rModel);
        }
    else if (m_Direct)
        {
        if (pi_rModel.IsIdentity())
            {
            pResultModel = new HGF2DLocalProjectiveGrid(*this);
            }

        // Check if the type of the given model can be represented by a matrix
        else if (pi_rModel.PreservesLinearity())
            {
            HFCMatrix<3, 3> MySelfMatrix(m_pPreTransfoModel->GetMatrix());

            // Compose the two matrix together
            HFCPtr<HGF2DTransfoModel> pResultPreModel = new HGF2DProjective();

            ((HGF2DProjective*)(&(*pResultPreModel)))->SetByMatrix(MySelfMatrix * pi_rModel.GetMatrix());

            pResultModel = new HGF2DLocalProjectiveGrid(m_Direct,
                                                        *m_pAdaptedTransfoModel,
                                                        *pResultPreModel,
                                                        *m_pPostTransfoModel,
                                                        m_pGridModel,
                                                        m_pBoundaryModel,
                                                        m_directExtent,
                                                        m_nColumns,
                                                        m_nRows);
            }
        else
            {
            // Type is not known ... build a complex
            // To do this we call the ancester ComposeYourself
            pResultModel = HGF2DTransfoModel::ComposeYourself(pi_rModel);
            }
        }
    else
        {
        // Inverse
        if (pi_rModel.IsIdentity())
            {
            pResultModel = new HGF2DLocalProjectiveGrid(*this);
            }

        // Check if the type of the given model can be represented by a matrix
        else if (pi_rModel.PreservesLinearity())
            {
            HFCPtr<HGF2DTransfoModel> pInversePostModel (m_pPostTransfoModel->Clone());
            pInversePostModel->Reverse();

            HFCMatrix<3, 3> MySelfMatrix(pInversePostModel->GetMatrix());

            // Compose the two matrix together
            HFCPtr<HGF2DTransfoModel> pResultPostModel = new HGF2DProjective();

            ((HGF2DProjective*)(&(*pResultPostModel)))->SetByMatrix(MySelfMatrix * pi_rModel.GetMatrix());
            pResultPostModel->Reverse();

            pResultModel = new HGF2DLocalProjectiveGrid(m_Direct,
                                                        *m_pAdaptedTransfoModel,
                                                        *m_pPreTransfoModel,
                                                        *pResultPostModel,
                                                        m_pGridModel,
                                                        m_pBoundaryModel,
                                                        m_directExtent,
                                                        m_nColumns,
                                                        m_nRows);
            }
        else
            {
            // Type is not known ... build a complex
            // To do this we call the ancester ComposeYourself
            pResultModel = HGF2DTransfoModel::ComposeYourself(pi_rModel);
            }
        }

    return (pResultModel);
    }

//-----------------------------------------------------------------------------
//  Copy
//  Copy method
//-----------------------------------------------------------------------------
void HGF2DLocalProjectiveGrid::Copy(const HGF2DLocalProjectiveGrid& pi_rObj)
    {

    m_Direct = pi_rObj.m_Direct;
    m_useGlobalAffineApproximation = pi_rObj.m_useGlobalAffineApproximation;

    // Copy master data
    m_pPreTransfoModel       = pi_rObj.m_pPreTransfoModel->Clone();
    m_pPostTransfoModel      = pi_rObj.m_pPostTransfoModel->Clone();

    // Copy model pointer
    m_pGlobalAffine         = pi_rObj.m_pGlobalAffine;
    m_pComposedAffine       = pi_rObj.m_pComposedAffine;
    m_pComposedAdaptedModel = pi_rObj.m_pComposedAdaptedModel;

    m_directExtent = pi_rObj.m_directExtent;
    m_nColumns = pi_rObj.m_nColumns;
    m_nRows    = pi_rObj.m_nRows;

    // Copy grid and boundary pointers
    m_pGridModel            = pi_rObj.m_pGridModel;
    m_pGridModelBooster     = pi_rObj.m_pGridModelBooster;
    m_pBoundaryModel        = pi_rObj.m_pBoundaryModel;
    m_pBoundaryModelBooster = pi_rObj.m_pBoundaryModelBooster;

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
HFCPtr<HGF2DTransfoModel> HGF2DLocalProjectiveGrid::GetModelFromCoordinate(double pi_X,
                                                                           double pi_Y) const
    {
    HASSERT(m_pGridModel != NULL);

    // Look in grid model first
    HFCPtr<HGF2DTransfoModel> pModel (m_pGridModel->GetModelFromCoordinate(pi_X, pi_Y));

    if (pModel != NULL)
        {
        return pModel;
        }

    // We are probably outside of the grid direct extent
    //HASSERT(m_pBoundaryModel != NULL);
    if (m_pBoundaryModel!=NULL)
        pModel = m_pBoundaryModel->GetModelFromCoordinate(pi_X, pi_Y);

    if (pModel != NULL)
        {
        return pModel;
        }

    if (GetUseApproximation())
        {
        // Use complex model built with Global affine approximation
        return GetGlobalAffine();
        }

    // No approximation, use complex built with non-linear model
    return m_pAdaptedTransfoModel;
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
HFCPtr<HGF2DTransfoModel> HGF2DLocalProjectiveGrid::GetModelFromInverseCoordinate(double pi_X,
        double pi_Y) const
    {
    HASSERT(m_pGridModel != NULL);

    // Look in grid model first
    HFCPtr<HGF2DTransfoModel> pModel (m_pGridModel->GetModelFromInverseCoordinate(pi_X, pi_Y));

    if (pModel != NULL)
        {
        return pModel;
        }

    //HASSERT(m_pBoundaryModel != NULL);

    // Second, search in triangles
    if (m_pBoundaryModel!=NULL)
        pModel = m_pBoundaryModel->GetModelFromInverseCoordinate(pi_X, pi_Y);

    if (pModel != NULL)
        {
        return pModel;
        }

    if (GetUseApproximation())
        {
        // Out of the extended extent
        // Use Affine approximation
        return GetGlobalAffine();
        }

    // Here no quadtree (Quadrilateres and Triangles) contains the coordinate, or the approximation is false.
    return m_pAdaptedTransfoModel;
    }

//-----------------------------------------------------------------------------
// GetComposedModelFromCoordinate
//-----------------------------------------------------------------------------
HFCPtr<HGF2DTransfoModel> HGF2DLocalProjectiveGrid::GetComposedModelFromCoordinate(double pi_X,
        double pi_Y) const
    {
    // Look in grid model first
    HASSERT(m_pGridModelBooster != NULL);
    m_stat.IncGetComposedModelFromCoordinateCount();

    HFCPtr<HGF2DTransfoModel> pModel (m_pGridModelBooster->GetModelFromCoordinate(pi_X, pi_Y));

    if (pModel != NULL)
        {
        m_stat.IncGetComposedModelFromCoordinateFoundInGrid();
        return pModel;
        }

    // We are probably outside of the grid direct extent
    //HASSERT(m_pBoundaryModelBooster != NULL);
    if (m_pBoundaryModel!=NULL)
        pModel = m_pBoundaryModelBooster->GetModelFromCoordinate(pi_X, pi_Y);

    if (pModel != NULL)
        {
        m_stat.IncGetComposedModelFromCoordinateFoundInBoundary();
        return pModel;
        }

    if (GetUseApproximation())
        {
        // Use complex model built with Global affine approximation
        m_stat.IncGetComposedModelFromCoordinateByApproximation();
        return GetComposedGlobalAffine();
        }

    // No approximation, use complex built with non-linear model
    m_stat.IncGetComposedModelFromCoordinateByUnAdapted();
    return GetComposedAdaptedModel();
    }

//-----------------------------------------------------------------------------
// GetComposedModelFromInverseCoordinate
//-----------------------------------------------------------------------------
HFCPtr<HGF2DTransfoModel> HGF2DLocalProjectiveGrid::GetComposedModelFromInverseCoordinate(double pi_X, double pi_Y) const
    {
    HASSERT(m_pGridModelBooster != NULL);
    m_stat.IncGetComposedModelFromInverseCoordinateCount();

    // Look in grid model first
    HFCPtr<HGF2DTransfoModel> pModel (m_pGridModelBooster->GetModelFromInverseCoordinate(pi_X, pi_Y));

    if (pModel != NULL)
        {
        m_stat.IncGetComposedModelFromInverseCoordinateFoundInGrid();
        return pModel;
        }

    //HASSERT(m_pBoundaryModelBooster != NULL);

    // Second, search in triangles
    if (m_pBoundaryModel!=NULL)
        pModel = m_pBoundaryModelBooster->GetModelFromInverseCoordinate(pi_X, pi_Y);

    if (pModel != NULL)
        {
        m_stat.IncGetComposedModelFromInverseCoordinateFoundInBoundary();
        return pModel;
        }

    if (GetUseApproximation())
        {
        // Out of the extended extent
        // Use Affine approximation
        m_stat.IncGetComposedModelFromInverseCoordinateByApproximation();
        return GetComposedGlobalAffine();
        }

    // Here no quadtree (Quadrilateres and Triangles) contains the coordinate, or the approximation is false.
    m_stat.IncGetComposedModelFromInverseCoordinateByUnAdapted();
    return GetComposedAdaptedModel();
    }

//-----------------------------------------------------------------------------
// ComputeGlobalAffineModel
//-----------------------------------------------------------------------------
HFCPtr<HGF2DTransfoModel> HGF2DLocalProjectiveGrid::ComputeGlobalAffineModel () const
    {
    // build 5 points inside the extent
    double TiePoints[30];
    double OffsetX (GetDirectGridWidth() / 2.0);
    double OffsetY (GetDirectGridHeight() / 2.0);

    TiePoints[0] = m_directExtent.GetXMin() + OffsetX;
    TiePoints[1] = m_directExtent.GetYMin() + OffsetY;
    TiePoints[2] = 0.0;
    GetModelFromCoordinate(TiePoints[0], TiePoints[1])->ConvertDirect(TiePoints[0], TiePoints[1], &TiePoints[3], &TiePoints[4]);
    TiePoints[5] = 0.0;

    TiePoints[6] = m_directExtent.GetXMax() - OffsetX;
    TiePoints[7] = m_directExtent.GetYMin() + OffsetY;
    TiePoints[8] = 0.0;
    GetModelFromCoordinate(TiePoints[6], TiePoints[7])->ConvertDirect(TiePoints[6], TiePoints[7], &TiePoints[9], &TiePoints[10]);
    TiePoints[11] = 0.0;

    TiePoints[12] = m_directExtent.GetXMax() - OffsetX;
    TiePoints[13] = m_directExtent.GetYMax() - OffsetY;
    TiePoints[14] = 0.0;
    GetModelFromCoordinate(TiePoints[12], TiePoints[13])->ConvertDirect(TiePoints[12], TiePoints[13], &TiePoints[15], &TiePoints[16]);
    TiePoints[17] = 0.0;

    TiePoints[18] = m_directExtent.GetXMin() + OffsetX;
    TiePoints[19] = m_directExtent.GetYMax() - OffsetY;
    TiePoints[20] = 0.0;
    GetModelFromCoordinate(TiePoints[18], TiePoints[19])->ConvertDirect(TiePoints[18], TiePoints[19], &TiePoints[21], &TiePoints[22]);
    TiePoints[23] = 0.0;

    TiePoints[24] = ((m_directExtent.GetXMin() + m_directExtent.GetXMax()) / 2.0);
    TiePoints[25] = ((m_directExtent.GetYMin() + m_directExtent.GetYMax()) / 2.0);
    TiePoints[26] = 0.0;
    GetModelFromCoordinate(TiePoints[24], TiePoints[25])->ConvertDirect(TiePoints[24], TiePoints[25], &TiePoints[27], &TiePoints[28]);
    TiePoints[29] = 0.0;

    return ComputeDirectModelFromPoints(TiePoints, 30, AFFINE);
    }

//-----------------------------------------------------------------------------
// ComputeDirectModelFromPoints
//-----------------------------------------------------------------------------
HFCPtr<HGF2DTransfoModel> HGF2DLocalProjectiveGrid::ComputeDirectModelFromPoints (double* pi_pPoints,
        uint32_t  pi_nbPoints,
        MODELTYPE pi_ModelType) const
    {
    double FlatMatrix[4][4];

    if (pi_ModelType == AFFINE)
        {
        GetAffineTransfoMatrixFromScaleAndTiePts(FlatMatrix, (unsigned short)pi_nbPoints, pi_pPoints);
        }
    else
        {
        HASSERT(pi_ModelType == PROJECTIVE);
        GetProjectiveTransfoMatrixFromScaleAndTiePts(FlatMatrix, (unsigned short)pi_nbPoints, pi_pPoints);
        }

    HFCMatrix<3, 3> MyMatrix;
    MyMatrix[0][0] = FlatMatrix[0][0];
    MyMatrix[0][1] = FlatMatrix[0][1];
    MyMatrix[0][2] = FlatMatrix[0][3];
    MyMatrix[1][0] = FlatMatrix[1][0];
    MyMatrix[1][1] = FlatMatrix[1][1];
    MyMatrix[1][2] = FlatMatrix[1][3];
    MyMatrix[2][0] = FlatMatrix[3][0];
    MyMatrix[2][1] = FlatMatrix[3][1];
    MyMatrix[2][2] = FlatMatrix[3][3];

    // Create model
    HFCPtr<HGF2DTransfoModel> pModel = new HGF2DProjective(MyMatrix);

    if (pi_ModelType == AFFINE)
        {
        HFCPtr<HGF2DTransfoModel> pSimplifiedModel = pModel->CreateSimplifiedModel();

        if (pSimplifiedModel != NULL)
            pModel = pSimplifiedModel;
        }

    return pModel;
    }

//-----------------------------------------------------------------------------
// ComputeTransitExtent
//-----------------------------------------------------------------------------
HGF2DLiteExtent HGF2DLocalProjectiveGrid::ComputeTransitExtent () const
    {
    double DeltaX  = m_directExtent.GetCorner().GetX() - m_directExtent.GetOrigin().GetX();
    double DeltaY  = m_directExtent.GetCorner().GetY() - m_directExtent.GetOrigin().GetY();
    double CenterX ((m_directExtent.GetXMin() + m_directExtent.GetXMax()) / 2.0);
    double CenterY ((m_directExtent.GetYMin() + m_directExtent.GetYMax()) / 2.0);

    double Delta = 5*(DeltaX + DeltaY);

    HGF2DLiteExtent ExtendedExtent(CenterX - Delta,
                                   CenterY - Delta,
                                   CenterX + Delta,
                                   CenterY + Delta);
    return ExtendedExtent;
    }

//-----------------------------------------------------------------------------
// GetModel
//-----------------------------------------------------------------------------
HFCPtr<HGF2DTransfoModel> HGF2DLocalProjectiveGrid::GetModel(uint32_t pi_Row, uint32_t pi_Column) const
    {
    return m_pGridModel->GetModelFromRowColumn(pi_Row, pi_Column);
    }

//-----------------------------------------------------------------------------
// GetPSSParameters
//-----------------------------------------------------------------------------
void HGF2DLocalProjectiveGrid::GetPSSParameters (HGF2DLiteExtent&                  po_rExtent,
                                                 uint32_t&                           po_rNColumns,
                                                 uint32_t&                           po_rNRows,
                                                 list<HFCPtr<HGF2DTransfoModel> >& po_rModelList) const
    {
    // Coumpute the region of adaptation
    HGF2DLiteExtent directExtent  = HGF2DPieceWiseModel::ConvertLiteExtent(m_directExtent, *m_pPreTransfoModel, HGF2DPieceWiseModel::INVERSE);
    directExtent  = HGF2DPieceWiseModel::ScaleExtent(directExtent, 1.001);

    HFCPtr<HGF2DComplexTransfoModel> pComplex(new HGF2DComplexTransfoModel());
    pComplex->AddModel(*m_pPreTransfoModel);
    pComplex->AddModel(*m_pAdaptedTransfoModel);
    pComplex->AddModel(*m_pPostTransfoModel);

    HGF2DLocalProjectiveGrid gridModel (*pComplex, directExtent, m_nColumns, m_nRows);


    po_rExtent   = gridModel.GetExtent();
    po_rNColumns = gridModel.GetNumberOfColumn();
    po_rNRows    = gridModel.GetNumberOfRow();

    po_rModelList.push_back(gridModel.GetGlobalAffine());

    for (uint32_t i = 0; i < gridModel.GetNumberOfRow(); i++)
        {
        for (uint32_t j = 0; j < gridModel.GetNumberOfColumn(); j++)
            {
            po_rModelList.push_back(gridModel.GetModel(i, j));
            }
        }
    }

//-----------------------------------------------------------------------------
// Dump
//-----------------------------------------------------------------------------
void HGF2DLocalProjectiveGrid::Dump(ofstream& outStream) const
    {
    if (m_pGridModel)
        m_pGridModel->Dump(outStream);
    if (m_pBoundaryModel!=NULL)
        m_pBoundaryModel->Dump(outStream);
    }
