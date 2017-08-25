/*--------------------------------------------------------------------------------------+
|
|     $Source: Source/RulesDriven/RulesEngine/ImageHelper.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include "ImageHelper.h"

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool ImageHelper::GetImageId(Utf8StringR imageId, IECCustomAttributeContainerCR container, bool expanded)
    {
    IECInstancePtr customImageAttribute = container.GetCustomAttribute("CustomImageSpecification");
    if (customImageAttribute.IsNull())
        return false;
        
    ECValue defaultValue;
    if (ECObjectsStatus::Success != customImageAttribute->GetValue(defaultValue, "Moniker_Default"))
        {
        BeAssert(false);
        return false;
        }

    Utf8CP propertyName = expanded ? "Moniker_Expanded" : "Moniker_Collapsed";
    ECValue value;
    if (ECObjectsStatus::Success != customImageAttribute->GetValue(value, propertyName) || value.IsNull())
        value = defaultValue;

    if (value.IsNull())
        return false;

    if (!value.IsString())
        {
        BeAssert(false);
        return false;
        }

    imageId.assign(value.GetUtf8CP());
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2015
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ImageHelper::GetImageId(ECClassCR ecClass, bool isInstanceImage, bool expanded)
    {
    Utf8String imageId;
    if (GetImageId(imageId, ecClass, expanded))
        return imageId;

    Utf8CP monikerType = isInstanceImage ? "ECInstanceImage" : (nullptr != ecClass.GetRelationshipClassCP()) ? "ECRelationshipClassImage" : "ECClassImage";
    return Utf8PrintfString("%s://%s", monikerType, Utf8String(ecClass.GetFullName()).c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2015
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ImageHelper::GetImageId(ECPropertyCR ecProperty, bool expanded)
    {
    Utf8String imageId;
    if (GetImageId(imageId, ecProperty, expanded))
        return imageId;

    return Utf8PrintfString("ECPropertyImage://%s--%s", Utf8String(ecProperty.GetClass().GetFullName()).c_str(), Utf8String(ecProperty.GetName().c_str()).c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2015
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ImageHelper::GetLabelGroupingNodeImageId(bool expanded) {return "ECLiteralImage://GroupByLabel";}
