/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/IAuxCoordSys.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
/** @cond BENTLEY_SDK_Internal */

#include "../DgnPlatform.h"
#include "ValueFormat.h"

//__PUBLISH_SECTION_END__

#include "DgnCoreEvent.h"

//__PUBLISH_SECTION_START__

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//=======================================================================================
//! @addtogroup AuxiliaryCoordinateSystems
//! The Auxiliary Coordinate System interface allows programs to manipulate named coordinate systems stored in dgn files.
//! @beginGroup
// @bsiclass 
enum class ACSType
    {
    None               = 0,
    Rectangular        = 1,
    Cylindrical        = 2,
    Spherical          = 3,
    Extended           = 4,
    };

enum class ACSSaveOptions
    {
    OverwriteByElemId  = 0,        // overwrite only, preserving element ID.
    OverwriteByName    = 1,        // overwrite only, preserving name.
    AllowNew           = 2,        // allow to be saved as new if name doesn't match.
    };

enum class ACSEventType
    {
    None              = 0,
    ParameterChanged  = (1 << 0), // An ACS nongeometry parameter changed (e.g. description).
    GeometryChanged   = (1 << 1), // An ACS geometry parameter changed (e.g. origin).
    ChangeWritten     = (1 << 2), // An ACS was written to the file.
    NewACS            = (1 << 3), // The ACS written was new.
    Delete            = (1 << 4), // the ACS was deleted from the file.
    };

ENUM_IS_FLAGS (ACSEventType)


enum class ACSDisplayOptions
    {
    None            = 0,        // used for testing individual bits.
    Inactive        = 0,
    Active          = (1 << 0),
    Hilite          = (1 << 2),
    Deemphasized    = (1 << 1),
    CheckVisible    = (1 << 3),
    };

ENUM_IS_FLAGS (ACSDisplayOptions)

enum class ACSFlags
    {
    None               = 0,      // Used for testing individual bits.
    Default            = 0,
    ViewIndependent    = (1<<0), // Whether ACS always orients itself to the current view rotations or is a fixed rotation...
    };

ENUM_IS_FLAGS (ACSFlags)


typedef RefCountedPtr<IAuxCoordSys> IAuxCoordSysPtr;

//! \ingroup auxCoords
//! Callback object for IACSManager::Traverse
struct     IACSTraversalHandler
{
virtual ~IACSTraversalHandler() {}

//! Called to initialize the traversal logic.
virtual UInt32 _GetACSTraversalOptions () = 0;

//! This method is called for each ACS encountered by the traversal.
//! @param[in]  acs         current object in the traversal.
//! @return  true to halt the traversal, false to continue.
//! @remarks  You can retain a reference to the IAuxCoordSys object, but you must remember
//!           to call AddRef() and Release() at the appropriate time.  A simple way to do
//!           this is to assign the address of the object to an IAuxCoordSysPtr.
virtual bool _HandleACSTraversal (IAuxCoordSysR acs) = 0;
};

//__PUBLISH_SECTION_END__

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct     IAuxCoordSystemExtender
{
/*---------------------------------------------------------------------------------**//**
* Return extended Coordinate Systems as long as StatusInt is nonzero.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool _TraverseExtendedACS (IACSTraversalHandler& traverser, DgnModelP modelRef) = 0;
virtual UInt32 _GetExtenderId () const = 0;
virtual IAuxCoordSysP _Deserialize (void *persistentData, UInt32 dataSize, DgnModelP modelRef) = 0;
}; 

//__PUBLISH_SECTION_START__
/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct     IACSEvents
{
virtual void _OnACSEvent (IAuxCoordSysP acs, ACSEventType eventType, DgnModelP modelRef) = 0;

}; // IACSEvents

//=======================================================================================
//!
//! Manager class to provide access to auxiliary coordinate systems.
//! Auxiliary coordinate systems (ACS) are entities that can be created, edited, deleted, and
//! activated under user control.
//!
//=======================================================================================
struct  IACSManager : DgnHost::IHostObject
{
//__PUBLISH_SECTION_END__
private:

bool                                        m_inhibitCurrentACSDisplay;
EventHandlerList<IAuxCoordSystemExtender>*  m_extenders;
EventHandlerList<IACSEvents>*               m_listeners;

public:
    DEFINE_BENTLEY_NEW_DELETE_OPERATORS

    IACSManager                             ();
    void SetInhibitCurrentACSDisplay        (bool inhibit) {m_inhibitCurrentACSDisplay = inhibit;}
    bool GetInhibitCurrentACSDisplay        () {return m_inhibitCurrentACSDisplay;}
    virtual void _OnHostTermination (bool isProcessShutdown) override {delete this;}

DGNPLATFORM_EXPORT void AddExtender (IAuxCoordSystemExtender* extender);
DGNPLATFORM_EXPORT void RemoveExtender (IAuxCoordSystemExtender* extender);
DGNPLATFORM_EXPORT IAuxCoordSystemExtender* FindExtender (UInt32 extenderID);

DGNPLATFORM_EXPORT void SaveSettings (PhysicalViewControllerCP viewController, ElementHandleCR eh);
DGNPLATFORM_EXPORT void ReadSettings (PhysicalViewControllerP viewController, ElementRefP elementRef, DgnModelP modelRef);
DGNPLATFORM_EXPORT IAuxCoordSysPtr ExtractACS (ElementRefP elementRef, DgnModelP modelRef);

DGNPLATFORM_EXPORT IAuxCoordSysPtr CreateACS (ACSType type, DPoint3dCR origin, RotMatrixCR rot, double scale, WCharCP name, WCharCP descr);
DGNPLATFORM_EXPORT IAuxCoordSysPtr CreateACSFromElement (ElementHandleCR eh);
DGNPLATFORM_EXPORT StatusInt CreateElementFromACS (EditElementHandleR eeh, IAuxCoordSysP acs, DgnModelP modelRef);
DGNPLATFORM_EXPORT StatusInt ElementFindByName (EditElementHandleR eeh, WCharCP inName, DgnModelP modelRef);
DGNPLATFORM_EXPORT static bool ElementIsACS (ElementHandleCR eh);

DGNPLATFORM_EXPORT void SendEvent (IAuxCoordSysP acs, ACSEventType eventType, DgnModelP modelRef);
DGNPLATFORM_EXPORT void DisplayCurrent (ViewportP viewport, bool isCursorView);
DGNPLATFORM_EXPORT void DisplayTransients (ViewContextR context, bool isPreUpdate);

DGNVIEW_EXPORT bool GetStandardRotation (RotMatrixR rMatrix, StandardView nStandard, ViewportP viewport, bool useACS, DgnCoordSystem coordSys);
DGNVIEW_EXPORT bool GetCurrentOrientation (RotMatrixR rMatrix, ViewportP viewport, bool checkAccuDraw, bool checkACS, DgnCoordSystem coordSys);

DGNVIEW_EXPORT static bool IsPointAdjustmentRequired (ViewportR viewport);
DGNVIEW_EXPORT static bool IsSnapAdjustmentRequired (ViewportR viewport);
DGNVIEW_EXPORT static bool IsContextRotationRequired (ViewportR viewport);

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__
public:

//! Add a listener for acs events.
DGNPLATFORM_EXPORT void AddListener (IACSEvents* acsListener);

//! Drop a listener for acs events.
DGNPLATFORM_EXPORT void DropListener (IACSEvents* acsListener);

//! Create ACS "smart pointer".
//! @return a "smart pointer" to an object that supports the IAuxCoordSysP interface.
DGNPLATFORM_EXPORT IAuxCoordSysPtr CreateACS ();

//! Gets the ACS object for the active ACS.
//! @param[in]      vp          View to apply acs to.
//! @return ACS if one is active, NULL otherwise.
//! @remarks  This is not a copy and should not be freed; Use Clone uses you want
//!           changes to directly affect the ACS attached to this view.
DGNPLATFORM_EXPORT IAuxCoordSysP GetActive (ViewportR vp);

//! Sets the active ACS properties.
//! @param[in]      auxCoordSys ACS to activate.
//! @param[in]      vp          View to apply acs to.
//! @return status
DGNPLATFORM_EXPORT StatusInt SetActive (IAuxCoordSysP auxCoordSys, ViewportR vp);

//! Gets an ACS object representing  a named ACS from a model.
//! @param[in]      name        The name of the ACS to retrieve
//! @param[in]      modelRef    The preferred model in which to search
//! @param[in]      options     Options to control searching
//! @return ACS if found, NULL otherwise.
DGNPLATFORM_EXPORT IAuxCoordSysPtr GetByName (WCharCP name, DgnModelP modelRef, UInt32 options);

//! Call a handler for each persistent named ACS in a model.
//! @param[in]      handler     Object to handle the callback for each ACS.
//! @param[in]      modelRef    The preferred model in which to traverse.
//! @return   true if handler stopped traversal early by returning true, false if traversed all.
DGNPLATFORM_EXPORT bool Traverse (IACSTraversalHandler& handler, DgnModelP modelRef);

//! Save and ACS persistently in a model.
//! @param[in]      auxCoordSys ACS to save.
//! @param[in]      modelRef    Model to contain the ACS.
//! @param[in]      saveOption  The options for saving.
//! @param[in]      eventType   The ACS event type to send.
//! @return status
DGNPLATFORM_EXPORT StatusInt Save(IAuxCoordSysP auxCoordSys, DgnModelP modelRef, ACSSaveOptions saveOption, ACSEventType eventType);

//! Deletes a persistent ACS from a model.
//! @param[in]      name        Name of the ACS to delete.
//! @param[in]      modelRef    Model that contains the ACS to delete.
//! @return status
DGNPLATFORM_EXPORT StatusInt Delete (WCharCP name, DgnModelP modelRef);

DGNPLATFORM_EXPORT static IACSManagerR GetManager ();

}; // IACSManager

//__PUBLISH_SECTION_END__

//__PUBLISH_SECTION_START__

//=======================================================================================
//! An IAuxCoordSys is an object that holds the data which
//! describes an auxiliary coordinate system
//=======================================================================================
struct          IAuxCoordSys : public RefCountedBase
{
//__PUBLISH_SECTION_END__
protected:

virtual IAuxCoordSysPtr             _Clone                      () const = 0;
virtual bool                        _Equals                     (IAuxCoordSysCP other) const = 0;
virtual WString                     _GetName                    () const = 0;
virtual WString                     _GetDescription             () const = 0;
virtual ACSType                     _GetType                    () const = 0;
virtual WString                     _GetTypeName                () const = 0;
virtual double                      _GetScale                   () const = 0;
virtual DPoint3dR                   _GetOrigin                  (DPoint3dR pOrigin) const = 0;
virtual RotMatrixR                  _GetRotation                (RotMatrixR pRot) const = 0;
virtual RotMatrixR                  _GetRotation                (RotMatrixR pRot, DPoint3dR pPosition) const = 0;
virtual ElementId                   _GetElementId               () const = 0;
virtual bool                        _GetIsReadOnly              () const = 0;
virtual ACSFlags                    _GetFlags                   () const = 0;
virtual StatusInt                   _SetName                    (WCharCP name) = 0;
virtual StatusInt                   _SetDescription             (WCharCP descr) = 0;
virtual StatusInt                   _SetType                    (ACSType type) = 0;
virtual StatusInt                   _SetScale                   (double scale) = 0;
virtual StatusInt                   _SetOrigin                  (DPoint3dCR pOrigin) = 0;
virtual StatusInt                   _SetRotation                (RotMatrixCR pRot) = 0;
virtual StatusInt                   _PointFromString            (DPoint3dR outPoint, WStringR errorMsg, WCharCP inString, bool relative, DPoint3dCP lastPoint, DgnModelR modelRef) = 0;
virtual StatusInt                   _StringFromPoint            (WStringR outString, WStringR errorMsg, DPoint3dCR inPoint, bool delta, DPoint3dCP deltaOrigin, DgnModelR modelRef, DistanceFormatterR distanceFormatter, DirectionFormatterR directionFormatter) = 0;
virtual StatusInt                   _SetFlags                   (ACSFlags flags) = 0;
virtual UInt32                      _GetExtenderId              () const = 0;
virtual UInt32                      _GetSerializedSize          () const = 0;
virtual StatusInt                   _Serialize                  (void *data, UInt32 maxSize) const = 0;
virtual StatusInt                   _CompleteSetupFromViewController(PhysicalViewControllerCP info) = 0;
virtual void                        _DrawGrid                   (ViewportP vp) const = 0;
virtual void                        _PointToGrid                (ViewportP vp, DPoint3dR point) const = 0;

virtual StatusInt _GetStandardGridParams (Point2dR gridReps, Point2dR gridOffset, double& uorPerGrid, double& gridRatio, UInt32& gridPerRef) const {return ERROR;}
virtual StatusInt _SetStandardGridParams (Point2dCR gridReps, Point2dCR gridOffset, double uorPerGrid, double gridRatio, UInt32 gridPerRef) {return ERROR;}

// these methods are called only internally, so they don't have corresponding nonvirtual public wrappers.
DGNPLATFORM_EXPORT virtual bool _IsOriginInView (DPoint3dR drawOrigin, ViewportP vp, bool adjustOrigin) const;
DGNPLATFORM_EXPORT virtual void _DrawZAxis (ViewportP vp, ICachedDrawP cached, TransformP transformP, ACSDisplayOptions options) const;
DGNPLATFORM_EXPORT virtual UInt32 _GetColor (ViewportP vp, UInt32 menuColor, UInt32 transparency, ACSDisplayOptions options) const;
DGNPLATFORM_EXPORT virtual void _DrawAxisText (ViewportP vp, ICachedDrawP cached, WCharCP label, bool isAxisLabel, double userOrgX, double userOrgY, double scale, double angle, ACSDisplayOptions options) const;
DGNPLATFORM_EXPORT virtual void _DrawAxisArrow (ViewportP vp, ICachedDrawP cached, TransformP transformP, UInt32 menuColor, WCharCP label, bool swapAxis, ACSDisplayOptions options, ACSFlags flags) const;
DGNPLATFORM_EXPORT virtual void _DisplayInView (ViewportP vp, ACSDisplayOptions options, bool drawName) const;
DGNPLATFORM_EXPORT virtual QvElem* _CreateQvElems (ViewportP vp, DPoint3dCP drawOrigin, double acsSizePixels, ACSDisplayOptions options, bool drawName) const;
DGNPLATFORM_EXPORT virtual UInt32 _GetTransparency (bool isFill, ACSDisplayOptions options) const;
DGNPLATFORM_EXPORT virtual WCharCP _GetAxisLabel (UInt32 axis, WCharP axisLabel, UInt32 length) const;

public:

// Only for ACS's of type ACS_TYPE_GeoCoordinate is the rotation matrix position dependent, don't publish this yet.
DGNPLATFORM_EXPORT RotMatrixR           GetRotation                 (RotMatrixR pRot, DPoint3dR pPosition) const;

// Standard grid settings don't apply to type ACS_TYPE_GeoCoordinate...
double GetGridScaleFactor (ViewportR vp) const;
StatusInt GetGridSpacing (DPoint2dR spacing, Point2dR gridReps, Point2dR gridOffset, ViewportR vp) const;

DGNPLATFORM_EXPORT StatusInt GetStandardGridParams (Point2dR gridReps, Point2dR gridOffset, double& uorPerGrid, double& gridRatio, UInt32& gridPerRef) const;
DGNPLATFORM_EXPORT StatusInt SetStandardGridParams (Point2dCR gridReps, Point2dCR gridOffset, double uorPerGrid, double gridRatio, UInt32 gridPerRef);

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__
public:

//! Return a copy of the ACS object.
DGNPLATFORM_EXPORT IAuxCoordSysPtr Clone () const;

//! Returns true if the is the same ACS object.
DGNPLATFORM_EXPORT bool Equals (IAuxCoordSysCP other) const;

//! Return name of the ACS object.
DGNPLATFORM_EXPORT WString GetName () const;

//! Return description of the ACS object.
DGNPLATFORM_EXPORT WString GetDescription () const;

//! Return type of the ACS object.
DGNPLATFORM_EXPORT ACSType GetType () const;

//! Return localized name of the Type of the ACS object.
DGNPLATFORM_EXPORT WString GetTypeName () const;

//! Return the scale factor stored in the ACS object.
DGNPLATFORM_EXPORT double GetScale () const;

//! Return the origin point stored in the ACS object.
DGNPLATFORM_EXPORT DPoint3dR GetOrigin (DPoint3dR pOrigin) const;

//! Return the rotation matrix stored in the ACS object.
DGNPLATFORM_EXPORT RotMatrixR GetRotation (RotMatrixR pRot) const;

//! Returns true if the coordinate system cannot be changed.
DGNPLATFORM_EXPORT bool GetIsReadOnly () const;

//! Change the name stored in the ACS object.
DGNPLATFORM_EXPORT ACSFlags GetFlags () const;

//! Change the name stored in the ACS object.
DGNPLATFORM_EXPORT StatusInt SetName (WCharCP name);

//! Change the description stored in the ACS object.
DGNPLATFORM_EXPORT StatusInt SetDescription (WCharCP descr);

//! Change the type stored in the ACS object.
DGNPLATFORM_EXPORT StatusInt SetType (ACSType type);

//! Change the scale factor stored in the ACS object.
DGNPLATFORM_EXPORT StatusInt SetScale (double scale);

//! Change the flags stored in the ACS object.
DGNPLATFORM_EXPORT StatusInt SetFlags (ACSFlags flags);

//! Change the origin point stored in the ACS object.
DGNPLATFORM_EXPORT StatusInt SetOrigin (DPoint3dCR pOrigin);

//! Change the rotation matrix stored in the ACS object.
DGNPLATFORM_EXPORT StatusInt SetRotation (RotMatrixCR pRot);

//! Get the point (in UORs) corresponding to the input string.
//! @bsimethod
DGNPLATFORM_EXPORT StatusInt PointFromString (DPoint3dR outPoint, WStringR errorMsg, WCharCP inString, bool relative, DPoint3dCP lastPoint, DgnModelR modelRef);

//! Get the string that represents the input point.
DGNPLATFORM_EXPORT StatusInt StringFromPoint (WStringR outString, WStringR errorMsg, DPoint3dCR inPoint, bool delta, DPoint3dCP deltaOrigin, DgnModelR modelRef, DistanceFormatterR distanceFromatter, DirectionFormatterR directionFormatter);

//__PUBLISH_SECTION_END__

//! Get the ACS extender id.
DGNPLATFORM_EXPORT StatusInt CompleteSetupFromViewController (PhysicalViewControllerCP info);

//! Display a representation of the ACS in the given view.
DGNPLATFORM_EXPORT void DisplayInView (ViewportP vp, ACSDisplayOptions options, bool drawName) const;

//! Boresite to ACS triad in the given view. The borePt and hitPt are in active coords...
//! @bsimethod
DGNPLATFORM_EXPORT bool Locate (DPoint3dR hitPt, ViewportR vp, DPoint3dCR borePt, double radius);

//! Get the ACS extender id.
//! @bsimethod
DGNPLATFORM_EXPORT UInt32 GetExtenderId () const;

//! Get the buffer size, in bytes, required to serialize the ACS.
DGNPLATFORM_EXPORT UInt32 GetSerializedSize () const;

//! Get the buffer size, in bytes, required to serialize the ACS.
DGNPLATFORM_EXPORT StatusInt Serialize (void *buffer, UInt32 maxSize) const;

//! Draw the grid to the specified Viewport.
DGNPLATFORM_EXPORT void DrawGrid (ViewportP viewPort) const;

//! Fix the point to the ACS's grid
DGNPLATFORM_EXPORT void PointToGrid (ViewportP viewPort, DPoint3dR point) const;

//__PUBLISH_SECTION_START__
};

/** @endGroup */

END_BENTLEY_DGNPLATFORM_NAMESPACE

/** @endcond */
