/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/IAuxCoordSys.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
/** @cond BENTLEY_SDK_Internal */

#include "Render.h"
#include "ValueFormat.h"
#include "DgnCoreEvent.h"

BEGIN_BENTLEY_DGN_NAMESPACE

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
    None    = 0, // Used for testing individual bits.
    Default = 0,
    };

ENUM_IS_FLAGS (ACSFlags)

typedef RefCountedPtr<IAuxCoordSys> IAuxCoordSysPtr;

//! \ingroup auxCoords
//__PUBLISH_SECTION_END__
/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct     IAuxCoordSystemExtender
{

virtual uint32_t _GetExtenderId() const = 0;
virtual IAuxCoordSysP _Deserialize(void *persistentData, uint32_t dataSize, DgnModelP modelRef) = 0;
}; 

//__PUBLISH_SECTION_START__
/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct     IACSEvents
{
virtual void _OnACSEvent(IAuxCoordSysP acs, ACSEventType eventType, DgnModelP modelRef) = 0;

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

    IACSManager();
    void SetInhibitCurrentACSDisplay(bool inhibit) {m_inhibitCurrentACSDisplay = inhibit;}
    bool GetInhibitCurrentACSDisplay() {return m_inhibitCurrentACSDisplay;}

DGNPLATFORM_EXPORT void AddExtender(IAuxCoordSystemExtender* extender);
DGNPLATFORM_EXPORT void RemoveExtender(IAuxCoordSystemExtender* extender);
DGNPLATFORM_EXPORT IAuxCoordSystemExtender* FindExtender(uint32_t extenderID);

DGNPLATFORM_EXPORT void SaveSettings(SpatialViewControllerCP viewController);
DGNPLATFORM_EXPORT void ReadSettings(SpatialViewControllerP viewController);

DGNPLATFORM_EXPORT IAuxCoordSysPtr CreateACS (ACSType type, DPoint3dCR origin, RotMatrixCR rot, double scale, Utf8CP name, Utf8CP descr);

DGNPLATFORM_EXPORT void SendEvent(IAuxCoordSysP acs, ACSEventType eventType, DgnModelP modelRef);
DGNPLATFORM_EXPORT void DisplayCurrent(DecorateContextR, bool isCursorView);

DGNVIEW_EXPORT bool GetStandardRotation(RotMatrixR rMatrix, StandardView nStandard, DgnViewportP viewport, bool useACS, DgnCoordSystem coordSys);
DGNVIEW_EXPORT bool GetCurrentOrientation(RotMatrixR rMatrix, DgnViewportP viewport, bool checkAccuDraw, bool checkACS, DgnCoordSystem coordSys);

DGNVIEW_EXPORT static bool IsPointAdjustmentRequired(DgnViewportR viewport);
DGNVIEW_EXPORT static bool IsSnapAdjustmentRequired(DgnViewportR viewport);
DGNVIEW_EXPORT static bool IsContextRotationRequired(DgnViewportR viewport);

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__
public:

//! Add a listener for acs events.
DGNPLATFORM_EXPORT void AddListener(IACSEvents* acsListener);

//! Drop a listener for acs events.
DGNPLATFORM_EXPORT void DropListener(IACSEvents* acsListener);

//! Create ACS "smart pointer".
//! @return a "smart pointer" to an object that supports the IAuxCoordSysP interface.
DGNPLATFORM_EXPORT IAuxCoordSysPtr CreateACS ();

//! Gets the ACS object for the active ACS.
//! @param[in]      vp          View to apply acs to.
//! @return ACS if one is active, NULL otherwise.
//! @remarks  This is not a copy and should not be freed; Use Clone uses you want
//!           changes to directly affect the ACS attached to this view.
DGNPLATFORM_EXPORT IAuxCoordSysP GetActive(DgnViewportR vp);

//! Sets the active ACS properties.
//! @param[in]      auxCoordSys ACS to activate.
//! @param[in]      vp          View to apply acs to.
//! @return status
DGNPLATFORM_EXPORT StatusInt SetActive(IAuxCoordSysP auxCoordSys, DgnViewportR vp);

//! Gets an ACS object representing  a named ACS from a model.
//! @param[in]      name        The name of the ACS to retrieve
//! @param[in]      modelRef    The preferred model in which to search
//! @param[in]      options     Options to control searching
//! @return ACS if found, NULL otherwise.
DGNPLATFORM_EXPORT IAuxCoordSysPtr GetByName(Utf8CP name, DgnModelP modelRef, uint32_t options);

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
DGNPLATFORM_EXPORT StatusInt Delete(Utf8CP name, DgnModelP modelRef);

DGNPLATFORM_EXPORT static IACSManagerR GetManager();

}; // IACSManager

//=======================================================================================
//! An IAuxCoordSys is an object that holds the data which
//! describes an auxiliary coordinate system
//=======================================================================================
struct IAuxCoordSys : RefCountedBase
{
protected:
    virtual IAuxCoordSysPtr _Clone() const = 0;
    virtual bool _Equals(IAuxCoordSysCP other) const = 0;
    virtual Utf8String _GetName() const = 0;
    virtual Utf8String _GetDescription() const = 0;
    virtual ACSType _GetType() const = 0;
    virtual Utf8String _GetTypeName() const = 0;
    virtual double _GetScale() const = 0;
    virtual DPoint3dR _GetOrigin(DPoint3dR pOrigin) const = 0;
    virtual RotMatrixR _GetRotation(RotMatrixR pRot) const = 0;
    virtual RotMatrixR _GetRotation(RotMatrixR pRot, DPoint3dR pPosition) const = 0;
    virtual bool _GetIsReadOnly() const = 0;
    virtual ACSFlags _GetFlags() const = 0;
    virtual StatusInt _SetName(Utf8CP name) = 0;
    virtual StatusInt _SetDescription(Utf8CP descr) = 0;
    virtual StatusInt _SetType(ACSType type) = 0;
    virtual StatusInt _SetScale(double scale) = 0;
    virtual StatusInt _SetOrigin(DPoint3dCR pOrigin) = 0;
    virtual StatusInt _SetRotation(RotMatrixCR pRot) = 0;
    virtual StatusInt _PointFromString(DPoint3dR outPoint, Utf8StringR errorMsg, Utf8CP inString, bool relative, DPoint3dCP lastPoint, DgnModelR modelRef) = 0;
    virtual StatusInt _StringFromPoint(Utf8StringR outString, Utf8StringR errorMsg, DPoint3dCR inPoint, bool delta, DPoint3dCP deltaOrigin, DgnModelR modelRef, DistanceFormatterR distanceFormatter, DirectionFormatterR directionFormatter) = 0;
    virtual StatusInt _SetFlags(ACSFlags flags) = 0;
    virtual uint32_t _GetExtenderId() const = 0;
    virtual uint32_t _GetSerializedSize() const = 0;
    virtual StatusInt _Serialize(void *data, uint32_t maxSize) const = 0;
    virtual StatusInt _CompleteSetupFromViewController(SpatialViewControllerCP info) = 0;
    virtual void _DrawGrid(DecorateContextR context) const = 0;
    virtual void _PointToGrid(DgnViewportR vp, DPoint3dR point) const = 0;

    virtual StatusInt _GetStandardGridParams(Point2dR gridReps, Point2dR gridOffset, double& uorPerGrid, double& gridRatio, uint32_t& gridPerRef) const {return ERROR;}
    virtual StatusInt _SetStandardGridParams(Point2dCR gridReps, Point2dCR gridOffset, double uorPerGrid, double gridRatio, uint32_t gridPerRef) {return ERROR;}

    // these methods are called only internally, so they don't have corresponding nonvirtual public wrappers.
    DGNPLATFORM_EXPORT virtual bool _IsOriginInView(DPoint3dR drawOrigin, DgnViewportCR, bool adjustOrigin) const;
    DGNPLATFORM_EXPORT virtual ColorDef _GetColor(DgnViewportCR, ColorDef color, uint32_t transparency, ACSDisplayOptions options) const;
    DGNPLATFORM_EXPORT virtual uint32_t _GetTransparency(bool isFill, ACSDisplayOptions options) const;
    DGNPLATFORM_EXPORT virtual Utf8String _GetAxisLabel(uint32_t axis) const;

    DGNPLATFORM_EXPORT virtual void _AddZAxis(Render::GraphicBuilderR, ColorDef color, ACSDisplayOptions options) const;
    DGNPLATFORM_EXPORT virtual void _AddXYAxis(Render::GraphicBuilderR, ColorDef color, Utf8CP label, bool swapAxis, ACSDisplayOptions options, ACSFlags flags) const;
    DGNPLATFORM_EXPORT virtual void _AddAxisText(Render::GraphicBuilderR, Utf8CP label, bool isAxisLabel, double userOrgX, double userOrgY, double scale, double angle, ACSDisplayOptions options) const;
    DGNPLATFORM_EXPORT virtual Render::GraphicBuilderPtr _CreateGraphic(DecorateContextR, DPoint3dCR drawOrigin, double acsSizePixels, ACSDisplayOptions options, bool drawName) const;
    DGNPLATFORM_EXPORT virtual void _DisplayInView(DecorateContextR, ACSDisplayOptions options, bool drawName) const;

public:
    // Only for ACS's of type ACS_TYPE_GeoCoordinate is the rotation matrix position dependent, don't publish this yet.
    RotMatrixR GetRotation(RotMatrixR pRot, DPoint3dR pPosition) const {return _GetRotation(pRot, pPosition);}

    // Standard grid settings don't apply to type ACS_TYPE_GeoCoordinate...
    double GetGridScaleFactor(DgnViewportR vp) const;
    StatusInt GetGridSpacing(DPoint2dR spacing, uint32_t& gridPerRef, Point2dR gridReps, Point2dR gridOffset, DgnViewportR vp) const;

    StatusInt GetStandardGridParams(Point2dR gridReps, Point2dR gridOffset, double& uorPerGrid, double& gridRatio, uint32_t& gridPerRef) const {return _GetStandardGridParams(gridReps, gridOffset, uorPerGrid, gridRatio, gridPerRef);}
    StatusInt SetStandardGridParams(Point2dCR gridReps, Point2dCR gridOffset, double uorPerGrid, double gridRatio, uint32_t gridPerRef) {return _SetStandardGridParams(gridReps, gridOffset, uorPerGrid, gridRatio, gridPerRef);}

    //! Return a copy of the ACS object.
    IAuxCoordSysPtr Clone() const {return _Clone();}

    //! Returns true if the is the same ACS object.
    bool Equals(IAuxCoordSysCP other) const {return _Equals(other);}

    //! Return name of the ACS object.
    Utf8String GetName() const {return _GetName();}

    //! Return description of the ACS object.
    Utf8String GetDescription() const {return _GetDescription();}

    //! Return type of the ACS object.
    ACSType GetType() const {return _GetType();}

    //! Return localized name of the Type of the ACS object.
    Utf8String GetTypeName() const {return _GetTypeName();}

    //! Return the scale factor stored in the ACS object.
    double GetScale() const {return _GetScale();}

    //! Return the origin point stored in the ACS object.
    DPoint3dR GetOrigin(DPoint3dR pOrigin) const {return _GetOrigin(pOrigin);}

    //! Return the rotation matrix stored in the ACS object.
    RotMatrixR GetRotation(RotMatrixR pRot) const {return _GetRotation(pRot);}

    //! Returns true if the coordinate system cannot be changed.
    bool GetIsReadOnly() const {return _GetIsReadOnly();}

    //! Change the name stored in the ACS object.
    ACSFlags GetFlags() const {return _GetFlags();}

    //! Change the name stored in the ACS object.
    StatusInt SetName(Utf8CP name) {return _SetName(name);}


    //! Change the description stored in the ACS object.
    StatusInt SetDescription(Utf8CP descr) {return _SetDescription(descr);}

    //! Change the type stored in the ACS object.
    StatusInt SetType(ACSType type) {return _SetType(type);}

    //! Change the scale factor stored in the ACS object.
    StatusInt SetScale(double scale) {return _SetScale(scale);}

    //! Change the flags stored in the ACS object.
    StatusInt SetFlags(ACSFlags flags) {return _SetFlags(flags);}

    //! Change the origin point stored in the ACS object.
    StatusInt SetOrigin(DPoint3dCR pOrigin) {return _SetOrigin(pOrigin);}

    //! Change the rotation matrix stored in the ACS object.
    StatusInt SetRotation(RotMatrixCR pRot) {return _SetRotation(pRot);}

    //! Get the point (in UORs) corresponding to the input string.
    StatusInt PointFromString(DPoint3dR outPoint, Utf8StringR errorMsg, Utf8CP inString, bool relative, DPoint3dCP lastPoint, DgnModelR modelRef) {return _PointFromString(outPoint, errorMsg, inString, relative, lastPoint, modelRef);}

    //! Get the string that represents the input point.
    StatusInt StringFromPoint(Utf8StringR outString, Utf8StringR errorMsg, DPoint3dCR inPoint, bool delta, DPoint3dCP deltaOrigin, DgnModelR modelRef, DistanceFormatterR distanceFromatter, DirectionFormatterR directionFormatter)
                                                        { return _StringFromPoint(outString, errorMsg, inPoint, delta, deltaOrigin, modelRef, distanceFromatter, directionFormatter); }

    //! Get the ACS extender id.
    StatusInt CompleteSetupFromViewController(SpatialViewControllerCP info) {return _CompleteSetupFromViewController(info);}

    //! Display a representation of the ACS in the given view.
    void DisplayInView(DecorateContextR context, ACSDisplayOptions options, bool drawName) const {return _DisplayInView(context, options, drawName);}

    //! Boresite to ACS triad in the given view. The borePt and hitPt are in active coords...
    DGNPLATFORM_EXPORT bool Locate(DPoint3dR hitPt, DgnViewportR vp, DPoint3dCR borePt, double radius);

    //! Get the ACS extender id.
    uint32_t GetExtenderId() const {return _GetExtenderId();}

    //! Get the buffer size, in bytes, required to serialize the ACS.
    uint32_t GetSerializedSize() const {return _GetSerializedSize();}

    //! Get the buffer size, in bytes, required to serialize the ACS.
    StatusInt Serialize(void *buffer, uint32_t maxSize) const {return _Serialize(buffer, maxSize);}

    //! Draw the grid to the specified context.
    void DrawGrid(DecorateContextR context) const {return _DrawGrid(context);}

    //! Fix the point to the ACS's grid
    void PointToGrid(DgnViewportR viewport, DPoint3dR point) const {_PointToGrid(viewport, point);}
};

/** @endGroup */

END_BENTLEY_DGN_NAMESPACE

/** @endcond */
