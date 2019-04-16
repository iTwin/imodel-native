/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include "HitDetail.h"

BEGIN_BENTLEY_DGN_NAMESPACE

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
struct IEditManipulator : RefCountedBase
{
public:
    //! Control selection modes.
    enum class SelectionMode
        {
        New         = 0,
        Add         = 1,
        Subtract    = 2,
        Inverse     = 3,
        Clear       = 4,
        All         = 5,
        };

    //! Called to have manipulator populate it's internal set of controls.
    //! @return true if controls were created.
    virtual bool _DoCreateControls() = 0;

    //! Called to have manipulator cleanup it's internal control data so that
    //! _DoCreateControls might be called again. For example after a successful drag,
    //! _DoCleanupControls will be called followed by _DoCreateControls in order to
    //! have the controls reflect the changes to the element.
    virtual void _DoCleanupControls() = 0;

    //! Flash control(s) at the button location. A flashed control is considered to be selected.
    //! Called on motion, controls should not remain selected when the button is no longer over them.
    //! @return true if a control is flashed.
    virtual bool _DoUpdateFlashedControls(DgnButtonEventCR ev) = 0;

    //! Select control(s) that the button location is over. Controls should remain selected
    //! until specifically de-selected.
    //! @return true if any controls remain selected.
    virtual bool _DoUpdateSelectedControls(DgnButtonEventCR ev, SelectionMode mode) = 0;

    //! Select control(s) that satisfy the given fence criteria. Controls should remain selected
    //! until specifically de-selected.
    //! @return true if any controls remain selected.
    virtual bool _DoUpdateSelectedControls(FenceParamsR fp, SelectionMode mode) = 0;

    //! Select control(s) that make the most sense for the supplied hit.
    //! This method supports modification tools that need to select both the
    //! element and controls from a single click and immediately start modification.
    //! The manipulator might not ever be asked to draw it's controls.
    //! @note A manipulator should always try to select at least one control.
    //! @return true if any control is selected.
    virtual bool _DoUpdateSelectedControls(HitDetailCR path) = 0;

    //! Check whether manipulator currently has any controls selected.
    //! @return true if manipulator has controls selected or flashed.
    virtual bool _HasSelectedControls() = 0;

    //! Return whether manipulator controls or dynamics should be shown in the supplied view.
    virtual bool _IsDisplayedInView(DgnViewportR vp) = 0;

    //! Called to display the manipulator's controls in the supplied viewport.
    //! This is commonly done by drawing sprites, but the manipulator can choose to draw any geometry it wants.
    virtual void _OnDraw(DecorateContextR ) = 0;

    //! When multiple manipulators are active on a selection set, only the one whose control is actually
    //! selected to start the modify operation is given responsibility for setting up the modification.
    //! If called, the manipulator is responsible for setting the anchor point that will be used by all
    //! active manipulators to the center of it's selected control. This manipulator can also enable AccuSnap/AccuDraw.
    //! @param[in,out] ev Current button event, point needs to be set to center of selected control.
    //! @note If you merely want a control to handle clicks, implement _OnClick instead.
    //! @return false to reject starting modify dynamics.
    //! @see _OnClick
    virtual bool _OnPreModify(DgnButtonEventR ev) = 0;

    //! Called when the modify operation is about to begin. It is the responsibily of the
    //! manipulator to keep the point in the supplied button event to use as the anchor
    //! point of the modify operation.
    virtual void _OnModifyStart(DgnButtonEventCR ev) = 0;

    //! Called if user aborts drag operation (reset), manipulator can cleanup anything done in _OnModifyStart.
    virtual void _OnModifyCancel(DgnButtonEventCR ev) = 0;

    //! Called to show modify dynamics. Expected to call _DoModify with isDynamics set
    //! to true, and to display the result if _DoModify returned SUCCESS.
    //! @return SUCCESS if modify operation could be applied.
    virtual StatusInt _OnModify(DgnButtonEventCR ev, DynamicsContextR context) = 0;

    //! Called to accept modify operation. Expected to call _DoModify with isDynamics set
    //! to false, and to update the element in the DgnDb if _DoModify returned SUCCESS.
    //! @return SUCCESS if modify operation could be applied and element was updated.
    virtual StatusInt _OnModifyAccept(DgnButtonEventCR ev) = 0;

    //! It is expected that this is where all the work is done to update the element.
    //! Using the anchor point from _OnModifyStart and the supplied button event for the cursor
    //! location, update the element data.
    //! @param[in] ev Current button event,
    //! @param[in] isDynamics Whether manipulator is being called from _OnModify or _OnModifyAccept.
    //! @return SUCCESS if modify operation could be applied.
    virtual StatusInt _DoModify(DgnButtonEventCR ev, bool isDynamics) = 0;

    //! Called when manipulator is displaying controls and user clicks on the same or different geometry
    //! instead of on a control. Manipulator may choose to present the user with a different set of controls or cleanup.
    //! Transient manipulators need to verify that the new hit is compatible as caller has no way of checking.
    //! @return true to clear manipulator, false if current manipulator is still ok.
    virtual bool _OnNewHit(HitDetailCR hit) {return !hit.GetElementId().IsValid();}

    //! Called on a data button event to allow controls to act on a single click.
    //! @note This method can be used to launch editors as it is called before _OnPreModify.
    //! @return true if event was handled and modify dynamics should not be started.
    virtual bool _OnClick(DgnButtonEventCR ev) {return false;}

    //! Called on right-click when not manipulating a control. One use of this event would be to
    //! present the user with a menu of editing options.
    //! @return true if manipulator wants to do something and handled the right-click event.
    //! @note It is up to the manipulator to check whether a control is selected or under the cursor by
    //!       calling HaveSelectedControls. A manipulator *could* also check if the current auto-locate HitDetail
    //!       is for the geometry that the manipulator was created for when not over a control.
    virtual bool _OnRightClick(DgnButtonEventCR ev) {return false;}

    //! Called when user double clicks on the manipulator's element. (ex. edit text)
    //! @return true if manipulator handled double-click event.
    virtual bool _OnDoubleClick(HitDetailCR path) {return false;}

    //! Called when a drag operation starts. This method is only useful for manipulators that
    //! wish to distinguish click-move-click from down-drag-release.
    //! @return true if the manipulator has handled the event.
    virtual bool _OnDragStart(DgnButtonEventCR ev) {return false;}

    //! Called when a drag operation ends.
    virtual void _OnDragEnd(DgnButtonEventCR ev) {}

    //! Called when a sub entity selection effects an element that is being manipulated.
    //! Typically the manipulator should get the current selection state from the element
    //! and update its display as appropriate.
    //! @return true if the manipulator has handled the event.
    virtual bool _OnSubSelection() {return false;}

    //! Called if a modifier key goes up or down while dragging. Manipulator can use this
    //! to cycle between different drag behaviors for a control, ex. move or copy.
    virtual void _OnModifierKeyTransition(bool wentDown, int key) {}

    //! Called when a keyboard key is pressed while an element is being manipulated.
    //! This method is only called for a small set of specific keys presses (See below).
    //! @param[in] wentDown true if the key was pressed, false if the key was released.
    //! @param[in] key One of VK_TAB, VK_RETURN, VK_END, VK_HOME, VK_LEFT, VK_UP, VK_RIGHT, VK_DOWN.
    //! @param[in] shiftIsDown The shift key was down during the transition.
    //! @param[in] ctrlIsDown The control key was down during the transition.
    //! @return true if the manipulator has handled the key. This will prevent further processing of the key press.
    virtual bool _OnKeyTransition(bool wentDown, int key, bool shiftIsDown, bool ctrlIsDown) {return false;}

    //! Return a string suitable for display in a tool tip that describes the currently selected controls.
    //! @return String to describe selected controls or an empty string for no description.
    virtual Utf8String _OnGetDescription() = 0;

    //! @private
    virtual ~IEditManipulator() {}
}; // IEditManipulator

DEFINE_REF_COUNTED_PTR(IEditManipulator)

/*================================================================================**//**
* Extension to provide IEditManipulator for an element.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct IEditManipulatorExtension : DgnDomain::Handler::Extension
{
public:

enum class ControlType
    {
    Placement   = 0, //!< Request for placement controls (if none, return nullptr for IEditManipulatorPtr and appropriate DefaultActions)
    Geometry    = 1, //!< Request for geometry/vertex controls (if none, return nullptr for IEditManipulatorPtr and appropriate DefaultActions)
    };

enum class DefaultActions : uint32_t
    {
    None        = 0,          //!< Default controls disabled
    TranslateXY = (1 << 0),   //!< Default controls may apply x/y translation
    TranslateZ  = (1 << 1),   //!< Default controls may apply z translation
    RotateXY    = (1 << 2),   //!< Default controls may apply rotation about x/y axes
    RotateZ     = (1 << 3),   //!< Default controls may apply rotation about z axis
    Scale       = (1 << 4),   //!< Default controls may apply scale
    Copy        = (1 << 5),   //!< Default controls may create a copy
    Geometry    = (1 << 6),   //!< Default controls may modify GeometryPrimitive(s)
    All         = 0xffffffff, //!< Default controls unrestricted
    Placement   = (TranslateXY | TranslateZ | RotateXY | RotateZ), //!< Default controls may freely change origin and angle(s) of element's Placement2d/Placement3d
    Placement2d = (TranslateXY | RotateZ), //!< Default controls may only translate in x/y and rotate about z (for treating 3d elements as 2d)
    };

virtual DefaultActions _GetAllowedDefaultActions() {return DefaultActions::Placement;} //!< Allowed modifications the default manipulators may apply

virtual IEditManipulatorPtr _GetIEditManipulator(GeometrySourceCR source) = 0; //!< Return IEditManipulatorPtr preferably for ControlType::Geometry (if any). Called directly on double-click event.
virtual IEditManipulatorPtr _GetIEditManipulator(HitDetailCR hit) = 0; //!< Return IEditManipulatorPtr preferably for ControlType::Geometry (if any). Called directly on double-click event.

virtual IEditManipulatorPtr _GetIEditManipulator(GeometrySourceCR source, ControlType type) {return ControlType::Geometry == type ? _GetIEditManipulator(source) : nullptr;}
virtual IEditManipulatorPtr _GetIEditManipulator(HitDetailCR hit, ControlType type) {return ControlType::Geometry == type ? _GetIEditManipulator(hit) : nullptr;}

HANDLER_EXTENSION_DECLARE_MEMBERS(IEditManipulatorExtension, DGNPLATFORM_EXPORT)

//! @private
virtual ~IEditManipulatorExtension() {}

}; // IEditManipulatorExtension

//=======================================================================================
// Controls tool behavior on grouped elements (either with a group or parent-child relationships)
// Avoid individual modifications and selection by default
// Tools won't modify/select individually if the extension is not present
// @bsistruct                                                   Diego.Pinate    05/18
//=======================================================================================
struct GroupEditExtension : DgnDomain::Handler::Extension
{
public:
virtual bool    _CanModifyChildrenIndividually() { return false; }
virtual bool    _CanSelectChildrenIndividually() { return false; }

HANDLER_EXTENSION_DECLARE_MEMBERS(GroupEditExtension, DGNPLATFORM_EXPORT)

//! @private
virtual ~GroupEditExtension() {}
}; // GroupEditExtension

END_BENTLEY_DGN_NAMESPACE

