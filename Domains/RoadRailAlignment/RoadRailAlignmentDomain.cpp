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
DgnDbStatus RoadRailAlignmentDomain::SetUpModelHierarchy(SubjectCR subject, Utf8CP partitionName)
    {
    DgnDbStatus status;

    auto alignmentPartitionPtr = SpatialLocationPartition::Create(subject, partitionName);
    if (alignmentPartitionPtr->Insert(&status).IsNull())
        return status;

    auto alignmentModelPtr = AlignmentModel::Create(AlignmentModel::CreateParams(subject.GetDgnDb(), alignmentPartitionPtr->GetElementId()));


    if (DgnDbStatus::Success != (status = alignmentModelPtr->Insert()))
        return status;

    auto horizontalPartitionCPtr = HorizontalAlignmentsPortion::InsertPortion(*alignmentModelPtr);
    if (horizontalPartitionCPtr.IsNull())
        return DgnDbStatus::BadModel;

    auto horizontalBreakDownModelPtr = HorizontalAlignmentModel::Create(HorizontalAlignmentModel::CreateParams(subject.GetDgnDb(), horizontalPartitionCPtr->GetElementId()));

    if (DgnDbStatus::Success != (status = horizontalBreakDownModelPtr->Insert()))
        return status;

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void RoadRailAlignmentDomain::_OnSchemaImported(DgnDbR dgndb) const
    {
    AlignmentCategoryModel::SetUp(dgndb);

    DgnDbStatus status = SetUpModelHierarchy(*dgndb.Elements().GetRootSubject(), "Alignments");
    if (DgnDbStatus::Success != status)
        {
        BeAssert(false);
        }

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
