/*--------------------------------------------------------------------------------------+
|
|     $Source: RoadRailAlignmentDomain.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <RoadRailAlignmentInternal.h>

DOMAIN_DEFINE_MEMBERS(RoadRailAlignmentDomain)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RoadRailAlignmentDomain::RoadRailAlignmentDomain() : DgnDomain(BRRA_SCHEMA_NAME, "Bentley RoadRailAlignment Domain", 1)
    {
    RegisterHandler(AlignmentModelHandler::GetHandler());
    RegisterHandler(AlignmentHandler::GetHandler());
    RegisterHandler(AlignmentHorizontalHandler::GetHandler());
    RegisterHandler(AlignmentReferentElementHandler::GetHandler());
    RegisterHandler(AlignmentStationHandler::GetHandler());
    RegisterHandler(AlignmentVerticalHandler::GetHandler());    
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus RoadRailAlignmentDomain::SetUpModelHierarchy(Dgn::DgnDbR db)
    {
    DgnDbStatus status;

    auto alignmentPartitionPtr = PhysicalPartition::Create(*db.Elements().GetRootSubject(), "Alignments");
    if (alignmentPartitionPtr->Insert(&status).IsNull())
        return status;

    auto& alignmentModelHandlerR = AlignmentModelHandler::GetHandler();
    auto alignmentModelPtr = alignmentModelHandlerR.Create(DgnModel::CreateParams(db, AlignmentModel::QueryClassId(db),
        alignmentPartitionPtr->GetElementId()));

    if (DgnDbStatus::Success != (status = alignmentModelPtr->Insert()))
        return status;

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void RoadRailAlignmentDomain::_OnSchemaImported(DgnDbR dgndb) const
    {
    AlignmentCategory::InsertDomainCategories(dgndb);

    auto authority = DatabaseScopeAuthority::Create(BRRA_AUTHORITY_Alignment, dgndb);
    BeAssert(authority.IsValid());
    if (authority.IsValid())
        {
        authority->Insert();
        BeAssert(authority->GetAuthorityId().IsValid());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnAuthorityId RoadRailAlignmentDomain::QueryAlignmentAuthorityId(DgnDbCR dgndb)
    {
    DgnAuthorityId authorityId = dgndb.Authorities().QueryAuthorityId(BRRA_AUTHORITY_Alignment);
    BeAssert(authorityId.IsValid());
    return authorityId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCode RoadRailAlignmentDomain::CreateCode(DgnDbR dgndb, Utf8StringCR value)
    {
    return DatabaseScopeAuthority::CreateCode(BRRA_AUTHORITY_Alignment, dgndb, value);
    }
