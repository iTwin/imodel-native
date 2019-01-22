/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/src/ArbitraryCompositeProfile.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ProfilesInternal.h"
#include <ProfilesInternal\ProfilesGeometry.h>
#include <ProfilesInternal\ProfilesLogging.h>
#include <ProfilesInternal\ArbitraryCompositeProfileAspect.h>
#include <Profiles\ArbitraryCompositeProfile.h>

BEGIN_BENTLEY_PROFILES_NAMESPACE

HANDLER_DEFINE_MEMBERS (ArbitraryCompositeProfileHandler)

typedef bvector<ArbitraryCompositeProfileAspectPtr> AspectVector;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
ArbitraryCompositeProfileComponent::ArbitraryCompositeProfileComponent (DgnElementId const& singleProfileId, DPoint2d const& offset,
                                                                        Angle const& rotation, bool mirrorAboutYAxis)
    : singleProfileId (singleProfileId)
    , offset (offset)
    , rotation (rotation)
    , mirrorAboutYAxis (mirrorAboutYAxis)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
ArbitraryCompositeProfileComponent::ArbitraryCompositeProfileComponent (SinglePerimeterProfile const& singleProfile, DPoint2d const& offset,
                                                                        Angle const& rotation, bool mirrorAboutYAxis)
    : singleProfileId (singleProfile.GetElementId())
    , offset (offset)
    , rotation (rotation)
    , mirrorAboutYAxis (mirrorAboutYAxis)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
ArbitraryCompositeProfile::CreateParams::CreateParams (DgnModel const& model, Utf8CP pName, ComponentVector const& components)
    : T_Super (model, QueryClassId (model.GetDgnDb()), pName)
    , components (components)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
ArbitraryCompositeProfile::ArbitraryCompositeProfile (CreateParams const& params)
    : T_Super (params)
    {
    if (params.m_isLoadingElement)
        return;

    SetComponents (params.components);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool ArbitraryCompositeProfile::_Validate() const
    {
    if (!T_Super::_Validate())
        return false;

    if (m_components.size() < 2)
        return false;

    int componentIndex = 0;
    for (ArbitraryCompositeProfileComponent const& component : m_components)
        {
        if (!ValidateComponent (component, componentIndex++))
            return false;
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool ArbitraryCompositeProfile::ValidateComponent (ArbitraryCompositeProfileComponent const& component, int componentIndex) const
    {
    bool const isSingleProfileValid = SinglePerimeterProfile::GetForEdit (m_dgndb, component.singleProfileId).IsValid();
    bool const isOffsetXValid = BeNumerical::BeFinite (component.offset.x);
    bool const isOffsetYValid = BeNumerical::BeFinite (component.offset.y);
    bool const isRotationValid = BeNumerical::BeFinite (component.rotation.Radians());
    bool const isMemberPriorityValid = component.GetMemberPriority() == componentIndex;

    return isSingleProfileValid && isOffsetXValid && isOffsetYValid && isRotationValid && isMemberPriorityValid;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
IGeometryPtr ArbitraryCompositeProfile::_CreateGeometry() const
    {
    return ProfilesGeometry::CreateArbitraryCompositeShape (*this, nullptr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
IGeometryPtr ArbitraryCompositeProfile::_UpdateGeometry (Profile const& relatedProfile) const
    {
    if (SinglePerimeterProfile const* pSingleProfile = dynamic_cast<SinglePerimeterProfile const*> (&relatedProfile))
        return ProfilesGeometry::CreateArbitraryCompositeShape (*this, pSingleProfile);

    BeAssert (false);
    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* Query all aspects that the 'profile' has.
* IMPORTANT: aspects cannot be cached and should be queried. This is due to obscure
* platfrom "features"/implementation related to 'm_appData' (aspects are appData hack).
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
static AspectVector queryAspects (ArbitraryCompositeProfile& profile)
    {
    AspectVector aspects;
    ECClass const* pAspectClass = profile.GetDgnDb().Schemas().GetClass (PRF_SCHEMA_NAME, PRF_CLASS_ArbitraryCompositeProfileAspect);

    ElementAspectIterator aspectIterator = profile.MakeAspectIterator();
    for (ElementAspectIteratorEntry const& aspectEntry : aspectIterator)
        {
        ArbitraryCompositeProfileAspectPtr aspectPtr = DgnElement::MultiAspect::GetP<ArbitraryCompositeProfileAspect>
            (profile, *pAspectClass, aspectEntry.GetECInstanceId());
        if (aspectPtr.IsNull())
            {
            BeAssert (false);
            return AspectVector();
            }

        aspects.push_back (aspectPtr);
        }

    return aspects;
    }

/*---------------------------------------------------------------------------------**//**
* Create and insert component corresponding Aspects.
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
static void insertAspects (ArbitraryCompositeProfile& profile, ArbitraryCompositeProfile::ComponentVector& components)
    {
    for (ArbitraryCompositeProfileComponent& component : components)
        {
        ArbitraryCompositeProfileAspectPtr aspectPtr = ArbitraryCompositeProfileAspect::Create (component);
        DgnElement::MultiAspect::AddAspect (profile, *aspectPtr);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ArbitraryCompositeProfile::_InsertInDb()
    {
    insertAspects (*this, m_components);

    return T_Super::_InsertInDb();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ArbitraryCompositeProfile::_OnUpdate (DgnElement const& original)
    {
    // TODO Karolis: Deleting every aspect and inserting new ones is wasteful - could update only those that actualy changed.
    bvector<ArbitraryCompositeProfileAspectPtr> aspects = queryAspects (*this);
    for (auto const& aspectPtr : aspects)
        aspectPtr->Delete();

    insertAspects (*this, m_components);

    return T_Super::_OnUpdate (original);
    }

/*---------------------------------------------------------------------------------**//**
* Element is loading from db - query aspects and populate local components vector.
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ArbitraryCompositeProfile::_LoadFromDb()
    {
    BeAssert (m_components.empty() && "Profile is loading from db - components should be empty");
    AspectVector aspects = queryAspects (*this);

    m_components.reserve (aspects.size());
    for (ArbitraryCompositeProfileAspectPtr const& aspectPtr : aspects)
        {
        ArbitraryCompositeProfileComponent component (aspectPtr->singleProfileId, aspectPtr->offset, aspectPtr->rotation, aspectPtr->mirrorAboutYAxis);
        component.m_memberPriority = aspectPtr->memberPriority;

        m_components.push_back (component);
        }

    return T_Super::_LoadFromDb();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void ArbitraryCompositeProfile::_CopyFrom (DgnElement const& source)
    {
    if (typeid (source) == typeid (ArbitraryCompositeProfile))
        m_components = static_cast<ArbitraryCompositeProfile const&> (source).m_components;
    else
        ProfilesLog::FailedCopyFrom_InvalidElement (PRF_CLASS_ArbitraryCompositeProfile, m_elementId, source.GetElementId());

    return T_Super::_CopyFrom (source);
    }

/*---------------------------------------------------------------------------------**//**
* Return a copy of components vector.
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
ArbitraryCompositeProfile::ComponentVector ArbitraryCompositeProfile::GetComponents() const
    {
    return m_components;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void ArbitraryCompositeProfile::SetComponents (ComponentVector const& components)
    {
    m_components = components;

    int memberPriority = 0;
    for (ArbitraryCompositeProfileComponent& component : m_components)
        component.m_memberPriority = memberPriority++;
    }

END_BENTLEY_PROFILES_NAMESPACE
