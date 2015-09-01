//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGF2DComplexTransfoModel.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HGF2DComplexTransfoModel
//-----------------------------------------------------------------------------
// Description of 2D transformation model. This is an abstract class
//-----------------------------------------------------------------------------

#pragma once

#include "HGF2DTransfoModel.h"

BEGIN_IMAGEPP_NAMESPACE
class HGF2DDisplacement;
// Id used by different transformation models


/** -----------------------------------------------------------------------------
    @version 1.0
    @author Alain Robert

    This class encapsulates a complex transformation model. An object of this
    class expresses the bidirectional transformations to apply to coordinates.
    Coordinates can therefore be transformed directly or inverse. A complex
    transformation model is first a transformation model, but it may contain
    many other transformation models. This model is therefore a list of HGF2DTransfoModel.

    The transformation model is composed of any number of other transformation models.
    It contains a list of models. The complex model is responsible of creating a
    copy of added models, and deleting them at its own destruction. The transformation
    models of the internal list can be of any class inheriting from HGF2DTransfoModel
    including another HGF2DComplexTransfoModel, which can in turn contain other
    transformation models of any type. Object of the class HGF2DComplexTransfoModel
    are not usually used knowingly by users. It is usually of use for developping new
    kinds of transformation models, which cannot be composed any other way than through
    complex transformation models.

    Every transformation model possesses units assigned to their direct and inverse
    channels. The complex transformation model uses by default the direct units of
    the first added transformation model for its direct channels, and the inverse units
    of the last added transformation model. After addition of models, it may be required
    to change either direct or inverse units of the complex, in order to set them to the
    desired units.

    If the complex transformation model does not contain any other transformation model,
    then the conversion methods will not implement any transformations.

    Internal Structure
    The internal structure of the complex transformation model is made of a list of models.
    For performance reason, all units of the transformations models of the list share
    the same direct and inverse units which are the direct units of the complex
    transformation model. The only exception is the inverse channel of the last transformation
    model which has the same units as the inverse channel units of the complex transformation
    model. This permits to accelerate the transformation of coordinates to the maximum.
    Since inverse units of a model is matched exactly with the direct units of the next
    model, and that the units of the direct channels of the first model are matched with
    the units of the direct channels of the complex model, and that the same match exists
    for the inverse units of the last model with those of the complex, internal transformation
    can be performed raw for any conversion functions, then if required units are transformed
    only at the end. This structure also insures the only one unit conversion will be done
    inside the chain of models. Some models can bypass the unit conversions if their inverse
    and direct units are matched, and thus accelerate the transformation. The transformation
    of coordinate is therefore optimized, to the disadvantage and the complexity of other
    methods such as Add(), SetXXXUnits(), which require much processing, to maintain the
    complex consistent in its structure.

    -----------------------------------------------------------------------------
*/
class HGF2DComplexTransfoModel : public HGF2DTransfoModel
    {
    HDECLARE_CLASS_ID(HGF2DTransfoModelId_Complex, HGF2DTransfoModel)

public:

    // Primary methods
    IMAGEPP_EXPORT                       HGF2DComplexTransfoModel();
                                        HGF2DComplexTransfoModel(const HGF2DComplexTransfoModel& pi_rObj);
    virtual                             ~HGF2DComplexTransfoModel();
    HGF2DComplexTransfoModel&           operator=(const HGF2DComplexTransfoModel& pi_rObj);

    // Methods specific to complex model
    IMAGEPP_EXPORT void                  AddModel          (const HGF2DTransfoModel& pi_rModelToAdd);
    IMAGEPP_EXPORT size_t                GetNumberOfModels() const;
    IMAGEPP_EXPORT HGF2DTransfoModel*
                                        GetModel(size_t modelNumber);  // Intentionaly non-const ... breaks the integrity of the complex model

    virtual bool IsConvertDirectThreadSafe()  const override;
    virtual bool IsConvertInverseThreadSafe() const override;

    // Conversion interface
    virtual StatusInt                   ConvertDirect(double*   pio_pXInOut,
                                                      double*   pio_pYInOut) const override;

    virtual StatusInt                   ConvertDirect(double    pi_XIn,
                                                      double    pi_YIn,
                                                      double*   po_pXOut,
                                                      double*   po_pYOut) const override;

    virtual StatusInt                   ConvertDirect (double    pi_YIn,
                                                       double    pi_XInStart,
                                                       size_t    pi_NumLoc,
                                                       double    pi_XInStep,
                                                       double*   po_aXOut,
                                                       double*   po_aYOut) const override;

    virtual StatusInt                   ConvertDirect(size_t    pi_NumLoc,
                                                      double*   pio_aXInOut,
                                                      double*   pio_aYInOut) const override;


    virtual StatusInt                   ConvertInverse(double    pi_YIn,
                                                       double    pi_XInStart,
                                                       size_t    pi_NumLoc,
                                                       double    pi_XInStep,
                                                       double*   po_aXOut,
                                                       double*   po_aYOut) const override;

    virtual StatusInt                   ConvertInverse(double*   pio_pXInOut,
                                                       double*   pio_pYInOut) const override;

    virtual StatusInt                   ConvertInverse(double    pi_XIn,
                                                       double    pi_YIn,
                                                       double*   po_pXOut,
                                                       double*   po_pYOut) const override;

    virtual StatusInt                   ConvertInverse(size_t    pi_NumLoc,
                                                       double*   pio_aXInOut,
                                                       double*   pio_aYInOut) const override;

    // Miscalenious
    virtual bool                        IsIdentity() const;
    virtual bool                        IsStretchable(double pi_AngleTolerance = 0) const;
    virtual void                        GetStretchParams(double*  po_pScaleFactorX,
                                                         double*  po_pScaleFactorY,
                                                         HGF2DDisplacement* po_pDisplacement) const;

    virtual HFCPtr<HGF2DTransfoModel>   ComposeInverseWithDirectOf(const HGF2DTransfoModel& pi_rModel) const;

    virtual bool                        CanBeRepresentedByAMatrix() const;
    virtual HFCMatrix<3, 3>             GetMatrix() const;
    virtual HFCMatrix<3, 3>&            GetMatrix(HFCMatrix<3, 3>& po_rRecipient) const;

    virtual HFCPtr<HGF2DTransfoModel>   CreateSimplifiedModel() const;

    // Geometric properties
    virtual bool                        PreservesLinearity() const;
    virtual bool                        PreservesParallelism() const;
    virtual bool                        PreservesShape() const;
    virtual bool                        PreservesDirection() const;

    // Domain related overrides: A complex model will have a domain
    // if any of the constituent transformation model has a domain.
    virtual bool                        HasDomain() const override;
    virtual HFCPtr<HGF2DShape>          GetDirectDomain() const override;
    virtual HFCPtr<HGF2DShape>          GetInverseDomain() const override;


protected:

    void                                Prepare ();

    virtual HGF2DTransfoModel*          Clone () const override;

    virtual void                        Reverse ();

    void                                AddFrontModel (const HGF2DTransfoModel& pi_rModelToAdd);


    virtual HFCPtr<HGF2DTransfoModel>   ComposeYourself (const HGF2DTransfoModel& pi_rModel) const;

private:

    void                                Copy (const HGF2DComplexTransfoModel& pi_rObj);


    // Attributes
    typedef list<HGF2DTransfoModel*, allocator<HGF2DTransfoModel*> >    List_TransfoModel;
    List_TransfoModel  m_ListOfModels;

    // Domain related variables. If the transformation model does not have
    // a domain then the direct and inverse domains will be null yet
    // by definition the GetDirectDomain and GetInverseDomain MUST return
    // a valid domain if called (Usually an HGF2DUniverse)
    HFCPtr<HGF2DShape>      m_directDomain;
    HFCPtr<HGF2DShape>      m_inverseDomain;
    bool                    m_hasDomain;
    };
END_IMAGEPP_NAMESPACE