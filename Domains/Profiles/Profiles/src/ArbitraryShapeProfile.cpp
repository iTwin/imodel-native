/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/src/ArbitraryShapeProfile.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ProfilesPch.h"
#include <ProfilesInternal\ProfilesLogging.h>
#include <Profiles\ArbitraryShapeProfile.h>

BEGIN_BENTLEY_PROFILES_NAMESPACE

HANDLER_DEFINE_MEMBERS (ArbitraryShapeProfileHandler)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
ArbitraryShapeProfile::CreateParams::CreateParams (DgnModel const& model, Utf8CP pName, IGeometryPtr const& geometryPtr)
    : T_Super (model, QueryClassId (model.GetDgnDb()), pName)
    , geometryPtr (geometryPtr)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
ArbitraryShapeProfile::ArbitraryShapeProfile (CreateParams const& params)
    : T_Super (params)
    , m_geometryPtr (nullptr)
    {
    if (params.m_isLoadingElement)
        return;

    m_geometryPtr = params.geometryPtr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
IGeometryPtr ArbitraryShapeProfile::_CreateShapeGeometry() const
    {
    return m_geometryPtr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void ArbitraryShapeProfile::_CopyFrom (DgnElement const& source)
    {
    if (auto const* pSourceProfile = dynamic_cast<ArbitraryShapeProfile const*> (&source))
        m_geometryPtr = pSourceProfile->m_geometryPtr;
    else
        ProfilesLog::FailedCopyFrom_InvalidElement (PRF_CLASS_ArbitraryShapeProfile, m_elementId, source.GetElementId());

    return T_Super::_CopyFrom (source);
    }

END_BENTLEY_PROFILES_NAMESPACE
