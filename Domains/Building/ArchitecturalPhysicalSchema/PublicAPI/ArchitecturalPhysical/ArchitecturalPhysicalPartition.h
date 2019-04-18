/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#include "PlanningDefinitions.h"

BEGIN_BENTLEY_PLANNING_NAMESPACE

//=======================================================================================
//! A PlanningPartition provides a starting point for a PlanningModel hierarchy
//! @note PlanningPartition elements only reside in the RepositoryModel
//! @ingroup GROUP_Planning
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE PlanningPartition : Dgn::InformationPartitionElement
{
    DGNELEMENT_DECLARE_MEMBERS(BP_CLASS_PlanningPartition, Dgn::InformationPartitionElement);
    friend struct PlanningPartitionHandler;

protected:
    PLANNING_EXPORT Dgn::DgnDbStatus _OnSubModelInsert(Dgn::DgnModelCR model) const override;
    explicit PlanningPartition(CreateParams const& params) : T_Super(params) {}

public:
    //! Create a new PlanningPartition
    //! @param[in] parentSubject The new PlanningPartition will be a child element of this Subject
    //! @param[in] name The name of the new partition which will be used as the CodeValue
    //! @param[in] description Optional description for this PlanningPartition
    //! @see DgnElements::GetRootSubject
    PLANNING_EXPORT static PlanningPartitionPtr Create(Dgn::SubjectCR parentSubject, Utf8CP name, Utf8CP description=nullptr);
    //! Create and insert a new PlanningPartition
    //! @param[in] parentSubject The new PlanningPartition will be a child element of this Subject
    //! @param[in] name The name of the new partition which will be used as the CodeValue
    //! @param[in] description Optional description for this PlanningPartition
    //! @see DgnElements::GetRootSubject
    PLANNING_EXPORT static PlanningPartitionCPtr CreateAndInsert(Dgn::SubjectCR parentSubject, Utf8CP name, Utf8CP description=nullptr);
};

//=================================================================================
//! @ingroup GROUP_Planning
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE PlanningPartitionHandler : Dgn::dgn_ElementHandler::InformationPartition
{
ELEMENTHANDLER_DECLARE_MEMBERS(BP_CLASS_PlanningPartition, PlanningPartition, PlanningPartitionHandler, Dgn::dgn_ElementHandler::InformationPartition, PLANNING_EXPORT)
};

END_BENTLEY_PLANNING_NAMESPACE
