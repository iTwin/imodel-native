/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/PublicAPI/Handlers.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "ProfilesDefinitions.h"

#include "ProfileMixins.h"
#include "DerivedProfile.h"
#include "AsymmetricIShapeProfile.h"
#include "BentPlateProfile.h"
#include "CapsuleProfile.h"
#include "CenteredProfile.h"
#include "CenterLineCShapeProfile.h"
#include "CenterLineLShapeProfile.h"
#include "CenterLineZShapeProfile.h"
#include "CompositeProfile.h"
#include "CShapeProfile.h"
#include "CustomCenterLineProfile.h"
#include "CustomCompositeProfile.h"
#include "CustomShapeProfile.h"
#include "DoubleCProfile.h"
#include "DoubleLProfile.h"
#include "EllipseProfile.h"
#include "HollowEllipseProfile.h"
#include "HollowRectangleProfile.h"
#include "IShapeProfile.h"
#include "LShapeProfile.h"
#include "Profile.h"
#include "RectangleProfile.h"
#include "RegularPolygonProfile.h"
#include "RoundedRectangleProfile.h"
#include "SinglePerimeterProfile.h"
#include "StandardProfileAspect.h"
#include "TrapeziumProfile.h"
#include "TShapeProfile.h"
#include "TTShapeProfile.h"
#include "ZShapeProfile.h"

BEGIN_BENTLEY_PROFILES_NAMESPACE

//=======================================================================================
//! Handler for StandardProfileAspect class
//! @ingroup GROUP_Profiles
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE StandardProfileAspectHandler : Dgn::dgn_AspectHandler::Aspect
{
DOMAINHANDLER_DECLARE_MEMBERS(PRF_CLASS_StandardProfileAspect, StandardProfileAspectHandler, Dgn::dgn_AspectHandler::Aspect, PROFILES_EXPORT);
}; // StandardProfileAspectHandler

//=======================================================================================
//! Handler for Profile class
//! @ingroup GROUP_Profiles
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ProfileHandler : Dgn::dgn_ElementHandler::Definition
{
ELEMENTHANDLER_DECLARE_MEMBERS(PRF_CLASS_Profile, Profile, ProfileHandler, Dgn::dgn_ElementHandler::Definition, PROFILES_EXPORT)
}; // ProfileHandler

//=======================================================================================
//! Handler for SinglePerimeterProfile class
//! @ingroup GROUP_Profiles
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE SinglePerimeterProfileHandler : ProfileHandler
{
ELEMENTHANDLER_DECLARE_MEMBERS(PRF_CLASS_SinglePerimeterProfile, SinglePerimeterProfile, SinglePerimeterProfileHandler, ProfileHandler, PROFILES_EXPORT)
}; // SinglePerimeterProfileHandler

//=======================================================================================
//! Handler for CompositeProfile class
//! @ingroup GROUP_Profiles
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE CompositeProfileHandler : ProfileHandler
{
ELEMENTHANDLER_DECLARE_MEMBERS(PRF_CLASS_CompositeProfile, CompositeProfile, CompositeProfileHandler, ProfileHandler, PROFILES_EXPORT)
}; // CompositeProfileHandler

//=======================================================================================
//! Handler for CustomCenterLineProfile class
//! @ingroup GROUP_Profiles
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE CustomCenterLineProfileHandler : SinglePerimeterProfileHandler
{
ELEMENTHANDLER_DECLARE_MEMBERS(PRF_CLASS_CustomCenterLineProfile, CustomCenterLineProfile, CustomCenterLineProfileHandler, SinglePerimeterProfileHandler, PROFILES_EXPORT)
}; // CustomCenterLineProfileHandler

//=======================================================================================
//! Handler for CustomShapeProfile class
//! @ingroup GROUP_Profiles
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE CustomShapeProfileHandler : SinglePerimeterProfileHandler
{
ELEMENTHANDLER_DECLARE_MEMBERS(PRF_CLASS_CustomShapeProfile, CustomShapeProfile, CustomShapeProfileHandler, SinglePerimeterProfileHandler, PROFILES_EXPORT)
}; // CustomShapeProfileHandler

//=======================================================================================
//! Handler for DerivedProfile class
//! @ingroup GROUP_Profiles
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE DerivedProfileHandler : SinglePerimeterProfileHandler
{
ELEMENTHANDLER_DECLARE_MEMBERS(PRF_CLASS_DerivedProfile, DerivedProfile, DerivedProfileHandler, SinglePerimeterProfileHandler, PROFILES_EXPORT)
}; // DerivedProfileHandler

//=======================================================================================
//! Handler for CenteredProfile class
//! @ingroup GROUP_Profiles
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE CenteredProfileHandler : SinglePerimeterProfileHandler
{
ELEMENTHANDLER_DECLARE_MEMBERS(PRF_CLASS_CenteredProfile, CenteredProfile, CenteredProfileHandler, SinglePerimeterProfileHandler, PROFILES_EXPORT)
}; // CenteredProfileHandler

//=======================================================================================
//! Handler for AsymmetricIShapeProfile class
//! @ingroup GROUP_Profiles
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE AsymmetricIShapeProfileHandler : CenteredProfileHandler
{
ELEMENTHANDLER_DECLARE_MEMBERS(PRF_CLASS_AsymmetricIShapeProfile, AsymmetricIShapeProfile, AsymmetricIShapeProfileHandler, CenteredProfileHandler, PROFILES_EXPORT)
}; // AsymmetricIShapeProfileHandler

//=======================================================================================
//! Handler for BentPlateProfile class
//! @ingroup GROUP_Profiles
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE BentPlateProfileHandler : CenteredProfileHandler
{
ELEMENTHANDLER_DECLARE_MEMBERS(PRF_CLASS_BentPlateProfile, BentPlateProfile, BentPlateProfileHandler, CenteredProfileHandler, PROFILES_EXPORT)
}; // BentPlateProfileHandler

//=======================================================================================
//! Handler for CShapeProfile class
//! @ingroup GROUP_Profiles
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE CShapeProfileHandler : CenteredProfileHandler
{
ELEMENTHANDLER_DECLARE_MEMBERS(PRF_CLASS_CShapeProfile, CShapeProfile, CShapeProfileHandler, CenteredProfileHandler, PROFILES_EXPORT)
}; // CShapeProfileHandler

//=======================================================================================
//! Handler for CapsuleProfile class
//! @ingroup GROUP_Profiles
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE CapsuleProfileHandler : CenteredProfileHandler
{
ELEMENTHANDLER_DECLARE_MEMBERS(PRF_CLASS_CapsuleProfile, CapsuleProfile, CapsuleProfileHandler, CenteredProfileHandler, PROFILES_EXPORT)
}; // CapsuleProfileHandler

//=======================================================================================
//! Handler for CenterLineCShapeProfile class
//! @ingroup GROUP_Profiles
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE CenterLineCShapeProfileHandler : CenteredProfileHandler
{
ELEMENTHANDLER_DECLARE_MEMBERS(PRF_CLASS_CenterLineCShapeProfile, CenterLineCShapeProfile, CenterLineCShapeProfileHandler, CenteredProfileHandler, PROFILES_EXPORT)
}; // CenterLineCShapeProfileHandler

//=======================================================================================
//! Handler for CenterLineLShapeProfile class
//! @ingroup GROUP_Profiles
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE CenterLineLShapeProfileHandler : CenteredProfileHandler
{
ELEMENTHANDLER_DECLARE_MEMBERS(PRF_CLASS_CenterLineLShapeProfile, CenterLineLShapeProfile, CenterLineLShapeProfileHandler, CenteredProfileHandler, PROFILES_EXPORT)
}; // CenterLineLShapeProfileHandler

//=======================================================================================
//! Handler for CenterLineZShapeProfile class
//! @ingroup GROUP_Profiles
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE CenterLineZShapeProfileHandler : CenteredProfileHandler
{
ELEMENTHANDLER_DECLARE_MEMBERS(PRF_CLASS_CenterLineZShapeProfile, CenterLineZShapeProfile, CenterLineZShapeProfileHandler, CenteredProfileHandler, PROFILES_EXPORT)
}; // CenterLineZShapeProfileHandler

//=======================================================================================
//! Handler for CustomCompositeProfile class
//! @ingroup GROUP_Profiles
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE CustomCompositeProfileHandler : CompositeProfileHandler
{
ELEMENTHANDLER_DECLARE_MEMBERS(PRF_CLASS_CustomCompositeProfile, CustomCompositeProfile, CustomCompositeProfileHandler, CompositeProfileHandler, PROFILES_EXPORT)
}; // CustomCompositeProfileHandler

//=======================================================================================
//! Handler for DoubleCProfile class
//! @ingroup GROUP_Profiles
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE DoubleCProfileHandler : CompositeProfileHandler
{
ELEMENTHANDLER_DECLARE_MEMBERS(PRF_CLASS_DoubleCProfile, DoubleCProfile, DoubleCProfileHandler, CompositeProfileHandler, PROFILES_EXPORT)
}; // DoubleCProfileHandler

//=======================================================================================
//! Handler for DoubleLProfile class
//! @ingroup GROUP_Profiles
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE DoubleLProfileHandler : CompositeProfileHandler
{
ELEMENTHANDLER_DECLARE_MEMBERS(PRF_CLASS_DoubleLProfile, DoubleLProfile, DoubleLProfileHandler, CompositeProfileHandler, PROFILES_EXPORT)
}; // DoubleLProfileHandler

//=======================================================================================
//! Handler for EllipseProfile class
//! @ingroup GROUP_Profiles
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE EllipseProfileHandler : CenteredProfileHandler
{
ELEMENTHANDLER_DECLARE_MEMBERS(PRF_CLASS_EllipseProfile, EllipseProfile, EllipseProfileHandler, CenteredProfileHandler, PROFILES_EXPORT)
}; // EllipseProfileHandler

//=======================================================================================
//! Handler for HollowEllipseProfile class
//! @ingroup GROUP_Profiles
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE HollowEllipseProfileHandler : CenteredProfileHandler
{
ELEMENTHANDLER_DECLARE_MEMBERS(PRF_CLASS_HollowEllipseProfile, HollowEllipseProfile, HollowEllipseProfileHandler, CenteredProfileHandler, PROFILES_EXPORT)
}; // HollowEllipseProfileHandler

//=======================================================================================
//! Handler for HollowRectangleProfile class
//! @ingroup GROUP_Profiles
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE HollowRectangleProfileHandler : CenteredProfileHandler
{
ELEMENTHANDLER_DECLARE_MEMBERS(PRF_CLASS_HollowRectangleProfile, HollowRectangleProfile, HollowRectangleProfileHandler, CenteredProfileHandler, PROFILES_EXPORT)
}; // HollowRectangleProfileHandler

//=======================================================================================
//! Handler for IShapeProfile class
//! @ingroup GROUP_Profiles
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE IShapeProfileHandler : CenteredProfileHandler
{
ELEMENTHANDLER_DECLARE_MEMBERS(PRF_CLASS_IShapeProfile, IShapeProfile, IShapeProfileHandler, CenteredProfileHandler, PROFILES_EXPORT)
}; // IShapeProfileHandler

//=======================================================================================
//! Handler for LShapeProfile class
//! @ingroup GROUP_Profiles
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE LShapeProfileHandler : CenteredProfileHandler
{
ELEMENTHANDLER_DECLARE_MEMBERS(PRF_CLASS_LShapeProfile, LShapeProfile, LShapeProfileHandler, CenteredProfileHandler, PROFILES_EXPORT)
}; // LShapeProfileHandler

//=======================================================================================
//! Handler for RectangleProfile class
//! @ingroup GROUP_Profiles
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RectangleProfileHandler : CenteredProfileHandler
{
ELEMENTHANDLER_DECLARE_MEMBERS(PRF_CLASS_RectangleProfile, RectangleProfile, RectangleProfileHandler, CenteredProfileHandler, PROFILES_EXPORT)
}; // RectangleProfileHandler

//=======================================================================================
//! Handler for RegularPolygonProfile class
//! @ingroup GROUP_Profiles
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RegularPolygonProfileHandler : CenteredProfileHandler
{
ELEMENTHANDLER_DECLARE_MEMBERS(PRF_CLASS_RegularPolygonProfile, RegularPolygonProfile, RegularPolygonProfileHandler, CenteredProfileHandler, PROFILES_EXPORT)
}; // RegularPolygonProfileHandler

//=======================================================================================
//! Handler for RoundedRectangleProfile class
//! @ingroup GROUP_Profiles
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RoundedRectangleProfileHandler : CenteredProfileHandler
{
ELEMENTHANDLER_DECLARE_MEMBERS(PRF_CLASS_RoundedRectangleProfile, RoundedRectangleProfile, RoundedRectangleProfileHandler, CenteredProfileHandler, PROFILES_EXPORT)
}; // RoundedRectangleProfileHandler
                                                    
//=======================================================================================
//! Handler for TShapeProfile class
//! @ingroup GROUP_Profiles
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE TShapeProfileHandler : CenteredProfileHandler
{
ELEMENTHANDLER_DECLARE_MEMBERS(PRF_CLASS_TShapeProfile, TShapeProfile, TShapeProfileHandler, CenteredProfileHandler, PROFILES_EXPORT)
}; // TShapeProfileHandler

//=======================================================================================
//! Handler for TTShapeProfile class
//! @ingroup GROUP_Profiles
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE TTShapeProfileHandler : CenteredProfileHandler
{
ELEMENTHANDLER_DECLARE_MEMBERS(PRF_CLASS_TTShapeProfile, TTShapeProfile, TTShapeProfileHandler, CenteredProfileHandler, PROFILES_EXPORT)
}; // TTShapeProfileHandler

//=======================================================================================
//! Handler for TrapeziumProfile class
//! @ingroup GROUP_Profiles
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE TrapeziumProfileHandler : CenteredProfileHandler
{
ELEMENTHANDLER_DECLARE_MEMBERS(PRF_CLASS_TrapeziumProfile, TrapeziumProfile, TrapeziumProfileHandler, CenteredProfileHandler, PROFILES_EXPORT)
}; // TrapeziumProfileHandler

//=======================================================================================
//! Handler for ZShapeProfile class
//! @ingroup GROUP_Profiles
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ZShapeProfileHandler : CenteredProfileHandler
{
ELEMENTHANDLER_DECLARE_MEMBERS(PRF_CLASS_ZShapeProfile, ZShapeProfile, ZShapeProfileHandler, CenteredProfileHandler, PROFILES_EXPORT)
}; // ZShapeProfileHandler

END_BENTLEY_PROFILES_NAMESPACE
