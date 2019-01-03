/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/src/SchifflerizedLShapeProfile.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ProfilesInternal.h"
#include <Profiles/SchifflerizedLShapeProfile.h>

BEGIN_BENTLEY_PROFILES_NAMESPACE

HANDLER_DEFINE_MEMBERS (SchifflerizedLShapeProfileHandler)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
SchifflerizedLShapeProfile::CreateParams::CreateParams (Dgn::DgnModel const& model, Utf8CP pName)
    : T_Super (model, QueryClassId (model.GetDgnDb()), pName)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
SchifflerizedLShapeProfile::CreateParams::CreateParams (Dgn::DgnModel const& model, Utf8CP pName, double legLength, double thickness,
                                                        double legBendOffset, double filletRadius, double edgeRadius)
    : T_Super (model, QueryClassId (model.GetDgnDb()), pName)
    , legLength (legLength)
    , thickness (thickness)
    , legBendOffset (legBendOffset)
    , filletRadius (filletRadius)
    , edgeRadius (edgeRadius)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
SchifflerizedLShapeProfile::SchifflerizedLShapeProfile (CreateParams const& params)
    : T_Super (params)
    {
    if (params.m_isLoadingElement)
        return;

    SetLegLength (params.legLength);
    SetThickness (params.thickness);
    SetLegBendOffset (params.legBendOffset);
    SetFilletRadius (params.filletRadius);
    SetEdgeRadius (params.edgeRadius);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double SchifflerizedLShapeProfile::GetLegLength() const
    {
    return GetPropertyValueDouble (PRF_PROP_SchifflerizedLShapeProfile_LegLength);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void SchifflerizedLShapeProfile::SetLegLength (double value)
    {
    SetPropertyValue (PRF_PROP_SchifflerizedLShapeProfile_LegLength, ECN::ECValue (value));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double SchifflerizedLShapeProfile::GetThickness() const
    {
    return GetPropertyValueDouble (PRF_PROP_SchifflerizedLShapeProfile_Thickness);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void SchifflerizedLShapeProfile::SetThickness (double value)
    {
    SetPropertyValue (PRF_PROP_SchifflerizedLShapeProfile_Thickness, ECN::ECValue (value));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
double SchifflerizedLShapeProfile::GetLegBendOffset() const
    {
    return GetPropertyValueDouble (PRF_PROP_SchifflerizedLShapeProfile_LegBendOffset);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void SchifflerizedLShapeProfile::SetLegBendOffset (double value)
    {
    SetPropertyValue (PRF_PROP_SchifflerizedLShapeProfile_LegBendOffset, ECN::ECValue (value));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double SchifflerizedLShapeProfile::GetFilletRadius() const
    {
    return GetPropertyValueDouble (PRF_PROP_SchifflerizedLShapeProfile_FilletRadius);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void SchifflerizedLShapeProfile::SetFilletRadius (double value)
    {
    SetPropertyValue (PRF_PROP_SchifflerizedLShapeProfile_FilletRadius, ECN::ECValue (value));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
double SchifflerizedLShapeProfile::GetEdgeRadius() const
    {
    return GetPropertyValueDouble (PRF_PROP_SchifflerizedLShapeProfile_EdgeRadius);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void SchifflerizedLShapeProfile::SetEdgeRadius (double value)
    {
    SetPropertyValue (PRF_PROP_SchifflerizedLShapeProfile_EdgeRadius, ECN::ECValue (value));
    }

END_BENTLEY_PROFILES_NAMESPACE
