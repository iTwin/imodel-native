---
Setup Profiles Domain
---

### Register Profiles Domain for usage
In order to use call Profiles API you need to register its domain.
Typically this is done durring the initialization of DgnPlatformLib::Host.
@code{.cpp}
#include <Profiles\ProfilesApi.h>
USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_PROFILES

MyHostClass::MyHostClass()
    {
    DgnPlatformLib::Initialize (*this, false);

    DgnDomains::RegisterDomain (ProfilesDomain::GetDomain(), DgnDomain::Required::Yes, DgnDomain::Readonly::No);
    }
@endcode

### Create a DefinitionModel
Definition model is needed to insert Profile elements since all Profile elements are Definition elemenets.

@code{.cpp}
Dgn::DgnDb& db = ...; // Reference to Dgn::DgnDb

Dgn::SubjectCPtr rootSubjectPtr = db.Elements().GetRootSubject();

DefinitionPartitionPtr partitionPtr = DefinitionPartition::Create (*rootSubjectPtr, "Definition Partition Name");
db.BriefcaseManager().AcquireForElementInsert (*partitionPtr);

Dgn::DgnDbStatus status;
partitionPtr->Insert (&status);

DefinitionModelPtr modelPtr = DefinitionModelPtr::Create (*partitionPtr);
modelPtr->Insert();
@endcode

---
ProfilesAPI Examples
---

### Create IShapeProfile
Parametric I shape profile

@code{.cpp}
IShapeProfile::CreateParams params (model, "C Shape Profile", 6.0, 10.0, 1.0, 1.0, 0.5, 0.5, Angle::FromDegrees (5.0));
IShapeProfilePtr profilePtr = IShapeProfile::Create (params);

profilePtr->Insert();
@endcode

### Create ArbitraryShapeProfile
Arbitrary profile of a plus symbol shape

@code{.cpp}
bvector<DPoint3d> points =
    {
    DPoint3d::From (-4.0, -0.5), DPoint3d::From (-4.0, 0.5), DPoint3d::From (-0.5, 0.5), DPoint3d::From (-0.5, 4.0),
    DPoint3d::From (0.5, 4.0), DPoint3d::From (0.5, 0.5), DPoint3d::From (4.0, 0.5), DPoint3d::From (4.0, -0.5),
    DPoint3d::From (0.5, -0.5), DPoint3d::From (0.5, -4.0), DPoint3d::From (-0.5, -4.0), DPoint3d::From (-0.5, -0.5), DPoint3d::From (-4.0, -0.5)
    };
CurveVectorPtr curveVectorPtr = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Outer, ICurvePrimitive::CreateLineString (points));
IGeometryPtr geometryPtr = IGeometry::Create (curveVectorPtr);

ArbitraryShapeProfile::CreateParams params (model, "+ shape profile", geometryPtr);
ArbitraryShapeProfilePtr profilePtr = ArbitraryShapeProfile::Create (params);

profilePtr->Insert();
@endcode

### Create ArbitraryCenterLineProfile
CenterLine shape of a half pipe

@code{.cpp}
DEllipse3d halfArc = DEllipse3d::FromPointsOnArc (DPoint3d::From (-2.0, 2.0), DPoint3d::From (0.0, 0.0), DPoint3d::From (2.0, 2.0));
ICurvePrimitivePtr curvePtr = ICurvePrimitive::CreateArc (halfArc);

ArbitraryCenterLineProfile::CreateParams params (model, "Arbitrary CenterLine - Half pipe", IGeometry::Create (curvePtr), 0.5);
ArbitraryCenterLineProfilePtr profilePtr = ArbitraryCenterLineProfile::Create (params);

profilePtr->Insert();
@endcode

### Create ArbitraryCompositeProfile
Arbitrary composite profile - I shape with plates on flanges

@code{.cpp}
IShapeProfile::CreateParams iShapeParams (model, "I shape", 6.0, 10.0, 0.5, 0.75, 0.5, 0.25, Angle::FromRadians (PI / 32));
IShapeProfilePtr iShapePtr = IShapeProfile::Create (iShapeParams);
iShapePtr->Insert();

RectangleProfile::CreateParams plateParams (model, "plate", 5.0, 0.75);
RectangleProfilePtr platePtr = RectangleProfile::Create (plateParams);
platePtr->Insert();

ArbitraryCompositeProfile::ComponentVector components =
    {
    ArbitraryCompositeProfileComponent (*iShapePtr, DPoint2d::From (0.0, 0.0)),
    ArbitraryCompositeProfileComponent (*platePtr, DPoint2d::From (0.0, 5.0 + 0.75 / 2.0)),
    ArbitraryCompositeProfileComponent (*platePtr, DPoint2d::From (0.0, -5.0 - 0.75 / 2.0))
    };

ArbitraryCompositeProfile::CreateParams compositeParams (model, "I shape beam with plates", components);
ArbitraryCompositeProfilePtr compositePtr = ArbitraryCompositeProfile::Create (compositeParams);
compositePtr->Insert();
@endcode

### Create DerivedProfile
Create a profile that inherits geometry from another profile and applies transformation to it

@code{.cpp}
IShapeProfile::CreateParams iShapeParams (model, "C Shape Profile", 6.0, 10.0, 1.0, 1.0);
IShapeProfilePtr profilePtr = IShapeProfile::Create (iShapeParams);
profilePtr->Insert();

DerivedProfile::CreateParams derivedParams (model, "Derived - scaled and mirrored profile", *profilePtr, DPoint2d::From (0.0, 0.0),
                                            DPoint2d::From (0.5, 0.5), Angle::FromRadians (0.0), true));
DerivedProfilePtr derivedPtr = DerivedProfile::Create (derivedParams);
derivedPtr->Insert();
@endcode

### Set StandardCatalogCode
Set StandardCatalogCode for a profile

@code{.cpp}
ProfilePtr profilePtr = GetSomeProfile();

profilePtr->SetName ("W12x14");
StandardCatalogCode catalogCode ("US", "AISC", "14");

profilePtr->SetStandardCatalogCode (catalogCode);
@endcode

### Create custom CardinalPoint
Create a custom (user defined) cardinal point for a profile

@code{.cpp}
ProfilePtr profilePtr = GetSomeProfile();

CardinalPoint customCardinalPoint ("MyCustomPoint", DPoint2d::From (1.2, 3.4));

profilePtr->AddCustomCardinalPoint (customCardinalPoint);
@endcode
