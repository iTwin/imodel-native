/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/Implementation/ProfilesDomain.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ProfilesInternal.h"
#include <Profiles\ProfilesDomain.h>

#include <Profiles\Profile.h>
#include <Profiles\SinglePerimeterProfile.h>
#include <Profiles\CompositeProfile.h>
#include <Profiles\CustomCenterLineProfile.h>
#include <Profiles\CustomCompositeProfile.h>
#include <Profiles\CenteredProfile.h>
#include <Profiles\DerivedProfile.h>
#include <Profiles\AsymmetricIShapeProfile.h>
#include <Profiles\BentPlateProfile.h>
#include <Profiles\CapsuleProfile.h>
#include <Profiles\CenterLineCShapeProfile.h>
#include <Profiles\CenterLineLShapeProfile.h>
#include <Profiles\CenterLineZShapeProfile.h>
#include <Profiles\CShapeProfile.h>
#include <Profiles\CustomShapeProfile.h>
#include <Profiles\DoubleCProfile.h>
#include <Profiles\DoubleLProfile.h>
#include <Profiles\EllipseProfile.h>
#include <Profiles\HollowEllipseProfile.h>
#include <Profiles\HollowRectangleProfile.h>
#include <Profiles\IShapeProfile.h>
#include <Profiles\LShapeProfile.h>
#include <Profiles\RectangleProfile.h>
#include <Profiles\RegularPolygonProfile.h>
#include <Profiles\RoundedRectangleProfile.h>
#include <Profiles\StandardProfileAspect.h>
#include <Profiles\TrapeziumProfile.h>
#include <Profiles\TShapeProfile.h>
#include <Profiles\TTShapeProfile.h>
#include <Profiles\ZShapeProfile.h>

BEGIN_BENTLEY_PROFILES_NAMESPACE

DOMAIN_DEFINE_MEMBERS(ProfilesDomain)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
ProfilesDomain::ProfilesDomain() : DgnDomain(PRF_SCHEMA_NAME, "Bentley Profiles Domain", 1)
    {
    RegisterHandler(StandardProfileAspectHandler::GetHandler());
    RegisterHandler(ProfileHandler::GetHandler());
    RegisterHandler(CompositeProfileHandler::GetHandler());
    RegisterHandler(DoubleLShapeProfileHandler::GetHandler());
    RegisterHandler(DoubleCProfileHandler::GetHandler());
    RegisterHandler(CustomCompositeProfileHandler::GetHandler());
    RegisterHandler(SinglePerimeterProfileHandler::GetHandler());
    RegisterHandler(CustomShapeProfileHandler::GetHandler());
    RegisterHandler(CustomCenterLineProfileHandler::GetHandler());
    RegisterHandler(DerivedProfileHandler::GetHandler());
    RegisterHandler(CenteredProfileHandler::GetHandler());
    RegisterHandler(AsymmetricIShapeProfileHandler::GetHandler());
    RegisterHandler(BentPlateProfileHandler::GetHandler());
    RegisterHandler(CapsuleProfileHandler::GetHandler());
    RegisterHandler(CenterLineCShapeProfileHandler::GetHandler());
    RegisterHandler(CenterLineLShapeProfileHandler::GetHandler());
    RegisterHandler(CenterLineZShapeProfileHandler::GetHandler());
    RegisterHandler(CShapeProfileHandler::GetHandler());
    RegisterHandler(EllipseProfileHandler::GetHandler());
    RegisterHandler(HollowEllipseProfileHandler::GetHandler());
    RegisterHandler(HollowRectangleProfileHandler::GetHandler());
    RegisterHandler(IShapeProfileHandler::GetHandler());
    RegisterHandler(LShapeProfileHandler::GetHandler());
    RegisterHandler(RectangleProfileHandler::GetHandler());
    RegisterHandler(RegularPolygonProfileHandler::GetHandler());
    RegisterHandler(RoundedRectangleProfileHandler::GetHandler());
    RegisterHandler(TrapeziumProfileHandler::GetHandler());
    RegisterHandler(TShapeProfileHandler::GetHandler());
    RegisterHandler(TTShapeProfileHandler::GetHandler());
    RegisterHandler(ZShapeProfileHandler::GetHandler());
    }

END_BENTLEY_PROFILES_NAMESPACE
