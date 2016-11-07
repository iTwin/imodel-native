/*--------------------------------------------------------------------------------------+
|
|     $Source: CrossSection.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <RoadRailPhysicalInternal.h>

HANDLER_DEFINE_MEMBERS(CrossSectionBreakDownModelHandler)
HANDLER_DEFINE_MEMBERS(CrossSectionDefinitionModelHandler)
HANDLER_DEFINE_MEMBERS(CrossSectionElementHandler)
HANDLER_DEFINE_MEMBERS(RoadCrossSectionHandler)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
CrossSectionElement::CrossSectionElement(CreateParams const& params):
    T_Super(params)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RoadCrossSection::RoadCrossSection(CreateParams const& params):
    T_Super(params)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnAuthorityId RoadCrossSection::QueryAuthorityId(Dgn::DgnDbCR dgndb)
    {
    DgnAuthorityId authorityId = dgndb.Authorities().QueryAuthorityId(BRRP_AUTHORITY_RoadCrossSection);
    BeAssert(authorityId.IsValid());
    return authorityId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCode RoadCrossSection::CreateCode(Dgn::DgnDbR dgndb, Utf8StringCR value)
    {
    return NamespaceAuthority::CreateCode(BRRP_AUTHORITY_RoadCrossSection, value, dgndb);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RoadCrossSectionPtr RoadCrossSection::Create(CrossSectionDefinitionModelCR model, Utf8StringCR code)
    {
    if (!model.GetModelId().IsValid())
        return nullptr;

    CreateParams createParams(model.GetDgnDb(), model.GetModelId(), QueryClassId(model.GetDgnDb()), CreateCode(model.GetDgnDb(), code));

    return new RoadCrossSection(createParams);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RoadCrossSectionCPtr RoadCrossSection::Insert(CrossSectionBreakDownModelPtr& breakDownModelPtr, DgnDbStatus* stat)
    {
    auto retVal = GetDgnDb().Elements().Insert<RoadCrossSection>(*this, stat);
    if (retVal.IsNull())
        return nullptr;

    breakDownModelPtr = CrossSectionBreakDownModel::Create(DgnModel::CreateParams(GetDgnDb(), CrossSectionBreakDownModel::QueryClassId(GetDgnDb()),
        retVal->GetElementId()));

    DgnDbStatus status;
    if (DgnDbStatus::Success != (status = breakDownModelPtr->Insert()))
        {
        if (stat) *stat = status;
        return nullptr;
        }

    return retVal;
    }