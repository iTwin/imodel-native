/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/PublicAPI/ProfilesDefinitions.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <Bentley/Bentley.h>
#include <Bentley/WString.h>
#include <ECDb/ECDbApi.h>
#include <DgnPlatform/DgnPlatformApi.h>

#ifdef __PROFILES_BUILD__
#define PROFILES_EXPORT EXPORT_ATTRIBUTE
#else
#define PROFILES_EXPORT IMPORT_ATTRIBUTE
#endif

#define BEGIN_BENTLEY_PROFILES_NAMESPACE        BEGIN_BENTLEY_NAMESPACE namespace Profiles {
#define END_BENTLEY_PROFILES_NAMESPACE          } END_BENTLEY_NAMESPACE
#define USING_NAMESPACE_BENTLEY_PROFILES        using namespace BENTLEY_NAMESPACE_NAME::Profiles;

BEGIN_BENTLEY_PROFILES_NAMESPACE
END_BENTLEY_PROFILES_NAMESPACE

#define PRF_SCHEMA_NAME                             "Profiles"
#define PRF_SCHEMA_FILE                             L"Profiles.ecschema.xml"
#define PRF_SCHEMA_LOCATION                         L"ECSchemas/Domain/"
#define PRF_SCHEMA_PATH                             PRF_SCHEMA_LOCATION PRF_SCHEMA_FILE
#define PRF_SCHEMA(name)                            PRF_SCHEMA_NAME "." name
#define PRF_SCHEMA_CODE(name)                       PRF_SCHEMA_NAME "_" name


//-----------------------------------------------------------------------------------------
// ECClass names
//-----------------------------------------------------------------------------------------

// Models


// Elements
#define PRF_CLASS_AsymmetricIShapeProfile                            "AsymmetricIShapeProfile"
#define PRF_CLASS_BentPlateProfile                                   "BentPlateProfile"
#define PRF_CLASS_CShapeProfile                                      "CShapeProfile"
#define PRF_CLASS_CapsuleProfile                                     "CapsuleProfile"
#define PRF_CLASS_CenterLineCShapeProfile                            "CenterLineCShapeProfile"
#define PRF_CLASS_CenterLineLShapeProfile                            "CenterLineLShapeProfile"
#define PRF_CLASS_CenterLineZShapeProfile                            "CenterLineZShapeProfile"
#define PRF_CLASS_CenteredProfile                                    "CenteredProfile"
#define PRF_CLASS_CompositeProfile                                   "CompositeProfile"
#define PRF_CLASS_CustomCenterLineProfile                            "CustomCenterLineProfile"
#define PRF_CLASS_CustomCompositeProfile                             "CustomCompositeProfile"
#define PRF_CLASS_CustomShapeProfile                                 "CustomShapeProfile"
#define PRF_CLASS_DerivedProfile                                     "DerivedProfile"
#define PRF_CLASS_DoubleCShapeProfile                                "DoubleCShapeProfile"
#define PRF_CLASS_DoubleLShapeProfile                                "DoubleLShapeProfile"
#define PRF_CLASS_CircleProfile                                      "CircleProfile"
#define PRF_CLASS_HollowCircleProfile                                "HollowCircleProfile"
#define PRF_CLASS_HollowRectangleProfile                             "HollowRectangleProfile"
#define PRF_CLASS_IShapeProfile                                      "IShapeProfile"
#define PRF_CLASS_LShapeProfile                                      "LShapeProfile"
#define PRF_CLASS_Profile                                            "Profile"
#define PRF_CLASS_RectangleProfile                                   "RectangleProfile"
#define PRF_CLASS_RegularPolygonProfile                              "RegularPolygonProfile"
#define PRF_CLASS_RoundedRectangleProfile                            "RoundedRectangleProfile"
#define PRF_CLASS_SinglePerimeterProfile                             "SinglePerimeterProfile"
#define PRF_CLASS_TShapeProfile                                      "TShapeProfile"
#define PRF_CLASS_TTShapeProfile                                     "TTShapeProfile"
#define PRF_CLASS_TrapeziumProfile                                   "TrapeziumProfile"
#define PRF_CLASS_ZShapeProfile                                      "ZShapeProfile"


// Aspects
#define PRF_CLASS_StandardProfileAspect                              "StandardProfileAspect"


// Relationships


// Properties
#define PRF_PROP_AsymmetricIShapeProfile_TopWidth                                "TopWidth"
#define PRF_PROP_AsymmetricIShapeProfile_BottomWidth                             "BottomWidth"
#define PRF_PROP_AsymmetricIShapeProfile_Depth                                   "Depth"
#define PRF_PROP_AsymmetricIShapeProfile_TopFlangeThickness                      "TopFlangeThickness"
#define PRF_PROP_AsymmetricIShapeProfile_BottomFlangeThickness                   "BottomFlangeThickness"
#define PRF_PROP_AsymmetricIShapeProfile_WebThickness                            "WebThickness"
#define PRF_PROP_AsymmetricIShapeProfile_TopFlangeFilletRadius                   "TopFlangeFilletRadius"
#define PRF_PROP_AsymmetricIShapeProfile_TopFlangeEdgeRadius                     "TopFlangeEdgeRadius"
#define PRF_PROP_AsymmetricIShapeProfile_TopFlangeSlope                          "TopFlangeSlope"
#define PRF_PROP_AsymmetricIShapeProfile_BottomFlangeFilletRadius                "BottomFlangeFilletRadius"
#define PRF_PROP_AsymmetricIShapeProfile_BottomFlangeEdgeRadius                  "BottomFlangeEdgeRadius"
#define PRF_PROP_AsymmetricIShapeProfile_BottomFlangeSlope                       "BottomFlangeSlope"
#define PRF_PROP_BentPlateProfile_Width                                          "Width"
#define PRF_PROP_BentPlateProfile_BendAngle                                      "BendAngle"
#define PRF_PROP_BentPlateProfile_BendRadius                                     "BendRadius"
#define PRF_PROP_BentPlateProfile_BendOffset                                     "BendOffset"
#define PRF_PROP_CShapeProfile_FlangeWidth                                       "FlangeWidth"
#define PRF_PROP_CShapeProfile_Depth                                             "Depth"
#define PRF_PROP_CShapeProfile_FlangeThickness                                   "FlangeThickness"
#define PRF_PROP_CShapeProfile_WebThickness                                      "WebThickness"
#define PRF_PROP_CShapeProfile_FilletRadius                                      "FilletRadius"
#define PRF_PROP_CShapeProfile_EdgeRadius                                        "EdgeRadius"
#define PRF_PROP_CShapeProfile_FlangeSlope                                       "FlangeSlope"
#define PRF_PROP_CenterLineCShapeProfile_FlangeWidth                             "FlangeWidth"
#define PRF_PROP_CenterLineCShapeProfile_Depth                                   "Depth"
#define PRF_PROP_CenterLineCShapeProfile_FilletRadius                            "FilletRadius"
#define PRF_PROP_CenterLineCShapeProfile_LipLength                               "LipLength"
#define PRF_PROP_CenterLineLShapeProfile_Width                                   "Width"
#define PRF_PROP_CenterLineLShapeProfile_Depth                                   "Depth"
#define PRF_PROP_CenterLineLShapeProfile_FilletRadius                            "FilletRadius"
#define PRF_PROP_CenterLineLShapeProfile_LipLength                               "LipLength"
#define PRF_PROP_CenterLineZShapeProfile_FlangeWidth                             "FlangeWidth"
#define PRF_PROP_CenterLineZShapeProfile_Depth                                   "Depth"
#define PRF_PROP_CenterLineZShapeProfile_FilletRadius                            "FilletRadius"
#define PRF_PROP_CenterLineZShapeProfile_LipLength                               "LipLength"
#define PRF_PROP_CircleProfile_Radius                                            "Radius"
#define PRF_PROP_CustomShapeProfile_OuterCurve                                   "OuterCurve"
#define PRF_PROP_CustomShapeProfile_InnerCurves                                  "InnerCurves"
#define PRF_PROP_DerivedProfile_MirrorProfileAboutYAxis                          "MirrorProfileAboutYAxis"
#define PRF_PROP_DerivedProfile_Offset                                           "Offset"
#define PRF_PROP_DerivedProfile_Rotation                                         "Rotation"
#define PRF_PROP_DerivedProfile_Scale                                            "Scale"
#define PRF_PROP_DerivedProfile_CardinalPoint                                    "CardinalPoint"
#define PRF_PROP_DoubleCShapeProfile_Spacing                                     "Spacing"
#define PRF_PROP_DoubleLShapeProfile_Spacing                                     "Spacing"
#define PRF_PROP_DoubleLShapeProfile_Type                                        "Type"
#define PRF_PROP_HollowCircleProfile_Radius                                      "Radius"
#define PRF_PROP_HollowRectangleProfile_Width                                    "Width"
#define PRF_PROP_HollowRectangleProfile_Depth                                    "Depth"
#define PRF_PROP_HollowRectangleProfile_FilletRadius                             "FilletRadius"
#define PRF_PROP_ICenterLineProfile_CenterLine                                   "CenterLine"
#define PRF_PROP_ICenterLineProfile_WallThickness                                "WallThickness"
#define PRF_PROP_IShapeProfile_Width                                             "Width"
#define PRF_PROP_IShapeProfile_Depth                                             "Depth"
#define PRF_PROP_IShapeProfile_FlangeThickness                                   "FlangeThickness"
#define PRF_PROP_IShapeProfile_WebThickness                                      "WebThickness"
#define PRF_PROP_IShapeProfile_FilletRadius                                      "FilletRadius"
#define PRF_PROP_IShapeProfile_FlangeEdgeRadius                                  "FlangeEdgeRadius"
#define PRF_PROP_IShapeProfile_FlangeSlope                                       "FlangeSlope"
#define PRF_PROP_LShapeProfile_Width                                             "Width"
#define PRF_PROP_LShapeProfile_Depth                                             "Depth"
#define PRF_PROP_LShapeProfile_Thickness                                         "Thickness"
#define PRF_PROP_LShapeProfile_FilletRadius                                      "FilletRadius"
#define PRF_PROP_LShapeProfile_EdgeRadius                                        "EdgeRadius"
#define PRF_PROP_LShapeProfile_HorizontalLegSlope                                "HorizontalLegSlope"
#define PRF_PROP_LShapeProfile_VerticalLegSlope                                  "VerticalLegSlope"
#define PRF_PROP_Profile_Name                                                    "Name"
#define PRF_PROP_Profile_Shape                                                   "Shape"
#define PRF_PROP_RectangleProfile_Width                                          "Width"
#define PRF_PROP_RectangleProfile_Depth                                          "Depth"
#define PRF_PROP_RoundedRectangleProfile_Width                                   "Width"
#define PRF_PROP_RoundedRectangleProfile_Depth                                   "Depth"
#define PRF_PROP_RoundedRectangleProfile_RoundingRadius                          "RoundingRadius"
#define PRF_PROP_StandardProfileAspect_Manufacturer                              "Manufacturer"
#define PRF_PROP_StandardProfileAspect_Revision                                  "Revision"
#define PRF_PROP_StandardProfileAspect_StandardsOrganization                     "StandardsOrganization"
#define PRF_PROP_TShapeProfile_Width                                             "Width"
#define PRF_PROP_TShapeProfile_Depth                                             "Depth"
#define PRF_PROP_TShapeProfile_FlangeThickness                                   "FlangeThickness"
#define PRF_PROP_TShapeProfile_WebThickness                                      "WebThickness"
#define PRF_PROP_TShapeProfile_FilletRadius                                      "FilletRadius"
#define PRF_PROP_TShapeProfile_FlangeEdgeRadius                                  "FlangeEdgeRadius"
#define PRF_PROP_TShapeProfile_FlangeSlope                                       "FlangeSlope"
#define PRF_PROP_TShapeProfile_WebEdgeRadius                                     "WebEdgeRadius"
#define PRF_PROP_TShapeProfile_WebSlope                                          "WebSlope"
#define PRF_PROP_TTShapeProfile_Width                                            "Width"
#define PRF_PROP_TTShapeProfile_Depth                                            "Depth"
#define PRF_PROP_TTShapeProfile_Span                                             "Span"
#define PRF_PROP_TTShapeProfile_FlangeThickness                                  "FlangeThickness"
#define PRF_PROP_TTShapeProfile_WebThickness                                     "WebThickness"
#define PRF_PROP_TTShapeProfile_FilletRadius                                     "FilletRadius"
#define PRF_PROP_TTShapeProfile_FlangeEdgeRadius                                 "FlangeEdgeRadius"
#define PRF_PROP_TTShapeProfile_FlangeSlope                                      "FlangeSlope"
#define PRF_PROP_TTShapeProfile_WebEdgeRadius                                    "WebEdgeRadius"
#define PRF_PROP_TTShapeProfile_WebSlope                                         "WebSlope"
#define PRF_PROP_TrapeziumProfile_TopWidth                                       "TopWidth"
#define PRF_PROP_TrapeziumProfile_BottomWidth                                    "BottomWidth"
#define PRF_PROP_TrapeziumProfile_Depth                                          "Depth"
#define PRF_PROP_TrapeziumProfile_TopOffset                                      "TopOffset"
#define PRF_PROP_ZShapeProfile_FlangeWidth                                       "FlangeWidth"
#define PRF_PROP_ZShapeProfile_Depth                                             "Depth"
#define PRF_PROP_ZShapeProfile_FlangeThickness                                   "FlangeThickness"
#define PRF_PROP_ZShapeProfile_WebThickness                                      "WebThickness"
#define PRF_PROP_ZShapeProfile_FilletRadius                                      "FilletRadius"
#define PRF_PROP_ZShapeProfile_FlangeEdgeRadius                                  "FlangeEdgeRadius"
#define PRF_PROP_ZShapeProfile_FlangeSlope                                       "FlangeSlope"


//-----------------------------------------------------------------------------------------
// Category names
//-----------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------
// CodeSpec names
//-----------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------
// Define standard static QueryClass/QueryClassId methods on Elements and Aspects
//-----------------------------------------------------------------------------------------
#define DECLARE_PROFILES_QUERYCLASS_METHODS(__name__) \
static Dgn::DgnClassId QueryClassId(Dgn::DgnDbCR db) { return Dgn::DgnClassId(db.Schemas().GetClassId(PRF_SCHEMA_NAME, PRF_CLASS_##__name__)); } \
static ECN::ECClassCP QueryClass(Dgn::DgnDbCR db) { return (db.Schemas().GetClass(PRF_SCHEMA_NAME, PRF_CLASS_##__name__)); }


//-----------------------------------------------------------------------------------------
// Macro to declare Get, GetForEdit, Insert, Update methods on elements. Pointers (Ptr, CPtr) must be defined.
//-----------------------------------------------------------------------------------------
#define DECLARE_PROFILES_ELEMENT_BASE_GET_METHODS(__name__) \
PROFILES_EXPORT static __name__##CPtr Get       (Dgn::DgnDbR db, Dgn::DgnElementId id) { return db.Elements().Get< __name__ >(id); } \
PROFILES_EXPORT static __name__##Ptr  GetForEdit(Dgn::DgnDbR db, Dgn::DgnElementId id) { return db.Elements().GetForEdit< __name__ >(id); }

#define DECLARE_PROFILES_ELEMENT_BASE_GET_UPDATE_METHODS(__name__) \
DECLARE_PROFILES_ELEMENT_BASE_GET_METHODS(__name__) \
PROFILES_EXPORT        __name__##CPtr Update(Dgn::DgnDbStatus* stat=nullptr) { return GetDgnDb().Elements().Update< __name__ >(*this, stat); }

#define DECLARE_PROFILES_ELEMENT_BASE_METHODS(__name__) \
DECLARE_PROFILES_ELEMENT_BASE_GET_UPDATE_METHODS(__name__) \
PROFILES_EXPORT        __name__##CPtr Insert(Dgn::DgnDbStatus* stat=nullptr) { return GetDgnDb().Elements().Insert< __name__ >(*this, stat); }


//-----------------------------------------------------------------------------------------
// Define standard typedefs (P, CP, R, CR) in the Profiles namespace
//-----------------------------------------------------------------------------------------
#define PROFILES_TYPEDEFS(_name_) \
BEGIN_BENTLEY_PROFILES_NAMESPACE DEFINE_POINTER_SUFFIX_TYPEDEFS(_name_) END_BENTLEY_PROFILES_NAMESPACE

//-----------------------------------------------------------------------------------------
// Define RefCountedPtr and CPtr types
//-----------------------------------------------------------------------------------------
#define PROFILES_REFCOUNTED_PTR(_name_) \
BEGIN_BENTLEY_PROFILES_NAMESPACE struct _name_; DEFINE_REF_COUNTED_PTR(_name_) END_BENTLEY_PROFILES_NAMESPACE

//-----------------------------------------------------------------------------------------
// Define both RefCounterPtr/CPtr and (P, CP, R, CR) types
//-----------------------------------------------------------------------------------------
#define PROFILES_REFCOUNTED_PTR_AND_TYPEDEFS(_name_) \
BEGIN_BENTLEY_PROFILES_NAMESPACE \
    DEFINE_POINTER_SUFFIX_TYPEDEFS(_name_) \
    DEFINE_REF_COUNTED_PTR(_name_) \
END_BENTLEY_PROFILES_NAMESPACE


//-----------------------------------------------------------------------------------------
// Define typedefs and Ptrs in the Profiles namespace
//-----------------------------------------------------------------------------------------
PROFILES_REFCOUNTED_PTR_AND_TYPEDEFS(AsymmetricIShapeProfile)
PROFILES_REFCOUNTED_PTR_AND_TYPEDEFS(BentPlateProfile)
PROFILES_REFCOUNTED_PTR_AND_TYPEDEFS(CShapeProfile)
PROFILES_REFCOUNTED_PTR_AND_TYPEDEFS(CapsuleProfile)
PROFILES_REFCOUNTED_PTR_AND_TYPEDEFS(CenterLineCShapeProfile)
PROFILES_REFCOUNTED_PTR_AND_TYPEDEFS(CenterLineLShapeProfile)
PROFILES_REFCOUNTED_PTR_AND_TYPEDEFS(CenterLineZShapeProfile)
PROFILES_REFCOUNTED_PTR_AND_TYPEDEFS(CenteredProfile)
PROFILES_REFCOUNTED_PTR_AND_TYPEDEFS(CompositeProfile)
PROFILES_REFCOUNTED_PTR_AND_TYPEDEFS(CustomCenterLineProfile)
PROFILES_REFCOUNTED_PTR_AND_TYPEDEFS(CustomCompositeProfile)
PROFILES_REFCOUNTED_PTR_AND_TYPEDEFS(CustomShapeProfile)
PROFILES_REFCOUNTED_PTR_AND_TYPEDEFS(DerivedProfile)
PROFILES_REFCOUNTED_PTR_AND_TYPEDEFS(DoubleCShapeProfile)
PROFILES_REFCOUNTED_PTR_AND_TYPEDEFS(DoubleLShapeProfile)
PROFILES_REFCOUNTED_PTR_AND_TYPEDEFS(CircleProfile)
PROFILES_REFCOUNTED_PTR_AND_TYPEDEFS(HollowCircleProfile)
PROFILES_REFCOUNTED_PTR_AND_TYPEDEFS(HollowRectangleProfile)
PROFILES_TYPEDEFS(ICenterLineProfile)
PROFILES_REFCOUNTED_PTR_AND_TYPEDEFS(IShapeProfile)
PROFILES_REFCOUNTED_PTR_AND_TYPEDEFS(LShapeProfile)
PROFILES_REFCOUNTED_PTR_AND_TYPEDEFS(Profile)
PROFILES_REFCOUNTED_PTR_AND_TYPEDEFS(RectangleProfile)
PROFILES_REFCOUNTED_PTR_AND_TYPEDEFS(RegularPolygonProfile)
PROFILES_REFCOUNTED_PTR_AND_TYPEDEFS(RoundedRectangleProfile)
PROFILES_REFCOUNTED_PTR_AND_TYPEDEFS(SinglePerimeterProfile)
PROFILES_REFCOUNTED_PTR_AND_TYPEDEFS(StandardProfileAspect)
PROFILES_REFCOUNTED_PTR_AND_TYPEDEFS(TShapeProfile)
PROFILES_REFCOUNTED_PTR_AND_TYPEDEFS(TTShapeProfile)
PROFILES_REFCOUNTED_PTR_AND_TYPEDEFS(TrapeziumProfile)
PROFILES_REFCOUNTED_PTR_AND_TYPEDEFS(ZShapeProfile)


//-----------------------------------------------------------------------------------------
// Define enums in the Profiles namespace
//-----------------------------------------------------------------------------------------
BEGIN_BENTLEY_PROFILES_NAMESPACE

enum class DoubleLProfileType : int32_t
{
    LLBB = 0,
    SLBB = 1,
}; // DoubleLProfileType

enum class ParametricProfileType : int32_t
{
    C = 0,
    I = 1,
    AsymmetricI = 2,
    L = 3,
    SchifflerizedL = 4,
    T = 5,
    TT = 6,
    Z = 7,
    CenterLineC = 8,
    CenterLineL = 9,
    CenterLineZ = 10,
    BentPlate = 11,
    Rectangle = 12,
    RoundedRectangle = 13,
    Hollowrectangle = 14,
    Ellipse = 15,
    Circle = 16,
    HollowCircle = 17,
    Trapezium = 18,
    RegularPolygon = 19,
    Capsule = 20,
}; // ParametricProfileType

END_BENTLEY_PROFILES_NAMESPACE
