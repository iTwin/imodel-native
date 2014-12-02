/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/IAnnotationHandler.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
/** @cond BENTLEY_SDK_Internal */

#include "ViewContext.h"
//__PUBLISH_SECTION_END__

/*----------------------------------------------------------------------+
|                                                                       |
|   Change Annotation Scale Typedefs                                    |
|                                                                       |
+----------------------------------------------------------------------*/
enum  CASTraverseElementContext
{
    CAS_TRAVERSE_ELEMENTS_ALL                   = 0,
    CAS_TRAVERSE_ELEMENTS_SELECTED              = 1,
    CAS_TRAVERSE_ELEMENTS_SPECIFIED             = 2         // Currently not supported
};

enum  CASFilter
{
    CAS_FILTER_NONE                             = 0,
    CAS_FILTER_ANNOTATION_SCALE_SPECIFIED       = 1
};

//__PUBLISH_SECTION_START__

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//__PUBLISH_SECTION_END__

typedef struct ChangeAnnotationScale
{
    DgnModelP                    modelRef;
    CASTraverseElementContext       traverseContext;
    int                             numElementRef;
    ElementRefP*                    elementRefArray;
    CASFilter                       filter;
    double                          filterAnnotationScale;
    AnnotationScaleAction           action;
    double                          newAnnotationScale;
} ChangeAnnotationScale;

typedef struct PropagateAnnotationScaleContext
{
    ChangeAnnotationScale *         changeContext;
    bool                            needsRewrite;
    bool                            forceTextNode;
} PropagateAnnotationScaleContext;

//__PUBLISH_SECTION_START__

/*=================================================================================**//**
*
* Interface for handlers that support annotation behavior
*
* @bsiinterface                                 Deepak.Malkan                   09/2007
+===============+===============+===============+===============+===============+======*/
struct IAnnotationHandler
{
//__PUBLISH_SECTION_END__
protected:
    //  Non-virtual utility methods:
DGNPLATFORM_EXPORT         StatusInt    GetAnnotationScaleChange            (bool& newScaleFlag, double& newScale, double& scaleChange, ElementHandleCR, TransformInfoCR);
DGNPLATFORM_EXPORT         StatusInt    ComputeAnnotationScaledRange        (ElementHandleCR, DRange3dR, double scaleFactor, DPoint3dCP  originForScale);

    //  Virtual methods:
              virtual bool          _GetAnnotationScale                 (double* annotationScale, ElementHandleCR) const = 0;
DGNPLATFORM_EXPORT virtual StatusInt    _ComputeAnnotationScaledRange       (ElementHandleCR, DRange3dR, double scaleFactor);
DGNPLATFORM_EXPORT virtual StatusInt    _UpdateAnnotationScale              (EditElementHandleR);

//DGNPLATFORM_EXPORT virtual StatusInt    _OnPreprocessCopyAnnotationScale    (EditElementHandleR, CopyContextP); removed in graphite

DGNPLATFORM_EXPORT virtual StatusInt    _AddAnnotationScale                 (EditElementHandleR, DgnModelP);
DGNPLATFORM_EXPORT virtual StatusInt    _RemoveAnnotationScale              (EditElementHandleR);
DGNPLATFORM_EXPORT virtual StatusInt    _ApplyAnnotationScaleDifferential   (EditElementHandleR, double scale); // Called by default implementation of _OnPreprocessCopyAnnotationScale when copying annotation through a reference in order to compensate for scaling

DGNPLATFORM_EXPORT virtual StatusInt    _UpdateSpecifiedAnnotationScale     (EditElementHandleR, double scale); // To support legacy functionality of multiple annotation scales
  //    Non-virtual public interface

public:

//! Compute the range that this annotation element would have if it were scaled by the specified factor.
//! This should be implemented only if the range of the element's graphics would be different from
//! the element's persistent range after annotation scaling is applied in the _Draw method.
//! @remarks
//! The annotation scale subsystem calls this method to get the derived range of an annotation element.
//! The handler for an annotation-able element type should override _ComputeScaledRange and should
//! implement it to return the best-fitting range that could be applied to the element if it were
//! scaled by the specified factor.
//! \em NB: The computed range for a given scale factor must enclose the range of the annotation at any smaller scale.
//! @bsimethod                                    Sam.Wilson                      02/2008
DGNPLATFORM_EXPORT StatusInt            ComputeAnnotationScaledRange                (ElementHandleCR, DRange3dR elemRangeOut, double scaleFactor);

//! Compute the effective reference annotation scale and reference display scale to
//!   be used in displaying annotations in the current model of \a context.
//! @remarks Use ComputeAnnotationDisplayParameters to get parameters for an individual annotation element.
//! @param context    The view context to query for current model and reference annotation scale.
//! @bsimethod                                    Sam.Wilson                      02/2008
DGNPLATFORM_EXPORT static bool ComputeAnnotationDisplayParametersOfCurrentModel     (AnnotationDisplayParameters&, ViewContextP context);

//! Compute the effective reference annotation scale and reference display scale to
//!   be used in displaying this element.
//! @remarks This function computes the same values as StrokeAnnotationElm::CalculateScaleFactors.
//!           This function is for use by handlers that do not inherit from IAnnotationHandler.
//! @bsimethod                                    Sam.Wilson                      02/2008
DGNPLATFORM_EXPORT bool                 ComputeAnnotationDisplayParameters          (AnnotationDisplayParameters&, ElementHandleCR, ViewContextR) const;

//! Update the annotation scale of the element to a specified value. This method is
//! added only to support legacy functionality of multiple annotation scales.
//! There is no need to publish or export it.
//! @param    element             input element handle
//! @param    scale               scale to update
//! @return  SUCCESS if the element is updated
StatusInt                           UpdateSpecifiedAnnotationScale              (EditElementHandleR element, double scale) { return _UpdateSpecifiedAnnotationScale (element, scale); }

//! Update the annotation scale of the element to match its model. This is rarely needed,
//! as changing model annotation scale updates all annotations in the model.
//! @param    element             input element handle
//! @return  true if the element has an annotation scale value
DGNPLATFORM_EXPORT   StatusInt          UpdateAnnotationScale                       (EditElementHandleR element) { return _UpdateAnnotationScale (element); }

//! Query if the specified element is an annotation.
//! @param[out] annotationScale     [optional] The element's annotation scale. Note: this normally matches the annotation scale of the containing model.
//! @param[in]  element             The element to test
//! @return  true if the element has an annotation scale value
DGNPLATFORM_EXPORT   bool               GetAnnotationScale                          (double* annotationScale, ElementHandleCR element) const;

//__PUBLISH_SECTION_START__
//__PUBLISH_CLASS_VIRTUAL__
public:

//! Query if the specified element is an annotation.
//! @param[out] annotationScale     [optional] The element's annotation scale. Note: this normally matches the annotation scale of the containing model.
//! @param[in]  element             The element to test
//! @return  true if the element has an annotation scale value
DGNPLATFORM_EXPORT   bool               HasAnnotationScale                          (double* annotationScale, ElementHandleCR element) const;

//! Set up annotation scale on \a eh.
//! @return non-zero error status if annotation scale could not be initialized.
//__PUBLISH_SECTION_END__
//! @remarks The default implementation is to call ApplyTransform with AnnotationScaleAction::Add.
//__PUBLISH_SECTION_START__
DGNPLATFORM_EXPORT   StatusInt          AddAnnotationScale                          (EditElementHandleR eh, DgnModelP model);

//! Remove annotation scale from \a eh.
//! @return non-zero error status if annotation scale could not be removed.
//__PUBLISH_SECTION_END__
//! @remarks The default implementation is to call ApplyTransform with AnnotationScaleAction::Remove and remove the annotation scale XAttribute, if any.
//__PUBLISH_SECTION_START__
DGNPLATFORM_EXPORT   StatusInt         RemoveAnnotationScale                        (EditElementHandleR eh);

}; // IAnnotationHandler

//__PUBLISH_SECTION_END__

/*=================================================================================**//**
*
* Interface for annotation scale related queries.
*
* @bsiinterface                                 Deepak.Malkan                   09/2007
+===============+===============+===============+===============+===============+======*/
struct  IAnnotationStrokeForCache : IStrokeForCache
{
public:

//! Scale specified elemHandle by newScale
//! @param    thisElem        IN input element handle
//! @param    context         IN context to use to create the cached representation.
//! @param    parms           IN annotation scaling to be applied
virtual void                StrokeScaledForCache
(
ElementHandleCR thisElem,
ViewContextR                        context,
AnnotationDisplayParameters const& parms
) = 0;

//! Helper method to handle the simple, common case of preparing to draw an annotation
//! element when reference annotation scale and/or skewed aspect ratio must be applied.
//! This method pushes a transform to scale all graphics in order to apply the reference
//! annotation scale. This is appropriate for simple annotation elements.
//! This method also calls viewContext_replaceAspectRatioSkewWithTranslationEffect, which is the most common
//! way for simple annotation elements to handle skewed aspect ratios.
//! @bsimethod                                    Sam.Wilson                      03/2008
DGNPLATFORM_EXPORT static void  PushTransformToRescale
(
ViewContextR                        context,
DPoint3dCR                          originForScale,
AnnotationDisplayParameters const& parms,
bool            divideByElementScale
);

}; // IAnnotationScaleForCache

//=======================================================================================
//! Adapter class to stroke reference elements that have an annotation scale.
// @bsiclass                                                      Deepak.Malkan   09/2007
//=======================================================================================
struct  StrokeAnnotationElm : IStrokeForCache
{
private:
    IAnnotationHandler*         m_annotationHandler;
    ElementHandleCR             m_element;
    ViewContextR                m_context;
    IAnnotationStrokeForCache*  m_elementStroker;
    AnnotationDisplayParameters m_parms;
    UInt32                      m_qvIndexBase;

public:
    // Constructor
    DGNPLATFORM_EXPORT               StrokeAnnotationElm    (IAnnotationHandler* handlerIn, ElementHandleCR elementIn, ViewContextR contextIn, IAnnotationStrokeForCache* elementStrokerIn, UInt32 qvIndexBase=0);

    DGNPLATFORM_EXPORT void          DrawUsingContext       ();

    // IStrokeForCache Interface Methods
    DGNPLATFORM_EXPORT virtual void  _StrokeForCache        (CachedDrawHandleCR dh, ViewContextR context, double pixelSize) override;

};

//__PUBLISH_SECTION_START__

END_BENTLEY_DGNPLATFORM_NAMESPACE

/** @endcond */
