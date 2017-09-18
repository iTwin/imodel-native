/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/RoadRailPhysical/TypicalSectionPoint.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

//__PUBLISH_SECTION_START__
#include "RoadRailPhysical.h"

BEGIN_BENTLEY_ROADRAILPHYSICAL_NAMESPACE

//=======================================================================================
//! Interface implemented by elements accepted as targets of TypicalSection constraints
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ITypicalSectionConstraintPoint
{
protected:
    virtual Dgn::DgnElementCR _GetITypicalSectionConstraintPointToDgnElement() const = 0;

public:
    Dgn::DgnElementId GetConstraintPointId() const { return _GetITypicalSectionConstraintPointToDgnElement().GetElementId(); }
}; // ITypicalSectionConstraintTarget

//=======================================================================================
//! Point construct in a TypicalSection portion definition.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE TypicalSectionPointName : Dgn::DefinitionElement, ITypicalSectionConstraintPoint
{
DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_TypicalSectionPointName, Dgn::DefinitionElement);
friend struct TypicalSectionPointNameHandler;

protected:
    //! @private
    explicit TypicalSectionPointName(CreateParams const& params);

    virtual Dgn::DgnElementCR _GetITypicalSectionConstraintPointToDgnElement() const override { return *this; }

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(TypicalSectionPointName)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_METHODS(TypicalSectionPointName)

    ROADRAILPHYSICAL_EXPORT static Dgn::CodeSpecId QueryCodeSpecId(Dgn::DgnDbCR dgndb);
    ROADRAILPHYSICAL_EXPORT static Dgn::DgnCode CreateCode(Dgn::DgnModelCR scope, Utf8StringCR value);

    ROADRAILPHYSICAL_EXPORT static TypicalSectionPointNamePtr Create(Dgn::DefinitionModelCR model, Utf8StringCR pointName, Utf8CP userLabel = nullptr);
    ROADRAILPHYSICAL_EXPORT static TypicalSectionPointNameCPtr CreateAndInsert(Dgn::DefinitionModelCR model, Utf8StringCR pointName, Utf8CP userLabel = nullptr);
    ROADRAILPHYSICAL_EXPORT static TypicalSectionPointNameCPtr QueryByName(Dgn::DefinitionModelCR model, Utf8StringCR pointName);
}; // TypicalSectionPointName

//=======================================================================================
//! Point construct in a TypicalSection portion definition.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE TypicalSectionPoint : Dgn::GeometricElement2d, ITypicalSectionConstraintPoint
{
DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_TypicalSectionPoint, Dgn::GeometricElement2d);
friend struct TypicalSectionPointHandler;

protected:
    //! @private
    explicit TypicalSectionPoint(CreateParams const& params) : T_Super(params) {}

    //! @private
    explicit TypicalSectionPoint(CreateParams const& params, TypicalSectionPointNameCR pointName);

    ROADRAILPHYSICAL_EXPORT static Dgn::CodeSpecId QueryCodeSpecId(Dgn::DgnDbCR dgndb);
    ROADRAILPHYSICAL_EXPORT static Dgn::DgnCode CreateCode(TypicalSectionPortionBreakDownModelCR scope);

    virtual Dgn::DgnElementCR _GetITypicalSectionConstraintPointToDgnElement() const override { return *this; }

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(TypicalSectionPoint)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_METHODS(TypicalSectionPoint)

    ROADRAILPHYSICAL_EXPORT static TypicalSectionPointPtr Create(TypicalSectionPortionBreakDownModelCR model);
    ROADRAILPHYSICAL_EXPORT static TypicalSectionPointCPtr CreateAndInsert(TypicalSectionPortionBreakDownModelCR model);
    ROADRAILPHYSICAL_EXPORT static TypicalSectionPointPtr Create(TypicalSectionPortionBreakDownModelCR model, TypicalSectionPointNameCR pointName);
    ROADRAILPHYSICAL_EXPORT static TypicalSectionPointCPtr CreateAndInsert(TypicalSectionPortionBreakDownModelCR model, TypicalSectionPointNameCR pointName);

    TypicalSectionPointNameCP GetPointName() const { return TypicalSectionPointName::Get(GetDgnDb(), GetPropertyValueId<Dgn::DgnElementId>("PointName")).get(); }
    void SetPointName(TypicalSectionPointNameCP newVal) { SetPropertyValue("PointName", newVal ? newVal->GetElementId() : Dgn::DgnElementId()); }
}; // TypicalSectionPoint

//=======================================================================================
//! Base class for Parameters used in Constraints to a Typical Section Point
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE TypicalSectionParameter : Dgn::DefinitionElement
{
DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_TypicalSectionParameter, Dgn::DefinitionElement);
friend struct TypicalSectionParameterHandler;

protected:
    //! @private
    explicit TypicalSectionParameter(CreateParams const& params);

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(TypicalSectionParameter)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_GET_METHODS(TypicalSectionParameter)

    ROADRAILPHYSICAL_EXPORT static Dgn::CodeSpecId QueryCodeSpecId(Dgn::DgnDbCR dgndb);
    ROADRAILPHYSICAL_EXPORT static Dgn::DgnCode CreateCode(Dgn::DefinitionModelCR scope, Utf8StringCR value);
}; // TypicalSectionParameter

//=======================================================================================
//! Parameters involving an Offset (Distance) in Constraints to a Typical Section Point
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE TypicalSectionOffsetParameter : TypicalSectionParameter
{
DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_TypicalSectionOffsetParameter, TypicalSectionParameter);
friend struct TypicalSectionOffsetParameterHandler;

protected:
    //! @private
    explicit TypicalSectionOffsetParameter(CreateParams const& params): T_Super(params) {}

    //! @private
    explicit TypicalSectionOffsetParameter(CreateParams const& params, double defaultVal);

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(TypicalSectionOffsetParameter)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_METHODS(TypicalSectionOffsetParameter)

    double GetDefaultValue() const { return GetPropertyValueDouble("DefaultValue"); }
    void SetDefaultValue(double newVal) { SetPropertyValue("DefaultValue", ECN::ECValue(newVal)); }

    ROADRAILPHYSICAL_EXPORT static TypicalSectionOffsetParameterPtr Create(Dgn::DefinitionModelCR model, Utf8StringCR name, double defaultVal = 0.0);
}; // TypicalSectionOffsetParameter

//=======================================================================================
//! Base class for Constraints applied to Typical Section Points
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE TypicalSectionConstraintSource : Dgn::InformationRecordElement
{
DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_TypicalSectionConstraintSource, Dgn::InformationRecordElement);
friend struct TypicalSectionConstraintSourceHandler;

protected:
    //! @private
    explicit TypicalSectionConstraintSource(CreateParams const& params): T_Super(params) {}

    virtual TypicalSectionHorizontalConstraintCP _ToHorizontalConstraint() const { return nullptr; }
    virtual TypicalSectionSlopeConstraintCP _ToSlopeConstraint() const { return nullptr; }
    virtual TypicalSectionVerticalConstraintCP _ToVerticalConstraint() const { return nullptr; }

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(TypicalSectionConstraintSource)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_GET_METHODS(TypicalSectionConstraintSource)

    ROADRAILPHYSICAL_EXPORT TypicalSectionConstraintSourceCPtr Insert(int priority, Dgn::DgnDbStatus* stat = nullptr);

    TypicalSectionHorizontalConstraintCP ToHorizontalConstraint() const { return _ToHorizontalConstraint(); }
    TypicalSectionSlopeConstraintCP ToSlopeConstraint() const { return _ToSlopeConstraint(); }
    TypicalSectionVerticalConstraintCP ToVerticalConstraint() const { return _ToVerticalConstraint(); }    
}; // TypicalSectionConstraintSource

//=======================================================================================
//! Base class for Constraints Values resolving to Offsets
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE TypicalSectionConstraintOffset : Dgn::InformationRecordElement
{
DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_TypicalSectionConstraintOffset, Dgn::InformationRecordElement);
friend struct TypicalSectionConstraintOffsetHandler;

protected:
    //! @private
    explicit TypicalSectionConstraintOffset(CreateParams const& params);

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(TypicalSectionConstraintOffset)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_GET_METHODS(TypicalSectionConstraintOffset)
}; // TypicalSectionConstraintOffset

//=======================================================================================
//! Base class for Constraints Values resolving to Slopes
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE TypicalSectionConstraintSlope : Dgn::InformationRecordElement
{
DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_TypicalSectionConstraintSlope, Dgn::InformationRecordElement);
friend struct TypicalSectionConstraintSlopeHandler;

protected:
    //! @private
    explicit TypicalSectionConstraintSlope(CreateParams const& params);

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(TypicalSectionConstraintSlope)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_GET_METHODS(TypicalSectionConstraintSlope)    
}; // TypicalSectionConstraintSlope

//=======================================================================================
//! Base class for Constraints applied to Typical Section Points
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE TypicalSectionConstraintWithOffset : TypicalSectionConstraintSource
{
DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_TypicalSectionConstraintWithOffset, TypicalSectionConstraintSource);
friend struct TypicalSectionConstraintWithOffsetHandler;

protected:
    //! @private
    explicit TypicalSectionConstraintWithOffset(CreateParams const& params);

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(TypicalSectionConstraintWithOffset)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_GET_METHODS(TypicalSectionConstraintWithOffset)

    TypicalSectionConstraintOffsetCP GetOffset() const { return TypicalSectionConstraintOffset::Get(GetDgnDb(), GetPropertyValueId<Dgn::DgnElementId>("Offset")).get(); }
    void SetOffset(TypicalSectionConstraintOffsetCP newOffset) { SetPropertyValue("Offset", (newOffset) ? newOffset->GetElementId() : Dgn::DgnElementId()); }

    ITypicalSectionConstraintPointCP GetPointRef() const { return dynamic_cast<ITypicalSectionConstraintPointCP>(GetDgnDb().Elements().GetElement(GetPropertyValueId<Dgn::DgnElementId>("PointRef")).get()); }
    void SetPointRef(ITypicalSectionConstraintPointCP pointRef) { SetPropertyValue("PointRef", (pointRef) ? pointRef->GetConstraintPointId() : Dgn::DgnElementId()); }
}; // TypicalSectionConstraintWithOffset

//=======================================================================================
//! Typical Section Horizontal Constraint for Points
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE TypicalSectionHorizontalConstraint : TypicalSectionConstraintWithOffset
{
DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_TypicalSectionHorizontalConstraint, TypicalSectionConstraintWithOffset);
friend struct TypicalSectionHorizontalConstraintHandler;

protected:
    //! @private
    explicit TypicalSectionHorizontalConstraint(CreateParams const& params) : T_Super(params) {}

    //! @private
    explicit TypicalSectionHorizontalConstraint(CreateParams const& params, ITypicalSectionConstraintPointCR pointRef);

    virtual TypicalSectionHorizontalConstraintCP _ToHorizontalConstraint() const override { return this; }

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(TypicalSectionHorizontalConstraint)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_GET_UPDATE_METHODS(TypicalSectionHorizontalConstraint)

    ROADRAILPHYSICAL_EXPORT static TypicalSectionHorizontalConstraintPtr Create(TypicalSectionPointCR constrainedPoint, ITypicalSectionConstraintPointCR pointRef);
    ROADRAILPHYSICAL_EXPORT static TypicalSectionHorizontalConstraintCPtr CreateAndInsert(TypicalSectionPointCR constrainedPoint, ITypicalSectionConstraintPointCR pointRef, TypicalSectionConstraintConstantOffsetR offset, int priority);
}; // TypicalSectionHorizontalConstraint

//=======================================================================================
//! Typical Section Vertical Constraint for Points
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE TypicalSectionVerticalConstraint : TypicalSectionConstraintWithOffset
{
DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_TypicalSectionVerticalConstraint, TypicalSectionConstraintWithOffset);
friend struct TypicalSectionVerticalConstraintHandler;

protected:
    //! @private
    explicit TypicalSectionVerticalConstraint(CreateParams const& params) : T_Super(params) {}

    //! @private
    explicit TypicalSectionVerticalConstraint(CreateParams const& params, ITypicalSectionConstraintPointCR pointRef);

    virtual TypicalSectionVerticalConstraintCP _ToVerticalConstraint() const override { return this; }

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(TypicalSectionVerticalConstraint)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_GET_UPDATE_METHODS(TypicalSectionVerticalConstraint)

    ROADRAILPHYSICAL_EXPORT static TypicalSectionVerticalConstraintPtr Create(TypicalSectionPointCR constrainedPoint, ITypicalSectionConstraintPointCR pointRef);
    ROADRAILPHYSICAL_EXPORT static TypicalSectionVerticalConstraintCPtr CreateAndInsert(TypicalSectionPointCR constrainedPoint, ITypicalSectionConstraintPointCR pointRef, TypicalSectionConstraintConstantOffsetR offset, int priority);
}; // TypicalSectionVerticalConstraint

//=======================================================================================
//! Slope Constraints applied to Typical Section Points
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE TypicalSectionSlopeConstraint : TypicalSectionConstraintSource
{
DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_TypicalSectionSlopeConstraint, TypicalSectionConstraintSource);
friend struct TypicalSectionSlopeConstraintHandler;

protected:
    //! @private
    explicit TypicalSectionSlopeConstraint(CreateParams const& params) : T_Super(params) {}

    //! @private
    explicit TypicalSectionSlopeConstraint(CreateParams const& params, ITypicalSectionConstraintPointCR pointRef);

    virtual TypicalSectionSlopeConstraintCP _ToSlopeConstraint() const override { return this; }

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(TypicalSectionSlopeConstraint)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_GET_METHODS(TypicalSectionSlopeConstraint)

    TypicalSectionConstraintSlopeCP GetSlope() const { return TypicalSectionConstraintSlope::Get(GetDgnDb(), GetPropertyValueId<Dgn::DgnElementId>("Slope")).get(); }
    void SetOffset(TypicalSectionConstraintSlopeCP newSlope) { SetPropertyValue("Slope", (newSlope) ? newSlope->GetElementId() : Dgn::DgnElementId()); }

    ITypicalSectionConstraintPointCP GetPointRef() const { return dynamic_cast<ITypicalSectionConstraintPointCP>(GetDgnDb().Elements().GetElement(GetPropertyValueId<Dgn::DgnElementId>("PointRef")).get()); }
    void SetPointRef(ITypicalSectionConstraintPointCP pointRef) { SetPropertyValue("PointRef", (pointRef) ? pointRef->GetConstraintPointId() : Dgn::DgnElementId()); }

    ROADRAILPHYSICAL_EXPORT static TypicalSectionSlopeConstraintPtr Create(TypicalSectionPointCR constrainedPoint, ITypicalSectionConstraintPointCR pointRef);
    ROADRAILPHYSICAL_EXPORT static TypicalSectionSlopeConstraintCPtr CreateAndInsert(TypicalSectionPointCR constrainedPoint, ITypicalSectionConstraintPointCR pointRef, TypicalSectionConstraintConstantSlopeR offset, int priority);
}; // TypicalSectionSlopeConstraint

//=======================================================================================
//! Constant Constraint Offset value
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE TypicalSectionConstraintConstantOffset : TypicalSectionConstraintOffset
{
DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_TypicalSectionConstraintConstantOffset, TypicalSectionConstraintOffset);
friend struct TypicalSectionConstraintConstantOffsetHandler;

protected:
    //! @private
    explicit TypicalSectionConstraintConstantOffset(CreateParams const& params) : T_Super(params) {}

    //! @private
    explicit TypicalSectionConstraintConstantOffset(CreateParams const& params, double value);

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(TypicalSectionConstraintConstantOffset)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_METHODS(TypicalSectionConstraintConstantOffset)

    double GetValue() const { return GetPropertyValueDouble("Value"); }
    void SetValue(double newVal) { SetPropertyValue("Value", ECN::ECValue(newVal)); }

    ROADRAILPHYSICAL_EXPORT static TypicalSectionConstraintConstantOffsetPtr Create(TypicalSectionPortionBreakDownModelCR model, double value);
    ROADRAILPHYSICAL_EXPORT static TypicalSectionConstraintConstantOffsetCPtr CreateAndInsert(TypicalSectionConstraintWithOffsetCR constraint, double value);
}; // TypicalSectionConstraintConstantOffset

//=======================================================================================
//! Constant Constraint Slope value
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE TypicalSectionConstraintConstantSlope : TypicalSectionConstraintSlope
{
DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_TypicalSectionConstraintConstantSlope, TypicalSectionConstraintSlope);
friend struct TypicalSectionConstraintConstantSlopeHandler;

protected:
    //! @private
    explicit TypicalSectionConstraintConstantSlope(CreateParams const& params) : T_Super(params) {}

    //! @private
    explicit TypicalSectionConstraintConstantSlope(CreateParams const& params, double value);

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(TypicalSectionConstraintConstantSlope)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_METHODS(TypicalSectionConstraintConstantSlope)

    double GetValue() const { return GetPropertyValueDouble("Value"); }
    void SetValue(double newVal) { SetPropertyValue("Value", ECN::ECValue(newVal)); }

    ROADRAILPHYSICAL_EXPORT static TypicalSectionConstraintConstantSlopePtr Create(TypicalSectionPortionBreakDownModelCR model, double value);
    ROADRAILPHYSICAL_EXPORT static TypicalSectionConstraintConstantSlopeCPtr CreateAndInsert(TypicalSectionSlopeConstraintCR constraint, double value);
}; // TypicalSectionConstraintConstantSlope



//=================================================================================
//! ElementHandler for Typical Section Point Names
//! @ingroup GROUP_RoadRailPhysical
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE TypicalSectionPointNameHandler : Dgn::dgn_ElementHandler::Definition
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_TypicalSectionPointName, TypicalSectionPointName, TypicalSectionPointNameHandler, Dgn::dgn_ElementHandler::Definition, ROADRAILPHYSICAL_EXPORT)
}; // TypicalSectionPointNameHandler

//=================================================================================
//! ElementHandler for Typical Section Points
//! @ingroup GROUP_RoadRailPhysical
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE TypicalSectionPointHandler : Dgn::dgn_ElementHandler::Geometric2d
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_TypicalSectionPoint, TypicalSectionPoint, TypicalSectionPointHandler, Dgn::dgn_ElementHandler::Geometric2d, ROADRAILPHYSICAL_EXPORT)
}; // TypicalSectionPointHandler

//=================================================================================
//! ElementHandler for Typical Section Parameters
//! @ingroup GROUP_RoadRailPhysical
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE TypicalSectionParameterHandler : Dgn::dgn_ElementHandler::Definition
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_TypicalSectionParameter, TypicalSectionParameter, TypicalSectionParameterHandler, Dgn::dgn_ElementHandler::Definition, ROADRAILPHYSICAL_EXPORT)
}; // TypicalSectionParameterHandler

//=================================================================================
//! ElementHandler for Typical Section Offset Parameters
//! @ingroup GROUP_RoadRailPhysical
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE TypicalSectionOffsetParameterHandler : TypicalSectionParameterHandler
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_TypicalSectionOffsetParameter, TypicalSectionOffsetParameter, TypicalSectionOffsetParameterHandler, TypicalSectionParameterHandler, ROADRAILPHYSICAL_EXPORT)
}; // TypicalSectionOffsetParameterHandler

//=================================================================================
//! ElementHandler for Typical Section Constraint Sources
//! @ingroup GROUP_RoadRailPhysical
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE TypicalSectionConstraintSourceHandler : Dgn::dgn_ElementHandler::InformationRecord
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_TypicalSectionConstraintSource, TypicalSectionConstraintSource, TypicalSectionConstraintSourceHandler, Dgn::dgn_ElementHandler::InformationRecord, ROADRAILPHYSICAL_EXPORT)
}; // TypicalSectionConstraintSourceHandler

//=================================================================================
//! ElementHandler for Typical Section Constraint Sources based on an Offset
//! @ingroup GROUP_RoadRailPhysical
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE TypicalSectionConstraintWithOffsetHandler : TypicalSectionConstraintSourceHandler
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_TypicalSectionConstraintWithOffset, TypicalSectionConstraintWithOffset, TypicalSectionConstraintWithOffsetHandler, TypicalSectionConstraintSourceHandler, ROADRAILPHYSICAL_EXPORT)
}; // TypicalSectionConstraintWithOffsetHandler

//=================================================================================
//! ElementHandler for Typical Section Horizontal Constraints
//! @ingroup GROUP_RoadRailPhysical
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE TypicalSectionHorizontalConstraintHandler : TypicalSectionConstraintWithOffsetHandler
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_TypicalSectionHorizontalConstraint, TypicalSectionHorizontalConstraint, TypicalSectionHorizontalConstraintHandler, TypicalSectionConstraintWithOffsetHandler, ROADRAILPHYSICAL_EXPORT)
}; // TypicalSectionHorizontalConstraintHandler

//=================================================================================
//! ElementHandler for Typical Section Vertical Constraints
//! @ingroup GROUP_RoadRailPhysical
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE TypicalSectionVerticalConstraintHandler : TypicalSectionConstraintWithOffsetHandler
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_TypicalSectionVerticalConstraint, TypicalSectionVerticalConstraint, TypicalSectionVerticalConstraintHandler, TypicalSectionConstraintWithOffsetHandler, ROADRAILPHYSICAL_EXPORT)
}; // TypicalSectionVerticalConstraintHandler

//=================================================================================
//! ElementHandler for Typical Section Slope Constraints
//! @ingroup GROUP_RoadRailPhysical
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE TypicalSectionSlopeConstraintHandler : TypicalSectionConstraintSourceHandler
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_TypicalSectionSlopeConstraint, TypicalSectionSlopeConstraint, TypicalSectionSlopeConstraintHandler, TypicalSectionConstraintSourceHandler, ROADRAILPHYSICAL_EXPORT)
}; // TypicalSectionSlopeConstraintHandler

//=================================================================================
//! ElementHandler for Typical Section Constraint Offset Values
//! @ingroup GROUP_RoadRailPhysical
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE TypicalSectionConstraintOffsetHandler : Dgn::dgn_ElementHandler::InformationRecord
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_TypicalSectionConstraintOffset, TypicalSectionConstraintOffset, TypicalSectionConstraintOffsetHandler, Dgn::dgn_ElementHandler::InformationRecord, ROADRAILPHYSICAL_EXPORT)
}; // TypicalSectionConstraintOffsetHandler

//=================================================================================
//! ElementHandler for Constant Offsets in Typical Section Constraints
//! @ingroup GROUP_RoadRailPhysical
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE TypicalSectionConstraintConstantOffsetHandler : TypicalSectionConstraintOffsetHandler
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_TypicalSectionConstraintConstantOffset, TypicalSectionConstraintConstantOffset, TypicalSectionConstraintConstantOffsetHandler, TypicalSectionConstraintOffsetHandler, ROADRAILPHYSICAL_EXPORT)
}; // TypicalSectionConstraintConstantOffsetHandler

//=================================================================================
//! ElementHandler for Typical Section Constraint Slope Values
//! @ingroup GROUP_RoadRailPhysical
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE TypicalSectionConstraintSlopeHandler : Dgn::dgn_ElementHandler::InformationRecord
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_TypicalSectionConstraintSlope, TypicalSectionConstraintSlope, TypicalSectionConstraintSlopeHandler, Dgn::dgn_ElementHandler::InformationRecord, ROADRAILPHYSICAL_EXPORT)
}; // TypicalSectionConstraintSlopeHandler

//=================================================================================
//! ElementHandler for Constant Slopes in Typical Section Constraints
//! @ingroup GROUP_RoadRailPhysical
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE TypicalSectionConstraintConstantSlopeHandler : TypicalSectionConstraintSlopeHandler
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_TypicalSectionConstraintConstantSlope, TypicalSectionConstraintConstantSlope, TypicalSectionConstraintConstantSlopeHandler, TypicalSectionConstraintSlopeHandler, ROADRAILPHYSICAL_EXPORT)
}; // TypicalSectionConstraintConstantSlopeHandler

END_BENTLEY_ROADRAILPHYSICAL_NAMESPACE