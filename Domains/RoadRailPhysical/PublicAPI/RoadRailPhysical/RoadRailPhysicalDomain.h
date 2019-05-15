/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "RoadRailPhysical.h"

//__PUBLISH_SECTION_START__
BEGIN_BENTLEY_ROADRAILPHYSICAL_NAMESPACE

//=======================================================================================
//! The DgnDomain for the RoadRailPhysical schema.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct RoadRailPhysicalDomain : Dgn::DgnDomain
{
DOMAIN_DECLARE_MEMBERS(RoadRailPhysicalDomain, ROADRAILPHYSICAL_EXPORT)

protected:
    //! @private
    void _OnSchemaImported(Dgn::DgnDbR dgndb) const override;

public:
    //! @private
    RoadRailPhysicalDomain();

    //! @private
    ROADRAILPHYSICAL_EXPORT static Dgn::DgnDbStatus SetUpNonPhysicalModelHierarchy(Dgn::SubjectCR subject);

    //! @private
    ROADRAILPHYSICAL_EXPORT static Dgn::DgnDbStatus SetUpPhysicalModelHierarchy(Dgn::SubjectCR physicalSubject, Utf8CP physicalPartitionName, Utf8CP networkName);

    //! @private
    ROADRAILPHYSICAL_EXPORT static Dgn::DgnViewId SetUpDefaultViews(Dgn::SubjectCR, Dgn::PhysicalModelR physicalNetworkModel, bvector<Dgn::DgnCategoryId> const* additionalCategoriesForSelector = nullptr);

    //! The name of the default Physical partition
    static Utf8CP GetDefaultPhysicalPartitionName() { return "Physical"; }

    //! The name of the default Physical network partition
    static Utf8CP GetDefaultPhysicalNetworkName() { return "Road/Rail Physical Network"; }

    //! The name of the RailwayStandards Partition
    static Utf8CP GetRailwayStandardsPartitionName() { return "Railway Standards"; }

    //! The name of the RoadwayStandards Partition
    static Utf8CP GetRoadwayStandardsPartitionName() { return "Roadway Standards"; }    

private:
    WCharCP _GetSchemaRelativePath() const override { return BRRP_SCHEMA_PATH; }
}; // RoadRailPhysicalDomain

//=======================================================================================
//! A long, narrow physical stretch that is designed for one or more modes of transportation 
//! which share a common course. It is typically defined along a main alignment. A Corridor 
//! assembles one or more Pathways with Pathway Separations in between them.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct RoadRailNetwork : Dgn::PhysicalElement
{
    DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_RoadRailNetwork, Dgn::PhysicalElement);
    friend struct RoadRailNetworkHandler;

protected:
    //! @private
    explicit RoadRailNetwork(CreateParams const& params) : T_Super(params) {}

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(RoadRailNetwork)

    //! @private
    ROADRAILPHYSICAL_EXPORT static RoadRailNetworkCPtr Get(Dgn::DgnDbR db, Dgn::DgnElementId id) { return db.Elements().Get<RoadRailNetwork>(id); }

    //! @private
    ROADRAILPHYSICAL_EXPORT static RoadRailNetworkCPtr Insert(Dgn::PhysicalModelR parentModel, Dgn::DgnCodeCR networkCode, Dgn::PhysicalModelPtr& breakDownModel);
    //! @private
    ROADRAILPHYSICAL_EXPORT static Dgn::DgnCode CreateCode(Dgn::PhysicalModelCR scopeModel, Utf8StringCR networkCode);
}; // RoadRailNetwork

//=======================================================================================
//! Helper methods for manipulation of Physical Models
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct PhysicalModelUtilities : NonCopyableClass
{
private:
    PhysicalModelUtilities() {}

public:
    //! Query for physical partition Ids associated with the provided subject
    //! @param[in] parentSubject The parent subject of the physical partitions to query for
    ROADRAILPHYSICAL_EXPORT static Dgn::DgnElementIdSet QueryPhysicalPartitions(Dgn::SubjectCR subject);

    //! Query for the physical model associated with the provided physical partition
    //! @param[in] parentSubject The parent subject of the physical model with \p modelName
    //! @param[in] physicalPartitionName Physical Partition Name associated to the physical model requested
    //! @return The PhysicalModel belonging to the \p parentSubject
    ROADRAILPHYSICAL_EXPORT static Dgn::PhysicalModelPtr QueryPhysicalNetworkModel(Dgn::SubjectCR parentSubject, Utf8CP physicalPartitionName, Utf8CP roadRailNetworkName);

    //! Query for the Parent Subject of a Physical Model
    //! @param[in] model The PhysicalModel who's parent subject is being queried for.
    //! @return The Subject of the \p model
    ROADRAILPHYSICAL_EXPORT static Dgn::SubjectCPtr GetParentSubject(Dgn::PhysicalModelCR model);

    //! @private
    ROADRAILPHYSICAL_EXPORT static Dgn::PhysicalPartitionCPtr CreateAndInsertPhysicalPartitionAndModel(Dgn::SubjectCR subject, Utf8CP physicalPartitionName);
}; // PhysicalModelUtilities

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

//=======================================================================================
//! Utility class for Railway Standards definition models.
//=======================================================================================
struct RailwayStandardsModelUtilities
{
private:
    RailwayStandardsModelUtilities() {}

public:
    //! @private
    ROADRAILPHYSICAL_EXPORT static Dgn::DefinitionModelCPtr Query(Dgn::SubjectCR parentSubject);
}; // RailwayStandardsModelUtilities

//__PUBLISH_SECTION_END__
//=================================================================================
//! ElementHandler for RoadRailNetwork Elements
//! @ingroup GROUP_RoadRailPhysical
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RoadRailNetworkHandler : Dgn::dgn_ElementHandler::Physical
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_RoadRailNetwork, RoadRailNetwork, RoadRailNetworkHandler, Dgn::dgn_ElementHandler::Physical, ROADRAILPHYSICAL_EXPORT)
}; // RoadRailNetworkHandler

   //__PUBLISH_SECTION_START__
END_BENTLEY_ROADRAILPHYSICAL_NAMESPACE
