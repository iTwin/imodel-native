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
    ROADRAILPHYSICAL_EXPORT static Dgn::DgnViewId SetUpDefaultViews(Dgn::SubjectCR, Dgn::PhysicalModelR physicalNetworkModel, bvector<Dgn::DgnCategoryId> const* additionalCategoriesForSelector = nullptr);

    //! The code name of the PathwayDesignCriteria element
    static Utf8CP GetPathwayDesignCriteriaCodeName() { return "Design Criteria"; }

private:
    WCharCP _GetSchemaRelativePath() const override { return BRRP_SCHEMA_PATH; }
}; // RoadRailPhysicalDomain

//=======================================================================================
//! Entry-point element leading to the physical modeling of a network of Corridors, 
//! primarily designed for one mode of transportation.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct TransportationNetwork : GeometricElementWrapper<Dgn::PhysicalElement>
{
    DGNELEMENTWRAPPER_DECLARE_MEMBERS(GeometricElementWrapper, Dgn::PhysicalElement)

protected:
    //! @private
    explicit TransportationNetwork(Dgn::PhysicalElementCR element) : T_Super(element) {}
    explicit TransportationNetwork(Dgn::PhysicalElementR element) : T_Super(element) {}

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(TransportationNetwork)

    //! @private
    ROADRAILPHYSICAL_EXPORT static TransportationNetworkCPtr Get(Dgn::DgnDbR db, Dgn::DgnElementId id) { return new TransportationNetwork(*db.Elements().Get<Dgn::PhysicalElement>(id)); }

    Dgn::PhysicalModelPtr GetNetworkModel() const { return get()->GetSub<Dgn::PhysicalModel>(); }
}; // TransportationNetwork

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
    ROADRAILPHYSICAL_EXPORT static Dgn::PhysicalModelPtr QueryRoadNetworkModel(Dgn::SubjectCR parentSubject, Utf8CP physicalPartitionName, Utf8StringCR roadNetworkName);
    ROADRAILPHYSICAL_EXPORT static Dgn::PhysicalModelPtr QueryRailNetworkModel(Dgn::SubjectCR parentSubject, Utf8CP physicalPartitionName, Utf8StringCR railNetworkName);

    //! Query for the Parent Subject of a Physical Model
    //! @param[in] model The PhysicalModel who's parent subject is being queried for.
    //! @return The Subject of the \p model
    ROADRAILPHYSICAL_EXPORT static Dgn::SubjectCPtr GetParentSubject(Dgn::PhysicalModelCR model);

    //! @private
    ROADRAILPHYSICAL_EXPORT static Dgn::PhysicalPartitionCPtr CreateAndInsertPhysicalPartitionAndModel(Dgn::SubjectCR subject, Utf8CP physicalPartitionName);
}; // PhysicalModelUtilities

END_BENTLEY_ROADRAILPHYSICAL_NAMESPACE
