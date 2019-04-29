/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "ProfilesPch.h"
#include <Profiles\ProfilesApi.h>
#include <ProfilesInternal\ArbitraryCompositeProfileAspect.h>
#include <ProfilesInternal\StandardCatalogCodeAspect.h>

BEGIN_BENTLEY_PROFILES_NAMESPACE

DOMAIN_DEFINE_MEMBERS (ProfilesDomain)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
ProfilesDomain::ProfilesDomain() : DgnDomain (PRF_SCHEMA_NAME, "Bentley Profiles Domain", 1)
    {
    RegisterHandler (ProfileHandler::GetHandler());
    RegisterHandler (SinglePerimeterProfileHandler::GetHandler());
    RegisterHandler (DerivedProfileHandler::GetHandler());
    RegisterHandler (ArbitraryShapeProfileHandler::GetHandler());
    RegisterHandler (ArbitraryCenterLineProfileHandler::GetHandler());
    RegisterHandler (ParametricProfileHandler::GetHandler());
    RegisterHandler (CShapeProfileHandler::GetHandler());
    RegisterHandler (IShapeProfileHandler::GetHandler());
    RegisterHandler (AsymmetricIShapeProfileHandler::GetHandler());
    RegisterHandler (LShapeProfileHandler::GetHandler());
    RegisterHandler (SchifflerizedLShapeProfileHandler::GetHandler());
    RegisterHandler (TShapeProfileHandler::GetHandler());
    RegisterHandler (TTShapeProfileHandler::GetHandler());
    RegisterHandler (ZShapeProfileHandler::GetHandler());
    RegisterHandler (CenterLineCShapeProfileHandler::GetHandler());
    RegisterHandler (CenterLineLShapeProfileHandler::GetHandler());
    RegisterHandler (CenterLineZShapeProfileHandler::GetHandler());
    RegisterHandler (BentPlateProfileHandler::GetHandler());
    RegisterHandler (RectangleProfileHandler::GetHandler());
    RegisterHandler (RoundedRectangleProfileHandler::GetHandler());
    RegisterHandler (HollowRectangleProfileHandler::GetHandler());
    RegisterHandler (CircleProfileHandler::GetHandler());
    RegisterHandler (HollowCircleProfileHandler::GetHandler());
    RegisterHandler (EllipseProfileHandler::GetHandler());
    RegisterHandler (TrapeziumProfileHandler::GetHandler());
    RegisterHandler (RegularPolygonProfileHandler::GetHandler());
    RegisterHandler (CapsuleProfileHandler::GetHandler());
    RegisterHandler (CompositeProfileHandler::GetHandler());
    RegisterHandler (DoubleCShapeProfileHandler::GetHandler());
    RegisterHandler (DoubleLShapeProfileHandler::GetHandler());
    RegisterHandler (ArbitraryCompositeProfileHandler::GetHandler());
    RegisterHandler (ArbitraryCompositeProfileAspectHandler::GetHandler());
    RegisterHandler (StandardCatalogCodeAspectHandler::GetHandler());
    RegisterHandler (MaterialProfileDefinitionHandler::GetHandler());
    RegisterHandler (MaterialProfileHandler::GetHandler());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void ProfilesDomain::_OnSchemaImported (DgnDb& db) const
    {
    CodeSpecPtr codeSpecPtr = CodeSpec::Create (db, PRF_CODESPEC_StandardCatalogProfile, CodeScopeSpec::CreateRepositoryScope());

    CodeFragmentSpecList& fragments = codeSpecPtr->GetFragmentSpecsR();
    fragments.push_back (CodeFragmentSpec::FromPropertyValue ("Manufacturer"));
    fragments.push_back (CodeFragmentSpec::FromFixedString (" "));
    fragments.push_back (CodeFragmentSpec::FromPropertyValue ("StandardsOrganization"));
    fragments.push_back (CodeFragmentSpec::FromFixedString (" "));
    fragments.push_back (CodeFragmentSpec::FromPropertyValue ("Revision"));
    fragments.push_back (CodeFragmentSpec::FromFixedString (" "));
    fragments.push_back (CodeFragmentSpec::FromPropertyValue ("Designation"));

    BeAssert (codeSpecPtr->Insert() == DgnDbStatus::Success);
    }

END_BENTLEY_PROFILES_NAMESPACE
