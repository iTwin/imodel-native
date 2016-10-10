/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/RoadRailPhysical/LinearReferencing.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

//__PUBLISH_SECTION_START__
#include "RoadRailPhysicalApi.h"

BEGIN_BENTLEY_ROADRAILPHYSICAL_NAMESPACE

//=======================================================================================
//! Helper interface to be implemented by linearly located attribution and elements
//! only accepting and exposing one from-to linearly referenced location.
//! Concrete subclasses are expected to explicitely add the single aspect on their
//! constructor as follows:
//! _AddLinearlyReferencedLocation(*_GetUnpersistedFromToLocation());
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct ILinearlyLocatedSingleFromTo
{
private:
    mutable LinearReferencing::LinearlyReferencedLocationId m_fromToLocationAspectId;
    LinearReferencing::LinearlyReferencedFromToLocationPtr m_unpersistedFromToLocationPtr;

    virtual Dgn::DgnElementCR ToElement() const { return *dynamic_cast<Dgn::DgnElementCP>(this); }
    virtual Dgn::DgnElementR ToElementR() { return *dynamic_cast<Dgn::DgnElementP>(this); }
    virtual LinearReferencing::ILinearlyLocatedCR ToLinearlyLocated() const { return *dynamic_cast<LinearReferencing::ILinearlyLocatedCP>(this); }
    virtual LinearReferencing::ILinearlyLocatedR ToLinearlyLocatedR() { return *dynamic_cast<LinearReferencing::ILinearlyLocatedP>(this); }

protected:
    ILinearlyLocatedSingleFromTo() {}

    ILinearlyLocatedSingleFromTo(double fromDistanceAlong, double toDistanceAlong);

    LinearReferencing::LinearlyReferencedFromToLocationPtr _GetUnpersistedFromToLocation() const { return m_unpersistedFromToLocationPtr; }

public:
    ROADRAILPHYSICAL_EXPORT double GetFromDistanceAlong() const;
    ROADRAILPHYSICAL_EXPORT void SetFromDistanceAlong(double newFrom);

    ROADRAILPHYSICAL_EXPORT double GetToDistanceAlong() const;
    ROADRAILPHYSICAL_EXPORT void SetToDistanceAlong(double newFrom);
}; // ILinearlyLocatedSingleFromTo

END_BENTLEY_ROADRAILPHYSICAL_NAMESPACE