/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#include "RoadRailPhysical.h"
#include "Corridor.h"

BEGIN_BENTLEY_ROADRAILPHYSICAL_NAMESPACE

//=======================================================================================
//! Standardized design-speed definition in the context of a Subject.
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE DesignSpeedDefinition : Dgn::DefinitionElement
{
DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_DesignSpeedDefinition, Dgn::DefinitionElement);
friend struct DesignSpeedDefinitionHandler;

public:
    enum class UnitSystem : int16_t { SI = 0, Imperial = 1 };

protected:
    //! @private
    ROADRAILPHYSICAL_EXPORT explicit DesignSpeedDefinition(CreateParams const& params);
    //! @private
    ROADRAILPHYSICAL_EXPORT explicit DesignSpeedDefinition(CreateParams const& params, double designSpeed, UnitSystem unitSystem);

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(DesignSpeedDefinition)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_METHODS(DesignSpeedDefinition)

    ROADRAILPHYSICAL_EXPORT static Dgn::DgnCode CreateCode(Dgn::DefinitionModelCR scope, double speed, UnitSystem unitSystem);
    ROADRAILPHYSICAL_EXPORT static DesignSpeedDefinitionCPtr QueryByCode(Dgn::DefinitionModelCR model, double speed, UnitSystem unitSystem);

    double GetDesignSpeed() const { return GetPropertyValueDouble("DesignSpeed"); }
    ROADRAILPHYSICAL_EXPORT UnitSystem GetUnitSystem() const;

    ROADRAILPHYSICAL_EXPORT static DesignSpeedDefinitionPtr Create(Dgn::DefinitionModelCR model, double designSpeed, UnitSystem unitSystem);
}; // DesignSpeedDefinition

//=======================================================================================
//! Linearly-located attribution on a Pathway whose value is its design-speed.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE DesignSpeed : LinearReferencing::LinearlyLocatedAttribution, LinearReferencing::ILinearlyLocatedSingleFromTo
{
    DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_DesignSpeed, LinearReferencing::LinearlyLocatedAttribution);
    friend struct DesignSpeedHandler;

protected:
    virtual Dgn::DgnElementCR _ILinearlyLocatedToDgnElement() const override { return *this; }

public:
    struct CreateFromToParams : LinearReferencing::ILinearlyLocatedSingleFromTo::CreateFromToParams
    {
    DEFINE_T_SUPER(LinearReferencing::ILinearlyLocatedSingleFromTo::CreateFromToParams)

    DesignSpeedDefinitionCPtr m_startDesignSpeedDefCPtr, m_endDesignSpeedDefCPtr;
    PathwayDesignCriteriaCPtr m_designCriteriaCPtr;

    CreateFromToParams(PathwayDesignCriteriaCR designCriteria, LinearReferencing::ILinearElementCR linearElement, 
        DesignSpeedDefinitionCR startDesignSpeedDef,
        DesignSpeedDefinitionCR endDesignSpeedDef, double fromDistanceFromStart, double toDistanceFromStart) :
        m_designCriteriaCPtr(&designCriteria),
        m_startDesignSpeedDefCPtr(&startDesignSpeedDef), m_endDesignSpeedDefCPtr(&endDesignSpeedDef),
        T_Super(linearElement, fromDistanceFromStart, toDistanceFromStart)
        {
        }
    }; // CreateFromToParams

protected:
    //! @private
    explicit DesignSpeed(CreateParams const& params);

    //! @private
    explicit DesignSpeed(CreateParams const& params, CreateFromToParams const& fromToParams);    

    static bool ValidateParams(CreateFromToParams const& params)
        {
        return params.m_designCriteriaCPtr.IsValid() && params.m_designCriteriaCPtr->GetSubModelId().IsValid() &&
            params.m_startDesignSpeedDefCPtr.IsValid() && params.m_startDesignSpeedDefCPtr->GetElementId().IsValid() &&
            params.m_endDesignSpeedDefCPtr.IsValid() && params.m_endDesignSpeedDefCPtr->GetElementId().IsValid();
        }

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(DesignSpeed)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_GET_METHODS(DesignSpeed)
    DECLARE_LINEARREFERENCING_LINEARLYLOCATED_SET_METHODS(DesignSpeed)

    //! Returns the id of the DesignSpeedDefinition at the start of the range
    Dgn::DgnElementId GetStartDefinitionId() const { return GetPropertyValueId<Dgn::DgnElementId>(BRRP_PROP_DesignSpeed_StartDefinition); }

    //! Returns the id of the DesignSpeedDefinition at the end of the range
    Dgn::DgnElementId GetEndDefinitionId() const { return GetPropertyValueId<Dgn::DgnElementId>(BRRP_PROP_DesignSpeed_EndDefinition); }

    //! @private
    ROADRAILPHYSICAL_EXPORT void SetStartDefinition(DesignSpeedDefinitionCR designSpeedDef);

    //! @private
    ROADRAILPHYSICAL_EXPORT void SetEndDefinition(DesignSpeedDefinitionCR designSpeedDef);

    //! @private
    ROADRAILPHYSICAL_EXPORT static DesignSpeedPtr Create(CreateFromToParams const& params);
}; // DesignSpeed


//__PUBLISH_SECTION_END__
//=======================================================================================
//! Handler for DesignSpeedDefinition Elements
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE DesignSpeedDefinitionHandler : Dgn::dgn_ElementHandler::Definition
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_DesignSpeedDefinition, DesignSpeedDefinition, DesignSpeedDefinitionHandler, Dgn::dgn_ElementHandler::Definition, ROADRAILPHYSICAL_EXPORT)
}; // DesignSpeedDefinitionHandler

//=======================================================================================
//! Handler for base DesignSpeed Elements
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE DesignSpeedHandler : LinearReferencing::LinearlyLocatedAttributionHandler
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_DesignSpeed, DesignSpeed, DesignSpeedHandler, LinearReferencing::LinearlyLocatedAttributionHandler, ROADRAILPHYSICAL_EXPORT)
}; // DesignSpeedHandler

//__PUBLISH_SECTION_START__
END_BENTLEY_ROADRAILPHYSICAL_NAMESPACE