/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnHandlers/Note.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

#define HORJUSTMODE(just)       (((just) == TextElementJustification::LeftTop || (just) == TextElementJustification::LeftMiddle || (just) == TextElementJustification::LeftBaseline) ? DIMSTYLE_VALUE_MLNote_Justification_Left : \
                                (((just) == TextElementJustification::CenterTop || (just) == TextElementJustification::CenterMiddle || (just) == TextElementJustification::CenterBaseline) ? DIMSTYLE_VALUE_MLNote_Justification_Center: DIMSTYLE_VALUE_MLNote_Justification_Right))

#define VERJUSTMODE(just)       (((just) == TextElementJustification::LeftTop || (just) == TextElementJustification::CenterTop || (just) == TextElementJustification::RightTop) ? DIMSTYLE_VALUE_MLNote_VerticalJustification_Top : \
                                (((just) == TextElementJustification::LeftMiddle || (just) == TextElementJustification::CenterMiddle || (just) == TextElementJustification::RightMiddle) ? DIMSTYLE_VALUE_MLNote_VerticalJustification_Center : DIMSTYLE_VALUE_MLNote_VerticalJustification_Bottom))

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

/*----------------------------------------------------------------------+
|                                                                       |
|   NoteDataLinkage                                                     |
|                                                                       |
+----------------------------------------------------------------------*/
typedef struct msNoteLinkage
    {
    LinkageHeader               header;
    MSNoteData                  data;
    } MSNoteLinkage;

END_BENTLEY_DGNPLATFORM_NAMESPACE

BEGIN_BENTLEY_API_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar 04/04
*
* NOTE : Every caller who wants to regenerate notes should call mdlNote_rectifySettings
*        before. mdlNote_create function assumes that all the settings are available,
*        and will not do any reverse-engineering rectification.
*
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT StatusInt     mdlNote_create
(
EditElementHandleR         newCellElmHandle,           // <=  new multiline note. If no text, NULL EdP will be returned
ElementHandleCP            newTextElemHandle,          //  => new text node with identity transform. NULL if removing entire text.
DPoint3dCP                 cellOrigin,                 //  => cell origin, required for new creation
ElementHandleCR            dimElement,                 //  => associated dim element with updated settings. If null, use geometry to derive settings
ElementHandleCP            oldCellElemHandle,          //  => required only when dimElemHandle is invalid
DgnModelP               modelRef,                   //  => modelref to create in
bool                       bRetainCellOrigin,          //  => recreate note around the previous cell origin. If No, the cell is allowed to shift due to some settings.
double *                   pNewAnnotScale,             //  => new annotation scale of note, if changing, else NULL
double *                   pOldAnnotScale              //  => annotation scale of note, if changing, NULL otherwise
);

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    SunandSandurkar 05/04
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT StatusInt     mdlNote_replaceText
(
EditElementHandleR         newCellElemHandle,          // <=  new multiline note. If no text, NULL EdP will be returned
ElementHandleCR            oldCellElemHandle,
ElementHandleCR            newTextElemHandle,
DgnModelP               modelRef
);

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    SunandSandurkar 12/05
* This function updates the missing dimsettings on the dimension element by reverse-
* engineering the note cell. Call this function before touching an existing note.
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT StatusInt     mdlNote_rectifySettingsAndCommit
(
ElementHandleR             cellElemHandle,     //  =>
DgnModelP               modelRef            //  =>
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar 04/04
*
*
*   pTextNodeEd -> next(TextFrameShape) -> next(InlineLeaderLine)
*        |
*         -> firstElem(TextElement1) -> next(TextElement2) ....
*
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT StatusInt     mdlNote_findTextNodeElement
(
ElementHandleR     txtNode,
ElementHandleCR    cellElement
);

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    SunandSandurkar 05/06
*
* Note : This function will directly return the pointer to the edP in the cell.
*
*   pTextNodeEd -> next(TextFrameShape) -> next(InlineLeaderLine)
*        |
*         -> firstElem(TextElement1) -> next(TextElement2) ....
*
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT StatusInt     mdlNote_findCellComponent
(
ElementHandleR      component,
int                 elementType,
ElementHandleCR     cellEdP
);

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    SunandSandurkar 05/04
* Note : Use this function when you have the cell
* ReturnStatus : MDLERR_ALREADYEXISTS if dependency already exists. SUCCESS if
*                added successfully.
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT StatusInt     mdlNote_addDimDependencyToCell
(
EditElementHandleR         cellElm
);

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    DonFu           09/02
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT BentleyStatus mdlNote_getRootNoteCellId
(
DgnPlatform::ElementId*             pCellId,
ElementHandleCR            dimensionElement
);

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    DonFu           09/02
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT ElementRefP    mdlNote_getRootNoteCellElmRef
(
ElementHandleCR            dimensionElement
);

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    BradRushing     09/95
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT int           mdlNote_appendNoteCellLinkage
(
DgnElementP               pNoteCell
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          09/12
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT StatusInt    mdlNote_addNoteCellLinkage (EditElementHandleR cellElement);

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    DonFu           09/02
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT int          mdlNote_setNoteAssocPoint
(
EditElementHandleR              dimElement,
DgnPlatform::ElementId          noteCellId,
int                             pointNo
);

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    DonFu           09/02
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT StatusInt mdlNote_getHookLineFromNoteCell (DPoint3dP pHookLine, ElementHandleCR dimensionElement);

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    SunandSandurkar 12/04
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT StatusInt     mdlNote_getTextNodeJust
(
DgnPlatform::TextElementJustification&  just,
ElementHandleCR                         element              /* Node or cell containing node */
);

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    SunandSandurkar 11/04
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT StatusInt     mdlNote_getRootDimension
(
DgnPlatform::ElementId* pRootDimId,
DgnElementCR             cellElem
);

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    SunandSandurkar 12/05
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT StatusInt     mdlNote_rectifyDimSettings
(
bool    *               changed,            // <=  true if any settings were changed. Use to determine if dim should be rewritten
EditElementHandleR      newDimElemHandle,   // <=  Rectified dimension
ElementHandleCR         dimElemHandle,      //  => Current dimension to be rectified
DgnModelP            modelRef            //  =>
);

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    SunandSandurkar 02/06
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT StatusInt               mdlNote_findTextNode
(
EditElementHandleR      textElemHandle,
ElementHandleCR         dimElemHandle,
DgnModelP            modelRef
);

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    SunandSandurkar 05/04
* Note : Use this function when you have the leader-dimension element
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT StatusInt        mdlNote_addDependencyToCell (ElementHandleCR dimensionElement);

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    SunandSandurkar 12/05
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT StatusInt        mdlNote_updateSettings
(
EditElementHandleR      newCellElemHandle,          // <=  new multiline note. If no text, NULL EdP will be returned
ElementHandleCP         newTextElemHandle,          //  => new text node with identity transform. NULL if removing entire text.
DPoint3dCP              cellOrigin,                 //  => cell origin, required for new creation
ElementHandleCR         dimElemHandle,              //  => associated dim element with updated settings. If null, use geometry to derive settings
ElementHandleCP         oldDimElemHandle,           //  => associated dim element w/o  updated settings. If settings are same, pass NULL
ElementHandleCP         oldCellElemHandle,          //  => required only when dimElemHandle is invalid
DgnModelP            modelRef,                   //  => modelref to create in
bool                    bRetainCellOrigin,          //  => recreate note around the previous cell origin. If No, the cell is allowed to shift due to some settings.
double *                pNewAnnotScale,             //  => new annotation scale of note, if changing, else NULL
double *                pOldAnnotScale              //  => annotation scale of note, if changing, NULL otherwise
);

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    SunandSandurkar 05/04
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT bool          mdlNote_isPropertyUpdateAllowed
(
DgnPlatform::DimStyleProp            eProp,
int                     option
);

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    SunandSandurkar 07/06
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT bool          mdlNote_isDimInNoteCell
(
ElementRefP             dimElemRef,
DgnModelP            modelRef
);

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    SunandSandurkar 07/06
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT  StatusInt    mdlNote_updateDimSettings
(
EditElementHandleR      newNoteElement,
ElementHandleCR         oldNoteElement,
ElementHandleCR         newDimElement
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar 07/06
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT bool          mdlNote_getAnnotationScale
(
double *                annotationScale,
ElementHandleCR         noteCellElemHandle
);

DGNPLATFORM_EXPORT bool     mdlNote_isNote (ElementHandleCR);

END_BENTLEY_API_NAMESPACE
