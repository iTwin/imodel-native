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
    EXPORT_VTABLE_ATTRIBUTE static BuiltUpProfileComponentPtr Create() {return new BuiltUpProfileComponent();}
    EXPORT_VTABLE_ATTRIBUTE void SetIndex(int index) { _index = index; }
    EXPORT_VTABLE_ATTRIBUTE int GetIndex() const{ return _index; }
    EXPORT_VTABLE_ATTRIBUTE void SetPlacementPointId(int placementPointId) { _placementPointId = placementPointId; }
    EXPORT_VTABLE_ATTRIBUTE int GetPlacementPointId() const{ return _placementPointId; }
    EXPORT_VTABLE_ATTRIBUTE void SetMirrorProfileAboutYAxis(bool mirrorProfileAboutYAxis) 
        { _mirrorProfileAboutYAxis = mirrorProfileAboutYAxis; }
    EXPORT_VTABLE_ATTRIBUTE bool GetMirrorProfileAboutYAxis() const{ return _mirrorProfileAboutYAxis; }
    EXPORT_VTABLE_ATTRIBUTE void SetOffsetPoint(DPoint2d offsetPoint) { _offsetPoint = offsetPoint; }
    EXPORT_VTABLE_ATTRIBUTE DPoint2d GetOffsetPoint() const{ return _offsetPoint; }
    EXPORT_VTABLE_ATTRIBUTE void SetRotation(double rotation) { _rotation = rotation; }
    EXPORT_VTABLE_ATTRIBUTE double GetRotation() const{ return _rotation; }
    EXPORT_VTABLE_ATTRIBUTE void SetProfileId(Dgn::DgnElementId profileId) { _profileId = profileId; }
    EXPORT_VTABLE_ATTRIBUTE Dgn::DgnElementId GetProfileId() const{ return _profileId; }
protected:
    EXPORT_VTABLE_ATTRIBUTE Dgn::DgnDbStatus _LoadProperties(Dgn::DgnElementCR) override;
    EXPORT_VTABLE_ATTRIBUTE Dgn::DgnDbStatus _UpdateProperties(Dgn::DgnElementCR, BeSQLite::EC::ECCrudWriteToken const*) override;
    Dgn::DgnDbStatus _GetPropertyValue(ECN::ECValueR, Utf8CP, Dgn::PropertyArrayIndex const&) const override { return Dgn::DgnDbStatus::NotEnabled; }
    Dgn::DgnDbStatus _SetPropertyValue(Utf8CP, ECN::ECValueCR, Dgn::PropertyArrayIndex const&) override { return Dgn::DgnDbStatus::NotEnabled; }
    };

struct EXPORT_VTABLE_ATTRIBUTE BuiltUpProfileComponentHandler : Dgn::dgn_AspectHandler::Aspect
    {
    DOMAINHANDLER_DECLARE_MEMBERS(PROFILES_CLASS_BuiltUpProfileComponent, BuiltUpProfileComponentHandler, Dgn::dgn_AspectHandler::Aspect, PROFILES_DOMAIN_EXPORT)
    RefCountedPtr<Dgn::DgnElement::Aspect> _CreateInstance() override { return new BuiltUpProfileComponent(); }
    };

END_BENTLEY_PROFILES_NAMESPACE