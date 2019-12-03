/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "RoadRailPhysical.h"
#include "RoadRailPhysicalDomain.h"

//__PUBLISH_SECTION_START__
BEGIN_BENTLEY_ROADPHYSICAL_NAMESPACE

//=======================================================================================
//! The DgnDomain for the RoadPhysical schema.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct RoadPhysicalDomain
{
private:
    //! @private
    RoadPhysicalDomain() {}

public:
    //! @private
    ROADRAILPHYSICAL_EXPORT static Dgn::DgnDbStatus SetUpDefinitionPartition(Dgn::SubjectCR subject);

    //! The name of the RoadwayStandards Partition
    static Utf8CP GetRoadwayStandardsPartitionName() { return "Roadway Standards"; }

    static WCharCP GetSchemaRelativePath() { return BRDP_SCHEMA_PATH; }
}; // RoadPhysicalDomain

//=======================================================================================
//! Entry-point element leading to the physical modeling of a Road network.
//! @ingroup GROUP_RoadRoadPhysical
//=======================================================================================
struct RoadNetwork : RoadRailPhysical::TransportationNetwork
{
    DGNELEMENTWRAPPER_DECLARE_MEMBERS(RoadRailPhysical::TransportationNetwork, Dgn::PhysicalElement)

protected:
    //! @private
    explicit RoadNetwork(Dgn::PhysicalElementCR element) : T_Super(element) {}
    explicit RoadNetwork(Dgn::PhysicalElementR element) : T_Super(element) {}

public:
    DECLARE_ROADPHYSICAL_QUERYCLASS_METHODS(RoadNetwork)

    //! @private
    ROADRAILPHYSICAL_EXPORT static RoadNetworkCPtr Get(Dgn::DgnDbR db, Dgn::DgnElementId id) { return new RoadNetwork(*db.Elements().Get<Dgn::PhysicalElement>(id)); }
    //! @private
    ROADRAILPHYSICAL_EXPORT static RoadNetworkCPtr Query(Dgn::DgnDbR db, Dgn::DgnCodeCR code) { return Get(db, db.Elements().QueryElementIdByCode(code)); }

    //! @private
    ROADRAILPHYSICAL_EXPORT static RoadNetworkCPtr Insert(Dgn::PhysicalModelR parentModel, Utf8StringCR networkName);
    //! @private
    ROADRAILPHYSICAL_EXPORT static Dgn::DgnCode CreateCode(Dgn::PhysicalModelCR scopeModel, Utf8StringCR networkCode);
}; // RoadNetwork

//=======================================================================================
//! Utility class for Roadway Standards definition models.
//=======================================================================================
struct RoadwayStandardsModelUtilities
{
private:
    RoadwayStandardsModelUtilities() {}

public:
    //! @private
    ROADRAILPHYSICAL_EXPORT static Dgn::DefinitionModelCPtr Query(Dgn::SubjectCR parentSubject);
}; // RoadwayStandardsModelUtilities

//__PUBLISH_SECTION_START__
END_BENTLEY_RAILPHYSICAL_NAMESPACE
