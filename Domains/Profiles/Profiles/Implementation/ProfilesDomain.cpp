/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/Implementation/ProfilesDomain.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ProfilesInternal.h"
#include <Profiles\ProfilesDomain.h>

#include <Profiles\Handlers.h>

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
    RegisterHandler(DoubleLProfileHandler::GetHandler());
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
