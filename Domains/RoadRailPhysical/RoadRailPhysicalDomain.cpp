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
    RegisterHandler(CrossSectionBreakDownModelHandler::GetHandler());
    RegisterHandler(CrossSectionDefinitionModelHandler::GetHandler());
    RegisterHandler(CrossSectionElementHandler::GetHandler());
    RegisterHandler(RoadCrossSectionHandler::GetHandler());

    RegisterHandler(SegmentRangeElementHandler::GetHandler());
    RegisterHandler(SegmentElementHandler::GetHandler());
    RegisterHandler(RegularSegmentElementHandler::GetHandler());
    RegisterHandler(IntersectionSegmentElementHandler::GetHandler());
    RegisterHandler(TransitionSegmentElementHandler::GetHandler());
    
    RegisterHandler(RoadDesignSpeedHandler::GetHandler());
    
    RegisterHandler(RailRangeHandler::GetHandler());
    RegisterHandler(RoadRangeHandler::GetHandler());
    RegisterHandler(RoadSegmentHandler::GetHandler());
    RegisterHandler(ElevatedRoadSegmentHandler::GetHandler());
    RegisterHandler(RoadIntersectionSegmentHandler::GetHandler());
    RegisterHandler(ElevatedRoadIntersectionSegmentHandler::GetHandler());
    RegisterHandler(RoadTransitionSegmentHandler::GetHandler());

    RegisterHandler(StatusAspectHandler::GetHandler());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void RoadRailPhysicalDomain::_OnSchemaImported(DgnDbR dgndb) const
    {
    DgnCategory roadCategory(DgnCategory::CreateParams(dgndb, BRRP_CATEGORY_Road, DgnCategory::Scope::Any, DgnCategory::Rank::Domain));
    roadCategory.Insert(DgnSubCategory::Appearance());
    BeAssert(roadCategory.GetCategoryId().IsValid());

    DgnCategory trackCategory(DgnCategory::CreateParams(dgndb, BRRP_CATEGORY_Track, DgnCategory::Scope::Any, DgnCategory::Rank::Domain));
    trackCategory.Insert(DgnSubCategory::Appearance());
    BeAssert(trackCategory.GetCategoryId().IsValid());

    auto authorityPtr = NamespaceAuthority::CreateNamespaceAuthority(BRRP_AUTHORITY_RoadCrossSection, dgndb);
    BeAssert(authorityPtr.IsValid());
    if (authorityPtr.IsValid())
        {
        authorityPtr->Insert();
        BeAssert(authorityPtr->GetAuthorityId().IsValid());
        }
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCategoryId RoadRailPhysicalDomain::QueryTrackCategoryId(DgnDbCR dgnDb)
    {
    static Utf8String s_categoryName(BRRP_CATEGORY_Track);
    DgnCategoryId categoryId = DgnCategory::QueryCategoryId(s_categoryName, const_cast<DgnDbR>(dgnDb)); // NEEDSWORK_CONST
    BeAssert(categoryId.IsValid());
    return categoryId;
    }