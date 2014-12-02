/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnHandlers/NoteHandler.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
/** @cond BENTLEY_SDK_Internal */

#include "CellHeaderHandler.h"
#include <Bentley/bvector.h>
#include <DgnPlatform/DgnHandlers/DimensionStyle.h>
BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

/** @addtogroup DisplayHandler */
/** @beginGroup */
#if defined (NEEDS_WORK_DGNITEM)

/*=================================================================================**//**
* A Note Cell is-a a sub-type of cell. The note element consists of a leader and a cell
* which are only accessible through this handler. The properties in a note element is 
* controlled by the dimensionstyle defined by the leader element. The note also stores 
* this data internally for leader less notes.
* @bsiclass                                                     Sunand.Sandurkar 10/2004
+===============+===============+===============+===============+===============+======*/
struct EXPORT_VTABLE_ATTRIBUTE NoteCellHeaderHandler : Type2Handler,
                                         IAnnotationHandler,
                                         ITextEdit
                                         //__PUBLISH_SECTION_END__
                                         ,ISubTypeHandlerQuery
                                         //__PUBLISH_SECTION_START__
{
    DEFINE_T_SUPER(Type2Handler)
    ELEMENTHANDLER_DECLARE_MEMBERS (NoteCellHeaderHandler, DGNPLATFORM_EXPORT)
//__PUBLISH_SECTION_END__
private:
void                            DrawWithAnnotationScale (ElementHandleCR, ViewContextP, double);
static BentleyStatus            AddDimensionNoteCellAssociation (EditElementHandleR dimElement, ElementId cellId);

protected:
DGNPLATFORM_EXPORT virtual void                    _GetTypeName (WStringR string, UInt32 desiredLength) override;
DGNPLATFORM_EXPORT virtual void                    _GetDescription (ElementHandleCR el, WStringR descr, UInt32 desiredLength) override;
DGNPLATFORM_EXPORT virtual void                    _GetPathDescription (ElementHandleCR, WStringR, DisplayPathCP, WCharCP levelStr, WCharCP modelStr, WCharCP groupStr, WCharCP delimiter) override;
DGNPLATFORM_EXPORT virtual void                    _EditProperties (EditElementHandleR eeh, PropertyContextR context) override;
DGNPLATFORM_EXPORT virtual StatusInt               _OnTransform (EditElementHandleR, TransformInfoCR) override;
DGNPLATFORM_EXPORT virtual bool                    _IsTransformGraphics (ElementHandleCR, TransformInfoCR) override;
DGNPLATFORM_EXPORT virtual void                    _Draw (ElementHandleCR, ViewContextR) override;
DGNPLATFORM_EXPORT virtual void                    _GetElemDisplayParams (ElementHandleCR, ElemDisplayParams&, bool wantMaterials = false) override;
//DGNPLATFORM_EXPORT virtual StatusInt               _OnPreprocessCopy (EditElementHandleR symbolEH, ElementCopyContextP ccP) override; removed in graphite
DGNPLATFORM_EXPORT virtual IAnnotationHandlerP     _GetIAnnotationHandler (ElementHandleCR)  override {return this;}
DGNPLATFORM_EXPORT virtual bool                    _GetAnnotationScale (double* annotationScale, ElementHandleCR element) const override;
DGNPLATFORM_EXPORT virtual StatusInt               _OnFenceStretch (EditElementHandleR, TransformInfoCR, FenceParamsP, FenceStretchFlags) override;
DGNPLATFORM_EXPORT virtual StatusInt               _OnDrop (ElementHandleCR eh, ElementAgendaR dropGeom, DropGeometryCR geometry) override;

// ITextEdit
DGNPLATFORM_EXPORT virtual bool                    _DoesSupportFields  (ElementHandleCR) const override;
DGNPLATFORM_EXPORT virtual ITextPartIdPtr          _GetTextPartId      (ElementHandleCR, HitPathCR) const override;
DGNPLATFORM_EXPORT virtual void                    _GetTextPartIds     (ElementHandleCR, ITextQueryOptionsCR, T_ITextPartIdPtrVectorR) const override;
DGNPLATFORM_EXPORT virtual TextBlockPtr            _GetTextPart        (ElementHandleCR, ITextPartIdCR) const override;
DGNPLATFORM_EXPORT virtual ReplaceStatus           _ReplaceTextPart    (EditElementHandleR, ITextPartIdCR, TextBlockCR) override;
BentleyStatus                                      GetInvisibleDimension (ElementHandleR dimElement, ElementHandleCR noteElement);
public:
DGNPLATFORM_EXPORT virtual bool _ClaimElement (ElementHandleCR) override;
void                                               DrawChildren (ElementHandleCR thisElm, ViewContextR context);
StatusInt                                          GetOffsetAssociationLocation (ElementHandleCR noteCell, DPoint3dR point);

//! Get Leader element. This is the dimension element which controls the note properties.
//! @Remarks If the note has no leader this returns an invisible dimension stored in a note cell which controls the note settings.
DGNPLATFORM_EXPORT BentleyStatus    GetLeaderDimension (ElementId& leaderId, DPoint3dP hookPoint, ElementHandleCR noteElement);

DGNPLATFORM_EXPORT BentleyStatus    SetupLeaderEndTangent(EditElementHandleR leaderDimension, DimStyleProp_MLNote_TextRotation textRotation, bool rightSideAttachment);

                   BentleyStatus    UpdateFieldTarget (EditElementHandleR noteCell, ElementHandleCR newtarget);

                   BentleyStatus    UpdateOffsetAssociation (EditElementHandleR noteCell, ElementHandleCR leader);

DGNPLATFORM_EXPORT BentleyStatus    SetupOffsetAssociation (EditElementHandleR noteCell, ElementHandleCR targetElement, AssocPoint const& assoc);

//__PUBLISH_SECTION_START__
//__PUBLISH_CLASS_VIRTUAL__
public:

typedef bvector<DPoint3d> StdDPointVector;

//! Creates a new note element.
//! @param    noteElem      OUT the newly created note element.
//! @param    leaderElement OUT The dimension element which will be the note Leader.
//! @param    text          IN  The text information for the note element
//! @param    dimStyle      IN  A style pointer which will provide the note settings.
//! @param    is3d          IN  flag to control whether to create a 3d element.
//! @param    modelRef      IN  destination model of the note.
//! @param    noteLeaderPoints      IN  the points which would define the note leader dimension.
//! @return   status.       SUCCESS or ERROR  
DGNPLATFORM_EXPORT static BentleyStatus     CreateNote (EditElementHandleR noteElem, EditElementHandleR leaderElement, TextBlockCR text, DimensionStyleCR dimStyle,
                                                            bool is3d, DgnModelR modelRef, StdDPointVector const & noteLeaderPoints);
//! Adds the note element to a model.
DGNPLATFORM_EXPORT static BentleyStatus     AddToModel (EditElementHandleR noteElement, EditElementHandleR leaderElement, DgnModelR dgnCache);

//! Get Leader element. 
DGNPLATFORM_EXPORT BentleyStatus     GetNoteLeader(ElementHandleR leader, ElementHandleCR noteElement);

//! Get dimension style associated with a note.
DGNPLATFORM_EXPORT DimensionStylePtr GetNoteDimensionStyle(ElementHandleCR noteElement);

//! Set dimension style associated with a note.
//! @remarks. Both the leader and note element needs to be replaced in the model after this
DGNPLATFORM_EXPORT BentleyStatus     SetNoteDimensionStyle(EditElementHandleR noteElement, EditElementHandleP leaderElement, DimensionStyleCR dimStyle);

}; // NoteCellHeaderHandler
#endif

/** @endGroup */

END_BENTLEY_DGNPLATFORM_NAMESPACE

/** @endcond */
