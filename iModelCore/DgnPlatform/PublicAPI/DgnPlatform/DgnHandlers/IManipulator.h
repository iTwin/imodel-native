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

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

/*=================================================================================**//**
* @addtogroup Manipulators
* A manipulator maintains a set of controls used to modify an element. The manipulator
* is a new object that gets created by the handler for a supplied element. The
* IEditManipulator controls are driven by view based point to point changes.
* @beginGroup
* @bsiclass
+===============+===============+===============+===============+===============+======*/
/*=================================================================================**//**
* Interface adopted by an element handler class to present manipulator controls.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct     IEditManipulator : RefCountedBase
{
public:

//! Called to have manipulator populate it's internal set of controls.
//! @return true if controls were created.
virtual bool _DoCreateControls () = 0;

//! Called to have manipulator cleanup it's internal control data so that
//! _DoCreateControls might be called again. For example after a successful drag,
//! _DoCleanupControls will be called followed by _DoCreateControls in order to
//! have the controls reflect the changes to the element.
virtual void _DoCleanupControls () = 0;

//! Flash control(s) at the button location. A flashed control is considered to be selected.
//! Called on motion, controls should not remain selected when the button is no longer over them.
//! @return true if a control is flashed.
virtual bool _DoUpdateFlashedControls (DgnButtonEventCR ev) = 0;

//! Select control(s) that the button location is over. Controls should remain selected
//! until specifically de-selected.
//! @return true if any controls remain selected.
virtual bool _DoUpdateSelectedControls (DgnButtonEventCR ev, SelectionMode mode) = 0;

//! Select control(s) that satisfy the given fence criteria. Controls should remain selected
//! until specifically de-selected.
//! @return true if any controls remain selected.
virtual bool _DoUpdateSelectedControls (FenceParamsR fp, SelectionMode mode) = 0;

//! Select control(s) that make the most sense for the supplied hit.
//! This method supports modification tools that need to select both the
//! element and controls from a single click and immediately start modification.
//! The manipulator might not ever be asked to draw it's controls.
//! @note A manipulator should always try to select at least one control.
//! @return true if any control is selected.
virtual bool _DoUpdateSelectedControls (HitDetailCR path) = 0;

//! Check whether manipulator currently has any controls selected.
//! @return true if manipulator has controls selected or flashed.
virtual bool _HasSelectedControls () = 0;

//! Called to display the manipulator's controls in the supplied viewport.
//! This is commonly done by drawing sprites, but the manipulator can choose to draw any geometry it wants.
virtual void _OnDraw (DgnViewportR vp) = 0;

//! When multiple manipulators are active on a selection set, only the one whose control is actually
//! selected to start the modify operation is given responsibility for setting up the modification.
//! If called, the manipulator is responsible for setting the anchor point that will be used by all 
//! active manipulators to the center of it's selected control. This manipulator can also enable AccuSnap/AccuDraw.
//! @param[in,out] ev Current button event, point needs to be set to center of selected control.
//! @note If you merely want a control to handle clicks, implement _OnClick instead.
//! @return false to reject starting modify dynamics.
//! @see _OnClick
virtual bool _OnPreModify (DgnButtonEventR ev) = 0;

//! Called when the modify operation is about to begin. It is the responsibily of the
//! manipulator to keep the point in the supplied button event to use as the anchor
//! point of the modify operation.
virtual void _OnModifyStart (DgnButtonEventCR ev) = 0;

//! Called if user aborts drag operation (reset), manipulator can cleanup anything done in _OnModifyStart.
virtual void _OnModifyCancel (DgnButtonEventCR ev) = 0;

//! Called to show modify dynamics. Expected to call _DoModify with isDynamics set
//! to true, and to display the result if _DoModify returned SUCCESS.
//! @return SUCCESS if modify operation could be applied.
virtual StatusInt _OnModify (DgnButtonEventCR ev) = 0;

//! Called to accept modify operation. Expected to call _DoModify with isDynamics set
//! to false, and to update the element in the DgnDb if _DoModify returned SUCCESS.
//! @return SUCCESS if modify operation could be applied and element was updated.
virtual StatusInt _OnModifyAccept (DgnButtonEventCR ev) = 0;

//! It is expected that this is where all the work is done to update the element.
//! Using the anchor point from _OnModifyStart and the supplied button event for the cursor
//! location, update the element data.
//! @param[in] ev Current button event,
//! @param[in] isDynamics Whether manipulator is being called from _OnModify or _OnModifyAccept.
//! @return SUCCESS if modify operation could be applied.
virtual StatusInt _DoModify (DgnButtonEventCR ev, bool isDynamics) = 0;

//! Called when manipulator is displaying controls and user clicks on the element again,
//! but not on a control. Manipulator may choose to present the user with a different set of controls.
virtual void _OnNewHit (HitDetailCR path) {}

//! Called on a data button event to allow controls to act on a single click.
//! @note This method can be used to launch editors as it is called before _OnPreModify.
//! @return true if event was handled and modify dynamics should not be started.
virtual bool _OnClick (DgnButtonEventCR ev) {return false;}

//! Called on right-click while over a control, one use of this event would be to
//! present the user with a menu of editing options.
//! @return true is manipulator wants to do something and handled the right-click event.
virtual bool _OnRightClick (DgnButtonEventCR ev) {return false;}

//! Called when user double clicks on the manipulator's element. (ex. edit text)
//! @return true if manipulator handled double-click event.
virtual bool _OnDoubleClick (HitDetailCR path) {return false;}

//! Called when a drag operation starts. This method is only useful for manipulators that 
//! wish to distinguish click-move-click from down-drag-release.
//! @return true if the manipulator has handled the event.
virtual bool _OnDragStart (DgnButtonEventCR ev) {return false;}

//! Called when a drag operation ends.
virtual void _OnDragEnd (DgnButtonEventCR ev) {}

//! Called when a sub entity selection effects an element that is being manipulated.
//! Typically the manipulator should get the current selection state from the element
//! and update its display as appropriate.
//! @return true if the manipulator has handled the event.
virtual bool _OnSubSelection () {return false;}

//! Called if a modifier key goes up or down while dragging. Manipulator can use this
//! to cycle between different drag behaviors for a control, ex. move or copy.
virtual void _OnModifierKeyTransition (bool wentDown, int key) {}

//! Called when a keyboard key is pressed while an element is being manipulated.
//! This method is only called for a small set of specific keys presses (See below).
//! @param[in] wentDown true if the key was pressed, false if the key was released.
//! @param[in] key One of VK_TAB, VK_RETURN, VK_END, VK_HOME, VK_LEFT, VK_UP, VK_RIGHT, VK_DOWN.
//! @param[in] shiftIsDown The shift key was down during the transition.
//! @param[in] ctrlIsDown The control key was down during the transition.
//! @return true if the manipulator has handled the key. This will prevent further processing of the key press.
virtual bool _OnKeyTransition (bool wentDown, int key, bool shiftIsDown, bool ctrlIsDown) {return false;}

//! Return a string suitable for display in a tool tip that describes the currently selected controls.
//! @return String to describe selected controls or an empty string for no description.
virtual Utf8String _OnGetDescription () = 0;

//! @private
virtual ~IEditManipulator() {}

}; // IEditManipulator

typedef RefCountedPtr<IEditManipulator> IEditManipulatorPtr;

/*================================================================================**//**
* Extension to provide IEditManipulator for an element.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct IEditManipulatorExtension : DgnDomain::Handler::Extension
{
public:

virtual IEditManipulatorPtr _GetIEditManipulator (GeometricElementCR elm) = 0;
virtual IEditManipulatorPtr _GetIEditManipulator (HitDetailCR hit) = 0;
HANDLER_EXTENSION_DECLARE_MEMBERS(IEditManipulatorExtension, DGNPLATFORM_EXPORT)

//! @private
virtual ~IEditManipulatorExtension() {}

}; // IEditManipulatorExtension

END_BENTLEY_DGNPLATFORM_NAMESPACE

/** @endcond */

