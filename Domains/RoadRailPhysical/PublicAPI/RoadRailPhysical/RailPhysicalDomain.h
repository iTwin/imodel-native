/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "RoadRailPhysical.h"
#include "RoadRailPhysicalDomain.h"

//__PUBLISH_SECTION_START__
BEGIN_BENTLEY_RAILPHYSICAL_NAMESPACE

//=======================================================================================
//! The DgnDomain for the RailPhysical schema.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct RailPhysicalDomain
{
private:
    RailPhysicalDomain() {}

public:
    //! @private
    ROADRAILPHYSICAL_EXPORT static Dgn::DgnDbStatus SetUpDefinitionPartition(Dgn::SubjectCR subject);

    //! The name of the RailwayStandards Partition
    static Utf8CP GetRailwayStandardsPartitionName() { return "Railway Standards"; }

    static WCharCP GetSchemaRelativePath() { return BRLP_SCHEMA_PATH; }
}; // RailPhysicalDomain

//=======================================================================================
//! Entry-point element leading to the physical modeling of a Rail network.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct RailNetwork : RoadRailPhysical::TransportationNetwork
{
    DGNELEMENTWRAPPER_DECLARE_MEMBERS(RoadRailPhysical::TransportationNetwork, Dgn::PhysicalElement)

protected:
    //! @private
    explicit RailNetwork(Dgn::PhysicalElementCR element) : T_Super(element) {}
    explicit RailNetwork(Dgn::PhysicalElementR element) : T_Super(element) {}

public:
    DECLARE_RAILPHYSICAL_QUERYCLASS_METHODS(RailNetwork)

    //! @private
    ROADRAILPHYSICAL_EXPORT static RailNetworkCPtr Get(Dgn::DgnDbR db, Dgn::DgnElementId id) { return new RailNetwork(*db.Elements().Get<Dgn::PhysicalElement>(id)); }
    //! @private
    ROADRAILPHYSICAL_EXPORT static RailNetworkCPtr Query(Dgn::DgnDbR db, Dgn::DgnCodeCR code) { return Get(db, db.Elements().QueryElementIdByCode(code)); }

    //! @private
    ROADRAILPHYSICAL_EXPORT static RailNetworkCPtr Insert(Dgn::PhysicalModelR parentModel, Utf8StringCR networkName);
    //! @private
    ROADRAILPHYSICAL_EXPORT static Dgn::DgnCode CreateCode(Dgn::PhysicalModelCR scopeModel, Utf8StringCR networkCode);
}; // RailNetwork

//=======================================================================================
//! Utility class for Railway Standards definition models.
//=======================================================================================
struct RailwayStandardsModelUtilities : NonCopyableClass
{
private:
    RailwayStandardsModelUtilities() {}

public:
    //! @private
    ROADRAILPHYSICAL_EXPORT static Dgn::DefinitionModelCPtr Query(Dgn::SubjectCR parentSubject);
}; // RailwayStandardsModelUtilities

//__PUBLISH_SECTION_START__
END_BENTLEY_RAILPHYSICAL_NAMESPACE
