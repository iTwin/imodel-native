/*--------------------------------------------------------------------------------------+
|
|     $Source: RoadRailPhysicalDomain.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <RoadRailPhysicalInternal.h>

DOMAIN_DEFINE_MEMBERS(RoadRailPhysicalDomain)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RoadRailPhysicalDomain::RoadRailPhysicalDomain() : DgnDomain(BRRP_SCHEMA_NAME, "Bentley RoadRailPhysical Domain", 1)
    {
    RegisterHandler(SegmentRangeElementHandler::GetHandler());
    RegisterHandler(StatusAspectHandler::GetHandler());
    RegisterHandler(RoadRangeHandler::GetHandler());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void RoadRailPhysicalDomain::_OnSchemaImported(DgnDbR dgndb) const
    {
    DgnCategory roadCategory(DgnCategory::CreateParams(dgndb, BRRP_CATEGORY_Road, DgnCategory::Scope::Any, DgnCategory::Rank::Domain));
    roadCategory.Insert(DgnSubCategory::Appearance());
    BeAssert(roadCategory.GetCategoryId().IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCategoryId RoadRailPhysicalDomain::QueryRoadCategoryId(DgnDbCR dgnDb)
    {
    static Utf8String s_categoryName(BRRP_CATEGORY_Road);
    DgnCategoryId categoryId = DgnCategory::QueryCategoryId(s_categoryName, const_cast<DgnDbR>(dgnDb)); // NEEDSWORK_CONST
    BeAssert(categoryId.IsValid());
    return categoryId;
    }