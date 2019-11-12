/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include "ImageHelper.h"

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2015
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ImageHelper::GetImageId(ECClassCR ecClass, bool isInstanceImage, bool expanded)
    {
    Utf8CP monikerType = isInstanceImage ? "ECInstanceImage" : (nullptr != ecClass.GetRelationshipClassCP()) ? "ECRelationshipClassImage" : "ECClassImage";
    return Utf8PrintfString("%s://%s", monikerType, Utf8String(ecClass.GetFullName()).c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2015
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ImageHelper::GetImageId(ECPropertyCR ecProperty, bool expanded)
    {
    return Utf8PrintfString("ECPropertyImage://%s--%s", Utf8String(ecProperty.GetClass().GetFullName()).c_str(), Utf8String(ecProperty.GetName().c_str()).c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2015
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ImageHelper::GetLabelGroupingNodeImageId(bool expanded) {return "ECLiteralImage://GroupByLabel";}
