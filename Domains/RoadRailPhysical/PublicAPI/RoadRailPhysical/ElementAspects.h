/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/RoadRailPhysical/ElementAspects.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

//__PUBLISH_SECTION_START__
#include "RoadRailPhysicalApi.h"

BEGIN_BENTLEY_ROADRAILPHYSICAL_NAMESPACE

//=======================================================================================
//! UniqueAspect class capturing the Status of Physical Elements
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct StatusAspect : Dgn::DgnElement::UniqueAspect
{
    DEFINE_T_SUPER(Dgn::DgnElement::UniqueAspect)
    friend struct StatusAspectHandler;

public:
    enum class Status : int { Existing = 0, Proposed, Construction };

private:
    Status m_status;

protected:
    StatusAspect() : m_status(Status::Existing) {}
    StatusAspect(Status status) : m_status(status) {}

    virtual Utf8CP _GetECSchemaName() const { return BRRP_SCHEMA_NAME; }
    virtual Utf8CP _GetECClassName() const { return BRRP_CLASS_StatusAspect; }
    virtual Utf8CP _GetSuperECClassName() const override { return T_Super::_GetECClassName(); }
    virtual Dgn::DgnDbStatus _UpdateProperties(Dgn::DgnElementCR el, BeSQLite::EC::ECCrudWriteToken const* writeToken) override;
    virtual Dgn::DgnDbStatus _LoadProperties(Dgn::DgnElementCR el) override;

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(StatusAspect)
    ROADRAILPHYSICAL_EXPORT static StatusAspectPtr Create(Status status);
    ROADRAILPHYSICAL_EXPORT static StatusAspectCP Get(Dgn::PhysicalElementCR el);
    ROADRAILPHYSICAL_EXPORT static StatusAspectP GetP(Dgn::PhysicalElementR el, StatusAspectR aspect);
    ROADRAILPHYSICAL_EXPORT static void Set(Dgn::PhysicalElementR el, StatusAspectR aspect);

    Status GetStatus() const { return m_status; }
    void SetStatus(Status status) { m_status = status; }
}; // StatusAspect

//=======================================================================================
//! Handler for StatusAspect
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE StatusAspectHandler : Dgn::dgn_AspectHandler::Aspect
{
DOMAINHANDLER_DECLARE_MEMBERS(BRRP_CLASS_StatusAspect, StatusAspectHandler, Dgn::dgn_AspectHandler::Aspect, ROADRAILPHYSICAL_EXPORT)

protected:
    RefCountedPtr<Dgn::DgnElement::Aspect> _CreateInstance() override { return new StatusAspect(); }
}; // StatusAspectHandler

END_BENTLEY_ROADRAILPHYSICAL_NAMESPACE