#pragma once
//__PUBLISH_SECTION_START__
#include <DgnPlatform/DgnCoreAPI.h>
#include "ConstantProfile.h"
#include "ProfilesDomainDefinitions.h"
#include "Profile.h"
#include "ProfilesModel.h"

BEGIN_BENTLEY_PROFILES_NAMESPACE

struct EXPORT_VTABLE_ATTRIBUTE BuiltUpProfileComponent : Dgn::DgnElement::MultiAspect
    {
    DGNASPECT_DECLARE_MEMBERS(BENTLEY_PROFILES_SCHEMA_NAME, PROFILES_CLASS_BuiltUpProfileComponent, Dgn::DgnElement::MultiAspect);
    friend struct  BuiltUpProfileComponentHandler;

private:
    BuiltUpProfileComponent() {};
    int _index;
    int _placementPointId;
    bool _mirrorProfileAboutYAxis;
    DPoint2d _offsetPoint;
    double _rotation;
    Dgn::DgnElementId _profileId;

public:
    PROFILES_DOMAIN_EXPORT static BuiltUpProfileComponentPtr Create() {return new BuiltUpProfileComponent();}
    PROFILES_DOMAIN_EXPORT void SetIndex(int index) { _index = index; }
    PROFILES_DOMAIN_EXPORT int GetIndex() const{ return _index; }
    PROFILES_DOMAIN_EXPORT void SetPlacementPointId(int placementPointId) { _placementPointId = placementPointId; }
    PROFILES_DOMAIN_EXPORT int GetPlacementPointId() const{ return _placementPointId; }
    PROFILES_DOMAIN_EXPORT void SetMirrorProfileAboutYAxis(bool mirrorProfileAboutYAxis) 
        { _mirrorProfileAboutYAxis = mirrorProfileAboutYAxis; }
    PROFILES_DOMAIN_EXPORT bool GetMirrorProfileAboutYAxis() const{ return _mirrorProfileAboutYAxis; }
    PROFILES_DOMAIN_EXPORT void SetOffsetPoint(DPoint2d offsetPoint) { _offsetPoint = offsetPoint; }
    PROFILES_DOMAIN_EXPORT DPoint2d GetOffsetPoint() const{ return _offsetPoint; }
    PROFILES_DOMAIN_EXPORT void SetRotation(double rotation) { _rotation = rotation; }
    PROFILES_DOMAIN_EXPORT double GetRotation() const{ return _rotation; }
    PROFILES_DOMAIN_EXPORT void SetProfileId(Dgn::DgnElementId profileId) { _profileId = profileId; }
    PROFILES_DOMAIN_EXPORT Dgn::DgnElementId GetProfileId() const{ return _profileId; }

protected:
    PROFILES_DOMAIN_EXPORT Dgn::DgnDbStatus _LoadProperties(Dgn::DgnElementCR) override;
    PROFILES_DOMAIN_EXPORT Dgn::DgnDbStatus _UpdateProperties(Dgn::DgnElementCR, BeSQLite::EC::ECCrudWriteToken const*) override;
    Dgn::DgnDbStatus _GetPropertyValue(ECN::ECValueR, Utf8CP, Dgn::PropertyArrayIndex const&) const override { return Dgn::DgnDbStatus::NotEnabled; }
    Dgn::DgnDbStatus _SetPropertyValue(Utf8CP, ECN::ECValueCR, Dgn::PropertyArrayIndex const&) override { return Dgn::DgnDbStatus::NotEnabled; }
    };

struct EXPORT_VTABLE_ATTRIBUTE BuiltUpProfileComponentHandler : Dgn::dgn_AspectHandler::Aspect
    {
    DOMAINHANDLER_DECLARE_MEMBERS(PROFILES_CLASS_BuiltUpProfileComponent, BuiltUpProfileComponentHandler, Dgn::dgn_AspectHandler::Aspect, PROFILES_DOMAIN_EXPORT)
    RefCountedPtr<Dgn::DgnElement::Aspect> _CreateInstance() override { return new BuiltUpProfileComponent(); }
    };

END_BENTLEY_PROFILES_NAMESPACE