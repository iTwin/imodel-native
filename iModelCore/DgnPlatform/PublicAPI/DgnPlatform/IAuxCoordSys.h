/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/IAuxCoordSys.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
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

//! \ingroup auxCoords
/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct IACSEvents
{
virtual void _OnACSEvent(AuxCoordSystemP acs, ACSEventType eventType, DgnModelP modelRef) = 0;

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

EventHandlerList<IACSEvents>* m_listeners = nullptr;

public:
    DEFINE_BENTLEY_NEW_DELETE_OPERATORS

IACSManager();

DGNPLATFORM_EXPORT void SendEvent(AuxCoordSystemP acs, ACSEventType eventType, DgnModelP modelRef);

DGNPLATFORM_EXPORT static bool IsPointAdjustmentRequired(DgnViewportR viewport);
DGNPLATFORM_EXPORT static bool IsSnapAdjustmentRequired(DgnViewportR viewport);
DGNPLATFORM_EXPORT static bool IsContextRotationRequired(DgnViewportR viewport);

DGNVIEW_EXPORT bool GetStandardRotation(RotMatrixR rMatrix, StandardView nStandard, DgnViewportP viewport, bool useACS);
DGNVIEW_EXPORT bool GetCurrentOrientation(RotMatrixR rMatrix, DgnViewportP viewport, bool checkAccuDraw, bool checkACS);

//__PUBLISH_SECTION_START__
public:

//! Add a listener for acs events.
DGNPLATFORM_EXPORT void AddListener(IACSEvents* acsListener);

//! Drop a listener for acs events.
DGNPLATFORM_EXPORT void DropListener(IACSEvents* acsListener);

//! Gets the ACS object for the active ACS.
//! @param[in]      vp          View to apply acs to.
//! @return ACS if one is active, NULL otherwise.
//! @remarks  This is not a copy and should not be freed; Use Clone uses you want
//!           changes to directly affect the ACS attached to this view.
DGNPLATFORM_EXPORT AuxCoordSystemCP GetActive(DgnViewportR vp);

//! Sets the active ACS properties.
//! @param[in]      auxCoordSys ACS to activate.
//! @param[in]      vp          View to apply acs to.
//! @return status
DGNPLATFORM_EXPORT StatusInt SetActive(AuxCoordSystemCP auxCoordSys, DgnViewportR vp);

DGNPLATFORM_EXPORT static IACSManagerR GetManager();

}; // IACSManager

namespace ACSElementHandler {struct CoordSys2d; struct CoordSys3d;}

//=======================================================================================
// @bsiclass                                                    Brien.Bastings  02/17
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE AuxCoordSystem : DefinitionElement
{
    DEFINE_T_SUPER(DefinitionElement);

protected:
    explicit AuxCoordSystem(CreateParams const& params) : T_Super(params) {}

    virtual ACSType _GetType() const {return static_cast<ACSType>(GetPropertyValueInt32("Type"));};
    virtual BentleyStatus _SetType(ACSType type) {SetPropertyValue("Type", static_cast<int32_t>(type)); return SUCCESS;};

    virtual DPoint3d _GetOrigin() const = 0;
    virtual BentleyStatus _SetOrigin(DPoint3dCR) = 0;

    virtual RotMatrix _GetRotation() const = 0;
    virtual BentleyStatus _SetRotation(RotMatrixCR) = 0;

    // Allow sub-classes to override how ACS triad is displayed in the view...
    DGNPLATFORM_EXPORT virtual ColorDef _GetAdjustedColor(ColorDef, bool isFill, DgnViewportCR, ACSDisplayOptions) const;
    DGNPLATFORM_EXPORT virtual Utf8String _GetAxisLabel(uint32_t axis) const;
    DGNPLATFORM_EXPORT virtual void _AddAxisLabel(Render::GraphicBuilderR, uint32_t axis, ACSDisplayOptions, DgnViewportCR vp) const;
    DGNPLATFORM_EXPORT virtual void _AddAxis(Render::GraphicBuilderR, uint32_t axis, ACSDisplayOptions, DgnViewportCR vp) const;
    DGNPLATFORM_EXPORT virtual Render::GraphicBuilderPtr _CreateGraphic(DecorateContextR, ACSDisplayOptions) const;

    DGNPLATFORM_EXPORT virtual void _Display(DecorateContextR, ACSDisplayOptions) const;

    virtual StatusInt _GetStandardGridParams(Point2dR gridReps, Point2dR gridOffset, DPoint2dR spacing, uint32_t& gridPerRef) const {return ERROR;}
    virtual StatusInt _SetStandardGridParams(Point2dCR gridReps, Point2dCR gridOffset, DPoint2dCR spacing, uint32_t gridPerRef) {return ERROR;}

    DGNPLATFORM_EXPORT virtual void _DrawGrid(DecorateContextR context) const;
    DGNPLATFORM_EXPORT virtual void _PointToGrid(DgnViewportR vp, DPoint3dR point) const;

    DGNPLATFORM_EXPORT virtual StatusInt _PointFromString(DPoint3dR outPoint, Utf8StringR errorMsg, Utf8CP inString, bool relative, DPoint3dCP lastPoint, DgnModelR modelRef) const;
    DGNPLATFORM_EXPORT virtual StatusInt _StringFromPoint(Utf8StringR outString, Utf8StringR errorMsg, DPoint3dCR inPoint, bool delta, DPoint3dCP deltaOrigin, DgnModelR modelRef, DistanceFormatterR distanceFormatter, DirectionFormatterR directionFormatter) const;

public:
    //! Return type of the ACS object.
    ACSType GetType() const {return _GetType();}

    //! Change the type stored in the ACS object.
    BentleyStatus SetType(ACSType type) {return _SetType(type);}

    //! Return the origin point stored in the ACS object.
    DPoint3d GetOrigin() const {return _GetOrigin();}

    //! Change the origin point stored in the ACS object.
    BentleyStatus SetOrigin(DPoint3dCR origin) {return _SetOrigin(origin);}

    //! Return the rotation matrix stored in the ACS object.
    RotMatrix GetRotation() const {return _GetRotation();}

    //! Change the rotation matrix stored in the ACS object.
    BentleyStatus SetRotation(RotMatrixCR rMatrix) {return _SetRotation(rMatrix);}

    //! Return a string describing the ACS object.
    Utf8String GetDescription() const {return GetPropertyValueString("Description");}

    //! Set the description of the ACS object.
    void SetDescription(Utf8CP description) {SetPropertyValue("Description", description);}

    //! Display a representation of the ACS in the given view.
    void Display(DecorateContextR context, ACSDisplayOptions options) const {return _Display(context, options);}

    // NOTE: Standard grid settings don't apply to type ACS_TYPE_GeoCoordinate...
    bool GetGridSpacing(DPoint2dR spacing, uint32_t& gridPerRef, Point2dR gridReps, Point2dR gridOffset, DgnViewportR vp) const; //!< NOTE: Returns true when ACS overrides view's grid settings...
    StatusInt GetStandardGridParams(Point2dR gridReps, Point2dR gridOffset, DPoint2dR spacing, uint32_t& gridPerRef) const {return _GetStandardGridParams(gridReps, gridOffset, spacing, gridPerRef);}
    StatusInt SetStandardGridParams(Point2dCR gridReps, Point2dCR gridOffset, DPoint2dCR spacing, uint32_t gridPerRef) {return _SetStandardGridParams(gridReps, gridOffset, spacing, gridPerRef);}

    //! Draw the grid to the specified context.
    void DrawGrid(DecorateContextR context) const {return _DrawGrid(context);}

    //! Fix the point to the ACS's grid
    void PointToGrid(DgnViewportR viewport, DPoint3dR point) const {_PointToGrid(viewport, point);}

    //! Get the point (in UORs) corresponding to the input string.
    StatusInt PointFromString(DPoint3dR outPoint, Utf8StringR errorMsg, Utf8CP inString, bool relative, DPoint3dCP lastPoint, DgnModelR modelRef) const {return _PointFromString(outPoint, errorMsg, inString, relative, lastPoint, modelRef);}

    //! Get the string that represents the input point.
    StatusInt StringFromPoint(Utf8StringR outString, Utf8StringR errorMsg, DPoint3dCR inPoint, bool delta, DPoint3dCP deltaOrigin, DgnModelR modelRef, DistanceFormatterR distanceFromatter, DirectionFormatterR directionFormatter) const {return _StringFromPoint(outString, errorMsg, inPoint, delta, deltaOrigin, modelRef, distanceFromatter, directionFormatter);}

}; // AuxCoordSystem

//=======================================================================================
// @bsiclass                                                    Brien.Bastings  02/17
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE AuxCoordSystem2d : AuxCoordSystem
{
    DGNELEMENT_DECLARE_MEMBERS(BIS_CLASS_AuxCoordSystem2d, AuxCoordSystem);
    friend struct ACSElementHandler::CoordSys2d;

protected:
    explicit AuxCoordSystem2d(CreateParams const& params) : T_Super(params) {}

    DPoint3d _GetOrigin() const override {return DPoint3d::From(GetOrigin2d());}
    BentleyStatus _SetOrigin(DPoint3dCR origin) override {SetOrigin2d(DPoint2d::From(origin)); return SUCCESS;}

    RotMatrix _GetRotation() const override {AngleInDegrees angle = GetAngle(); return RotMatrix::FromAxisAndRotationAngle(2, angle.Radians());}
    BentleyStatus _SetRotation(RotMatrixCR rMatrix) override {YawPitchRollAngles angles; YawPitchRollAngles::TryFromRotMatrix(angles, rMatrix); SetAngle(angles.GetYaw()); return SUCCESS;}

public:
    //! Construct a new 2d Auxiliary Coordinate System.
    //! @param[in] db The DgnDb to hold the Auxiliary Coordinate System
    //! @param[in] name The name of the Auxiliary Coordinate System. Must be unique across all 2d Auxiliary Coordinate Systems.
    AuxCoordSystem2d(DgnDbR db, Utf8StringCR name="") : T_Super(CreateParams(db, DgnModel::DictionaryId(), QueryClassId(db), CreateCode(db, name))) {}

    static DgnCode CreateCode(DgnDbR db, Utf8StringCR name) {return name.empty() ? DgnCode() : CodeSpec::CreateCode(db, BIS_CODESPEC_AuxCoordSystem2d, name);} //!< @private
    static DgnClassId QueryClassId(DgnDbR db) {return DgnClassId(db.Schemas().GetECClassId(BIS_ECSCHEMA_NAME, BIS_CLASS_AuxCoordSystem2d));} //!< @private

    DPoint2d GetOrigin2d() const {return GetPropertyValueDPoint2d("Origin");}
    void SetOrigin2d(DPoint2dCR origin) {SetPropertyValue("Origin", origin);}

    AngleInDegrees GetAngle() const {return AngleInDegrees::FromDegrees(GetPropertyValueDouble("Angle"));}
    void SetAngle(AngleInDegrees angle) {SetPropertyValue("Angle", angle.Degrees());}

}; // AuxCoordSystem2d

//=======================================================================================
// @bsiclass                                                    Brien.Bastings  02/17
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE AuxCoordSystem3d : AuxCoordSystem
{
    DGNELEMENT_DECLARE_MEMBERS(BIS_CLASS_AuxCoordSystem3d, AuxCoordSystem);
    friend struct ACSElementHandler::CoordSys3d;

protected:
    explicit AuxCoordSystem3d(CreateParams const& params) : T_Super(params) {}

    DPoint3d _GetOrigin() const override {return GetOrigin3d();}
    BentleyStatus _SetOrigin(DPoint3dCR origin) override {SetOrigin3d(origin); return SUCCESS;}

    RotMatrix _GetRotation() const override {YawPitchRollAngles angles = GetAngles(); return angles.ToRotMatrix();}
    BentleyStatus _SetRotation(RotMatrixCR rMatrix) override {YawPitchRollAngles angles; YawPitchRollAngles::TryFromRotMatrix(angles, rMatrix); SetAngles(angles); return SUCCESS;}

public:
    //! Construct a new 3d Auxiliary Coordinate System.
    //! @param[in] db The DgnDb to hold the Auxiliary Coordinate System
    //! @param[in] name The name of the Auxiliary Coordinate System. Must be unique across all 3d Auxiliary Coordinate Systems.
    AuxCoordSystem3d(DgnDbR db, Utf8StringCR name="") : T_Super(CreateParams(db, DgnModel::DictionaryId(), QueryClassId(db), CreateCode(db, name))) {}

    static DgnCode CreateCode(DgnDbR db, Utf8StringCR name) {return name.empty() ? DgnCode() : CodeSpec::CreateCode(db, BIS_CODESPEC_AuxCoordSystem3d, name);} //!< @private
    static DgnClassId QueryClassId(DgnDbR db) {return DgnClassId(db.Schemas().GetECClassId(BIS_ECSCHEMA_NAME, BIS_CLASS_AuxCoordSystem3d));} //!< @private

    DPoint3d GetOrigin3d() const {return GetPropertyValueDPoint3d("Origin");}
    void SetOrigin3d(DPoint3dCR origin) {SetPropertyValue("Origin", origin);}

    YawPitchRollAngles GetAngles() const {return YawPitchRollAngles(AngleInDegrees::FromDegrees(GetPropertyValueDouble("Yaw")), AngleInDegrees::FromDegrees(GetPropertyValueDouble("Pitch")), AngleInDegrees::FromDegrees(GetPropertyValueDouble("Roll")));}
    void SetAngles(YawPitchRollAngles angles) {SetPropertyValue("Yaw", angles.GetYaw().Degrees()); SetPropertyValue("Pitch", angles.GetPitch().Degrees()); SetPropertyValue("Roll", angles.GetRoll().Degrees());}

}; // AuxCoordSystem3d

//=======================================================================================
// @bsiclass                                                    Brien.Bastings  02/17
//=======================================================================================
namespace ACSElementHandler
{
    struct CoordSys2d : dgn_ElementHandler::Definition
    {
        ELEMENTHANDLER_DECLARE_MEMBERS(BIS_CLASS_AuxCoordSystem2d, AuxCoordSystem2d, CoordSys2d, Definition, DGNPLATFORM_EXPORT);
    };

    struct CoordSys3d : dgn_ElementHandler::Definition
    {
        ELEMENTHANDLER_DECLARE_MEMBERS(BIS_CLASS_AuxCoordSystem3d, AuxCoordSystem3d, CoordSys3d, Definition, DGNPLATFORM_EXPORT);
    };
 };

/** @endGroup */

END_BENTLEY_DGN_NAMESPACE

/** @endcond */
