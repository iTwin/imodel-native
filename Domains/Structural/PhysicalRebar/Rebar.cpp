/*--------------------------------------------------------------------------------------+
|
|     $Source: PhysicalRebar/Rebar.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PhysicalRebarInternal.h"
#include <PhysicalRebar/Rebar.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                             Jacob.Nogle                             09/2018
+---------------+---------------+---------------+---------------+---------------+------*/
RebarPtr Rebar::Create(/*TODO: args*/)
    {
    return nullptr; // TODO: Not Implemented
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                             Jacob.Nogle                             09/2018
+---------------+---------------+---------------+---------------+---------------+------*/
RebarTypePtr RebarType::Create(/*TODO: args*/)
    {
    return nullptr; // TODO: Not Implemented
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                             Jacob.Nogle                             09/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void RebarType::SetShape(Bentley.Geometry.Common.IGeometry val)
    {
    SetPropertyValue(SPR_PROP_RebarType_Shape, val);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                             Jacob.Nogle                             09/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void RebarType::SetSimplifiedShape(Bentley.Geometry.Common.IGeometry val)
    {
    SetPropertyValue(SPR_PROP_RebarType_SimplifiedShape, val);
    }

