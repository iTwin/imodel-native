
/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/AnnotationScale.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

#include "../DgnPlatform.h"
#include "SheetDef.h"

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//=======================================================================================
//! Utility class for Annotation Scale related methods.
//!
//! @bsiclass                                                     Deepak.Malkan   07/2006
//=======================================================================================
struct  AnnotationScale
{
private:
    AnnotationScale ();

public:
DGNPLATFORM_EXPORT static StatusInt    SetAsXAttribute      (EditElementHandleR eeh, double const* annotationScaleIn);
DGNPLATFORM_EXPORT static StatusInt    GetFromXAttribute    (double* annotationScaleOut, ElementHandleCR eh);

}; // AnnotationScale
END_BENTLEY_DGNPLATFORM_NAMESPACE

/*----------------------------------------------------------------------+
|                                                                       |
|   Change Annotation Scale Typedefs                                    |
|                                                                       |
+----------------------------------------------------------------------*/
BEGIN_BENTLEY_API_NAMESPACE

//! Create a new change annotation scale context specifier for specified model-ref
//! @Param[in]      modelRefIn              Sheet model.
//! @Return A new context specifier structure.
//! @Group          "Model Properties"
DGNPLATFORM_EXPORT ChangeAnnotationScaleP mdlChangeAnnotationScale_new
(
DgnModelP                modelRefIn
);

//!  Copy specified change context
//!  @Param[in]     changeContextIn         Change context to copy.
//!  @Return A new context specifier structure which is a copy of changeContextIn
//!  @Group          "Model Properties"
DGNPLATFORM_EXPORT ChangeAnnotationScaleP mdlChangeAnnotationScale_copy
(
ChangeAnnotationScaleP      changeContextIn
);

//! Free a change annotation scale context specifier
//! @Param[in]      changeContextIn         Change context specifier to free.
//! @Return SUCCESS if the context specifier is successfully freed
//! @Group          "Model Properties"
DGNPLATFORM_EXPORT StatusInt    mdlChangeAnnotationScale_free
(
ChangeAnnotationScaleP*     changeContextIn
);

//! Get model ref of change annotation scale context specifier
//! @Param[in]      modelRef                Sheet model.
//! @Return model ref
//! @Group          "Model Properties"
DGNPLATFORM_EXPORT DgnModelP mdlChangeAnnotationScale_getDgnModel
(
ChangeAnnotationScaleCP     changeContextIn
);

//! To change the annotation scale of elements in a model, set the element which will be traversed.
//! @Param[in]      changeContextIn         Change annotation scale context specifier.
//! @Param[in]      traverseContextIn       Element traverse context.
//! @Param[in]      numElementRefIn         Number of element refs to traverse (when traverseContextIn = CAS_TRAVERSE_ELEMENT_SPECIFIED).
//! @Param[in]      elementRefArrayIn       Array of element refs to traverse (when traverseContextIn = CAS_TRAVERSE_ELEMENT_SPECIFIED).
//! @Return SUCCESS if the traverse element context is successfully set
//! @Group          "Model Properties"
DGNPLATFORM_EXPORT StatusInt   mdlChangeAnnotationScale_setTraverseElementContext
(
ChangeAnnotationScaleP      changeContextIn,
CASTraverseElementContext   traverseContextIn,
int                         numElementRefIn,
ElementRefP const *         elementRefArrayIn
);

//! To change the annotation scale of elements in a model, get the elements which will be traversed.
//! @Param[in]      changeContextIn         Change annotation scale context specifier.
//! @Param[out]     traverseContextOut      Element traverse context.
//! @Param[out]     numElementRefOut        Number of element refs to traverse (when traverseContextIn = CAS_TRAVERSE_ELEMENT_SPECIFIED).
//! @Param[out]     elementRefArrayOut      Array of element refs to traverse (when traverseContextIn = CAS_TRAVERSE_ELEMENT_SPECIFIED).
//! @Return SUCCESS if the traverse element context is successfully set
//! @Group          "Model Properties"
DGNPLATFORM_EXPORT StatusInt   mdlChangeAnnotationScale_getTraverseElementContext
(
ChangeAnnotationScaleCP     changeContextIn,
CASTraverseElementContext * traverseContextOut,
int *                       numElementRefOut,
ElementRefP **              elementRefArrayOut
);

//! To change the annotation scale of elements in a model, set a filter which controls which of the traversed elements are filtered out
//! @Param[in]      changeContextIn         Change annotation scale context specifier.
//! @Param[in]      filterIn                Filter.
//! @Param[in]      filterAnnotationScaleIn Annotation scale value to pass (when filterIn = CAS_FILTER_ANNOTATION_SCALE_SPECIFIED).
//! @Return SUCCESS if the filter is successfully set
//! @Group          "Model Properties"
DGNPLATFORM_EXPORT StatusInt   mdlChangeAnnotationScale_setFilter
(
ChangeAnnotationScaleP      changeContextIn,
CASFilter                   filterIn,
double                      filterAnnotationScaleIn
);

//! To change the annotation scale of elements in a model, get the filter.
//! @Param[in]      changeContextIn             Change annotation scale context specifier.
//! @Param[out]     filterOut                   Filter.
//! @Param[out]     filterAnnotationScaleOut    Annotation scale value to pass.
//! @Return SUCCESS if the filter can be successfully got
//! @Group          "Model Properties"
DGNPLATFORM_EXPORT StatusInt   mdlChangeAnnotationScale_getFilter
(
ChangeAnnotationScaleCP     changeContextIn,
CASFilter *                 filterOut,
double *                    filterAnnotationScaleOut
);

//! To change the annotation scale of elements in a model, set the action type
//! @Param[in]      changeContextIn         Change annotation scale context specifier.
//! @Param[in]      actionIn                Action.
//! @Param[in]      newAnnotationScaleIn    Annotation scale value to apply on element (when actionIn = CAS_APPLY_ANNOTATION_SCALE).
//! @Return SUCCESS if the action is successfully set
//! @Group          "Model Properties"
DGNPLATFORM_EXPORT StatusInt   mdlChangeAnnotationScale_setAction
(
ChangeAnnotationScaleP      changeContextIn,
DgnPlatform::AnnotationScaleAction actionIn,
double                      newAnnotationScaleIn
);

//! To change the annotation scale of elements in a model, get the action.
//! @Param[in]      changeContextIn         Change annotation scale context specifier.
//! @Param[out]     actionOut               Action.
//! @Param[out]     newAnnotationScaleOut   Annotation scale value to apply on element.
//! @Return Action
//! @Group          "Model Properties"
DGNPLATFORM_EXPORT StatusInt   mdlChangeAnnotationScale_getAction
(
ChangeAnnotationScaleCP     changeContextIn,
DgnPlatform::AnnotationScaleAction * actionOut,
double *                    newAnnotationScaleOut
);

//! Get the effective annotation scale and flag based on the input values
//!                 and CASAction.
//! @Param[in]      changeContextIn         Change annotation scale context specifier.
//! @Param[out]     newAnnotationScale      New annotation scale value to apply on element.
//! @Param[out]     newUseAnnotationScale   New use annotation scale value to apply on element.
//! @Param[out]     annotationScaleChange   Change in annotation scale value to apply on element.
//! @Param[in]      currAnnotationScale     Current annotation scale value.
//! @Param[in]      currUseAnnotationScale  Current annotation scale flag.
//! @Return Action
//! @Group          "Model Properties"
DGNPLATFORM_EXPORT StatusInt   mdlChangeAnnotationScale_getEffectiveValues
(
ChangeAnnotationScaleCP     changeContextIn,
double *                    newAnnotationScale,
bool    *                   newUseAnnotationScale,
double *                    annotationScaleChange,
double                      currAnnotationScale,
bool                        currUseAnnotationScale
);

END_BENTLEY_API_NAMESPACE
