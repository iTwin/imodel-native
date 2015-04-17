/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnHandlers/IManipulator.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
/** @cond BENTLEY_SDK_Internal */

#include <DgnPlatform/DgnCore/ElementHandle.h>

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

/*=================================================================================**//**
* @addtogroup Manipulators
* A manipulator maintains a set of controls used to modify an element. The manipulator
* is a new object that gets created by the handler for a supplied ElementHandle. The
* IDragManipulator is one type of manipulator in which the controls are driven by
* view based point to point changes.
* @beginGroup
* @bsiclass
+===============+===============+===============+===============+===============+======*/
/*=================================================================================**//**
* Interface adopted by an element handler class to present manipulator controls.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct     IDragManipulator : public IElementState
{
//__PUBLISH_SECTION_END__
protected:

virtual bool        _OnCreateControls (ElementHandleCR elHandle) = 0;
virtual void        _OnCleanupControls (ElementHandleCR elHandle) = 0;
virtual bool        _OnSelectControl (ElementHandleCR elHandle, DgnButtonEventCR ev) = 0;
virtual bool        _OnSelectControl (ElementHandleCR elHandle, HitPathCP path) = 0;
virtual bool        _OnMultiSelectControls (ElementHandleCR elHandle, DgnButtonEventCR ev, SelectionMode mode) = 0;
virtual bool        _OnMultiSelectControls (ElementHandleCR elHandle, FenceParamsP fp, SelectionMode mode) = 0;
virtual bool        _HasSelectedControls (ElementHandleCR elHandle) = 0;
virtual void        _OnDraw (ElementHandleCR elHandle, DgnViewportP vp) = 0;
virtual Utf8String  _OnGetDescription (ElementHandleCR elHandle) = 0;
virtual bool        _OnSetupDrag (DgnButtonEventR ev, EditElementHandleR elHandle) = 0;
virtual void        _OnStartDrag (ElementHandleCR elHandle, DgnButtonEventCR ev) = 0;
virtual void        _OnCancelDrag (ElementHandleCR elHandle, DgnButtonEventCR ev) = 0;
virtual StatusInt   _OnDrag (EditElementHandleR elHandle, DgnButtonEventCR ev) = 0;
virtual StatusInt   _OnEndDrag (EditElementHandleR elHandle, DgnButtonEventCR ev) = 0;
virtual StatusInt   _DoDragControls (EditElementHandleR elHandle, DgnButtonEventCR ev, bool isDynamics) = 0;
virtual bool        _OnRightClick (ElementHandleCR elHandle, DgnButtonEventCR ev) {return false;}
virtual void        _OnNewPath (ElementHandleCR elHandle, DisplayPathCP path) {}
virtual bool        _OnDoubleClick (ElementHandleCR elHandle, DisplayPathCP path) {return false;}
virtual void        _OnDragModifierKeyTransition (ElementHandleCR elHandle, bool wentDown, int key) {}
virtual bool        _IsViewDynamicsControl (ElementHandleCR elHandle, DgnViewportP vp) {return false;}
virtual StatusInt   _OnViewMotion (EditElementHandleR elHandle, DgnButtonEventCR ev) {return ERROR;}
virtual bool        _OnClick(DgnButtonEventCR, EditElementHandleR) { return false; }

public:

virtual ~IDragManipulator () {}

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__
public:

/*---------------------------------------------------------------------------------**//**
* Called to have manipulator extract the data from ElementHandle and populate it's
* internal set of controls.
* @param[in] elHandle           The element to create controls for.
* @return true if controls were created for this elemHandle.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT bool OnCreateControls (ElementHandleCR elHandle);

/*---------------------------------------------------------------------------------**//**
* Called to have manipulator cleanup it's internal control data so that
* OnCreateControls might be called again. For example after a successful drag,
* OnCleanupControls will be called followed by OnCreateControls in order to
* have the controls reflect the changes to the element.
* @param[in] elHandle           The element being manipulated.
* @return true if controls were created for this elemHandle.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT void OnCleanupControls (ElementHandleCR elHandle);

/*---------------------------------------------------------------------------------**//**
* Flash control(s) button event is over, hilited control is considered to be selected.
* Called during motion event, controls should not remain selected when button event
* moves off them.
* @param[in] elHandle           The element being manipulated.
* @param[in] ev                 Current button event
* @return true if button event is over a control.
* @see
*   ManipulatorUtils::DoSelectControl
*   ManipulatorUtils::TestPointOnSprite
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT bool OnSelectControl (ElementHandleCR elHandle, DgnButtonEventCR ev);

/*---------------------------------------------------------------------------------**//**
* Select control(s) using the supplied locate hit path. Choose the control(s)
* that make the most sense. This type of selection is necessary for modify type tools
* that want to use manipulators and wish to select both the element and controls with
* a single data button. It should be uncommon for no control to be selected, so
* the ManipulatorUtils method to select the closest control is a good backup for when
* the pick is ambiguous. The drag would start immediately and most likely the
* manipulator will not ever be asked to draw it's controls.
* @param[in] elHandle           The element being manipulated.
* @param[in] path               Current locate path
* @return true if button event is over a control.
* @see
*   ManipulatorUtils::DoSelectControl
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT bool OnSelectControl (ElementHandleCR elHandle, HitPathCP path);

/*---------------------------------------------------------------------------------**//**
* Select control(s) that the button event is over. Controls should stay selected
* until specifically de-selectled.
* @param[in] elHandle           The element being manipulated.
* @param[in] ev                 Current button event.
* @param[in] mode               Current selection mode.
* @return true any controls remain selected.
* @see
*   ManipulatorUtils::DoMultiSelectControls
*   ManipulatorUtils::TestPointOnSprite
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT bool OnMultiSelectControls (ElementHandleCR elHandle, DgnButtonEventCR ev, SelectionMode mode);

/*---------------------------------------------------------------------------------**//**
* Select control(s) that are inside the fence. Controls should stay selected
* until specifically de-selectled.
* @param[in] elHandle           The element being manipulated.
* @param[in] fp                 Current fence.
* @param[in] mode               Current selection mode.
* @return true any controls remain selected.
* @see
*   ManipulatorUtils::DoMultiSelectControls
*   FenceParams::PointInside
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT bool OnMultiSelectControls (ElementHandleCR elHandle, FenceParamsP fp, SelectionMode mode);

/*---------------------------------------------------------------------------------**//**
* Return whether manipulator currently has any controls selected.
* @param[in] elHandle           The element being manipulated.
* @return true if manipulator has controls currently selected.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT bool HasSelectedControls (ElementHandleCR elHandle);

/*---------------------------------------------------------------------------------**//**
* Manipulator is being asked to display some representation of it's controls in the
* supplied viewport. Typically this is done by drawing sprites. The commonly used
* control sprites are defined in MstnWinRsrcIds.h
*<p>
* ICONID_HANDLE_FOCUSED - default selected control sprite.
*<p>
* ICONID_HANDLE_UNFOCUSED - default unselected control sprite.
*<p>
* ICONID_HANDLE_MOVE_FOCUSED - selected sprite for control that is a translation.
*<p>
* ICONID_HANDLE_MOVE_UNFOCUSED - unselected sprite for control that is a translation.
*<p>
* ICONID_HANDLE_COPY_FOCUSED - selected sprite for control that translates/copies.
*<p>
* ICONID_HANDLE_COPY_UNFOCUSED - selected sprite for control that translates/copies.
*<p>
* ICONID_HANDLE_VECTOR_FOCUSED - selected sprite for control that defines a bvector.
*<p>
* ICONID_HANDLE_VECTOR_UNFOCUSED - unselected sprite for control that defines a bvector.
*<p>
* ICONID_HANDLE_INDIRECT_FOCUSED - selected sprite for control that does not perform point-to-point modification.
*<p>
* ICONID_HANDLE_INDIRECT_UNFOCUSED - unselected sprite for control that does not perform point-to-point modification.
* @param[in] elHandle           The element being manipulated.
* @param[in] vp                 DgnViewport to display the controls in.
* @see
*   ManipulatorUtils::DoDrawControls
*   ViewManager::LoadSpriteFromRsrc
*   IViewDraw::DrawSprite
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT void OnDraw (ElementHandleCR elHandle, DgnViewportP vp);

/*---------------------------------------------------------------------------------**//**
* Return a string suitable for display in a tool tip to describe the currently
* selected controls. Shown when the cursor pauses over a control.
* @param[in] elHandle           The element being manipulated.
* @return String to describe selected controls or an empty string for no description.
* @see
*   ManipulatorUtils::GetDefaultDescription
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT Utf8String OnGetDescription (ElementHandleCR elHandle);

/*---------------------------------------------------------------------------------**//**
* Called on right-click while over a control, one use of this event would be to
* present the user with a menu of editing options.
* @param[in] elHandle           Element being manipulated.
* @param[in] ev                 Current button event,
* @return true is manipulator wants to do something and handled the right-click event.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT bool OnRightClick (ElementHandleCR elHandle, DgnButtonEventCR ev);

/*---------------------------------------------------------------------------------**//**
* Called when manipulator is displaying controls and user clicks on the element again,
* but not on a control. Manipulator may choose to present the user with a different
* set of controls.
* @param[in] elHandle           The element being manipulated.
* @param[in] path               Locate path.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT void OnNewPath (ElementHandleCR elHandle, DisplayPathCP path);

/*---------------------------------------------------------------------------------**//**
* Called when user double clicks on the manipulator's element. (ex. edit text)
* @param[in] elHandle           The element being manipulated.
* @param[in] path               Locate path.
* @return true if manipulator handled double-click event.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT bool OnDoubleClick (ElementHandleCR elHandle, DisplayPathCP path);

/*---------------------------------------------------------------------------------**//**
* When manipulators are active on a selection set, only the one whose control is actually
* selected to begin the drag operation is given responsibility for setting up the drag.
* Main required responsbility is setting the anchor point used by all active manipulators
* to the center of the selected control. This manipulator is also responsible for
* enabling AccuSnap and setting up AccuDraw.
* @note If you merely want a control to handle clicks (and not drags), implement OnClick instead. OnClick is called before this method; if OnClick returns true (e.g. handled), then this is NOT called (e.g. no drag operation is attempted).
* @param[in,out] ev             Current button event, point needs to be set to center of selected control.
* @param[in] elHandle           Element being manipulated.
* @return false to reject drag
* @see
*   ManipulatorUtils::DoSetupDrag
*   OnClick
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT bool OnSetupDrag (DgnButtonEventR ev, EditElementHandleR elHandle);

/*---------------------------------------------------------------------------------**//**
* Called when the drag operation is about to begin. It is the responsibily of the
* manipulator to keep the point in the supplied button event to use as the anchor
* point of the drag operation.
* @param[in] elHandle           Element being manipulated.
* @param[in] ev                 Current button event,
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT void OnStartDrag (ElementHandleCR elHandle, DgnButtonEventCR ev);

/*---------------------------------------------------------------------------------**//**
* Called if user aborts drag operation (reset), manipulator can cleanup anything done
* in OnStartDrag.
* @param[in] elHandle           Element being manipulated.
* @param[in] ev                 Current button event,
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT void OnCancelDrag (ElementHandleCR elHandle, DgnButtonEventCR ev);

/*---------------------------------------------------------------------------------**//**
* Called to show drag dynamics. Expected to call DoDragControls with isDynamics set
* to true, and to display the result if DoDragControls returned SUCCESS.
* @param[in,out] elHandle       Element being manipulated.
* @param[in] ev                 Current button event,
* @return SUCCESS if drag operation was a success.
* @see
*   ManipulatorUtils::DoDrag
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT StatusInt OnDrag (EditElementHandleR elHandle, DgnButtonEventCR ev);

/*---------------------------------------------------------------------------------**//**
* Called to accept drag operation. Expected to call DoDragControls with isDynamics set
* to false, and to rewrite the result to the design file if DoDragControls returned SUCCESS.
* @param[in,out] elHandle       Element being manipulated.
* @param[in] ev                 Current button event,
* @return SUCCESS if drag operation was a success and EditElementHandleR was updated.
* @see
*   ManipulatorUtils::DoEndDrag
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT StatusInt OnEndDrag (EditElementHandleR elHandle, DgnButtonEventCR ev);

/*---------------------------------------------------------------------------------**//**
* It is expected that this is where all the work is done to update the element.
* Using the anchor point from OnStartDrag and the supplied button event for the cursor
* location, update the element data.
* @param[in,out] elHandle       Element being manipulated.
* @param[in] ev                 Current button event,
* @param[in] isDynamics         Whether tool is being called from OnDrag or OnEndDrag.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT StatusInt DoDragControls (EditElementHandleR elHandle, DgnButtonEventCR ev, bool isDynamics);

/*---------------------------------------------------------------------------------**//**
* Called if a modifier key goes up or down while dragging. Manipulator can use this
* to cycle between different drag behaviors for a control, ex. move or copy.
* @param[in] elHandle           Element being manipulated.
* @param[in] wentDown           Modifier key going down or up.
* @param[in] key                What modifier is changing.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT void OnDragModifierKeyTransition (ElementHandleCR elHandle, bool wentDown, int key);

/*---------------------------------------------------------------------------------**//**
* For a manipulator that wants to dynamically update the view, not just show element
* dynamics, it will be given the opportunity by calling this method only in the case
* where it is the only active manipulator. If this returns true, OnViewMotion will be
* called instead of OnDrag and complex dynamics will not be setup.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT bool IsViewDynamicsControl (ElementHandleCR elHandle, DgnViewportP vp);

/*---------------------------------------------------------------------------------**//**
* Called to allow manipulator that returned true for IsViewDynamicsControl to implement
* view dynamics.
* @param[in,out] elHandle       Element being manipulated.
* @param[in] ev                 Current button event,
* @return SUCCESS if drag operation was a success.
* @see
*   ManipulatorUtils::DoDrag
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT StatusInt OnViewMotion (EditElementHandleR elHandle, DgnButtonEventCR ev);

//! This is called on a data button down. The intent is to allow controls to react to mere clicks, and prevent drag processing.
//! @note This is called prior to _OnSetupDrag. If you return true from this method (e.g. you handled the event), then the drag operation is NOT attempted.
DGNPLATFORM_EXPORT bool OnClick(DgnButtonEventCR, EditElementHandleR);

}; // IDragManipulator

typedef RefCountedPtr<IDragManipulator> IDragManipulatorPtr;

/*================================================================================**//**
* Extension to provide DragManipulators for an element handler.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct IDragManipulatorExtension : DgnDomain::Handler::Extension
{
protected:

virtual IDragManipulatorPtr _GetIDragManipulator (ElementHandleCR elHandle, DisplayPathCP path) = 0;

public:

HANDLER_EXTENSION_DECLARE_MEMBERS(IDragManipulatorExtension, DGNPLATFORM_EXPORT)
DGNPLATFORM_EXPORT IDragManipulatorPtr GetIDragManipulator (ElementHandleCR elHandle, DisplayPathCP path);

}; // IDragManipulatorExtension

/*=================================================================================**//**
* Interface adopted by ggwwan element handler class to participate with transform tools.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct     ITransformManipulator : public IElementState
{
//__PUBLISH_SECTION_END__
protected:

virtual bool        _IsTransformGraphics (ElementHandleCR elHandle, TransformInfoCR tInfo) const = 0;
virtual StatusInt   _OnTransform (EditElementHandleR elHandle, TransformInfoCR tInfo, ViewContextP context) = 0;

virtual bool        _AcceptElement (ElementHandleCR elHandle, Utf8StringP cantAcceptReason, ElementAgendaP agenda) {return true;}
virtual bool        _OnModifyChanged (ElementHandleCR elHandle, ElementAgendaP agenda, AgendaOperation opType, AgendaModify modify) {return true;}
//virtual StatusInt   _OnPreTransform (EditElementHandleR elHandle, CopyContextP ccP) {return ERROR;} removed in graphite
virtual StatusInt   _OnFenceClip (EditElementHandleR elHandle, FenceParamsP fp, FenceClipFlags flags) {return ERROR;}
virtual bool        _GetAboutCenterPoint (DPoint3d& pt, ElementHandleCR elHandle) {return false;}
virtual bool        _GetAlignExtents (DRange3d* range, EditElementHandleR elHandle, TransformCP transform) {return false;}

public:

virtual ~ITransformManipulator () {}

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__
public:

/*---------------------------------------------------------------------------------**//**
* Called to allow manipulator to reject this element for a given transform operation.
* @param[in] elHandle           The element manipulator is for.
* @param[in] cantAcceptReason   Reason why element can't be manipulated (NULL when not called from auto-locate).
* @param[in] agenda             ElementAgenda being processed, may be NULL.
* @return true if this element wants to be part of transform agenda.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT bool AcceptElement (ElementHandleCR elHandle, Utf8StringP cantAcceptReason, ElementAgendaP agenda);

/*---------------------------------------------------------------------------------**//**
* Called to allow manipulator to reject this element for a given transform operation.
* @param[in] elHandle           The element manipulator is for.
* @param[in] agenda             ElementAgenda being processed, may be NULL.
* @param[in] opType             The type operation to be applied.
* @param[in] modify             The modification type for the operation.
* @return false if manipulator doesn't support new modify operation.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT bool OnModifyChanged (ElementHandleCR elHandle, ElementAgendaP agenda, AgendaOperation opType, AgendaModify modify);

/*---------------------------------------------------------------------------------**//**
* Called to allow manipulator to override IsTransformGraphics on handler.
* @param[in] elHandle           The element manipulator is for.
* @param[in] tInfo              The transform to apply to element.
* @return true if transform can be pushed for dynamics and OnTransform doesn't need to be called.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT bool IsTransformGraphics (ElementHandleCR elHandle, TransformInfoCR tInfo) const;

/*---------------------------------------------------------------------------------**//**
* Called to allow manipulator participate in pre-transform logic like calling mdlElmdscr_copy.
* @param[in,out] elHandle       The element manipulator is for.
* @param[in] ccP                Current copy context (NULL if not copying).
* @return SUCCESS If manipulator completely handled this event and default action is not to be taken.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
//DGNPLATFORM_EXPORT StatusInt OnPreTransform (EditElementHandleR elHandle, CopyContextP ccP); removed in graphite

/*---------------------------------------------------------------------------------**//**
* Called to allow manipulator to override ApplyTransform on handler.
* @param[in,out] elHandle       The element manipulator is for.
* @param[in] tInfo              The transform to apply to element.
* @param[in] context            NULL for accept...otherwise it's dynamics.
* @return ERROR If manipulator completely handled...including adding/rewrite of element to file.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT StatusInt OnTransform (EditElementHandleR elHandle, TransformInfoCR tInfo, ViewContextP context);

/*---------------------------------------------------------------------------------**//**
* Called to allow manipulator to override OnFenceClip on handler.
* @param[in,out] elHandle       The element manipulator is for.
* @param[in] fp                 The fence parameters to apply
* @param[in] flags              Current fence options
* @return SUCCESS If manipulator completely handled this event and default action is not to be taken.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT StatusInt OnFenceClip (EditElementHandleR elHandle, FenceParamsP fp, FenceClipFlags flags);

/*---------------------------------------------------------------------------------**//**
* Called to allow manipulator to return "About Center" point for rotate/scale tools.
* @param[out] pt                The element center/origin point.
* @param[in] elHandle           The element manipulator is for.
* @return true if pt is valid.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT bool GetAboutCenterPoint (DPoint3d& pt, ElementHandleCR elHandle);

/*---------------------------------------------------------------------------------**//**
* Called to allow manipulator to return "About Center" point for rotate/scale tools.
* @param[out] range             The element range box.
* @param[in] elHandle           The element manipulator is for.
* @param[in] transform          The coordinate system to compute the range in (NULL if active requested).
* @return true if range is valid.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT bool GetAlignExtents (DRange3d* range, EditElementHandleR elHandle, TransformCP  transform);

}; // ITransformManipulator

typedef RefCountedPtr<ITransformManipulator> ITransformManipulatorPtr;

/*================================================================================**//**
* Extension to provide transform manipulator for an element handler.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ITransformManipulatorExtension : DgnDomain::Handler::Extension
{
protected:

    virtual ITransformManipulatorPtr _GetITransformManipulator (ElementHandleCR elHandle, AgendaOperation opType, AgendaModify modify, DisplayPathCP path) = 0;

public:
    HANDLER_EXTENSION_DECLARE_MEMBERS(ITransformManipulatorExtension, DGNPLATFORM_EXPORT)
    DGNPLATFORM_EXPORT ITransformManipulatorPtr GetITransformManipulator (ElementHandleCR elHandle, AgendaOperation opType, AgendaModify modify, DisplayPathCP path);

}; // ITransformManipulatorExtension

/*=================================================================================**//**
* Interface adopted by an element handler class to participate with delete tools.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct     IDeleteManipulator : public IElementState
{
private:
    DECLARE_KEY_METHOD

//__PUBLISH_SECTION_END__
protected:

virtual StatusInt   _OnDelete (EditElementHandleR elHandle) = 0;

virtual bool        _AcceptElement (ElementHandleCR elHandle, Utf8StringP cantAcceptReason, ElementAgendaP agenda) {return true;}
virtual StatusInt   _OnPreDelete (EditElementHandleR elHandle) {return ERROR;}
virtual StatusInt   _OnFenceClip (EditElementHandleR elHandle, FenceParamsP fp, FenceClipFlags flags) {return ERROR;}

public:

virtual ~IDeleteManipulator () {}

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__
public:

/*---------------------------------------------------------------------------------**//**
* Called to allow manipulator to reject this element for a given delete operation.
* @param[in] elHandle           The element manipulator is for.
* @param[in] cantAcceptReason   Reason why element can't be manipulated (NULL when not called from auto-locate).
* @param[in] agenda             ElementAgenda being processed, may be NULL.
* @return true if this element wants to be part of transform agenda.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT bool AcceptElement (ElementHandleCR elHandle, Utf8StringP cantAcceptReason, ElementAgendaP agenda);

/*---------------------------------------------------------------------------------**//**
* Called to allow manipulator participate in pre-delete logic.
* @param[in,out] elHandle       The element manipulator is for.
* @return SUCCESS If manipulator completely handled this event and default action is not to be taken.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT StatusInt OnPreDelete (EditElementHandleR elHandle);

/*---------------------------------------------------------------------------------**//**
* Called to allow manipulator to participate before OnDeleted is called and delete has happened.
* @param[in,out] elHandle       The element manipulator is for.
* @return ERROR If manipulator completely handled...including delete/adding/rewrite of elements.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT StatusInt OnDelete (EditElementHandleR elHandle);

/*---------------------------------------------------------------------------------**//**
* Called to allow manipulator to override OnFenceClip on handler.
* @param[in,out] elHandle       The element manipulator is for.
* @param[in] fp                 The fence parameters to apply
* @param[in] flags              Current fence options
* @return SUCCESS If manipulator completely handled this event and default action is not to be taken.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT StatusInt OnFenceClip (EditElementHandleR elHandle, FenceParamsP fp, FenceClipFlags flags);

}; // IDeleteManipulator

typedef RefCountedPtr<IDeleteManipulator> IDeleteManipulatorPtr;

/*================================================================================**//**
* Extension to provide delete manipulator for an element handler.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct IDeleteManipulatorExtension : DgnDomain::Handler::Extension
{
protected:
    virtual IDeleteManipulatorPtr _GetIDeleteManipulator (ElementHandleCR elHandle, AgendaOperation opType, AgendaModify modify, DisplayPathCP path) = 0;

public:
    HANDLER_EXTENSION_DECLARE_MEMBERS(IDeleteManipulatorExtension, DGNPLATFORM_EXPORT)
    DGNPLATFORM_EXPORT IDeleteManipulatorPtr GetIDeleteManipulator (ElementHandleCR elHandle, AgendaOperation opType, AgendaModify modify, DisplayPathCP path);

}; // IDeleteManipulatorExtension

/*=================================================================================**//**
* Interface adopted by an element handler class to participate with property-change tools.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct     IPropertyManipulator : public IElementState
{
private:
    DECLARE_KEY_METHOD

//__PUBLISH_SECTION_END__
protected:

virtual StatusInt   _OnPropertyChange (EditElementHandleR elHandle, PropertyContextR context) = 0;

virtual bool        _AcceptElement (ElementHandleCR elHandle, Utf8StringP cantAcceptReason, ElementAgendaP agenda) {return true;}
virtual bool        _OnModifyChanged (ElementHandleCR elHandle, ElementAgendaP agenda, AgendaOperation opType, AgendaModify modify) {return true;}
#ifdef DGN_IMPORTER_REORG_WIP
virtual StatusInt   _OnPrePropertyChange (EditElementHandleR elHandle, CopyContextP ccP) {return ERROR;}
#endif
virtual StatusInt   _OnFenceClip (EditElementHandleR elHandle, FenceParamsP fp, FenceClipFlags flags) {return ERROR;}

public:

virtual ~IPropertyManipulator () {}

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__
public:

/*---------------------------------------------------------------------------------**//**
* Called to allow manipulator to reject this element for a given property-change operation.
* @param[in] elHandle           The element manipulator is for.
* @param[in] cantAcceptReason   Reason why element can't be modified (NULL when not called from auto-locate).
* @param[in] agenda             ElementAgenda being processed, may be NULL.
* @return true if this element wants to be part of property-change agenda.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT bool AcceptElement (ElementHandleCR elHandle, Utf8StringP cantAcceptReason, ElementAgendaP agenda);

/*---------------------------------------------------------------------------------**//**
* Called to allow manipulator to reject this element for a given property-change operation.
* @param[in] elHandle           The element manipulator is for.
* @param[in] agenda             ElementAgenda being processed, may be NULL.
* @param[in] opType             The type operation to be applied.
* @param[in] modify             The modification type for the operation.
* @return false if manipulator doesn't support new modify operation.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT bool OnModifyChanged (ElementHandleCR elHandle, ElementAgendaP agenda, AgendaOperation opType, AgendaModify modify);

/*---------------------------------------------------------------------------------**//**
* Called to allow manipulator to override OnFenceClip on handler.
* @param[in,out] elHandle       The element manipulator is for.
* @param[in] fp                 The fence parameters to apply
* @param[in] flags              Current fence options
* @return SUCCESS If manipulator completely handled this event and default action is not to be taken.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT StatusInt OnFenceClip (EditElementHandleR elHandle, FenceParamsP fp, FenceClipFlags flags);

#ifdef DGN_IMPORTER_REORG_WIP
/*---------------------------------------------------------------------------------**//**
* Called to allow manipulator participate in pre-change logic.
* @param[in,out] elHandle       The element manipulator is for.
* @param[in] ccP                Current copy context (NULL if not copying).
* @return SUCCESS If manipulator completely handled this event and default action is not to be taken.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT StatusInt OnPrePropertyChange (EditElementHandleR elHandle, CopyContextP ccP);
#endif

/*---------------------------------------------------------------------------------**//**
* Called to allow manipulator to override ApplyProperty on handler.
* @param[in,out] elHandle       The element manipulator is for.
* @param[in] context            context for changes to apply.
* @return ERROR If manipulator completely handled...including adding/rewrite of element to file.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT StatusInt OnPropertyChange (EditElementHandleR elHandle, PropertyContextR context);

}; // IPropertyManipulator

typedef RefCountedPtr<IPropertyManipulator> IPropertyManipulatorPtr;

/*================================================================================**//**
* Extension to provide change attribute tools for an element handler.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct IPropertyManipulatorExtension : DgnDomain::Handler::Extension
{
protected:
    virtual IPropertyManipulatorPtr _GetIPropertyManipulator (ElementHandleCR elHandle, AgendaOperation opType, AgendaModify modify, DisplayPathCP path) = 0;

public:
    HANDLER_EXTENSION_DECLARE_MEMBERS(IPropertyManipulatorExtension, DGNPLATFORM_EXPORT)
    DGNPLATFORM_EXPORT IPropertyManipulatorPtr GetIPropertyManipulator (ElementHandleCR elHandle, AgendaOperation opType, AgendaModify modify, DisplayPathCP path);

}; // IPropertyManipulatorExtension

/*=================================================================================**//**
* Interface adopted by an element handler class to participate with insert/delete vertex tool.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct     IVertexManipulator : public IElementState
{
private:
    DECLARE_KEY_METHOD

//__PUBLISH_SECTION_END__
protected:

virtual StatusInt   _OnModifyElement (EditElementHandleR eeh, DgnButtonEventCP ev, DPoint3dCR locatePoint, ViewContextP context) = 0;

virtual bool        _AcceptElement (ElementHandleCR eh, Utf8StringP cantAcceptReason) {return true;}

public:

virtual ~IVertexManipulator () {}

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__
public:

/*---------------------------------------------------------------------------------**//**
* Called to allow manipulator to reject this element for insert/delete vertex operation.
* @param[in] eh           The element manipulator is for.
* @param[in] cantAcceptReason   Reason why element can't be manipulated (NULL when not called from auto-locate).
* @return true if this element wants to be part of transform agenda.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT bool AcceptElement (ElementHandleCR eh, Utf8StringP cantAcceptReason);

/*---------------------------------------------------------------------------------**//**
* Called to allow manipulator to override insert/delete vertex operation on handler.
* @param[in,out] eeh       The element manipulator is for.
* @param[in] ev                 Current cursor location (new vertex location for insert vertex, NULL for delete vertex).
* @param[in] locatePoint        location on element to insert/delete vertex.
* @param[in] context            NULL for accept...otherwise it's dynamics.
* @return ERROR If manipulator completely handled...including adding/rewrite of element to file.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT StatusInt OnModifyElement (EditElementHandleR eeh, DgnButtonEventCP ev, DPoint3dCR locatePoint, ViewContextP context);

}; // IVertexManipulator

typedef RefCountedPtr<IVertexManipulator> IVertexManipulatorPtr;

/*================================================================================**//**
*  Extension to provide insert/delete vertex manipulator for an element handler.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct IVertexManipulatorExtension : DgnDomain::Handler::Extension
{
protected:
    virtual IVertexManipulatorPtr _GetIVertexManipulator (ElementHandleCR eh, bool insert, HitPathCR path) = 0;

public:
    HANDLER_EXTENSION_DECLARE_MEMBERS(IVertexManipulatorExtension, DGNPLATFORM_EXPORT)
    DGNPLATFORM_EXPORT IVertexManipulatorPtr GetIVertexManipulator (ElementHandleCR eh, bool insert, HitPathCR path);
}; // IVertexManipulatorExtension

/*=================================================================================**//**
* Interface adopted by an element handler class to popup mini toolbar when user hover cursor on element.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct     IPopupDialogManipulator : public IElementState
{
//__PUBLISH_SECTION_END__
protected:

virtual bool        _DoPopup (ElementHandleCR el) = 0;
virtual void        _ShowPopupDialog (ElementHandleCR el, DgnViewportP viewport, Point2dCR point) = 0;

public:

virtual ~IPopupDialogManipulator () {}

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__
public:

/*---------------------------------------------------------------------------------**//**
* Called to allow manipulator to popup a mini toolbar when user hover cursor on element.
* @param[in] elHandle           The element manipulator is for.
* @param[in] viewport           DgnViewport to display mini toolbar in.
* @param[in] point              Location of cursor in view coordinates.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT void ShowPopupDialog (ElementHandleCR elHandle, DgnViewportP viewport, Point2dCR point);

/*---------------------------------------------------------------------------------**//**
* Called to allow manipulator to popup a mini toolbar.
* @param[in] elHandle           The element manipulator is for.
* @return true if this element wants to popup a mini toolbar.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT bool DoPopup (ElementHandleCR elHandle);

}; // IPopupDialogManipulator

typedef RefCountedPtr<IPopupDialogManipulator> IPopupDialogManipulatorPtr;

/*================================================================================**//**
* Extension to provide popup dialog manipulator for an element handler.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct IPopupDialogManipulatorExtension : DgnDomain::Handler::Extension
{
protected:
    virtual IPopupDialogManipulatorPtr _GetIPopupDialogManipulator (ElementHandleCR elHandle, AgendaOperation opType, AgendaModify modify, DisplayPathCP path) = 0;
public:
    HANDLER_EXTENSION_DECLARE_MEMBERS(IPopupDialogManipulatorExtension, DGNPLATFORM_EXPORT)
    DGNPLATFORM_EXPORT IPopupDialogManipulatorPtr GetIPopupDialogManipulator (ElementHandleCR elHandle, AgendaOperation opType, AgendaModify modify, DisplayPathCP path);

}; // IPopupDialogManipulatorExtension

//! @endGroup

END_BENTLEY_DGNPLATFORM_NAMESPACE

/** @endcond */
