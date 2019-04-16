/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include "Render.h"
#include "ValueFormat.h"

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
    };

enum class ACSDisplayOptions
    {
    None            = 0, // used for testing individual bits.
    Active          = (1 << 0),
    Deemphasized    = (1 << 1),
    Hilite          = (1 << 2),
    CheckVisible    = (1 << 3),
    Dynamics        = (1 << 4),
    };

ENUM_IS_FLAGS (ACSDisplayOptions)

namespace ACSElementHandler {struct CoordSys2d; struct CoordSys3d; struct CoordSysSpatial;}

//=======================================================================================
// @bsiclass                                                    Brien.Bastings  02/17
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE AuxCoordSystem : DefinitionElement
{
    DEFINE_T_SUPER(DefinitionElement);

protected:
    explicit AuxCoordSystem(CreateParams const& params) : T_Super(params) {}

    virtual AuxCoordSystem2dCP _GetAsAuxCoordSystem2d() const = 0; // Either this method or _GetAsAuxCoordSystem3d must return non-null.
    virtual AuxCoordSystem3dCP _GetAsAuxCoordSystem3d() const = 0; // Either this method or _GetAsAuxCoordSystem2d must return non-null.
    virtual AuxCoordSystemSpatialCP _GetAsAuxCoordSystemSpatial() const {return nullptr;}

    DGNPLATFORM_EXPORT virtual bool _IsValidForView(ViewControllerCR viewController) const;

    virtual ACSType _GetType() const {return static_cast<ACSType>(GetPropertyValueInt32("Type"));}
    virtual BentleyStatus _SetType(ACSType type) {SetPropertyValue("Type", static_cast<int32_t>(type)); return SUCCESS;}

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
    DGNPLATFORM_EXPORT virtual void _Pick(PickContextR) const;

    virtual StatusInt _GetStandardGridParams(Point2dR gridReps, Point2dR gridOffset, DPoint2dR spacing, uint32_t& gridPerRef) const {return ERROR;}
    virtual StatusInt _SetStandardGridParams(Point2dCR gridReps, Point2dCR gridOffset, DPoint2dCR spacing, uint32_t gridPerRef) {return ERROR;}

    DGNPLATFORM_EXPORT virtual void _DrawGrid(DecorateContextR context) const;
    DGNPLATFORM_EXPORT virtual void _PointToGrid(DgnViewportR vp, DPoint3dR point) const;

    DGNPLATFORM_EXPORT virtual StatusInt _PointFromString(DPoint3dR outPoint, Utf8StringR errorMsg, Utf8CP inString, bool relative, DPoint3dCP lastPoint, DgnModelR modelRef) const;
    DGNPLATFORM_EXPORT virtual StatusInt _StringFromPoint(Utf8StringR outString, Utf8StringR errorMsg, DPoint3dCR inPoint, bool delta, DPoint3dCP deltaOrigin, DgnModelR modelRef, DistanceFormatterR distanceFormatter, DirectionFormatterR directionFormatter) const;

public:
    //! Create a new acs from an existing acs that is suitable for insert unlike acs->MakeCopy<AuxCoordSystem>() which just create a writeable copy for update.
    static AuxCoordSystemPtr CreateFrom(AuxCoordSystemCR acs) {return dynamic_cast<AuxCoordSystem*>(acs.Clone().get());}
    DGNPLATFORM_EXPORT static AuxCoordSystemPtr CreateNew(ViewDefinitionCR def, Utf8StringCR name=""); // Create default ACS for this type of view.
    DGNPLATFORM_EXPORT static AuxCoordSystemPtr CreateNew(DgnModelR model, DefinitionModelP defnModel, Utf8StringCR name); // Create default ACS for this type of model.
    DGNPLATFORM_EXPORT static DgnCode CreateCode(ViewDefinitionCR def, Utf8StringCR name=""); //!< @private
    DGNPLATFORM_EXPORT static DgnCode CreateCode(DgnModelR model, DefinitionModelP defnModel, Utf8StringCR name); //!< @private

    AuxCoordSystem2dCP ToAuxCoordSystem2d() const {return _GetAsAuxCoordSystem2d();}
    AuxCoordSystem3dCP ToAuxCoordSystem3d() const {return _GetAsAuxCoordSystem3d();}
    AuxCoordSystemSpatialCP ToAuxCoordSystemSpatial() const {return _GetAsAuxCoordSystemSpatial();}

    AuxCoordSystem2dP ToAuxCoordSystem2dP() {return const_cast<AuxCoordSystem2dP>(ToAuxCoordSystem2d());} //!< more efficient substitute for dynamic_cast<AuxCoordSystem2dP>(el)
    AuxCoordSystem3dP ToAuxCoordSystem3dP() {return const_cast<AuxCoordSystem3dP>(ToAuxCoordSystem3d());} //!< more efficient substitute for dynamic_cast<AuxCoordSystem3dP>(el)
    AuxCoordSystemSpatialP ToAuxCoordSystemSpatialP() {return const_cast<AuxCoordSystemSpatialP>(ToAuxCoordSystemSpatial());} //!< more efficient substitute for dynamic_cast<AuxCoordSystemSpatialP>(el)

    bool IsAuxCoordSystem2d() const {return nullptr != _GetAsAuxCoordSystem2d();} //!< Determine whether this ACS is 2d or not
    bool IsAuxCoordSystem3d() const {return nullptr != _GetAsAuxCoordSystem3d();} //!< Determine whether this ACS is 3d or not
    bool IsAuxCoordSystemSpatial() const {return nullptr != _GetAsAuxCoordSystemSpatial();} //!< Determine whether this ACS is a spatial coordinate system or not
    bool IsValidForView(ViewControllerCR viewController) const {return _IsValidForView(viewController);}

    //! Return type of the ACS object.
    ACSType GetType() const {return _GetType();}

    //! Change the type stored in the ACS object.
    BentleyStatus SetType(ACSType type) {return _SetType(type);}

    //! Return a string for the ACS type name.
    Utf8String GetTypeName() const;

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

    //! Allow snapping to ACS origin.
    void Pick(PickContextR context) const {return _Pick(context);}

    // Optional grid settings for this ACS that override the view definition's grid settings when drawing a grid aligned with the ACS.
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

    AuxCoordSystem2dCP _GetAsAuxCoordSystem2d() const override final {return this;}
    AuxCoordSystem3dCP _GetAsAuxCoordSystem3d() const override final {return nullptr;}

    DPoint3d _GetOrigin() const override {return DPoint3d::From(GetOrigin2d());}
    BentleyStatus _SetOrigin(DPoint3dCR origin) override {SetOrigin2d(DPoint2d::From(origin)); return SUCCESS;}

    RotMatrix _GetRotation() const override {AngleInDegrees angle = GetAngle(); return RotMatrix::FromAxisAndRotationAngle(2, angle.Radians());}
    BentleyStatus _SetRotation(RotMatrixCR rMatrix) override {YawPitchRollAngles angles; YawPitchRollAngles::TryFromRotMatrix(angles, rMatrix); SetAngle(angles.GetYaw()); return SUCCESS;}

public:
    //! Construct a new 2d Auxiliary Coordinate System.
    //! @param[in] model The DefinitionModel to hold the Auxiliary Coordinate System
    //! @param[in] name The name of the Auxiliary Coordinate System. Must be unique across all 2d Auxiliary Coordinate Systems.
    AuxCoordSystem2d(DefinitionModelR model, Utf8StringCR name="") : T_Super(CreateParams(model.GetDgnDb(), model.GetModelId(), QueryClassId(model.GetDgnDb()), CreateCode(model, name))) {}

    //! Create a DgnCode for an AuxCoordSystem2d given a name that is meant to be unique within the scope of the specified DefinitionModel
    static DgnCode CreateCode(DefinitionModelCR scope, Utf8StringCR name) {return name.empty() ? DgnCode() : CodeSpec::CreateCode(BIS_CODESPEC_AuxCoordSystem2d, scope, name);}

    static DgnClassId QueryClassId(DgnDbR db) {return DgnClassId(db.Schemas().GetClassId(BIS_ECSCHEMA_NAME, BIS_CLASS_AuxCoordSystem2d));} //!< @private

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

    AuxCoordSystem2dCP _GetAsAuxCoordSystem2d() const override final {return nullptr;}
    AuxCoordSystem3dCP _GetAsAuxCoordSystem3d() const override final {return this;}

    DPoint3d _GetOrigin() const override {return GetOrigin3d();}
    BentleyStatus _SetOrigin(DPoint3dCR origin) override {SetOrigin3d(origin); return SUCCESS;}

    RotMatrix _GetRotation() const override {YawPitchRollAngles angles = GetAngles(); return angles.ToRotMatrix();}
    BentleyStatus _SetRotation(RotMatrixCR rMatrix) override {YawPitchRollAngles angles; YawPitchRollAngles::TryFromRotMatrix(angles, rMatrix); SetAngles(angles); return SUCCESS;}

public:
    //! Construct a new 3d Auxiliary Coordinate System.
    //! @param[in] model The DefinitionModel to hold the Auxiliary Coordinate System
    //! @param[in] name The name of the Auxiliary Coordinate System. Must be unique across all 3d Auxiliary Coordinate Systems.
    AuxCoordSystem3d(DefinitionModelR model, Utf8StringCR name="") : T_Super(CreateParams(model.GetDgnDb(), model.GetModelId(), QueryClassId(model.GetDgnDb()), CreateCode(model, name))) {}

    //! Create a DgnCode for an AuxCoordSystem3d given a name that is meant to be unique within the scope of the specified DefinitionModel
    static DgnCode CreateCode(DefinitionModelCR scope, Utf8StringCR name) {return name.empty() ? DgnCode() : CodeSpec::CreateCode(BIS_CODESPEC_AuxCoordSystem3d, scope, name);}

    static DgnClassId QueryClassId(DgnDbR db) {return DgnClassId(db.Schemas().GetClassId(BIS_ECSCHEMA_NAME, BIS_CLASS_AuxCoordSystem3d));} //!< @private

    DPoint3d GetOrigin3d() const {return GetPropertyValueDPoint3d("Origin");}
    void SetOrigin3d(DPoint3dCR origin) {SetPropertyValue("Origin", origin);}

    YawPitchRollAngles GetAngles() const {return YawPitchRollAngles(AngleInDegrees::FromDegrees(GetPropertyValueDouble("Yaw")), AngleInDegrees::FromDegrees(GetPropertyValueDouble("Pitch")), AngleInDegrees::FromDegrees(GetPropertyValueDouble("Roll")));}
    void SetAngles(YawPitchRollAngles angles) {SetPropertyValue("Yaw", angles.GetYaw().Degrees()); SetPropertyValue("Pitch", angles.GetPitch().Degrees()); SetPropertyValue("Roll", angles.GetRoll().Degrees());}

}; // AuxCoordSystem3d

//=======================================================================================
// @bsiclass                                                    Brien.Bastings  02/17
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE AuxCoordSystemSpatial : AuxCoordSystem3d
{
    DGNELEMENT_DECLARE_MEMBERS(BIS_CLASS_AuxCoordSystemSpatial, AuxCoordSystem3d);
    friend struct ACSElementHandler::CoordSysSpatial;

protected:
    explicit AuxCoordSystemSpatial(CreateParams const& params) : T_Super(params) {}

    AuxCoordSystemSpatialCP _GetAsAuxCoordSystemSpatial() const override final {return this;}

public:
    //! Construct a new spatial Auxiliary Coordinate System.
    //! @param[in] model The DefinitionModel to hold the Auxiliary Coordinate System
    //! @param[in] name The name of the Auxiliary Coordinate System. Must be unique across all spatial Auxiliary Coordinate Systems.
    AuxCoordSystemSpatial(DefinitionModelR model, Utf8StringCR name="") : T_Super(CreateParams(model.GetDgnDb(), model.GetModelId(), QueryClassId(model.GetDgnDb()), CreateCode(model, name))) {SetOrigin3d(model.GetDgnDb().GeoLocation().GetGlobalOrigin());}

    //! Create a DgnCode for an AuxCoordSystemSpatial given a name that is meant to be unique within the scope of the specified DefinitionModel
    static DgnCode CreateCode(DefinitionModelCR scope, Utf8StringCR name) {return name.empty() ? DgnCode() : CodeSpec::CreateCode(BIS_CODESPEC_AuxCoordSystemSpatial, scope, name);}

    static DgnClassId QueryClassId(DgnDbR db) {return DgnClassId(db.Schemas().GetClassId(BIS_ECSCHEMA_NAME, BIS_CLASS_AuxCoordSystemSpatial));} //!< @private

}; // AuxCoordSystemSpatial

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

    struct CoordSysSpatial : CoordSys3d
    {
        ELEMENTHANDLER_DECLARE_MEMBERS(BIS_CLASS_AuxCoordSystemSpatial, AuxCoordSystemSpatial, CoordSysSpatial, CoordSys3d, DGNPLATFORM_EXPORT);
    };
 };

/** @endGroup */

END_BENTLEY_DGN_NAMESPACE
