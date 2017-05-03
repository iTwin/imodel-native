/*--------------------------------------------------------------------------------------+
|
|     $Source: RoadRailAlignmentDomain.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <RoadRailAlignmentInternal.h>

DOMAIN_DEFINE_MEMBERS(RoadRailAlignmentDomain)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RoadRailAlignmentDomain::RoadRailAlignmentDomain() : DgnDomain(BRRA_SCHEMA_NAME, "Bentley RoadRailAlignment Domain", 1)
    {
    RegisterHandler(AlignmentCategoryModelHandler::GetHandler());
    RegisterHandler(AlignmentModelHandler::GetHandler());
    RegisterHandler(AlignmentHandler::GetHandler());
    RegisterHandler(HorizontalAlignmentModelHandler::GetHandler());
    RegisterHandler(HorizontalAlignmentsPortionHandler::GetHandler());
    RegisterHandler(HorizontalAlignmentHandler::GetHandler());    
    RegisterHandler(AlignmentReferentElementHandler::GetHandler());
    RegisterHandler(AlignmentStationHandler::GetHandler());
    RegisterHandler(VerticalAlignmentModelHandler::GetHandler());
    RegisterHandler(VerticalAlignmentHandler::GetHandler());    
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus RoadRailAlignmentDomain::SetUpModelHierarchy(Dgn::DgnDbR db)
    {
    DgnDbStatus status;

    auto alignmentPartitionPtr = SpatialLocationPartition::Create(*db.Elements().GetRootSubject(), GetPartitionName());
    if (alignmentPartitionPtr->Insert(&status).IsNull())
        return status;

    AlignmentCategoryModel::SetUp(db);

    auto alignmentModelPtr = AlignmentModel::Create(AlignmentModel::CreateParams(db, alignmentPartitionPtr->GetElementId()));

    if (DgnDbStatus::Success != (status = alignmentModelPtr->Insert()))
        return status;

    auto horizontalPartitionCPtr = HorizontalAlignmentsPortion::InsertPortion(*alignmentModelPtr);
    if (horizontalPartitionCPtr.IsNull())
        return DgnDbStatus::BadModel;

    auto horizontalBreakDownModelPtr = HorizontalAlignmentModel::Create(HorizontalAlignmentModel::CreateParams(db, horizontalPartitionCPtr->GetElementId()));

    if (DgnDbStatus::Success != (status = horizontalBreakDownModelPtr->Insert()))
        return status;

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void RoadRailAlignmentDomain::_OnSchemaImported(DgnDbR dgndb) const
    {
    SetUpModelHierarchy(dgndb);

    auto codeSpec = CodeSpec::Create(dgndb, BRRA_CODESPEC_Alignment);
    BeAssert(codeSpec.IsValid());
    if (codeSpec.IsValid())
        {
        codeSpec->Insert();
        BeAssert(codeSpec->GetCodeSpecId().IsValid());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
CodeSpecId RoadRailAlignmentDomain::QueryAlignmentCodeSpecId(DgnDbCR dgndb)
    {
    CodeSpecId codeSpecId = dgndb.CodeSpecs().QueryCodeSpecId(BRRA_CODESPEC_Alignment);
    BeAssert(codeSpecId.IsValid());
    return codeSpecId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCode RoadRailAlignmentDomain::CreateCode(DgnDbR dgndb, Utf8StringCR value)
    {
    return CodeSpec::CreateCode(dgndb, BRRA_CODESPEC_Alignment, value);
    }
