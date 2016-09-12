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
    RegisterHandler(AlignmentStationHandler::GetHandler());
    RegisterHandler(AlignmentVerticalHandler::GetHandler());    
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void RoadRailAlignmentDomain::_OnSchemaImported(DgnDbR dgndb) const
    {
    DgnCategory alignmentCategory(DgnCategory::CreateParams(dgndb, BRRA_CATEGORY_Alignment, DgnCategory::Scope::Any, DgnCategory::Rank::Domain));
    alignmentCategory.Insert(DgnSubCategory::Appearance());
    BeAssert(alignmentCategory.GetCategoryId().IsValid());

    auto authority = NamespaceAuthority::CreateNamespaceAuthority(BRRA_AUTHORITY_Alignment, dgndb);
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
DgnCategoryId RoadRailAlignmentDomain::QueryAlignmentCategoryId(Dgn::DgnDbCR dgnDb)
    {
    static Utf8String s_categoryName(BRRA_CATEGORY_Alignment);
    DgnCategoryId categoryId = DgnCategory::QueryCategoryId(s_categoryName, const_cast<DgnDbR>(dgnDb)); // NEEDSWORK_CONST
    BeAssert(categoryId.IsValid());
    return categoryId;
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
    return NamespaceAuthority::CreateCode(BRRA_AUTHORITY_Alignment, value, dgndb);
    }
