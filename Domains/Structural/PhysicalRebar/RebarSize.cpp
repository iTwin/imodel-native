/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "PhysicalRebarInternal.h"
#include <PhysicalRebar/RebarSize.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                             Jacob.Nogle                             09/2018
+---------------+---------------+---------------+---------------+---------------+------*/
RebarSizePtr RebarSize::Create(/*TODO: args*/)
    {
    return nullptr; // TODO: Not Implemented
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                             Jacob.Nogle                             09/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void RebarSize::SetName(string val)
    {
    SetPropertyValue(SPR_PROP_RebarSize_Name, val);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                             Jacob.Nogle                             09/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void RebarSize::SetDiameter(double val)
    {
    SetPropertyValue(SPR_PROP_RebarSize_Diameter, val);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                             Jacob.Nogle                             09/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void RebarSize::SetArea(double val)
    {
    SetPropertyValue(SPR_PROP_RebarSize_Area, val);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                             Jacob.Nogle                             09/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void RebarSize::SetPublisher(string val)
    {
    SetPropertyValue(SPR_PROP_RebarSize_Publisher, val);
    }

