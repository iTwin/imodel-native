/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/RoadRailPhysical/TypicalSection.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#include "RoadRailPhysical.h"
#include "TypicalSectionPoint.h"

BEGIN_BENTLEY_ROADRAILPHYSICAL_NAMESPACE

//=======================================================================================
//! Base class representing the definition of a portion of overall Typical-sections of a Pathway.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE TypicalSectionPortionDefinitionElement : Dgn::DefinitionElement
{
DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_TypicalSectionPortionDefinitionElement, Dgn::DefinitionElement);
friend struct TypicalSectionPortionDefinitionElementHandler;

protected:
    //! @private
    explicit TypicalSectionPortionDefinitionElement(CreateParams const& params);

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(TypicalSectionPortionDefinitionElement)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_GET_METHODS(TypicalSectionPortionDefinitionElement)

    ROADRAILPHYSICAL_EXPORT TypicalSectionPointCPtr QueryOriginPoint() const;

    ROADRAILPHYSICAL_EXPORT static Dgn::DgnDbStatus SetOriginPoint(TypicalSectionPortionDefinitionElementCR, TypicalSectionPointCR point);
}; // TypicalSectionPortionDefinitionElement

//=======================================================================================
//! Model breaking-down a TypicalSectionPortion elements
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE TypicalSectionPortionBreakDownModel : Dgn::GeometricModel2d
{
DGNMODEL_DECLARE_MEMBERS(BRRP_CLASS_TypicalSectionPortionBreakDownModel, Dgn::GeometricModel2d);
friend struct TypicalSectionPortionBreakDownModelHandler;

public:
    struct CreateParams : T_Super::CreateParams
    {
    DEFINE_T_SUPER(TypicalSectionPortionBreakDownModel::T_Super::CreateParams);

    //! Parameters to create a new instance of an TypicalSectionPortionBreakDownModel.
    //! @param[in] dgndb The DgnDb for the new DgnModel
    //! @param[in] modeledElementId The DgnElementId of the element this this DgnModel is describing/modeling
    CreateParams(Dgn::DgnDbR dgndb, Dgn::DgnElementId modeledElementId)
        : T_Super(dgndb, TypicalSectionPortionBreakDownModel::QueryClassId(dgndb), modeledElementId)
        {}

    //! @private
    //! This constructor is used only by the model handler to create a new instance, prior to calling ReadProperties on the model object
    CreateParams(DgnModel::CreateParams const& params) : T_Super(params) {}
    }; // CreateParams

protected:
    explicit TypicalSectionPortionBreakDownModel(CreateParams const& params) : T_Super(params) { }

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(TypicalSectionPortionBreakDownModel)

    static TypicalSectionPortionBreakDownModelCPtr Get(Dgn::DgnDbR db, Dgn::DgnModelId id) { return db.Models().Get< TypicalSectionPortionBreakDownModel >(id); }
    static TypicalSectionPortionBreakDownModelPtr Create(CreateParams const& params) { return new TypicalSectionPortionBreakDownModel(params); }
}; // TypicalSectionPortionBreakDownModel

//=======================================================================================
//! Base class for definitions of Travelways.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE TravelwayDefinitionElement : TypicalSectionPortionDefinitionElement
{
DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_TravelwayDefinitionElement, TypicalSectionPortionDefinitionElement);
friend struct TravelwayDefinitionElementHandler;

protected:
    //! @private
    explicit TravelwayDefinitionElement(CreateParams const& params);

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(TravelwayDefinitionElement)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_GET_METHODS(TravelwayDefinitionElement)
}; // TravelwayDefinitionElement

//=======================================================================================
//! Travelway definition for Roadways.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RoadTravelwayDefinition : TravelwayDefinitionElement
{
DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_RoadTravelwayDefinition, TravelwayDefinitionElement);
friend struct RoadTravelwayDefinitionHandler;

protected:
    //! @private
    explicit RoadTravelwayDefinition(CreateParams const& params);

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(RoadTravelwayDefinition)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_GET_UPDATE_METHODS(RoadTravelwayDefinition)

    ROADRAILPHYSICAL_EXPORT static Dgn::CodeSpecId QueryCodeSpecId(Dgn::DgnDbCR dgndb);
    ROADRAILPHYSICAL_EXPORT static Dgn::DgnCode CreateCode(Dgn::DefinitionModelCR scope, Utf8StringCR value);

    ROADRAILPHYSICAL_EXPORT static RoadTravelwayDefinitionPtr Create(Dgn::DefinitionModelCR model, Utf8StringCR code);

    ROADRAILPHYSICAL_EXPORT RoadTravelwayDefinitionCPtr Insert(TypicalSectionPortionBreakDownModelPtr& breakDownModelPtr, Dgn::DgnDbStatus* stat = nullptr);
}; // RoadTravelwayDefinition

//=======================================================================================
//! Definition for TravelwaySide elements.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE TravelwaySideDefinition : TypicalSectionPortionDefinitionElement
{
DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_TravelwaySideDefinition, TypicalSectionPortionDefinitionElement);
friend struct TravelwaySideDefinitionHandler;

protected:
    //! @private
    explicit TravelwaySideDefinition(CreateParams const& params);

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(TravelwaySideDefinition)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_GET_UPDATE_METHODS(TravelwaySideDefinition)

    ROADRAILPHYSICAL_EXPORT static Dgn::CodeSpecId QueryCodeSpecId(Dgn::DgnDbCR dgndb);
    ROADRAILPHYSICAL_EXPORT static Dgn::DgnCode CreateCode(Dgn::DefinitionModelCR scope, Utf8StringCR value);

    ROADRAILPHYSICAL_EXPORT static TravelwaySideDefinitionPtr Create(Dgn::DefinitionModelCR model, Utf8StringCR code);

    ROADRAILPHYSICAL_EXPORT TravelwaySideDefinitionCPtr Insert(TypicalSectionPortionBreakDownModelPtr& breakDownModelPtr, Dgn::DgnDbStatus* stat = nullptr);
}; // EndConditionDefinition

//=======================================================================================
//! Definition for TravelwayStructure Elements.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE TravelwayStructureDefinition : TypicalSectionPortionDefinitionElement
{
DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_TravelwayStructureDefinition, TypicalSectionPortionDefinitionElement);
friend struct TravelwayStructureDefinitionHandler;

protected:
    //! @private
    explicit TravelwayStructureDefinition(CreateParams const& params);

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(TravelwayStructureDefinition)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_GET_UPDATE_METHODS(TravelwayStructureDefinition)

    ROADRAILPHYSICAL_EXPORT static Dgn::CodeSpecId QueryCodeSpecId(Dgn::DgnDbCR dgndb);
    ROADRAILPHYSICAL_EXPORT static Dgn::DgnCode CreateCode(Dgn::DefinitionModelCR scope, Utf8StringCR value);

    ROADRAILPHYSICAL_EXPORT static TravelwayStructureDefinitionPtr Create(Dgn::DefinitionModelCR model, Utf8StringCR code);

    ROADRAILPHYSICAL_EXPORT TravelwayStructureDefinitionCPtr Insert(TypicalSectionPortionBreakDownModelPtr& breakDownModelPtr, Dgn::DgnDbStatus* stat = nullptr);
}; // BufferDefinition

//=======================================================================================
//! Base class for Travelway Definition Elements.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE OverallTypicalSection : Dgn::TemplateRecipe2d
{
DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_OverallTypicalSection, Dgn::TemplateRecipe2d);
friend struct OverallTypicalSectionHandler;

protected:
    //! @private
    explicit OverallTypicalSection(CreateParams const& params);

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(OverallTypicalSection)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_GET_UPDATE_METHODS(OverallTypicalSection)

    ROADRAILPHYSICAL_EXPORT static Dgn::DgnCode CreateCode(Dgn::DefinitionModelCR scope, Utf8StringCR value);

    ROADRAILPHYSICAL_EXPORT static OverallTypicalSectionPtr Create(Dgn::DefinitionModelCR model, Utf8StringCR code);

    ROADRAILPHYSICAL_EXPORT OverallTypicalSectionCPtr Insert(OverallTypicalSectionBreakDownModelPtr& breakDownModelPtr, Dgn::DgnDbStatus* stat = nullptr);

    ROADRAILPHYSICAL_EXPORT OverallTypicalSectionAlignmentCPtr QueryMainAlignment() const;

    ROADRAILPHYSICAL_EXPORT static Dgn::DgnDbStatus SetMainAlignment(OverallTypicalSectionCR, OverallTypicalSectionAlignmentCR alignment);
}; // OverallTypicalSection

//=======================================================================================
//! Model breaking-down a TypicalSectionPortion elements
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE OverallTypicalSectionBreakDownModel : Dgn::GeometricModel2d
{
DGNMODEL_DECLARE_MEMBERS(BRRP_CLASS_OverallTypicalSectionBreakDownModel, Dgn::GeometricModel2d);
friend struct OverallTypicalSectionBreakDownModelHandler;

public:
    struct CreateParams : T_Super::CreateParams
    {
    DEFINE_T_SUPER(OverallTypicalSectionBreakDownModel::T_Super::CreateParams);

    //! Parameters to create a new instance of an TypicalSectionPortionBreakDownModel.
    //! @param[in] dgndb The DgnDb for the new DgnModel
    //! @param[in] modeledElementId The DgnElementId of the element this this DgnModel is describing/modeling
    CreateParams(Dgn::DgnDbR dgndb, Dgn::DgnElementId modeledElementId)
        : T_Super(dgndb, TypicalSectionPortionBreakDownModel::QueryClassId(dgndb), modeledElementId)
        {}

    //! @private
    //! This constructor is used only by the model handler to create a new instance, prior to calling ReadProperties on the model object
    CreateParams(DgnModel::CreateParams const& params) : T_Super(params) {}
    }; // CreateParams

protected:
    explicit OverallTypicalSectionBreakDownModel(CreateParams const& params) : T_Super(params) { }

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(TypicalSectionPortionBreakDownModel)

    static OverallTypicalSectionBreakDownModelCPtr Get(Dgn::DgnDbR db, Dgn::DgnModelId id) { return db.Models().Get< OverallTypicalSectionBreakDownModel >(id); }
    static OverallTypicalSectionBreakDownModelPtr Create(CreateParams const& params) { return new OverallTypicalSectionBreakDownModel(params); }
}; // OverallTypicalSectionBreakDownModel

//=======================================================================================
//! Alignment location in context of an Overall Typical Section
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE OverallTypicalSectionAlignment : Dgn::GeometricElement2d, ITypicalSectionConstraintPoint
{
DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_OverallTypicalSectionAlignment, Dgn::GeometricElement2d);
friend struct OverallTypicalSectionAlignmentHandler;

protected:
    //! @private
    explicit OverallTypicalSectionAlignment(CreateParams const& params) : T_Super(params) {}

    virtual Dgn::DgnElementCR _GetITypicalSectionConstraintPointToDgnElement() const override { return *this; }
    virtual void _GenerateElementGeom() override;

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(OverallTypicalSectionAlignment)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_METHODS(OverallTypicalSectionAlignment)

    ROADRAILPHYSICAL_EXPORT static OverallTypicalSectionAlignmentPtr Create(OverallTypicalSectionBreakDownModelCR model, DPoint2dCR position);
    ROADRAILPHYSICAL_EXPORT static OverallTypicalSectionAlignmentCPtr CreateAndInsert(OverallTypicalSectionBreakDownModelCR model, DPoint2dCR position);
}; // OverallTypicalSectionAlignment

//=======================================================================================
//! Typical Section Portion Definition location in context of an Overall Typical Section
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE OverallTypicalSectionPortion : Dgn::GeometricElement2d
{
DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_OverallTypicalSectionPortion, Dgn::GeometricElement2d);
friend struct OverallTypicalSectionPortionHandler;

protected:
    //! @private
    explicit OverallTypicalSectionPortion(CreateParams const& params) : T_Super(params) {}

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(OverallTypicalSectionPortion)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_METHODS(OverallTypicalSectionPortion)

    OverallTypicalSectionAlignmentCP GetAlignment() const { OverallTypicalSectionAlignment::Get(GetDgnDb(), GetPropertyValueId<Dgn::DgnElementId>("Alignment")).get(); }
    void SetAlignment(OverallTypicalSectionAlignmentCR alignment) { SetPropertyValue("Alignment", ECN::ECValue(alignment.GetElementId())); }
    TypicalSectionPortionDefinitionElementCP GetDefinition() const { TypicalSectionPortionDefinitionElement::Get(GetDgnDb(), GetPropertyValueId<Dgn::DgnElementId>("Definition")).get(); }
    void SetDefinition(TypicalSectionPortionDefinitionElementCR refDefinition) { SetPropertyValue("Definition", ECN::ECValue(refDefinition.GetElementId())); }

    ROADRAILPHYSICAL_EXPORT static OverallTypicalSectionPortionPtr Create(OverallTypicalSectionBreakDownModelCR model, 
        TypicalSectionPortionDefinitionElementCR refDefinition, OverallTypicalSectionAlignmentCR alignment);
}; // OverallTypicalSectionPortion

//=======================================================================================
//! Base class for TypicalSection Components.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE TypicalSectionComponentElement : Dgn::GeometricElement2d
{
DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_TypicalSectionComponentElement, Dgn::GeometricElement2d);
friend struct TypicalSectionComponentElementHandler;

protected:
    //! @private
    explicit TypicalSectionComponentElement(CreateParams const& params);

    virtual void _GenerateElementGeom();
    virtual bool _IsClosed() const { return false; }

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(TypicalSectionComponentElement)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_GET_METHODS(TypicalSectionComponentElement)

    ROADRAILPHYSICAL_EXPORT static Dgn::DgnDbStatus SetPoints(TypicalSectionComponentElementCR component, bvector<ITypicalSectionConstraintPointCP> const& points);

    ROADRAILPHYSICAL_EXPORT bvector<Dgn::DgnElementId> QueryPointIds() const;

    ROADRAILPHYSICAL_EXPORT void GenerateElementGeom() { _GenerateElementGeom(); }
}; // TypicalSectionComponentElement

//=======================================================================================
//! Buffer Components.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE TravelwayStructureComponentElement : TypicalSectionComponentElement
{
DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_TravelwayStructureComponentElement, TypicalSectionComponentElement);
friend struct TravelwayStructureComponentElementHandler;

protected:
    //! @private
    explicit TravelwayStructureComponentElement(CreateParams const& params);

    virtual bool _IsClosed() const override { return true; }

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(TravelwayStructureComponentElement)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_GET_METHODS(TravelwayStructureComponentElement)
}; // TravelwayStructureComponentElement

//=======================================================================================
//! Base class for Travelway Components.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE TravelwayComponentElement : TypicalSectionComponentElement
{
DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_TravelwayComponentElement, TypicalSectionComponentElement);
friend struct TravelwayComponentElementHandler;

protected:
    //! @private
    explicit TravelwayComponentElement(CreateParams const& params);

    virtual bool _IsClosed() const override { return false; }

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(TravelwayComponentElement)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_GET_METHODS(TravelwayComponentElement)    
}; // TravelwayComponentElement

//=======================================================================================
//! Lane Components for Roadways.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RoadLaneComponent : TravelwayComponentElement
{
DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_RoadLaneComponent, TravelwayComponentElement);
friend struct RoadLaneComponentHandler;

protected:
    //! @private
    explicit RoadLaneComponent(CreateParams const& params) : T_Super(params) {}

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(RoadLaneComponent)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_METHODS(RoadLaneComponent)

    ROADRAILPHYSICAL_EXPORT static RoadLaneComponentPtr Create(TypicalSectionPortionBreakDownModelCR model);
    ROADRAILPHYSICAL_EXPORT static RoadLaneComponentCPtr CreateAndInsert(TypicalSectionPortionBreakDownModelCR model, bvector<ITypicalSectionConstraintPointCP> const& points);
}; // RoadLaneComponent

//=======================================================================================
//! EndCondition Components.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE TravelwaySideComponentElement : TypicalSectionComponentElement
{
DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_TravelwaySideComponentElement, TypicalSectionComponentElement);
friend struct TravelwaySideComponentElementHandler;

protected:
    //! @private
    explicit TravelwaySideComponentElement(CreateParams const& params);

    virtual bool _IsClosed() const override { return true; }

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(TravelwaySideComponentElement)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_GET_METHODS(TravelwaySideComponentElement)
}; // TravelwaySideComponentElement

//=======================================================================================
//! Barrier Components for Pathways.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE BarrierComponent : TravelwaySideComponentElement
{
DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_BarrierComponent, TravelwaySideComponentElement);
friend struct BarrierComponentHandler;

protected:
    //! @private
    explicit BarrierComponent(CreateParams const& params) : T_Super(params) {}

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(BarrierComponent)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_METHODS(BarrierComponent)

    ROADRAILPHYSICAL_EXPORT static BarrierComponentPtr Create(TypicalSectionPortionBreakDownModelCR model);
    ROADRAILPHYSICAL_EXPORT static BarrierComponentCPtr CreateAndInsert(TypicalSectionPortionBreakDownModelCR model, bvector<ITypicalSectionConstraintPointCP> const& points);
}; // BarrierComponent

//=======================================================================================
//! Shoulder Components for Roadways.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RoadShoulderComponent : TravelwaySideComponentElement
{
DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_RoadShoulderComponent, TravelwaySideComponentElement);
friend struct RoadShoulderComponentHandler;

protected:
    //! @private
    explicit RoadShoulderComponent(CreateParams const& params) : T_Super(params) {}

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(RoadShoulderComponent)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_METHODS(RoadShoulderComponent)

    ROADRAILPHYSICAL_EXPORT static RoadShoulderComponentPtr Create(TypicalSectionPortionBreakDownModelCR model);
    ROADRAILPHYSICAL_EXPORT static RoadShoulderComponentCPtr CreateAndInsert(TypicalSectionPortionBreakDownModelCR model, bvector<ITypicalSectionConstraintPointCP> const& points);
}; // RoadShoulderComponent

//=======================================================================================
//! Curb Components for Pathways.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE CurbComponent : TravelwaySideComponentElement
{
DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_CurbComponent, TravelwaySideComponentElement);
friend struct CurbComponentHandler;

protected:
    //! @private
    explicit CurbComponent(CreateParams const& params) : T_Super(params) {}

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(CurbComponent)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_METHODS(CurbComponent)

    ROADRAILPHYSICAL_EXPORT static CurbComponentPtr Create(TypicalSectionPortionBreakDownModelCR model);
    ROADRAILPHYSICAL_EXPORT static CurbComponentCPtr CreateAndInsert(TypicalSectionPortionBreakDownModelCR model, bvector<ITypicalSectionConstraintPointCP> const& points);
}; // CurbComponent

//=======================================================================================
//! Buffer Components for Pathways.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE BufferComponent : TravelwaySideComponentElement
{
DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_BufferComponent, TravelwaySideComponentElement);
friend struct BufferComponentHandler;

protected:
    //! @private
    explicit BufferComponent(CreateParams const& params) : T_Super(params) {}

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(BufferComponent)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_METHODS(BufferComponent)

    ROADRAILPHYSICAL_EXPORT static BufferComponentPtr Create(TypicalSectionPortionBreakDownModelCR model);
    ROADRAILPHYSICAL_EXPORT static BufferComponentCPtr CreateAndInsert(TypicalSectionPortionBreakDownModelCR model, bvector<ITypicalSectionConstraintPointCP> const& points);
}; // BufferComponent

//=======================================================================================
//! Side-Slope Condition Components for Pathways.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE SideSlopeConditionComponent : TravelwaySideComponentElement
{
DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_SideSlopeConditionComponent, TravelwaySideComponentElement);
friend struct SideSlopeConditionComponentHandler;

protected:
    //! @private
    explicit SideSlopeConditionComponent(CreateParams const& params);

    virtual bool _IsClosed() const override { return false; }

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(SideSlopeConditionComponent)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_METHODS(SideSlopeConditionComponent)

    int32_t GetPriority() const { return GetPropertyValueInt32("Priority"); }
    void SetPriority(int32_t newVal) { SetPropertyValue("Priority", ECN::ECValue(newVal)); }
    bool GetIsInfiniteLength() const { return GetPropertyValueBoolean("IsInfiniteLength"); }
    void SetIsInfiniteLength(bool newVal) { SetPropertyValue("IsInfiniteLength", ECN::ECValue(newVal)); }
    ROADRAILPHYSICAL_EXPORT Nullable<int32_t> GetBenchingCount() const;
    ROADRAILPHYSICAL_EXPORT void SetBenchingCount(Nullable<int32_t> newVal);

    ROADRAILPHYSICAL_EXPORT static SideSlopeConditionComponentPtr Create(TypicalSectionPortionBreakDownModelCR model, int32_t priority);
    ROADRAILPHYSICAL_EXPORT static SideSlopeConditionComponentCPtr CreateAndInsert(TypicalSectionPortionBreakDownModelCR model, int32_t priority, bvector<ITypicalSectionConstraintPointCP> const& points);
}; // SideSlopeConditionComponent

//=======================================================================================
//! Pavement Components for Roadways.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE PavementComponent : TravelwayStructureComponentElement
{
DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_PavementComponent, TravelwayStructureComponentElement);
friend struct PavementComponentHandler;

protected:
    //! @private
    explicit PavementComponent(CreateParams const& params) : T_Super(params) {}

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(PavementComponent)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_METHODS(PavementComponent)

    ROADRAILPHYSICAL_EXPORT static PavementComponentPtr Create(TypicalSectionPortionBreakDownModelCR model);
    ROADRAILPHYSICAL_EXPORT static PavementComponentCPtr CreateAndInsert(TypicalSectionPortionBreakDownModelCR model, bvector<ITypicalSectionConstraintPointCP> const& points);
}; // PavementComponent



//=================================================================================
//! ElementHandler for OverallTypicalSection Elements
//! @ingroup GROUP_RoadRailPhysical
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE OverallTypicalSectionHandler : Dgn::dgn_ElementHandler::TemplateRecipe2d
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_OverallTypicalSection, OverallTypicalSection, OverallTypicalSectionHandler, Dgn::dgn_ElementHandler::TemplateRecipe2d, ROADRAILPHYSICAL_EXPORT)
}; // OverallTypicalSectionHandler

//=================================================================================
//! ElementHandler for TypicalSectionAlignment Elements
//! @ingroup GROUP_RoadRailPhysical
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE OverallTypicalSectionAlignmentHandler : Dgn::dgn_ElementHandler::Geometric2d
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_OverallTypicalSectionAlignment, OverallTypicalSectionAlignment, OverallTypicalSectionAlignmentHandler, Dgn::dgn_ElementHandler::Geometric2d, ROADRAILPHYSICAL_EXPORT)
}; // OverallTypicalSectionAlignmentHandler

//=================================================================================
//! ElementHandler for TypicalSectionPortion Elements
//! @ingroup GROUP_RoadRailPhysical
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE OverallTypicalSectionPortionHandler : Dgn::dgn_ElementHandler::Geometric2d
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_OverallTypicalSectionPortion, OverallTypicalSectionPortion, OverallTypicalSectionPortionHandler, Dgn::dgn_ElementHandler::Geometric2d, ROADRAILPHYSICAL_EXPORT)
}; // OverallTypicalSectionPortionHandler

//=================================================================================
//! ElementHandler for TypicalSection DefinitionElements
//! @ingroup GROUP_RoadRailPhysical
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE TypicalSectionPortionDefinitionElementHandler : Dgn::dgn_ElementHandler::Definition
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_TypicalSectionPortionDefinitionElement, TypicalSectionPortionDefinitionElement, TypicalSectionPortionDefinitionElementHandler, Dgn::dgn_ElementHandler::Definition, ROADRAILPHYSICAL_EXPORT)
}; // TypicalSectionPortionDefinitionElementHandler

//=================================================================================
//! ElementHandler for Travelway Definition Elements
//! @ingroup GROUP_RoadRailPhysical
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE TravelwayDefinitionElementHandler : TypicalSectionPortionDefinitionElementHandler
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_TravelwayDefinitionElement, TravelwayDefinitionElement, TravelwayDefinitionElementHandler, TypicalSectionPortionDefinitionElementHandler, ROADRAILPHYSICAL_EXPORT)
}; // TravelwayDefinitionElementHandler

//=================================================================================
//! ElementHandler for Road-specific Travelway Definition Elements
//! @ingroup GROUP_RoadRailPhysical
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RoadTravelwayDefinitionHandler : TravelwayDefinitionElementHandler
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_RoadTravelwayDefinition, RoadTravelwayDefinition, RoadTravelwayDefinitionHandler, TravelwayDefinitionElementHandler, ROADRAILPHYSICAL_EXPORT)
}; // RoadTravelwayDefinitionHandler

//=================================================================================
//! ElementHandler for TravelwaySide Definition Elements
//! @ingroup GROUP_RoadRailPhysical
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE TravelwaySideDefinitionHandler : TypicalSectionPortionDefinitionElementHandler
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_TravelwaySideDefinition, TravelwaySideDefinition, TravelwaySideDefinitionHandler, TypicalSectionPortionDefinitionElementHandler, ROADRAILPHYSICAL_EXPORT)
}; // TravelwaySideDefinitionHandler

//=================================================================================
//! ElementHandler for TravelwayStructure Definition Elements
//! @ingroup GROUP_RoadRailPhysical
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE TravelwayStructureDefinitionHandler : TypicalSectionPortionDefinitionElementHandler
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_TravelwayStructureDefinition, TravelwayStructureDefinition, TravelwayStructureDefinitionHandler, TypicalSectionPortionDefinitionElementHandler, ROADRAILPHYSICAL_EXPORT)
}; // TravelwayStructureDefinitionHandler

//=======================================================================================
//! The ModelHandler for TypicalSectionPortionBreakDownModel
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE OverallTypicalSectionBreakDownModelHandler : Dgn::dgn_ModelHandler::Geometric2d
{
    MODELHANDLER_DECLARE_MEMBERS(BRRP_CLASS_OverallTypicalSectionBreakDownModel, OverallTypicalSectionBreakDownModel, OverallTypicalSectionBreakDownModelHandler, Dgn::dgn_ModelHandler::Geometric2d, ROADRAILPHYSICAL_EXPORT)
}; // OverallTypicalSectionBreakDownModelHandler

//=======================================================================================
//! The ModelHandler for TypicalSectionPortionBreakDownModel
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE TypicalSectionPortionBreakDownModelHandler : Dgn::dgn_ModelHandler::Geometric2d
{
    MODELHANDLER_DECLARE_MEMBERS(BRRP_CLASS_TypicalSectionPortionBreakDownModel, TypicalSectionPortionBreakDownModel, TypicalSectionPortionBreakDownModelHandler, Dgn::dgn_ModelHandler::Geometric2d, ROADRAILPHYSICAL_EXPORT)
}; // TypicalSectionPortionBreakDownModelHandler

//=================================================================================
//! ElementHandler for TypicalSection Elements
//! @ingroup GROUP_RoadRailPhysical
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE TypicalSectionComponentElementHandler : Dgn::dgn_ElementHandler::Geometric2d
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_TypicalSectionComponentElement, TypicalSectionComponentElement, TypicalSectionComponentElementHandler, Dgn::dgn_ElementHandler::Geometric2d, ROADRAILPHYSICAL_EXPORT)
}; // TypicalSectionComponentElementHandler

//=================================================================================
//! ElementHandler for EndCondition Component Elements
//! @ingroup GROUP_RoadRailPhysical
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE TravelwaySideComponentElementHandler : TypicalSectionComponentElementHandler
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_TravelwaySideComponentElement, TravelwaySideComponentElement, TravelwaySideComponentElementHandler, TypicalSectionComponentElementHandler, ROADRAILPHYSICAL_EXPORT)
}; // TravelwaySideComponentElementHandler

//=================================================================================
//! ElementHandler for Buffer Component Elements
//! @ingroup GROUP_RoadRailPhysical
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE TravelwayStructureComponentElementHandler : TypicalSectionComponentElementHandler
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_TravelwayStructureComponentElement, TravelwayStructureComponentElement, TravelwayStructureComponentElementHandler, TypicalSectionComponentElementHandler, ROADRAILPHYSICAL_EXPORT)
}; // TravelwayStructureComponentElementHandler

//=================================================================================
//! ElementHandler for Travelway Component Elements
//! @ingroup GROUP_RoadRailPhysical
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE TravelwayComponentElementHandler : TypicalSectionComponentElementHandler
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_TravelwayComponentElement, TravelwayComponentElement, TravelwayComponentElementHandler, TypicalSectionComponentElementHandler, ROADRAILPHYSICAL_EXPORT)
}; // TravelwayComponentElementHandler

//=================================================================================
//! ElementHandler for Road lane Component Elements
//! @ingroup GROUP_RoadRailPhysical
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RoadLaneComponentHandler : TravelwayComponentElementHandler
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_RoadLaneComponent, RoadLaneComponent, RoadLaneComponentHandler, TravelwayComponentElementHandler, ROADRAILPHYSICAL_EXPORT)
}; // RoadLaneComponentHandler

//=================================================================================
//! ElementHandler for Road shoulder Component Elements
//! @ingroup GROUP_RoadRailPhysical
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RoadShoulderComponentHandler : TravelwaySideComponentElementHandler
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_RoadShoulderComponent, RoadShoulderComponent, RoadShoulderComponentHandler, TravelwaySideComponentElementHandler, ROADRAILPHYSICAL_EXPORT)
}; // RoadShoulderComponentHandler

//=================================================================================
//! ElementHandler for Buffer Component Elements
//! @ingroup GROUP_RoadRailPhysical
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE BufferComponentHandler : TravelwaySideComponentElementHandler
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_BufferComponent, BufferComponent, BufferComponentHandler, TravelwaySideComponentElementHandler, ROADRAILPHYSICAL_EXPORT)
}; // BufferComponentHandler

//=================================================================================
//! ElementHandler for SideSlopeCondition Component Elements
//! @ingroup GROUP_RoadRailPhysical
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE SideSlopeConditionComponentHandler : TravelwaySideComponentElementHandler
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_SideSlopeConditionComponent, SideSlopeConditionComponent, SideSlopeConditionComponentHandler, TravelwaySideComponentElementHandler, ROADRAILPHYSICAL_EXPORT)
}; // SideSlopeConditionComponentHandler

//=================================================================================
//! ElementHandler for Barrier Component Elements
//! @ingroup GROUP_RoadRailPhysical
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE BarrierComponentHandler : TravelwaySideComponentElementHandler
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_BarrierComponent, BarrierComponent, BarrierComponentHandler, TravelwaySideComponentElementHandler, ROADRAILPHYSICAL_EXPORT)
}; // BarrierComponentHandler

//=================================================================================
//! ElementHandler for Curb Component Elements
//! @ingroup GROUP_RoadRailPhysical
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE CurbComponentHandler : TravelwaySideComponentElementHandler
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_CurbComponent, CurbComponent, CurbComponentHandler, TravelwaySideComponentElementHandler, ROADRAILPHYSICAL_EXPORT)
}; // CurbComponentHandler

//=================================================================================
//! ElementHandler for Pavement Component Elements
//! @ingroup GROUP_RoadRailPhysical
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE PavementComponentHandler : TravelwayStructureComponentElementHandler
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_PavementComponent, PavementComponent, PavementComponentHandler, TravelwayStructureComponentElementHandler, ROADRAILPHYSICAL_EXPORT)
}; // PavementComponentHandler

END_BENTLEY_ROADRAILPHYSICAL_NAMESPACE